#include "LightManager.h"

#include "../Camera.h"

vec3 LightManager::LightDirection = vec3(-1,-1,1);

float LightManager::LightDistance = 20;
int LightManager::ShadowMapResolution = 512;
float LightManager::LightDistanceMultiplier = 1;

mat4 LightManager::lightView;
mat4 LightManager::lightProjection;

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
    float lightDistance = LightManager::LightDistance;
    float shadowRes = LightManager::ShadowMapResolution;  // e.g. 2048
    float nearPlane = -100;
    float farPlane = lightDistance;
    float halfSize = lightDistance;

    // 2) Position your “light camera” out along the light direction
    glm::vec3 camPos = Camera::finalizedPosition;
    glm::vec3 lightCamPos = camPos;

    // 3) Build the raw view & projection
    lightView = glm::lookAt(
        lightCamPos,
        lightCamPos + LightDirection,
        glm::vec3(0, 0, 1)
    );
    lightProjection = glm::ortho(
        -halfSize, +halfSize,
        -halfSize, +halfSize,
        nearPlane, farPlane
    );

    //
    // ——— 4) SNAP TO TEXEL GRID ———
    // Instead of inverting matrices, we simply figure out
    // how much to shift the view so that the origin of the
    // light‐space grid lands on exact integer texel centers.
    //
    // (a) Transform world‐origin into light‐view space:
    glm::vec4 originLS = lightView * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    // (b) Figure out how many world‐space units each shadow‐map texel covers:
    float worldUnitsPerTexel = (2.0f * halfSize) / shadowRes;

    // (c) Round the x/y of that origin into exact texel increments:
    float snappedX = std::floor(originLS.x / worldUnitsPerTexel) * worldUnitsPerTexel;
    float snappedY = std::floor(originLS.y / worldUnitsPerTexel) * worldUnitsPerTexel;

    // (d) Compute the small offset needed to move originLS → snapped:
    glm::vec3 offset = glm::vec3(snappedX - originLS.x,
        snappedY - originLS.y,
        0.0f);

    // (e) Pre‑concatenate a translation so the entire view is shifted:
    lightView = glm::translate(glm::mat4(1.0f), offset) * lightView;

}


