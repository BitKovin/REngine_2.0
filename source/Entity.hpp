#pragma once

#include <vector>

#include "EObject.hpp"
#include "LevelObject.hpp"

#include "IDrawMesh.h"

#include "glm.h"

#include "Physics.h"

#include "RegisterLevelObject.h"

#include "MapData.h"

#include "Level.hpp"

using namespace std;

class Entity : public LevelObject
{
public:

	vec3 Position = vec3();

	vec3 Rotation = vec3();

	vector<IDrawMesh*> Drawables;

	Body* LeadBody = nullptr;

	vector<Body*> Bodies;

	string ClassName = "Entity";

	string Name = "";

	bool Destroyed = false;

	Entity()
	{

	}
	~Entity()
	{

	}

	void UpdatePhysics()
	{
		if (LeadBody)
		{
			Position = FromPhysics(LeadBody->GetPosition());
			Rotation = MathHelper::ToYawPitchRoll(FromPhysics(LeadBody->GetRotation()));
		}
	}

	virtual void FromData(EntityData data)
	{

		Name = data.GetPropertyString("targetName");

		Position = data.GetPropertyVectorPosition("origin");

	}

	void Finalize()
	{

	}

	virtual void OnDamage(float Damage, Entity* DamageCauser = nullptr, Entity* Weapon = nullptr){}

	virtual void OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone = "", Entity* DamageCauser = nullptr, Entity* Weapon = nullptr)
	{
		OnDamage(Damage, DamageCauser, Weapon);
	}

	vector<IDrawMesh*> GetDrawMeshes()
	{
		return Drawables;
	}

	void DestroyDrawables()
	{
		for (IDrawMesh* mesh : Drawables)
		{
			delete(mesh);
		}

		Drawables.clear();
	}

	void FinalLevelRemove()
	{
		DestroyDrawables();
	}

	virtual void Destroy()
	{

		Destroyed = true;

		Physics::DestroyBody(LeadBody);
		LeadBody = nullptr;
		for (Body* body : Bodies)
		{
			Physics::DestroyBody(body);
		}
		Bodies.clear();

		Level::Current->RemoveEntity(this);

	}

protected:

	void OnDispose()
	{
		Destroy();
	}

private:

};