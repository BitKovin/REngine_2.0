#include <Entity.hpp>
#include <StaticMesh.h>

class StaticMeshEntity : public Entity
{
public:
	
	StaticMesh* mesh;

	void FromData(EntityData data)
	{
		Entity::FromData(data);

		mesh = new StaticMesh(this);

		mesh->LoadFromFile("GameData/" + data.GetPropertyString("path"));

		vec3 angles = data.GetPropertyVectorRotation("angles");

		mesh->Rotation = EntityData::ConvertRotation(angles);
		mesh->Position = Position;

		mesh->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/" + data.GetPropertyString("texture"));

		Drawables.push_back(mesh);
	}

private:

};

REGISTER_ENTITY(StaticMeshEntity, "static_mesh")