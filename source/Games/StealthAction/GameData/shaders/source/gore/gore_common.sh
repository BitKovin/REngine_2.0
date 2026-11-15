#ifndef GORE_COMMON_SH
#define GORE_COMMON_SH

#define GORE_MAX_WOUNDS 8
#define GORE_MAX_CUTS 4
#define GORE_MAX_SPLATS 16

uniform vec4 u_goreCounts;        // x wounds, y cuts, z splats
uniform vec4 u_goreParams;        // x unused, y isGib, z fleshScale, w gore level (0 full, 1 reduced, 2 minimal)
uniform vec4 u_goreRangeMask;     // per index range: x wound bits, y cut bits, z splat bits
uniform vec4 u_goreLayer;         // x layer index, y inset fraction, z 1 = anatomy / cap / cavity / chunk (no inset), w 1 = cap, 2 = wound cavity
uniform vec4 u_goreBloodColor;
uniform vec4 u_goreCameraPos;
uniform vec4 u_goreWoundBind[24];
uniform vec4 u_goreWoundParams[8];   // x roughness, y rimWidth, z healScale, w seed
uniform vec4 u_goreWoundExtra[8];    // x taper (0 = a cavity closes the hole), y 1 = the anatomy (bones, organs) stays behind this wound, z 1 = all layers share the hole edge
uniform vec4 u_goreCutBind[4];
uniform vec4 u_goreCutBindSphere[4];
uniform vec4 u_goreCutRegrowBind[4];
uniform vec4 u_goreCutParams[4];
uniform vec4 u_goreSplats[16];       // xyz centre, w extent
uniform vec4 u_goreSplatLook[16];    // xyz direction of the blow (zero = a plain soft soak), w 1 = a bullet hole picture, 0 = a splash

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

