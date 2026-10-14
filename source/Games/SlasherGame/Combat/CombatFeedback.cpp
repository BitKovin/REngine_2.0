#include <Characters/CombatCharacter.h>
#include <Combat/CombatEvents.h>
#include <Control/PlayerController.h>

#include <Entities/SoundPlayer.h>
#include <Particle/GlobalParticleSystem.hpp>

// Feedback for combat notifications (COMBAT_DESIGN.md §9.4). Rules never depend on these handlers. Events with
// unresolvable handles (including the startup registration events) are ignored.
//
//   hit effect  the victim's hitEffect (blood...) at the contact, spraying along the blow, scaled by the hit's
//               impactScale; a hit naming its own particle / sound (projectiles) overrides it
//   camera      only for the player's body: a hit it lands kicks the view by the hit's `camera` weight along the blow
//               (light hits weigh 0); being hit, kills, poise breaks and finishers add the player.json feedback.
//               CameraImpact keeps it comfortable (speed limits, the strongest rumble wins)

namespace
{
	const std::string NoEffect;

	struct PlayerView
	{
		PlayerController* controller = nullptr;
		EntityHandle body;
	};

	PlayerView GetPlayerView()
	{
		PlayerView view;
		view.controller = PlayerController::GetActive();
		if (view.controller != nullptr)
			if (CombatCharacter* body = view.controller->GetPossessedBody())
				view.body = EntityHandle::FromEntity(body);
		return view;
	}

	const HitEffectData* GetHitEffect(EntityHandle victim)
	{
		auto* body = dynamic_cast<CombatCharacter*>(ResolveAlive(victim));
		return body != nullptr && body->GetData() != nullptr ? &body->GetData()->hitEffect : nullptr;
	}
}

EVENT_HANDLER(PlayHitFeedback, HitLandedEvent)
{
	if (!e.victim.IsValid())
		return;

	// ---- Hit effect ----
	const HitEffectData* effect = GetHitEffect(e.victim);
	const bool ownParticle = !e.hitParticle.empty();
	const std::string& particle = ownParticle ? e.hitParticle : (effect ? effect->particle : NoEffect);
	const std::string& sound = !e.hitSound.empty() ? e.hitSound : (effect ? effect->sound : NoEffect);

	if (!sound.empty())
		SoundPlayer::PlayOneshot(sound, 2.0f, 1.0f, false, e.point);

	const float scale = (ownParticle || effect == nullptr ? 1.0f : effect->scale) * e.impactScale;
	if (!particle.empty() && scale > 0.0f)
	{
		const glm::vec3 spray = glm::dot(e.spray, e.spray) > 1e-6f ? e.spray : e.direction;
		GlobalParticleSystem::SpawnParticleAt(particle, e.point, MathHelper::FindLookAtRotation(glm::vec3(0.0f), spray), glm::vec3(scale));
	}

	// ---- Camera ----
	const PlayerView player = GetPlayerView();
	if (!player.body.IsValid())
		return;

	const PlayerFeedbackData& feedback = player.controller->GetFeedback();
	PlayerCamera& camera = player.controller->GetCamera();
	if (e.victim == player.body)
	{
		// Bullets only nudge: automatic fire must never turn into a lasting shake.
		if (e.source == DamageSource::Firearm)
		{
			camera.AddKick(e.direction, feedback.takenFirearmKick);
		}
		else
		{
			camera.AddKick(e.direction, feedback.takenKick + e.camera);
			camera.AddShake(feedback.takenShake);
		}
	}
	else if (e.attacker == player.body)
	{
		camera.AddKick(e.spray, e.camera);
		camera.AddShake(e.camera * feedback.dealtShake + (e.killed ? feedback.killShake : 0.0f));
	}
}

EVENT_HANDLER(PlayPoiseBrokenFeedback, PoiseBrokenEvent)
{
	if (!e.victim.IsValid())
		return;
	const PlayerView player = GetPlayerView();
	if (player.body.IsValid() && e.attacker == player.body)
		player.controller->GetCamera().AddShake(player.controller->GetFeedback().poiseBreakShake);
}

EVENT_HANDLER(PlayFinisherFeedback, FinisherEvent)
{
	if (!e.attacker.IsValid() || !e.completed)
		return;
	const PlayerView player = GetPlayerView();
	if (player.body.IsValid() && (e.attacker == player.body || e.victim == player.body))
		player.controller->GetCamera().AddShake(player.controller->GetFeedback().finisherShake);
}
