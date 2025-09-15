#include "PointLight.h"
#include "../LightSystem/LightManager.h"

PointLight::PointLight()
{
}

PointLight::~PointLight()
{
}

void PointLight::Finalize()
{

	LightManager::UpdateLightSource(LightManager::PointLightInfo{ 
		Position,color, radius, 
		MathHelper::GetForwardVector(Rotation),
		innerConeAngleDegrees, outerConeAngleDegrees });

}