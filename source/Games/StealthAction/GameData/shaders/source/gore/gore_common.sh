#ifndef GORE_COMMON_SH
#define GORE_COMMON_SH

#define GORE_MAX_WOUNDS 8
#define GORE_MAX_CUTS 4
#define GORE_MAX_SPLATS 16
#define GORE_MAX_INTERVALS 16
#define GORE_FAR 1.0e9

uniform vec4 u_goreCounts;
uniform vec4 u_goreParams;
uniform vec4 u_goreRangeMask;
uniform vec4 u_goreBloodColor;
uniform vec4 u_goreCameraPos;
uniform mat4 u_goreViewProj;
uniform vec4 u_goreWoundBind[24];
uniform vec4 u_goreWoundWorld[24];
uniform vec4 u_goreWoundParams[8];
uniform vec4 u_goreWoundExtra[8];
uniform vec4 u_goreCutBind[4];
uniform vec4 u_goreCutBindSphere[4];
uniform vec4 u_goreCutRegrowBind[4];
uniform vec4 u_goreCutParams[4];
uniform vec4 u_goreCutWorld[4];
uniform vec4 u_goreCutWorldSphere[4];
uniform vec4 u_goreCutRegrowWorld[4];
uniform vec4 u_goreCutCenter[4];
uniform vec4 u_goreCutFrame[8];
uniform vec4 u_goreSplats[16];

SAMPLER2D(u_goreFlesh,       2);
SAMPLER2D(u_goreFleshNormal, 3);
SAMPLER2D(u_goreEdgeNoise,   4);
SAMPLER2D(u_goreBlood,       5);

bool goreBit(float mask, int i) { return mod(floor(mask / exp2(float(i))), 2.0) >= 1.0; }

// Edge noise in [-1, 1]. Lod 0: called inside loops/branches with discard.
float goreNoise(vec2 uv) { return texture2DLod(u_goreEdgeNoise, uv, 0.0).r * 2.0 - 1.0; }

vec3 goreWoundLocal(vec4 r0, vec4 r1, vec4 r2, vec3 p)
{
    vec4 p4 = vec4(p, 1.0);
    return vec3(dot(r0, p4), dot(r1, p4), dot(r2, p4));
}

void gorePlaneBasis(vec3 n, out vec3 t, out vec3 b)
{
    vec3 up = abs(n.y) < 0.99 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    t = normalize(cross(n, up));
    b = cross(n, t);
}

// Is bind point p removed by cut i (body side)? Also returns the fresh-growth amount (0..1).
bool goreCutRemoves(int i, vec3 p, out float fresh)
{
    fresh = 0.0;
    vec4 pl = u_goreCutBind[i];
    vec4 sph = u_goreCutBindSphere[i];
    if (length(p - sph.xyz) > sph.w) return false;
    vec4 cp = u_goreCutParams[i];
    vec3 t; vec3 b; gorePlaneBasis(pl.xyz, t, b);
    float edge = goreNoise(vec2(dot(p, t), dot(p, b)) * 6.0 + cp.y) * cp.x;
    float d = dot(pl.xyz, p) - pl.w;
    if (d <= -edge) return false;                       // kept side
    vec4 rg = u_goreCutRegrowBind[i];
    if (rg.w > 0.0)
    {
        float g = rg.w - length(p - rg.xyz) + goreNoise(p.xy * 9.0 + p.z * 3.0 + cp.y) * 0.01;
        if (g > 0.0) { fresh = 1.0 - smoothstep(0.0, 0.05, g); return false; }   // already regrown
    }
    return true;
}

