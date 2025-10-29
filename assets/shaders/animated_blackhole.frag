#version 330 core
out vec4 FragColor;
in vec2 vNDC;

uniform mat4 uInvVP;
uniform vec2 uResolution;
uniform float uTime;
uniform vec3 uCameraPos;

// ==================== Physical params ====================
const float RS         = 0.8;
const float HZN_ISO    = RS * 0.25;
const float R_DISK_IN  = 0.9 * RS;
const float R_DISK_OUT = 12.0 * RS;

// Integrator controls
const int   N_STEPS    = 1200;
const float LAMBDA_MAX = 120.0;
const float H_BASE     = 0.04;

// Photon-sphere
const float R_PH_ISO   = 0.9330127019f * RS;

// Disk animation controls
const float SPIN_SCALE   = 1.0;
const float FLICKER_AMT  = 0.25;
const int   HOTSPOTS     = 3;
const float HOTSIGMA_A   = 0.20;
const float HOTSIGMA_R   = 0.7*RS;
const float HOT_R0       = 2.2*RS;

// ==================== Utility / background ====================
float hash31(vec3 p) {
    p = fract(p * 0.3183099 + 0.1);
    p += dot(p, p.yzx + 19.19);
    return fract(p.x*p.y*p.z);
}

vec3 starBackground(vec3 rd) {
    vec3 p = normalize(rd);
    float d = 200.0;
    float s = 0.0;
    for (int i=0; i<3; ++i) {
        vec3 cell = floor(p*d + float(i)*37.0);
        float h = hash31(cell);
        s += smoothstep(0.995, 1.0, h) * (1.0 + 3.0*float(i));
        d *= 1.7;
    }
    vec3 base = vec3(0.04, 0.05, 0.08);
    return base + s * vec3(0.9, 0.9, 1.0);
}

// Reconstruct world ray from NDC
vec3 rayDirection(vec2 ndc)
{
    vec4 pNear = vec4(ndc, -1.0, 1.0);
    vec4 pFar  = vec4(ndc,  1.0, 1.0);
    vec4 wNear = uInvVP * pNear;  wNear /= wNear.w;
    vec4 wFar  = uInvVP * pFar;   wFar  /= wFar.w;
    return normalize(vec3(wFar - wNear));
}

// ==================== Metric ====================
struct ABVals { float A, B, dA, dB; };

ABVals metricAB(float rho) {
    float r = max(rho, 1e-6);
    float s = RS / (4.0 * r);
    float A = (1.0 - s) / (1.0 + s);
    float B = (1.0 + s) * (1.0 + s);
    float dA = (2.0 * s) / ((1.0 + s)*(1.0 + s) * r);
    float dB = -2.0 * s * (1.0 + s) / r;
    ABVals v; v.A=A; v.B=B; v.dA=dA; v.dB=dB; return v;
}

struct RHS { vec3 dx; vec3 dp; };

RHS rhs(vec3 x, vec3 p) {
    float rho = length(x);
    ABVals m = metricAB(rho);
    float invB2 = 1.0 / (m.B * m.B);
    vec3 dx = p * invB2;

    float p2 = dot(p, p);
    float termA = m.dA / (m.A*m.A*m.A);
    float termB = m.dB / (m.B*m.B*m.B);
    vec3  n = (rho > 0.0) ? (x / rho) : vec3(0.0);
    vec3 dp = -n * ( termA - termB * p2 );
    RHS r; r.dx=dx; r.dp=dp; return r;
}

vec3 renorm_p(vec3 p, float rho) {
    ABVals m = metricAB(rho);
    float target = m.B / m.A;
    float cur = max(length(p), 1e-6);
    return p * (target / cur);
}

void rk4(inout vec3 x, inout vec3 p, float h) {
    RHS k1 = rhs(x, p);
    vec3 x2 = x + 0.5*h*k1.dx;
    vec3 p2 = p + 0.5*h*k1.dp; p2 = renorm_p(p2, length(x2));
    RHS k2 = rhs(x2, p2);

    vec3 x3 = x + 0.5*h*k2.dx;
    vec3 p3 = p + 0.5*h*k2.dp; p3 = renorm_p(p3, length(x3));
    RHS k3 = rhs(x3, p3);

    vec3 x4 = x + h*k3.dx;
    vec3 p4 = p + h*k3.dp;     p4 = renorm_p(p4, length(x4));
    RHS k4 = rhs(x4, p4);

    x += (h/6.0) * (k1.dx + 2.0*k2.dx + 2.0*k3.dx + k4.dx);
    p += (h/6.0) * (k1.dp + 2.0*k2.dp + 2.0*k3.dp + k4.dp);
    p  = renorm_p(p, length(x));
}