// One blood decal (GoreTypes.h Splat) at bind point p. x = how much of the skin it covers (0..1), y = how thick and dark the blood
// lies (0 thin fresh blood .. 1 the black puncture).
//
// With a direction the decal is a picture of the atlas u_goreDecal (Tools/gore/make_blood_decals.py: 4x2 cells, R coverage, G
// thickness; the top row has four splashes, the bottom row four bullet holes, look.w picks the row), laid on the plane square to
// the blow like a projected decal: the plane coordinates of p scaled so the extent is 1, turned by a random angle, with the cell
// chosen at random; the random numbers come from where the decal is, so no two alike.
// Without a direction (a stump, a protected body) it is a plain soft soak.
vec2 goreSplatLook(vec3 p, vec4 s, vec4 look)
{
    vec3 v = p - s.xyz;
    float R = max(s.w, 0.0001);
    float dist = length(v);
    if (dist > R) return vec2(0.0, 0.0);

    if (dot(look.xyz, look.xyz) < 0.25)
    {
        float k = 1.0 - smoothstep(0.0, R, dist);
        return vec2(k * (0.6 + 0.4 * texture2DLod(u_goreBlood, p.xy * 7.0 + p.z * 2.0, 0.0).r), 0.0);
    }

    vec3 t; vec3 b; gorePlaneBasis(normalize(look.xyz), t, b);
    vec2 uv = vec2(dot(v, t), dot(v, b)) / R;
    float seed = fract(sin(dot(s.xyz, vec3(12.9898, 78.233, 37.719))) * 43758.5453);
    float turn = seed * 6.2831853;
    uv = vec2(cos(turn) * uv.x - sin(turn) * uv.y, sin(turn) * uv.x + cos(turn) * uv.y);
    vec2 corner = vec2(floor(fract(seed * 7.31) * 4.0), look.w > 0.5 ? 1.0 : 0.0);
    vec4 d = texture2DLod(u_goreDecal, (corner + uv * 0.5 + 0.5) * vec2(0.25, 0.5), 1.0);   // lod 1: the decal is a few centimetres, a texel is a fraction of a millimetre
    return vec2(d.r, min(d.g / max(d.r, 0.05), 1.0));   // the atlas stores the thickness already multiplied by the coverage
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

// Surface test in bind space for the layer in u_goreLayer.x. true = discard.
// rim: 0..1 near a hole edge of this layer. soak: blood soaked in (skin), clot: how thick and dark it lies (goreSplatLook).
// fresh: regrowing.
bool goreSurface(vec3 p, out float rim, out float soak, out float clot, out float fresh)
{
    rim = 0.0; soak = 0.0; clot = 0.0; fresh = 0.0;
    bool isGib = u_goreParams.y > 0.5;
    float layer = u_goreLayer.x;
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
        // Skin and the shells open up with a hole that gets smaller with depth; the inside of the hole is closed by the wound
        // cavity (GoreCavity.h). The anatomy behind a shot or a gash is not removed (the ribs show in the hole): it only turns
        // bloody inside the wound. (Cuts and sever wounds still remove everything: the cap closes those.)
        bool inner = layer > 3.5 && u_goreWoundExtra[i].y > 0.5;
        float scale = inner ? wp.z * 0.8 : wp.z * max(0.0, 1.0 - layer * u_goreWoundExtra[i].x);   // deeper layers: smaller hole
        if (scale < 0.02) continue;
        vec3 q = goreWoundLocal(u_goreWoundBind[i*3], u_goreWoundBind[i*3+1], u_goreWoundBind[i*3+2], p) / scale;
        float r = length(q);
        // Two octaves. A wound whose hole is closed by a cavity cuts every layer along the same edge (one wall, no stacked layers);
        // otherwise each layer has its own pattern, so the layer edges never line up.
        vec2 nuv = q.xy * 0.5 + 0.5 + wp.w + layer * 0.37 * (1.0 - u_goreWoundExtra[i].z);
        float edge = (goreNoise(nuv) * 0.7 + goreNoise(nuv * 2.7 + 0.31) * 0.3) * wp.x;
        if (r < 1.0 + edge)
        {
            if (!inner) return true;
            rim = 1.0;   // the inside of a wound is wet red
            continue;
        }
        rim = max(rim, (1.0 - smoothstep(1.0 + edge, 1.0 + edge + wp.y, r)) * min(wp.z * 2.0, 1.0));
    }
    for (int i = 0; i < GORE_MAX_SPLATS; i++)
    {
        if (i >= ns) break;
        if (!goreBit(u_goreRangeMask.z, i)) continue;
        vec2 decal = goreSplatLook(p, u_goreSplats[i], u_goreSplatLook[i]);
        if (decal.x > soak) { soak = decal.x; clot = decal.y; }   // where decals overlap, the heavier one gives the tone
    }
    return false;
}


// The surface of cavity `own` (the inside of one wound, GoreCavity.h) is removed where a cut or another wound already took the
// body away: wounds subtract from each other, so only the outline of their union is left. true = discard.
bool goreCavityRemoved(vec3 p, float own)
{
    bool isGib = u_goreParams.y > 0.5;
    int nw = int(u_goreCounts.x + 0.5);
    int nc = int(u_goreCounts.y + 0.5);

    for (int i = 0; i < GORE_MAX_CUTS; i++)
    {
        if (i >= nc) break;
        if (!goreBit(u_goreRangeMask.y, i)) { if (isGib) return true; continue; }
        float f;
        bool removedForBody = goreCutRemoves(i, p, f);
        if (isGib ? !removedForBody : removedForBody) return true;
    }
    for (int i = 0; i < GORE_MAX_WOUNDS; i++)
    {
        if (i >= nw) break;
        if (abs(float(i) - own) < 0.5) continue;
        if (!goreBit(u_goreRangeMask.x, i)) continue;
        vec4 wp = u_goreWoundParams[i];
        if (wp.z < 0.02) continue;
        vec3 q = goreWoundLocal(u_goreWoundBind[i*3], u_goreWoundBind[i*3+1], u_goreWoundBind[i*3+2], p) / wp.z;
        if (length(q) < 1.0 + wp.x) return true;   // inside the other hole (its surface is the nominal ellipsoid grown by the edge roughness)
    }
    return false;
}

#endif
