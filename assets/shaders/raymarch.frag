#version 330 core
out vec4 FragColor;
in vec2 vNDC;

uniform mat4 uInvVP;
uniform vec2 uResolution;
uniform float uTime;
uniform vec3 uCameraPos;

// =========================================================
// Signed-distance scene
// =========================================================
float sdSphere(vec3 p, float r) { return length(p) - r; }
float sdPlaneY(vec3 p, float h) { return p.y - h; }

float mapSDF(vec3 p, out int matID)
{
    vec3 c = vec3(0.0, 0.6 + 0.2 * sin(uTime), 0.0);
    float d0 = sdSphere(p - c, 0.75);
    float d1 = sdPlaneY(p, -1.0);
    if (d0 < d1) { matID = 1; return d0; }
    matID = 2; return d1;
}

// =========================================================
// Helpers
// =========================================================
vec3 estimateNormal(vec3 p)
{
    const float e = 1e-3;
    int m;
    vec2 k = vec2(1.0, -1.0);
    return normalize(
          k.xyy * mapSDF(p + k.xyy * e, m)
        + k.yyx * mapSDF(p + k.yyx * e, m)
        + k.yxy * mapSDF(p + k.yxy * e, m)
        + k.xxx * mapSDF(p + k.xxx * e, m));
}

// reconstruct a ray direction from NDC using inverse VP
vec3 rayDirection(vec2 ndc)
{
    vec4 pNear = vec4(ndc, -1.0, 1.0);
    vec4 pFar  = vec4(ndc,  1.0, 1.0);
    vec4 wNear = uInvVP * pNear;  wNear /= wNear.w;
    vec4 wFar  = uInvVP * pFar;   wFar  /= wFar.w;
    return normalize(vec3(wFar - wNear));
}

// =========================================================
// Ray march
// =========================================================
struct Hit { bool ok; float t; vec3 p; vec3 n; int matID; };

Hit raymarch(vec3 ro, vec3 rd)
{
    float t = 0.0;
    int matID = 0;
    for (int i = 0; i < 128; ++i) {
        vec3 p = ro + rd * t;
        float d = mapSDF(p, matID);
        if (d < 1e-3) {
            Hit h; h.ok = true; h.t = t; h.p = p; h.n = estimateNormal(p); h.matID = matID; return h;
        }
        t += d;
        if (t > 100.0) break;
    }
    Hit miss; miss.ok = false; return miss;
}

// =========================================================
// Lighting
// =========================================================
vec3 shade(Hit h, vec3 ro)
{
    vec3 L = normalize(vec3(0.6, 0.9, 0.3));
    float ndl = max(dot(h.n, L), 0.0);
    vec3 albedo = (h.matID == 1)
        ? vec3(0.9, 0.5, 0.3)
        : vec3(0.25, 0.35, 0.45);
    vec3 diff = albedo * ndl + 0.05 * albedo;
    float fog = clamp(h.t / 40.0, 0.0, 1.0);
    return mix(diff, vec3(0.02, 0.03, 0.05), fog);
}

// =========================================================
// Main
// =========================================================
void main()
{
    vec3 ro = uCameraPos;
    vec3 rd = rayDirection(vNDC);

    Hit h = raymarch(ro, rd);
    if (!h.ok) {
        float t = 0.5 * (rd.y + 1.0);
        vec3 sky = mix(vec3(0.06, 0.07, 0.10),
                       vec3(0.15, 0.20, 0.35), t);
        FragColor = vec4(sky, 1.0);
        return;
    }

    vec3 color = shade(h, ro);
    FragColor = vec4(color, 1.0);
}
