#pragma once

#include <Entity.h>
#include <imgui/imgui.h>

class Door : public Entity
{
public:

	StaticMesh* doorMesh = nullptr;
	StaticMesh* doorFrameMesh = nullptr;

	float DoorRotation = 0.0f;

	vec2 doorSize = vec2(1.3, 2.45);

	void FromData(EntityData data)
	{
		Entity::FromData(data);

		LoadAssetsIfNeeded();

		Rotation.y = data.GetPropertyFloat("angle");

		doorFrameMesh->Position = Position;
		doorFrameMesh->Rotation = Rotation;
		doorMesh->Position = Position;
		doorMesh->Rotation = Rotation;

	}

	void Start() override;
	
	void UpdateDebugUI() override
	{
		Entity::UpdateDebugUI();
		ImGui::Begin("Door Debug UI");
		ImGui::SliderFloat("Door Rotation", &DoorRotation, 0.0f, 90.0f);
		ImGui::End();
	}

	void UpdatePhysics() override
	{
	}

	void Update();

	void LoadAssets();

private:

};
