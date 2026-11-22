#ifndef GORE_FLESH_SH
#define GORE_FLESH_SH

// Shared flesh shading for the cut caps, shells, anatomy and chunks. Include after lighting_common.sh, gore_common.sh and the
// u_goreFlesh sampler declaration of the including .sc.

// Flesh colour projected along the three axes (all reads at lod 0: this runs inside divergent control flow).
vec3 goreTriplanar(vec3 p, vec3 n, float scale)
{
    vec3 w = abs(n); w = w / (w.x + w.y + w.z + 0.0001);
    return texture2DLod(u_goreFlesh, p.yz * scale, 0.0).rgb * w.x
         + texture2DLod(u_goreFlesh, p.xz * scale, 0.0).rgb * w.y
         + texture2DLod(u_goreFlesh, p.xy * scale, 0.0).rgb * w.z;
}

// Flesh lighting: the stepped diffuse of the directional and point lights plus a warm red scatter at the terminator.
// There is deliberately no normal map and no specular (no highlight, no fresnel): the game is LDR (no HDR, no tone mapping),
// where a bump or a highlight on a dark wet surface clips to white sparkle. scatterAmount 0..1 scales the red scatter.
vec3 goreShade(vec3 albedo, vec3 N, vec3 P, vec3 viewDir, float scatterAmount)
{
    vec3 lightDir = normalize(direct_light_dir.xyz);
    vec3 diffuse = CalculateDirectionalDiffuse(N, lightDir);
    vec3 pd = vec3(0.0, 0.0, 0.0); vec3 ps = vec3(0.0, 0.0, 0.0);
    CalculatePointLights(N, P, viewDir, pd, ps);   // ps (the point lights' specular) is not used

    float ndl = dot(N, lightDir);
    vec3 scatter = albedo * vec3(0.55, 0.10, 0.06) * light_color.rgb * (0.35 + 0.45 * (1.0 - abs(ndl))) * min(scatterAmount * 2.0, 1.0);
    return albedo * (diffuse + pd) + scatter * 0.6;
}

#endif
