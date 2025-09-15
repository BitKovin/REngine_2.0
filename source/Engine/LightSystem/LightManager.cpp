#include "LightManager.h"

#include "../Camera.h"

#include "../BoundingSphere.hpp"

#include "../DebugDraw.hpp"

vec3 LightManager::LightDirection = vec3(-1,-1,1);

const float cascadeExponent = 3.0f;

float LightManager::LightDistance = 200;

float LightManager::LightDistance1 = 0;
float LightManager::LightDistance2 = 0;
float LightManager::LightDistance3 = 0;
float LightManager::LightDistance4 = 0;

float LightManager::LightRadius1 = 0;
float LightManager::LightRadius2 = 0;
float LightManager::LightRadius3 = 0;
float LightManager::LightRadius4 = 0;

int LightManager::ShadowMapResolution = 1024*2;
float LightManager::LightDistanceMultiplier = 1;
float LightManager::ShaddowOffsetScale = 1;

mat4 LightManager::lightView1;
mat4 LightManager::lightProjection1;

mat4 LightManager::lightView2;
mat4 LightManager::lightProjection2;

mat4 LightManager::lightView3;
mat4 LightManager::lightProjection3;

mat4 LightManager::lightView4;
mat4 LightManager::lightProjection4;

bool LightManager::DirectionalShadowsEnabled = false;

LightManager::LightManager()
{
}

LightManager::~LightManager()
{
}


std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
{
    const auto inv = glm::inverse(proj * view);

    std::vector<glm::vec4> frustumCorners;
    for (unsigned int x = 0; x < 2; ++x)
    {
        for (unsigned int y = 0; y < 2; ++y)
        {
            for (unsigned int z = 0; z < 2; ++z)
            {
                const glm::vec4 pt =
                    inv * glm::vec4(
                        2.0f * x - 1.0f,
                        2.0f * y - 1.0f,
                        2.0f * z - 1.0f,
                        1.0f);

                frustumCorners.push_back(pt / pt.w);
            }
        }
    }

    return frustumCorners;
}

void DebugDrawCorders(std::vector<glm::vec4> points)
{
    for (auto p : points)
    {
        DebugDraw::Line(Camera::position, p, 3);
    }
}

BoundingSphere CalculateCascadeInfo(const glm::mat4& proj, const glm::mat4& view)
{
    auto corners = getFrustumCornersWorldSpace(proj, view);

    return BoundingSphere::FromPoints(corners);

}

void LightManager::Update()
{
    // 1) Tweakable parameters
    LightDirection = glm::normalize(LightDirection);
    
    LightDistance4 = LightDistance;
    LightDistance3 = LightDistance4 / cascadeExponent;
    LightDistance2 = LightDistance3 / cascadeExponent;
    LightDistance1 = LightDistance2 / cascadeExponent;

    mat4 view = Camera::finalizedView;
    mat4 proj1 = perspective(radians(Camera::FOV), Camera::AspectRatio, 0.05f, LightDistance1);
    mat4 proj2 = perspective(radians(Camera::FOV), Camera::AspectRatio, LightDistance1, LightDistance2);
    mat4 proj3 = perspective(radians(Camera::FOV), Camera::AspectRatio, LightDistance2, LightDistance3);
    mat4 proj4 = perspective(radians(Camera::FOV), Camera::AspectRatio, LightDistance3, LightDistance4);


    BoundingSphere casc1 = CalculateCascadeInfo(proj1, view);
    BoundingSphere casc2 = CalculateCascadeInfo(proj2, view);
    BoundingSphere casc3 = CalculateCascadeInfo(proj3, view);
    BoundingSphere casc4 = CalculateCascadeInfo(proj4, view);

    LightRadius1 = casc1.Radius;
    LightRadius2 = casc2.Radius;
    LightRadius3 = casc3.Radius;
    LightRadius4 = casc4.Radius;

    CalculateLightMatrices(casc1.Radius, casc1.offset,lightView1, lightProjection1);
    CalculateLightMatrices(casc2.Radius, casc2.offset, lightView2, lightProjection2);
    CalculateLightMatrices(casc3.Radius, casc3.offset, lightView3, lightProjection3);
    CalculateLightMatrices(casc4.Radius, casc4.offset, lightView4, lightProjection4);

    FinishPointLights();

}

void LightManager::UpdateLightSource(PointLightInfo light)
{

    m_lights.push_back(light);

}

