#version 330 core
out vec4 FragColor;
in vec2 vNDC;

uniform mat4 uInvVP;
uniform vec2 uResolution;
uniform float uTime;
uniform vec3 uCameraPos;

// ==================== Physical params (scene units, c=1) ====================
const float RS         = 0.8;             // Schwarzschild radius
const float HZN_ISO    = RS * 0.25;       // event horizon in isotropic radius: ρ_h = RS/4
const float R_DISK_IN  = 100 * RS;
const float R_DISK_OUT = 1 * RS;

// Integrator controls
const int   N_STEPS    = 1;     // RK4 steps
const float LAMBDA_MAX = 60.0;    // affine "time" cap
const float H_BASE     = 0.06;    // base step (smaller near the hole)

// --- add after the other consts ---
const float B_CRIT    = 2.598076211f * RS;     // 3*sqrt(3)/2 * RS
const float R_PH_ISO  = 0.9330127019f * RS;    // photon-sphere radius in isotropic coords

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
    vec3 base = vec3(0.03, 0.04, 0.06);
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

// ==================== Schwarzschild (isotropic Cartesian) ====================
// Metric functions in isotropic radius ρ = |x|
/*
   ds^2 = -A(ρ)^2 dt^2 + B(ρ)^2 (dx^2 + dy^2 + dz^2)
   with  A = (1 - s)/(1 + s),  B = (1 + s)^2,  s = RS/(4ρ)
   Inverse metric: g^{00} = -1/A^2,  g^{ij} = δ^{ij}/B^2
   Hamiltonian (null): H = 1/2 (g^{00} p0^2 + g^{ij} p_i p_j) = 0, with p0 = -E (const).
   We set E=1 → p0^2 = 1. Equations:
     x' = ∂H/∂p = g^{ij} p_j = p / B^2
     p' = -∂H/∂x = -½ ∂_k g^{00} p0^2 - ½ ∂_k g^{ij} p_i p_j
        = - (x/ρ) [ A'(ρ) A(ρ)^{-3} * 1  -  B'(ρ) B(ρ)^{-3} * |p|^2 ]
*/
struct ABVals { float A, B, dA, dB; };

ABVals metricAB(float rho) {
    float r = max(rho, 1e-6);
    float s = RS / (4.0 * r);
    float A = (1.0 - s) / (1.0 + s);
    float B = (1.0 + s) * (1.0 + s);
    // derivatives w.r.t. ρ
    // s' = -s/ρ
    float dA = (2.0 * s) / ((1.0 + s)*(1.0 + s) * r);   // dA/dρ
    float dB = -2.0 * s * (1.0 + s) / r;                 // dB/dρ
    ABVals v; v.A=A; v.B=B; v.dA=dA; v.dB=dB; return v;
}

struct RHS { vec3 dx; vec3 dp; };

// Right-hand side of geodesic ODEs (affine parameter λ)
RHS rhs(vec3 x, vec3 p) {
    float rho = length(x);

    ABVals m = metricAB(rho);
    float invB2 = 1.0 / (m.B * m.B);
    vec3 dx = p * invB2;

    float p2 = dot(p, p);
    float termA = m.dA / (m.A*m.A*m.A);     // A'(ρ) A^{-3}
    float termB = m.dB / (m.B*m.B*m.B);     // B'(ρ) B^{-3}
    vec3  n = (rho > 0.0) ? (x / rho) : vec3(0.0);
    vec3 dp = -n * ( termA * 1.0 - termB * p2 ); // p0^2 = 1

    RHS r; r.dx=dx; r.dp=dp; return r;
}

// Enforce H=0 by rescaling |p| to |p| = B/A while keeping its direction
vec3 renorm_p(vec3 p, float rho) {
    ABVals m = metricAB(rho);
    float target = m.B / m.A;  // from H=0: |p| = B/A (with E=1)
    float cur = max(length(p), 1e-6);
    return p * (target / cur);
}

// One RK4 step
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
    p  = renorm_p(p, length(x)); // final projection
}

