#include "LightManager.h"

#include "../Camera.h"

vec3 LightManager::LightDirection = vec3(-1,-1,1);

float LightManager::LightDistance1 = 30;
float LightManager::LightDistance2 = 60;
float LightManager::LightDistance3 = 120;
float LightManager::LightDistance4 = 240;

int LightManager::ShadowMapResolution = 1024;
float LightManager::LightDistanceMultiplier = 1;

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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void LightManager::Update()
{
    // 1) Tweakable parameters
    LightDirection = glm::normalize(LightDirection);
    
    CalculateLightMatrices(LightDistance1, lightView1, lightProjection1);
    CalculateLightMatrices(LightDistance2, lightView2, lightProjection2);
    CalculateLightMatrices(LightDistance3, lightView3, lightProjection3);
    CalculateLightMatrices(LightDistance4, lightView4, lightProjection4);

}

void LightManager::CalculateLightMatrices(
    float lightDistance,
    glm::mat4& outLightView,
    glm::mat4& outLightProjection
) {
    // 1) prep
    glm::vec3 cameraPos = Camera::finalizedPosition;
    glm::vec3 L = glm::normalize(LightDirection);
    float halfSize = lightDistance;
    float nearPlane = -100.0f;
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