// ==================== Relativistic helpers ====================
float v_orbit(float r_iso) {
    float r_phys = r_iso * metricAB(max(r_iso,1e-6)).B;
    return clamp(sqrt(max(0.0, RS / (2.0 * max(r_phys, R_DISK_IN)))), 0.0, 0.75);
}
float grav_redshift(float r_iso) {
    float A = metricAB(max(r_iso,1e-6)).A;
    return max(A, 0.0);
}
float doppler(vec3 v, vec3 n) {
    float v2 = dot(v,v);
    float gamma = 1.0 / sqrt(max(1e-6, 1.0 - v2));
    return 1.0 / (gamma * max(0.05, 1.0 - dot(v, n)));
}

// ==================== Ray tracer ====================
struct Sample { bool absorbed; vec3 col; };

Sample traceGeodesic(vec3 ro_world, vec3 rd_world)
{
    vec3 x = ro_world;
    float rho0 = length(x);
    ABVals m0 = metricAB(max(rho0,1e-6));
    vec3 p = normalize(rd_world) * (m0.B / m0.A);

    float lambda = 0.0;
    vec3 accum = vec3(0.0);

    for (int i=0; i<N_STEPS; ++i) {
        float rho = length(x);
        if (rho < R_PH_ISO && dot(x,p)<0.0) { Sample s; s.absorbed=true; s.col=vec3(0.0); return s; }
        if (rho <= HZN_ISO) { Sample s; s.absorbed=true; s.col=vec3(0.0); return s; }

        float h = H_BASE * mix(0.15,1.0,smoothstep(RS*0.6,6.0*RS,rho));
        float r_iso  = max(rho, 1e-6);
        float r_phys = r_iso * metricAB(r_iso).B;

        // --- Disk emission (animated) ---
        if (abs(x.y) < 0.12 && r_phys > R_DISK_IN && r_phys < R_DISK_OUT) {
            float ang   = atan(x.z, x.x);
            float vK    = v_orbit(r_iso);
            float omega = vK / max(r_phys, 1e-4);
            float phase = ang + omega * uTime * SPIN_SCALE;

            vec3 vphi = normalize(vec3(-sin(phase),0.0,cos(phase))) * vK;

            float t     = clamp(R_DISK_IN / r_phys,0.0,1.0);
            float emi   = 4.0*(t*t);

            float sector = floor(mod(phase,6.28318)*18.0);
            float ring   = floor(clamp((r_phys-R_DISK_IN)/(R_DISK_OUT-R_DISK_IN),0.0,0.999)*20.0);
            float twRnd  = hash31(vec3(sector,ring,7.0));
            float tw     = 0.9 + 0.2 * twRnd;

            float hs = 0.0;
            for(int j=0;j<HOTSPOTS;++j){
                float phi_j = 6.28318*float(j)/float(max(HOTSPOTS,1));
                float dphi  = acos(clamp(cos(phase-phi_j),-1.0,1.0));
                float ga = exp(- (dphi*dphi)/(2.0*HOTSIGMA_A*HOTSIGMA_A));
                float gr = exp(- ((r_phys-HOT_R0)*(r_phys-HOT_R0)) / (2.0*HOTSIGMA_R*HOTSIGMA_R));
                hs += ga*gr;
            }
            hs = clamp(hs,0.0,2.0);
            float flick = 1.0 + FLICKER_AMT*sin(uTime*(1.7+0.3*twRnd)+4.0*twRnd);
            vec3 disk = vec3(2.0,1.0,0.6) * emi * (tw*flick) * (1.0 + 0.6*hs);

            float g = grav_redshift(r_iso);
            float D = pow(doppler(vphi, normalize(p)),1.3);
            accum += disk * g * D * 0.05;
        }

        // --- Coronal gas (soft halo) ---
        {
            const float Hscale = 0.45*RS;
            const float rMin=0.9*RS, rMax=25.0*RS;
            const float alpha=1.2;
            float inRange = step(rMin,r_phys)*(1.0-step(rMax,r_phys));
            float vz=exp(-abs(x.y)/Hscale);
            float vr=pow(max(r_phys/RS,1.0),-alpha);
            float tw=0.85+0.3*hash31(floor(x*3.5));
            float g=grav_redshift(r_iso);
            vec3 coronaColor=vec3(1.2,0.85,0.55);
            accum += coronaColor*(vz*vr*inRange*tw)*g*(0.03*h);
        }

        rk4(x,p,h);
        lambda+=h;
        if(lambda>LAMBDA_MAX)break;
        if(length(x-ro_world)>200.0)break;
    }

    vec3 lensedSky = starBackground(normalize(p));
    Sample s; s.absorbed=false; s.col=accum+lensedSky; return s;
}

// ==================== Main ====================
void main(){
    vec3 ro=uCameraPos;
    vec3 rd=rayDirection(vNDC);
    Sample s=traceGeodesic(ro,rd);
    if(s.absorbed){FragColor=vec4(0.0);return;}
    FragColor=vec4(s.col,1.0);
}