// Front surface test in bind space. removed = discard. rim/soak/fresh in 0..1.
bool goreSurface(vec3 p, out float rim, out float soak, out float fresh)
{
    rim = 0.0; soak = 0.0; fresh = 0.0;
    bool isGib = u_goreParams.y > 0.5;
    int nw = int(u_goreCounts.x + 0.5);
    int nc = int(u_goreCounts.y + 0.5);
    int ns = int(u_goreCounts.z + 0.5);

    for (int i = 0; i < GORE_MAX_CUTS; i++)
    {
        if (i >= nc) break;
        if (!goreBit(u_goreRangeMask.y, i)) { if (isGib) return true; continue; }
        float f;
        bool removedForBody = goreCutRemoves(i, p, f);
        fresh = max(fresh, f);
        if (isGib ? !removedForBody : removedForBody) return true;
    }
    for (int i = 0; i < GORE_MAX_WOUNDS; i++)
    {
        if (i >= nw) break;
        if (!goreBit(u_goreRangeMask.x, i)) continue;
        vec4 wp = u_goreWoundParams[i];
        vec3 q = goreWoundLocal(u_goreWoundBind[i*3], u_goreWoundBind[i*3+1], u_goreWoundBind[i*3+2], p) / max(wp.z, 0.001);
        float r = length(q);
        float edge = goreNoise(q.xy * 0.5 + 0.5 + wp.w) * wp.x;
        if (r < 1.0 + edge) return true;
        rim = max(rim, (1.0 - smoothstep(1.0 + edge, 1.0 + edge + wp.y, r)) * min(wp.z * 2.0, 1.0));
    }
    for (int i = 0; i < GORE_MAX_SPLATS; i++)
    {
        if (i >= ns) break;
        if (!goreBit(u_goreRangeMask.z, i)) continue;
        vec4 s = u_goreSplats[i];
        float k = 1.0 - smoothstep(0.0, max(s.w, 0.0001), length(p - s.xyz));
        soak = max(soak, k * (0.6 + 0.4 * texture2DLod(u_goreBlood, p.xy * 7.0 + p.z * 2.0, 0.0).r));
    }
    return false;
}

// ---- interior ------------------------------------------------------------------------------------------

// Ray (world, dir normalised) vs ellipsoid given by world->unit rows. Returns false on miss.
bool goreRayEllipsoid(int i, vec3 ro, vec3 rd, out vec2 t)
{
    float heal = max(u_goreWoundParams[i].z, 0.001);
    vec4 r0 = u_goreWoundWorld[i*3]; vec4 r1 = u_goreWoundWorld[i*3+1]; vec4 r2 = u_goreWoundWorld[i*3+2];
    vec3 o = vec3(dot(r0, vec4(ro,1.0)), dot(r1, vec4(ro,1.0)), dot(r2, vec4(ro,1.0))) / heal;
    vec3 d = vec3(dot(r0.xyz, rd), dot(r1.xyz, rd), dot(r2.xyz, rd)) / heal;
    float a = dot(d, d);
    float b = dot(o, d);
    float c = dot(o, o) - 1.0;
    float disc = b*b - a*c;
    if (disc <= 0.0) { t = vec2(0.0, 0.0); return false; }
    float s = sqrt(disc);
    t = vec2((-b - s) / a, (-b + s) / a);
    return true;
}

bool goreRaySphere(vec3 ro, vec3 rd, vec4 sph, out vec2 t)
{
    vec3 oc = ro - sph.xyz;
    float b = dot(oc, rd);
    float c = dot(oc, oc) - sph.w * sph.w;
    float disc = b*b - c;
    if (sph.w <= 0.0 || disc <= 0.0) { t = vec2(0.0, 0.0); return false; }
    float s = sqrt(disc);
    t = vec2(-b - s, -b + s);
    return true;
}

