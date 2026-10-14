#include <Characters/CombatCharacter.h>
#include <Control/PlayerController.h>
#include <Data/CombatData.h>

#include <Entity.h>
#include <LevelTraversalSystem.h>
#include <Logger.hpp>

// Map entity "info_player_start": spawns the PlayerController and the hero body (type from player.json) once
// the level has loaded. Both types are preloaded in LoadAssets, so spawning does no file access.
class SlasherPlayerStart : public Entity
{
public:
	SlasherPlayerStart()
	{
		ClassName = "info_player_start";
	}

	void FromData(EntityData data) override
	{
		Entity::FromData(data);
		yaw = data.GetPropertyFloat("angle") + 90.0f;
	}

	void PostLoadStart() override
	{
		// Level traversal picks a named spawn point; a level opened directly uses the unnamed start.
		if (LevelTraversalSystem::DesiredSpawnPointName != Name)
			return;
		if (PlayerController::GetActive() != nullptr)
			return;

		std::shared_ptr<const PlayerConfigData> config = CombatData::LoadPlayerConfig();
		if (!config)
		{
			Logger::Error("[PlayerStart] cannot spawn the player: %s failed to load", CombatData::PlayerConfigPath);
			return;
		}

		auto* controller = new PlayerController();
		controller->Position = Position;
		Level::Current->AddEntity(controller);
		controller->Start();

		Entity* heroEntity = LevelObjectFactory::instance().create(config->heroEntity);
		auto* hero = dynamic_cast<CombatCharacter*>(heroEntity);
		if (hero == nullptr)
		{
			delete heroEntity;
			Logger::Error("[PlayerStart] hero entity '%s' is not a CombatCharacter", config->heroEntity.c_str());
			return;
		}

		hero->Position = Position;
		hero->Rotation.y = yaw;
		Level::Current->AddEntity(hero);
		hero->Start();
		controller->SetHero(hero);
	}

protected:
	void LoadAssets() override
	{
		CombatData::LoadScope scope;
		if (std::shared_ptr<const PlayerConfigData> config = CombatData::LoadPlayerConfig())
			PreloadEntityType(config->heroEntity);
		PreloadEntityType("player_controller");
	}

private:
	float yaw = 0.0f;
};

REGISTER_ENTITY(SlasherPlayerStart, "info_player_start")
