#pragma once
#include "../Physics.h"
#include "../Entity.h"

class CharacterController
{
public:
	CharacterController();
	~CharacterController();

	void Init(Entity* owner, vec3 position, float radius = 0.5, float height = 1.8);

	void Destroy();

	void Update(float deltaTime);

	vec3 GetPosition();
	vec3 GetSmoothPosition();
	void SetPosition(vec3 position);

	void UpdateSmoothPosition(float deltaTime);

	vec3 GetVelocity();
	void SetVelocity(vec3 vel);

	Body* body = nullptr;



	float gravity = 27;

	float stepHeight = 0.4;

	float height = 1.8;
	float radius = 0.5;

	float groundMaxAngle = 46;

	float stepSmoothingSpeed = 3.5f;

	bool onGround = false;


	static float GroundAngleRad(const glm::vec3& normal);

	/// Same, but returns degrees.
	static float GroundAngleDeg(const glm::vec3& normal);

private:

	float heightSmoothOffset = 0;

	void UpdateGroundCheck(bool& hitsGround, float& calculatedCharacterHeight);

	bool CheckGroundAt(vec3 location, float& height);

	//vec3 velocity = vec3(0);

};

