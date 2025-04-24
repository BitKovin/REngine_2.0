#include <Entity.hpp>
#include <SkeletalMesh.hpp>
#include <Physics.h>

#include <imgui/imgui.h>

class SkeletalEditorEntity : public Entity
{
public:
	SkeletalMesh* mesh = new SkeletalMesh();

	string filePath = "GameData/dog.glb";

	int selectedHitbox = -1;

	SkeletalEditorEntity()
	{
		mesh->LoadFromFile("GameData/dog.glb");
		mesh->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/cat.png");
		Drawables.push_back(mesh);
		mesh->PlayAnimation("run", true);

		Physics::DebugDraw = true;
	}

	void Update()
	{
		mesh->CreateHitboxes(this, mesh->metaData);
		mesh->Update();
		mesh->UpdateHitboxes();
	}

	// Callback to resize the std::string buffer when editing
	static int InputTextCallback(ImGuiInputTextCallbackData* data)
	{
		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
		{
			std::string* str = (std::string*)data->UserData;
			str->resize(data->BufTextLen);
			data->Buf = (char*)str->c_str();
		}
		return 0;
	}

	void LoadModel()
	{
		mesh->ClearHitboxes();
		mesh->LoadFromFile(filePath);
	}

	vector<const char*> GetHitboxNames()
	{
		vector<const char*> hitboxes;
		hitboxes.reserve(mesh->metaData.hitboxes.size());
		for (const auto& hitbox : mesh->metaData.hitboxes)  // ← note const auto&
			hitboxes.push_back(hitbox.boneName.c_str());
		return hitboxes;
	}

	void HitboxWindow(HitboxData* hitbox)
	{
		ImGui::Begin("hitbox editor");

		static char buffer[256];  // Fixed-size buffer (adjust size as needed)
		strncpy_s(buffer, hitbox->boneName.c_str(), sizeof(buffer));
		buffer[sizeof(buffer) - 1] = '\0';  // Ensure null termination

		if (ImGui::InputText("bone name", buffer, sizeof(buffer)))
		{
			hitbox->boneName = std::string(buffer);  // Update std::string only if user edited the input
		}

		ImGui::DragFloat3("position", &hitbox->position.x, 0.1f);
		ImGui::DragFloat3("rotation", &hitbox->rotation.x, 0.1f);
		ImGui::DragFloat3("size", &hitbox->size.x, 0.1f, 0.001);

		ImGui::End();
	}

	void UpdateDebugUI()
	{
		ImGui::Begin("hitbox editor");

		static char buffer[256];  // Fixed-size buffer (adjust size as needed)
		strncpy_s(buffer, filePath.c_str(), sizeof(buffer));
		buffer[sizeof(buffer) - 1] = '\0';  // Ensure null termination

		if (ImGui::InputText("model path", buffer, sizeof(buffer)))
		{
			filePath = std::string(buffer);  // Update std::string only if user edited the input
		}

		if (ImGui::Button("load model"))
		{
			LoadModel();
		}

		if (ImGui::Button("save model"))
		{
			mesh->SaveMetaToFile();
		}

		ImGui::Spacing();
		ImGui::Spacing();

		vector<const char*> names = GetHitboxNames();

		ImGui::ListBox("hitboxes:", &selectedHitbox, names.data(), names.size());


		if (ImGui::Button("add"))
		{
			mesh->metaData.hitboxes.push_back(HitboxData{"none"});
		}

		ImGui::SameLine();

		if (ImGui::Button("remove"))
		{
			if (selectedHitbox >= 0 && selectedHitbox < names.size())
			{
				mesh->metaData.hitboxes.erase(mesh->metaData.hitboxes.begin() + selectedHitbox);
			}
		}

		HitboxData* selectedHitboxRef = nullptr;

		if (selectedHitbox >= 0 && selectedHitbox < names.size())
		{
			selectedHitboxRef = &mesh->metaData.hitboxes[selectedHitbox];
		}

		ImGui::End();

		if (selectedHitboxRef)
		{
			HitboxWindow(selectedHitboxRef);
		}

	}

};

REGISTER_ENTITY(SkeletalEditorEntity, "skeletal_editor")