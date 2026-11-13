#ifndef GORE_SHADOW_SH
#define GORE_SHADOW_SH

// Shadow volumes of a body with severed parts: every bind-space position beyond a cut plane is flattened onto the
// plane, so the missing part casts no shadow (and a regrown part casts its own again).
uniform vec4 u_goreCounts;
uniform vec4 u_goreCutBind[4];
uniform vec4 u_goreCutBindSphere[4];
uniform vec4 u_goreCutRegrowBind[4];

vec3 goreFlatten(vec3 p)
{
    int nc = int(u_goreCounts.y + 0.5);
    for (int i = 0; i < 4; i++)
    {
        if (i >= nc) break;
        vec4 pl = u_goreCutBind[i];
        vec4 sph = u_goreCutBindSphere[i];
        float s = dot(pl.xyz, p) - pl.w;
        if (s <= 0.0 || length(p - sph.xyz) > sph.w) continue;
        vec4 rg = u_goreCutRegrowBind[i];
        if (rg.w > 0.0 && length(p - rg.xyz) < rg.w) continue;   // the regrown part casts its shadow
        p -= pl.xyz * s;
    }
    return p;
}

#endif
