#include <Characters/CombatCharacter.h>

// Registered body types. A new enemy = character + moveset JSON + a class like these (COMBAT_DESIGN.md §14).
// They can be placed in maps or spawned by the engine Spawner ("className": "enemy_grunt").

class SlasherHero : public CombatCharacter
{
public:
	SlasherHero() : CombatCharacter("GameData/combat/characters/hero.character.json") { ClassName = "slasher_hero"; Tags = { "player" }; }
};
REGISTER_ENTITY(SlasherHero, "slasher_hero")

class EnemyGrunt : public CombatCharacter
{
public:
	EnemyGrunt() : CombatCharacter("GameData/combat/characters/grunt.character.json") { ClassName = "enemy_grunt"; }
};
REGISTER_ENTITY(EnemyGrunt, "enemy_grunt")

class EnemyGunner : public CombatCharacter
{
public:
	EnemyGunner() : CombatCharacter("GameData/combat/characters/gunner.character.json") { ClassName = "enemy_gunner"; }
};
REGISTER_ENTITY(EnemyGunner, "enemy_gunner")

class EnemyBrute : public CombatCharacter
{
public:
	EnemyBrute() : CombatCharacter("GameData/combat/characters/brute.character.json") { ClassName = "enemy_brute"; }
};
REGISTER_ENTITY(EnemyBrute, "enemy_brute")

class TrainingDummy : public CombatCharacter
{
public:
	TrainingDummy() : CombatCharacter("GameData/combat/characters/dummy.character.json") { ClassName = "training_dummy"; }
};
REGISTER_ENTITY(TrainingDummy, "training_dummy")

// Generic body: the character file comes from the map entity's "def" key.
REGISTER_ENTITY(CombatCharacter, "combat_character")