// kind: 0 cavity (wound), 1 cap (cut plane), 2 regrow front. prim = wound or cut index.
// Returns false when the fragment must be discarded.
bool goreInterior(vec3 worldB, vec3 bindB, out vec3 P, out float kind, out float prim, out float thickness)
{
    P = worldB; kind = 0.0; prim = 0.0; thickness = 0.0;

    float rimD; float soakD; float freshD;
    if (goreSurface(bindB, rimD, soakD, freshD)) return false;      // the back wall itself is removed: through-hole

    vec3 ro = u_goreCameraPos.xyz;
    vec3 rd = normalize(worldB - ro);
    float tB = length(worldB - ro);

    vec2  iv[GORE_MAX_INTERVALS];
    float ivKind[GORE_MAX_INTERVALS];
    float ivPrim[GORE_MAX_INTERVALS];
    float ivReach[GORE_MAX_INTERVALS];
    int count = 0;

    int nw = int(u_goreCounts.x + 0.5);
    int nc = int(u_goreCounts.y + 0.5);

    for (int i = 0; i < GORE_MAX_WOUNDS; i++)
    {
        if (i >= nw) break;
        if (!goreBit(u_goreRangeMask.x, i)) continue;
        vec2 t;
        if (goreRayEllipsoid(i, ro, rd, t) && count < GORE_MAX_INTERVALS)
        {
            iv[count] = t; ivKind[count] = 0.0; ivPrim[count] = float(i); ivReach[count] = u_goreWoundExtra[i].x; count++;
        }
    }
    for (int i = 0; i < GORE_MAX_CUTS; i++)
    {
        if (i >= nc) break;
        if (!goreBit(u_goreRangeMask.y, i)) continue;
        vec2 ts;
        if (!goreRaySphere(ro, rd, u_goreCutWorldSphere[i], ts)) continue;
        vec4 pl = u_goreCutWorld[i];
        if (u_goreParams.y > 0.5) pl = -pl;                    // gib keeps the body's removed side
        float dn = dot(pl.xyz, rd);
        float s0 = dot(pl.xyz, ro) - pl.w;
        vec2 h;
        if (abs(dn) < 1.0e-5) { if (s0 <= 0.0) continue; h = vec2(-GORE_FAR, GORE_FAR); }
        else { float tp = -s0 / dn; h = dn > 0.0 ? vec2(tp, GORE_FAR) : vec2(-GORE_FAR, tp); }
        float a = max(h.x, ts.x);
        float b = min(h.y, ts.y);
        if (a >= b) continue;
        float endKind = 1.0;                                   // ends on the plane (or the influence sphere)
        vec2 g;
        if (u_goreCutRegrowWorld[i].w > 0.0 && goreRaySphere(ro, rd, u_goreCutRegrowWorld[i], g))
        {
            // removed = [a,b] minus [g.x,g.y]
            if (g.x > a && count < GORE_MAX_INTERVALS)
            { iv[count] = vec2(a, min(b, g.x)); ivKind[count] = 2.0; ivPrim[count] = float(i); ivReach[count] = u_goreCutFrame[i*2+1].w; count++; }
            if (g.y < b && count < GORE_MAX_INTERVALS)
            { iv[count] = vec2(max(a, g.y), b); ivKind[count] = endKind; ivPrim[count] = float(i); ivReach[count] = u_goreCutFrame[i*2+1].w; count++; }
        }
        else if (count < GORE_MAX_INTERVALS)
        {
            iv[count] = vec2(a, b); ivKind[count] = endKind; ivPrim[count] = float(i); ivReach[count] = u_goreCutFrame[i*2+1].w; count++;
        }
    }

    // The back face point inside a removed interval = removed back wall (world test agrees with the bind test above;
    // this catches the rigid-approximation cases).
    for (int k = 0; k < GORE_MAX_INTERVALS; k++)
    {
        if (k >= count) break;
        if (tB > iv[k].x && tB < iv[k].y) return false;
    }

    float best = -1.0; int bestK = -1;
    for (int k = 0; k < GORE_MAX_INTERVALS; k++)
    {
        if (k >= count) break;
        float e = iv[k].y;
        if (e <= 0.0 || e >= tB) continue;
        if (tB - e > ivReach[k]) continue;
        bool inside = false;
        for (int j = 0; j < GORE_MAX_INTERVALS; j++)
        {
            if (j >= count) break;
            if (j != k && e > iv[j].x && e < iv[j].y) inside = true;
        }
        if (inside) continue;
        if (e > best) { best = e; bestK = k; }
    }
    if (bestK < 0) return false;

    P = ro + rd * best;
    for (int k = 0; k < GORE_MAX_INTERVALS; k++)
    {
        if (k == bestK) { kind = ivKind[k]; prim = ivPrim[k]; }
    }
    thickness = tB - best;
    return true;
}

float goreDepth(vec3 P)
{
    vec4 clip = mul(u_goreViewProj, vec4(P, 1.0));
    float z = clip.z / clip.w;
    return u_goreParams.x > 0.5 ? z * 0.5 + 0.5 : z;
}

#endif
