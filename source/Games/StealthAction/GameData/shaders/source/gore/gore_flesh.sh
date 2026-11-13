#ifndef GORE_FLESH_SH
#define GORE_FLESH_SH

// Shared flesh shading for the cut caps and the wound cavities. Include after lighting_common.sh, gore_common.sh and the
// u_goreFlesh / u_goreFleshNormal sampler declarations of the including .sc.

// Flesh colour projected along the three axes (all reads at lod 0: this runs inside divergent control flow).
vec3 goreTriplanar(vec3 p, vec3 n, float scale)
{
    vec3 w = abs(n); w = w / (w.x + w.y + w.z + 0.0001);
    return texture2DLod(u_goreFlesh, p.yz * scale, 0.0).rgb * w.x
         + texture2DLod(u_goreFlesh, p.xz * scale, 0.0).rgb * w.y
         + texture2DLod(u_goreFlesh, p.xy * scale, 0.0).rgb * w.z;
}

// Surface bump: the tangent-space normal map projected along the three axes, applied to the geometric normal.
vec3 goreBumpTriplanar(vec3 n, vec3 p, float scale, float k)
{
    vec3 w = abs(n); w = w / (w.x + w.y + w.z + 0.0001);
    vec3 nx = texture2DLod(u_goreFleshNormal, p.yz * scale, 0.0).xyz * 2.0 - 1.0;
    vec3 ny = texture2DLod(u_goreFleshNormal, p.xz * scale, 0.0).xyz * 2.0 - 1.0;
    vec3 nz = texture2DLod(u_goreFleshNormal, p.xy * scale, 0.0).xyz * 2.0 - 1.0;
    vec3 bump = vec3(0.0, nx.x, nx.y) * w.x + vec3(ny.x, 0.0, ny.y) * w.y + vec3(nz.x, nz.y, 0.0) * w.z;
    return normalize(n + bump * k);
}

// Wet-flesh lighting: soft wrap diffuse, warm scattering at the terminator, a crisp gloss highlight and a sky fresnel.
vec3 goreWetShade(vec3 albedo, vec3 N, vec3 P, vec3 viewDir, float wet, float gloss)
{
    vec3 lightDir = normalize(direct_light_dir.xyz);
    vec3 diffuse = CalculateDirectionalDiffuse(N, lightDir);
    vec3 pd = vec3(0.0, 0.0, 0.0); vec3 ps = vec3(0.0, 0.0, 0.0);
    CalculatePointLights(N, P, viewDir, pd, ps);

    float ndl = dot(N, lightDir);
    float ndv = clamp(dot(N, viewDir), 0.0, 1.0);
    vec3 h = normalize(lightDir + viewDir);
    float ndh = clamp(dot(N, h), 0.0, 1.0);

    vec3 scatter = albedo * vec3(0.55, 0.10, 0.06) * light_color.rgb * (0.35 + 0.45 * (1.0 - abs(ndl))) * min(wet * 2.0, 1.0);
    vec3 base = albedo * (diffuse + pd) + scatter * 0.6;

    float crisp = pow(ndh, 90.0) * 1.1 + pow(ndh, 24.0) * 0.35;
    vec3 spec = direct_light_color.rgb * crisp * wet * gloss
              + CalculateDirectionalSpecular(N, lightDir, viewDir) * (0.4 + 1.6 * wet)
              + ps * wet;
    float fres = pow(1.0 - ndv, 4.0) * 0.30 * wet * gloss;
    return base + spec + light_color.rgb * fres;
}

#endif
