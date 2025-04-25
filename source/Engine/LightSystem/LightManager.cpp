#include "LightManager.h"

#include "../Camera.h"

#include "../BoundingSphere.hpp"

#include "../DebugDraw.hpp"

vec3 LightManager::LightDirection = vec3(-1,-1,1);

const float cascadeExponent = 3;

float LightManager::LightDistance = 300;

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

}

void LightManager::CalculateLightMatrices(
    float lightDistance, glm::vec3 cameraPos,
    glm::mat4& outLightView,
    glm::mat4& outLightProjection
) {

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