// ==================== Disk emission & relativistic effects ====================
float v_orbit(float r_iso) {
    // Approx physical r ≈ ρ * B(ρ)
    float r_phys = r_iso * metricAB(max(r_iso,1e-6)).B;
    return clamp(sqrt(max(0.0, RS / (2.0 * max(r_phys, R_DISK_IN)))), 0.0, 0.75);
}
float grav_redshift(float r_iso) {
    float A = metricAB(max(r_iso,1e-6)).A; // photon frequency ∝ A at infinity
    return max(A, 0.0);
}
float doppler(vec3 v, vec3 n) {
    float v2 = dot(v,v);
    float gamma = 1.0 / sqrt(max(1e-6, 1.0 - v2));
    return 1.0 / (gamma * max(0.05, 1.0 - dot(v, n)));
}

// ==================== Main tracer (physically-accurate bending) ====================
struct Sample { bool absorbed; vec3 col; };

Sample traceGeodesic(vec3 ro_world, vec3 rd_world)
{
    // State variables in isotropic Cartesian coordinates (we use world coords as isotropic)
    vec3 x = ro_world;
    // Set initial p parallel to rd and satisfy H=0 → |p|=B/A at start
    float rho0 = length(x);
    ABVals m0 = metricAB(max(rho0,1e-6));
    vec3 p = normalize(rd_world) * (m0.B / m0.A);

    float lambda = 0.0;
    vec3 accum = vec3(0.0);

    for (int i=0; i<N_STEPS; ++i) {
        float rho = length(x);
        if (rho < R_PH_ISO && dot(x, p) < 0.0) {
            Sample s; s.absorbed = true; s.col = vec3(0.0); 
            return s; // inside photon sphere moving inward -> captured
        }

        // absorb at horizon (ρ <= RS/4)
        if (rho <= HZN_ISO) {
            Sample s; s.absorbed = true; s.col = vec3(0.0); return s;
        }

        // thin accretion disk in y≈0 plane, emissive + GR + Doppler beaming
        if (abs(x.y) < 0.03) {
            float r_iso = length(x);
            float r_phys = r_iso * metricAB(max(r_iso,1e-6)).B;
            if (r_phys > R_DISK_IN && r_phys < R_DISK_OUT) {
                float ang = atan(x.z, x.x);
                vec3 vphi = normalize(vec3(-sin(ang), 0.0, cos(ang))) * v_orbit(r_iso);
                float emi  = 4.0 * pow(clamp(R_DISK_IN / r_phys, 0.0, 1.0), 2.0);
                float tw   = hash31(floor(x * 4.0)) * 0.2 + 0.9;
                vec3 disk  = vec3(1.6, 0.95, 0.55) * emi * tw;
                float g    = grav_redshift(r_iso);         // gravitational redshift
                float D    = doppler(vphi, normalize(p));  // Doppler/beaming toward ray dir
                accum += disk * g * D * 0.02;
            }
        }

        // adaptive affine step: smaller near the hole
        float h = H_BASE * mix(0.15, 1.0, smoothstep(RS*0.6, 6.0*RS, rho));
        rk4(x, p, h);

        lambda += h;
        if (lambda > LAMBDA_MAX) break;
        if (length(x - ro_world) > 200.0) break; // escape far enough
    }

    // Lensed skybox: sample background in the FINAL ray direction (normalize(p))
    vec3 lensedSky = starBackground(normalize(p));

    Sample s; 
    s.absorbed = false; 
    s.col = accum + lensedSky;    // disk emission + GR-lensed background
    return s;
}

// ==================== Main ====================
void main()
{
    vec3 ro = uCameraPos;
    vec3 rd = rayDirection(vNDC);

    float b = length(cross(rd, normalize(-ro))) * length(ro);
    if (b < B_CRIT) { FragColor = vec4(0.0); return; }


    Sample s = traceGeodesic(ro, rd);

    if (s.absorbed) { FragColor = vec4(0.0); return; }

    // Output the physically bent background + disk emission
    FragColor = vec4(s.col, 1.0);
}