void LightManager::ApplyPointLightToShader(ShaderProgram* shader, vec3 boundsMin, vec3 boundsMax)
{
    if (!shader) return;

    BoundingBox box(boundsMin, boundsMax);

    const size_t MAX_LIGHTS = 16u;

    std::vector<vec4> positions;    positions.reserve(MAX_LIGHTS); // xyz = pos, w = innerConeCos
    std::vector<vec3> colors;       colors.reserve(MAX_LIGHTS);
    std::vector<vec4> directions;   directions.reserve(MAX_LIGHTS); // xyz = dir(normalized), w = outerConeCos
    std::vector<float> radiuses;    radiuses.reserve(MAX_LIGHTS);

    auto degToRad = [](float deg) -> float {
        return deg * (float)(M_PI / 180.0);
        };

    // clamp degrees to [0, 179.999] to avoid cos() producing degenerate equal edges
    auto clampAngleDegSafe = [](float deg) -> float {
        // If user uses 360 to mean "omni", treat it like ~180 degrees so cos is near -1
        if (deg >= 360.0f) return 179.999f;
        if (deg < 0.0f) return 0.0f;
        return std::min(deg, 179.999f);
        };

    int added = 0;
    for (const auto& L : m_finalLights)
    {
        if (added >= static_cast<int>(MAX_LIGHTS)) break;
        if (!box.Intersects(L.position, L.radius)) continue;

        // read and sanitize angles
        float innerDeg = clampAngleDegSafe(L.innerConeAngleDegrees);
        float outerDeg = clampAngleDegSafe(L.outerConeAngleDegrees);

        // ensure innerAngle is the smaller cone (full-bright), outer is larger (falloff)
        if (innerDeg > outerDeg) std::swap(innerDeg, outerDeg);

        // convert to cosines (matches dot(...) output of shader)
        float innerCos = std::cos(degToRad(innerDeg));
        float outerCos = std::cos(degToRad(outerDeg));

        // numeric safety: innerCos should be >= outerCos for smoothstep(outer, inner, x)
        if (innerCos < outerCos) std::swap(innerCos, outerCos);

        // store position + innerCos
        positions.emplace_back(L.position, innerCos);

        // color (rgb) — keep alpha if you want by using vec4 color uploads instead
        colors.emplace_back(L.color.r, L.color.g, L.color.b);

        // normalize direction (fallback to +Z if degenerate) and store outerCos
        vec3 dir = L.direction;
        float dirLen = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
        if (dirLen > 1e-6f) dir /= dirLen;
        else dir = vec3(0.0f, 0.0f, 1.0f);

        directions.emplace_back(dir, outerCos);

        radiuses.push_back(L.radius);

        ++added;
    }

    // pad to MAX_LIGHTS to keep uniform array sizes stable
    while (positions.size() < MAX_LIGHTS) positions.emplace_back(vec4(0.0f));
    while (colors.size() < MAX_LIGHTS) colors.emplace_back(vec3(0.0f));
    while (directions.size() < MAX_LIGHTS) directions.emplace_back(vec4(0.0f));
    while (radiuses.size() < MAX_LIGHTS) radiuses.push_back(0.0f);

    shader->SetUniform("PointLightsNumber", added);
    shader->SetUniform("LightPositions", positions);
    shader->SetUniform("LightColors", colors);
    shader->SetUniform("LightRadiuses", radiuses);
    shader->SetUniform("LightDirections", directions);
}



void LightManager::CalculateLightMatrices(
    float lightDistance, glm::vec3 cameraPos,
    glm::mat4& outLightView,
    glm::mat4& outLightProjection) 
{

    glm::vec3 L = glm::normalize(LightDirection);
    float halfSize = lightDistance;
    float nearPlane = -1000.0f;
    float farPlane = lightDistance;
    float shadowResH = ShadowMapResolution * 0.5f;

    // 2) build raw view + ortho
    glm::mat4 view = glm::lookAt(
        cameraPos,
        cameraPos + L,
        glm::vec3(0, 0, 1)
    );
    glm::mat4 proj = glm::ortho(
        -halfSize, +halfSize,
        -halfSize, +halfSize,
        nearPlane, farPlane
    );

    // 3) snap-to-texel-grid
    glm::vec4 originLS = view * glm::vec4(0, 0, 0, 1);
    float worldPerTexel = (2.0f * halfSize) / shadowResH;
    float snappedX = std::floor(originLS.x / worldPerTexel) * worldPerTexel;
    float snappedY = std::floor(originLS.y / worldPerTexel) * worldPerTexel;
    glm::vec3 offset(
        snappedX - originLS.x,
        snappedY - originLS.y,
        0.0f
    );
    view = glm::translate(glm::mat4(1.0f), offset) * view;

    // 4) output
    outLightView = view;
    outLightProjection = proj;
}

void LightManager::FinishPointLights()
{

    m_finalLights.clear();


    for (auto& light : m_lights)
    {

        if (Camera::frustum.IsSphereVisible(light.position, light.radius))
        {

            m_finalLights.push_back(light);

        }

    }

    std::sort(m_finalLights.begin(), m_finalLights.end(),
        [](PointLightInfo a, PointLightInfo b) {
            return distance2(a.position,Camera::position) < distance2(b.position, Camera::position);
        });

    m_lights.clear();

}


