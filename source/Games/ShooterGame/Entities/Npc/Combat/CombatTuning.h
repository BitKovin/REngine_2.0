#pragma once

// Every number the combat AI plays by, in one place (COMBAT_AI_DESIGN.md §22).
// Read with CombatTuning::Get(); change with CombatTuning::Set() (for example
// per difficulty), back to the defaults with CombatTuning::Reset().
//
// Set() between frames (menu, level load), not while the NPCs are updating:
// the brains read the values from worker threads. A few values are read once
// when an NPC is created (tactic weights); those take effect for NPCs spawned
// after the change.

// ── NPC body ─────────────────────────────────────────────────────────────────
struct CombatMovementTuning
{
	// Strafing is a brisk walk (the walk clips run up to 3 m/s) and stays under
	// rangedMaxFireSpeed, so an NPC shoots while it walks and only a sprint stops it.
	float strafeSpeed = 2.8f;
	float runSpeed = 5.0f;
	float crouchWalkSpeed = 2.2f;
	float rushSpeed = 5.5f;
	float meleeFootworkSpeed = 3.0f;  // stepping around inside a melee exchange
};

struct CombatShotTuning
{
	float bulletSpeed = 50.0f;
	float bulletDamage = 5.0f;
	float maxFireSpeed = 3.3f;        // m/s; faster than this the NPC doesn't shoot
	float suppressMaxBlockDistance = 2.5f; // a suppressing shot is held back when the world blocks it this far short
};

// A gunner caught up close kicks the attacker away to make room (§6.11).
struct CombatShoveTuning
{
	float range = 2.2f;
	float damage = 5.0f;
	float knockback = 7.0f;           // m/s away from the NPC
	float lift = 2.0f;
	double cooldown = 3.0;
};

// Accuracy, magazine and fairness of one gun NPC (§9).
struct CombatFireTuning
{
	int   magazineSize = 8;
	float reloadTime = 1.5f;          // seconds with the weapon lowered
	float idleRefillDelay = 4.0f;     // seconds out of combat before the magazine refills silently

	float playerAccuracy = 0.8f;      // base hit chance against the player
	float npcAccuracy = 0.5f;         // base hit chance against NPC targets
	float suppressedAccuracy = 0.1f;  // ceiling for shooters without an attack token

	float rampStart = 0.2f;           // fraction of base accuracy when line of sight begins
	float rampTime = 2.0f;            // seconds of continuous line of sight to reach full accuracy
	float damagedRampFactor = 0.5f;   // getting hit scales the ramp progress by this
	float losResetTime = 1.0f;        // line of sight lost for this long starts a new acquisition

	float movingFactor = 0.6f;        // shooter moving
	float offscreenFactor = 0.3f;     // shooter outside the player's view
	float offscreenRampScale = 2.0f;  // ... also ramps this much slower
	float falloffNear = 10.0f;        // full accuracy up to this distance
	float falloffFar = 25.0f;         // accuracy bottoms out here
	float falloffMin = 0.6f;          // factor at falloffFar and beyond

	float aimUpTime = 0.3f;           // pause before the first shot of a burst
	float burstGap = 1.0f;            // a longer pause between shots starts a new burst

	float missMin = 0.4f;             // metres a missed shot lands off the aim point
	float missMax = 1.2f;
	float hitSpread = 0.1f;           // metres of scatter on a hitting shot

	float holderAccuracyFactor = 1.25f; // an NPC holding an angle reacts faster and shoots better ...
	float holderAlertTime = 2.0f;       // ... for this many seconds after the target shows up

	// A shooter bids for an attack token only while it is out in the open to
	// shoot and for this long after.
	double tokenExposureGrace = 0.4;
};

// ── Brain ────────────────────────────────────────────────────────────────────
struct CombatBrainTuning
{
	// Suppression (§13.1): a near miss adds nearMissSuppression, a hit hitSuppression;
	// it decays and ducks the NPC above duckSuppression.
	float nearMissSuppression = 0.35f;
	float hitSuppression = 0.5f;
	float maxSuppression = 1.2f;
	float suppressionDecayPerSecond = 0.5f;
	float duckSuppression = 0.6f;

	// Flinch (§13.2)
	float flinchDamage = 15.0f;       // hits at least this big always flinch
	float smallHitFlinchChance = 0.25f;
	double flinchCooldown = 1.0;
	float flinchAnimationTime = 0.35f;
	double flinchDuckTime = 0.6;      // how long after a flinch tactics treat it as fresh

	// Without a squad belief only a short loss of sight is held through.
	float briefLossGraceTime = 1.5f;

	float meleeSlotRange = 10.0f;     // melee NPCs ask for a slot on the player from this far

	// Body rules (§11.3)
	float runWhenIdleDistance = 2.0f;  // can't shoot and more than this to go: run

	// Watching the corner (§11.3): when a wall stands between the NPC and where
	// it believes the target is, it faces and aims at the first bend of its path
	// there (where the target would come round), not through the wall.
	double watchUpdateInterval = 0.3;
	float watchOcclusionMargin = 3.0f; // the line blocked further than this from the believed point: a wall in between
	float watchCornerDistance = 2.0f;  // the first path point at least this far away
	float watchEyeHeight = 1.4f;       // above the path point
	int watchMaxCorners = 6;           // path points checked for the farthest one in view
	float watchMaxAngleDegrees = 100.0f; // a corner further round than this from the target: watch the target's direction instead

	// Aiming without sight of the target (watching a corner, a doorway, the
	// estimate) keeps the gun level-ish: pitch limited to this, and level for
	// points this close (a steep angle to a point at its feet reads as aiming at
	// the sky or the ground).
	float blindAimMaxPitch = 20.0f;

	// Heard its target (a shot) without seeing it: turns to the sound at once,
	// for this long, whatever its tactic faces.
	double heardTurnTime = 1.0;
	float closeAimFlatDistance = 1.5f;
	float pinnedCrouchMoveDistance = 1.0f; // pinned in the open and (nearly) standing still: crouch

	// Strafing is the fallback for when no cover is found; the suppressor takes
	// cover first when it has some on the way. Read when an NPC is created.
	float strafeWeight = 0.5f;
	float suppressLastKnownWeight = 0.9f;
};

// Hybrid weapon choice (§7.3): the gun, except very close and only when safe.
struct CombatHybridTuning
{
	float swordDistance = 3.0f;       // against a gun target, and only if it can't punish the switch
	float swordLeaveDistance = 4.5f;  // the sword stays out while the target is this close
	float corneredDistance = 2.5f;    // against a target without a gun
};

// ── Squad (CombatCoordinator) ────────────────────────────────────────────────
struct CombatTokenTuning
{
	int playerAttackTokens = 2;       // NPCs at once that aim to hit the player
	double minTokenHoldTime = 1.0;
	float playerDamageBudget = 18.0f; // damage per second all NPCs together may deal the player

	int meleeSlotsPerTarget = 2;
	int meleeSlotsVsGunTarget = 3;    // a gun target can't block them
	double minSlotHoldTime = 2.0;

	float coverSpacing = 1.2f;        // spots closer than this count as one (one NPC per spot)
};

// Squad belief about a target that ducked out of sight (§5).
struct CombatBeliefTuning
{
	float exposedBlindTime = 0.5f;    // NPCs out shooting have looked this long without seeing it: hidden
	double exposedNowWindow = 0.25;   // reported exposure this recent counts as "out right now"
	double maxBlindTime = 4.0;        // nobody has seen it this long: hidden
	float estimateLead = 0.4f;        // seconds of the target's last velocity added to the estimate
	double concealedMin = 6.0;        // seconds before giving up on a hiding target
	double concealedMax = 10.0;
	float estimateGrowth = 0.5f;      // m/s the estimate radius grows without evidence
	float maxEstimateRadius = 6.0f;
	float emptyConfirmTime = 1.0f;    // a clear look at the estimate this long means it is gone
	double minConcealedTime = 3.0;    // the spot can't be declared empty sooner than this
	float emptyCheckViewCos = 0.5f;   // the looker must face the spot (60 degrees either side)
	float emptyCheckDistance = 15.0f;
	double lostMaxTime = 12.0;        // backstop for the check-the-spot phase

	// Combat awareness (§5.7): a squad in a fight keeps track of a target moving
	// about nearby by ear, so it can't vanish just by walking round a wall.
	// Sneaking off slowly, far enough from all of them, still loses them.
	double trackInterval = 1.0;       // how often the estimate is refreshed this way
	float trackCloseDistance = 8.0f;  // within this of any squad member: always tracked
	float trackHearingDistance = 25.0f; // within this: tracked while it moves faster than trackNoisySpeed
	float trackNoisySpeed = 3.0f;     // m/s
	float trackError = 1.5f;          // metres the tracked estimate is off by

	// A sound from a target the NPC fights (a shot) further than this from where
	// it last had a line of sight on it moves that last-sight position there, so
	// it doesn't act lost about a target that just gave itself away.
	float heardUpdateDistance = 2.0f;

	// The target turns up (heard, or tracked) this far from where the squad last
	// placed it: it has moved, so the squad goes to find it (§5.7). For seekTime
	// the holders don't hold the old angles: each moves to a spot that sees the
	// new position, closer if need be.
	float relocateDistance = 5.0f;
	double seekTime = 6.0;
};

// The squad plan (§8.6, §8.7).
struct CombatSquadTuning
{
	double planInterval = 0.25;
	float roleMaxDistance = 25.0f;    // roles go to NPCs within this of the target

	// Settling (§8.8): a new fight, the squad takes positions before anyone goes
	// round, and only goes round a target that holds its ground.
	double settleMinTime = 1.5;       // seconds into a fight before the first flank
	double settleMaxTime = 5.0;       // ... and the squad counts as settled after this even if not all in cover
	float settledCoverFraction = 0.5f; // or once this share of the base of fire is in cover
	float anchorRadius = 4.0f;        // the target moving less than this from where it was is holding its ground
	double holdTime = 2.0;            // ... for this long
	float retreatMargin = 1.0f;       // a move that takes it this much further from the squad is running away

	// Flanking
	float maxFlankFraction = 0.5f;    // at most this share of the gun NPCs go round (at least one from two up)
	int maxFlankers = 3;
	double flankCooldown = 2.0;       // after a flank ends, before the next one
	double flankTimeout = 15.0;       // an open-ground flank that hasn't got there by then is given up
	double laneFlankTimeout = 25.0;   // a lane flank likewise
	double flankerDelay = 0.5;        // target hidden: seconds before somebody goes round
	float flankMinStartDistance = 4.0f; // too close to bother going round
	float flankDistanceWeight = 0.03f;  // open-ground flanker choice: radians of "already round" per metre
	double flankRestTime = 8.0;       // an NPC back from a flank isn't sent on another for this long

	// Lanes and the breach (§8.7)
	float minFlankLaneAngle = 60.0f;  // degrees round from the front lane
	float flankLaneCostWeight = 0.5f; // lane choice: degrees of "round" per metre of route to its stack
	float maxFlankRoute = 60.0f;      // metres of path to a lane's stack spot
	int flankCandidates = 3;          // nearest NPCs whose routes are checked per lane
	float flankRouteClearance = 2.5f; // routes may not brush past the target
	float stackSpacing = 1.0f;        // metres between NPCs stacked on one lane
	double stackWaitMax = 3.0;        // first one stacked: breach after this even if others aren't
	double stackWaitVisible = 1.0;    // ... while the base of fire is already fighting the target
	double breachTimeout = 8.0;       // first lane order: breach after this whatever happens
	double assaultTime = 3.0;         // the base of fire is all out shooting while the stack goes in
	int frontLaneSwitchVotes = 2;     // another lane must have this many more gunners to become the front
	float laneKeepDistance = 3.0f;    // a lane order keeps following a lane whose entry moved this little
	float closeQuartersDistance = 8.0f; // pushers this close to a hiding target work its corner instead of a lane

	// Hidden target: the main force keeps the pressure on from where it is
	// (suppressing, holding the ways out) and doesn't walk onto the target; only
	// the flankers move in, or this many pushers when nobody is flanking.
	int pushersWithoutFlank = 0;
	int maxSuppressors = 3;
	double suppressMinTime = 5.0;     // seconds the spot is suppressed
	double suppressMaxTime = 8.0;

	// After a flank (§8.7): locked into attacking from the angle it won.
	double postFlankPressTime = 5.0;
	double flankHoldTime = 15.0;      // then cover only at its distance to the target or closer
};

// ── Cover (§10) ──────────────────────────────────────────────────────────────
struct CombatCoverTuning
{
	double searchInterval = 0.8;
	double searchJitter = 0.6;
	double retryInterval = 1.2;       // after a search found nothing
	double validateInterval = 0.5;    // how often the spot is re-checked against the target
	float arriveDistance = 0.8f;      // metres from the spot that count as being in it
	float minThreatDistance = 5.0f;   // cover is pointless closer than this
	float rangedMaxFraction = 0.85f;  // of the attack range
	float maxTravelDistance = 8.0f;   // straight-line metres an NPC goes for cover
	float maxRetreat = 3.0f;          // how much further from the threat the spot may be

	double minStayTime = 5.0;         // then it moves to other cover ...
	double stayJitter = 4.0;
	float relocateBlindTime = 1.0f;   // ... but only once it hasn't seen the target for this long
	float avoidPreviousRadius = 2.5f;

	// The target reloads: move up to cover at least advanceDistance closer.
	float advanceDistance = 3.0f;
	float advanceTravelDistance = 10.0f;
	double advanceMinStay = 0.8;
	double advanceCooldown = 6.0;     // an NPC moves up at most this often (reloads, pursuit)
	bool advanceOnReload = false;     // the main force moves up while the target reloads
	// A retreating target is followed only by NPCs it has left out of their
	// fighting range (this fraction of the attack range).
	float pursuitRangeFraction = 0.85f;

	// Spot scoring
	float movePenalty = 0.10f;        // per metre to walk
	float rangePenalty = 0.12f;       // per metre from the preferred distance to the threat
	float lowCoverBonus = 0.3f;       // can shoot without moving
	float clusterPenalty = 1.5f;      // squadmates close by
	float laneLinePenalty = 1.5f;     // in the line of fire out of the squad's entry
	float laneLineWidth = 1.5f;
};

// Hide / peek rhythm of a cover fight (seconds).
struct CombatCoverRhythm
{
	float hiddenMin, hiddenMax;
	float exposedMin, exposedMax;
};

struct CombatCoverFightTuning
{
	CombatCoverRhythm normal{ 0.5f, 1.0f, 1.8f, 3.2f };
	CombatCoverRhythm lowCover{ 0.8f, 1.5f, 1.5f, 2.5f };    // ducking reads as ducking
	CombatCoverRhythm patient{ 1.2f, 2.2f, 1.0f, 1.8f };     // base of fire while a flank is under way
	CombatCoverRhythm suppress{ 0.4f, 0.8f, 2.0f, 3.5f };    // suppressing a hidden target's spot

	float duckExtraHideTime = 0.5f;
	float returnFireDelay = 0.3f;     // a hidden NPC comes out this fast when nobody is shooting ...
	float patientReturnFireDelay = 1.0f; // ... or this fast while a flank is under way
	float braveSuppression = 1.0f;    // below this a lone shooter comes out anyway
	float blindPeekLimit = 1.2f;      // seconds of peeking without seeing the target: the spot is useless
};

// ── Approach map (§8.7) ──────────────────────────────────────────────────────
struct CombatApproachTuning
{
	float radius = 35.0f;             // metres of navmesh around the target
	float maxVisibleCost = 30.0f;     // nodes further by path aren't traced
	float bodyHeight = 1.4f;          // above the navmesh: what the target would see of an NPC
	float laneAngleDegrees = 25.0f;   // crossings this close in direction are one lane
	float laneMergeDistance = 3.0f;   // ... or this close to each other
	int minHiddenNodes = 3;           // a sliver of shadow behind a crate is not a way in
	float stackBehind = 2.0f;         // metres behind the entry the stack spot sits at most
	double rebuildInterval = 1.5;
	float rebuildMoveDistance = 3.0f;
};

// ── Tactics ──────────────────────────────────────────────────────────────────
struct CombatBoundingTuning
{
	double sprintMin = 1.5, sprintMax = 2.5;
	double stopMin = 0.8, stopMax = 1.2;
	float kneelChance = 0.5f;
};

// Fighting a gun target in the open, as the main force (§6.1): it fights from
// the range it is at and only closes in when it can't fight from there.
struct CombatStrafeTuning
{
	float engageFraction = 0.85f;     // of the attack range: further than this it closes in
	float advanceSlack = 2.0f;        // closes in to the engage range - this
	float boundSlack = 6.0f;          // further than the engage range + this: bound in
	float maxCloserPerStep = 0.5f;    // a sidestep may bring it at most this much closer
	bool pressInOnReload = false;     // steps in while the target reloads
	float spreadOffset = 4.0f;        // metres to either side each NPC advances on
	float minDistance = 5.0f;         // sidesteps never bring it closer than this
	float stepLength = 3.5f;
	double stepInterval = 1.4;        // a new step at the latest after this
	double stepJitter = 1.0;
};

struct CombatKiteTuning
{
	float kiteMinDistance = 7.0f;     // closer: back off
	float panicDistance = 3.5f;       // closer: turn and run
	float retreatStep = 5.0f;
	double retreatSearchInterval = 0.4;
	float maxRetreatStepHeight = 1.5f;
	float strafeMinDistance = 9.0f;   // range band held while out of reach
	float strafeMaxDistance = 14.0f;
	float strafeStep = 3.5f;
	double strafeInterval = 1.8;
	double strafeJitter = 1.4;
	float farSlack = 3.0f;            // beyond the band by this: bound in
	float spreadOffset = 3.0f;
};

struct CombatRegainSightTuning
{
	float arriveDistance = 0.9f;
	float creepDistance = 7.0f;       // metres from a hiding target where the NPC starts creeping
	double searchRetry = 0.8;
	double noSightRetryTime = 1.5;    // seconds at a spot without the target before trying another
	float reacquireDelay = 1.2f;      // seconds without sight before an NPC moves for it
	float reacquireMaxCloser = 2.0f;  // the new spot may be at most this much closer to the target
	float retargetDistance = 4.0f;    // the target moved this far from where the spot was picked for
	int maxFailedSearches = 3;
	float fallbackPushDistance = 4.0f;
	float flankedAngleDegrees = 50.0f; // round from the squad axis that already counts as flanked

	// Lanes
	float stackArriveDistance = 1.0f;
	float stackApproachDistance = 3.0f; // the last stretch to the stack is crept
	float stackLeftDistance = 2.5f;
	float breachDepth = 1.0f;         // metres past the entry: into the doorway
	float breachDoneDistance = 3.5f;
	double breachTime = 4.0;
	float entryAimHeight = 1.2f;
	float spottedBreachDistance = 10.0f; // spotted this close to its entry: goes in at once
	// Stacked, but the target is no longer through its entry (off to the side by
	// more than this, or nearer than the entry): the stack is pointless, fight it.
	float stackStaleAngleDegrees = 75.0f;
	float stallSpeed = 0.3f;
	float stallTime = 0.5f;

	// Close quarters ("slicing the pie")
	float sliceStep = 1.5f;
	float sliceMinDistance = 3.0f;
	double sliceInterval = 1.0;
};

// Locked attack after a flank: no cover, keep a line of sight (§8.7).
struct CombatPressTuning
{
	float stepLength = 2.0f;          // sidesteps while it sees the target
	float minDistance = 4.0f;
	double stepInterval = 1.2;
	float closeInDistance = 4.0f;     // without sight: closes on the believed position to this
};

// Holding an angle on a hidden target (§8.6): a holder with no line on the
// spot moves a little to get one.
struct CombatHoldTuning
{
	double lineSearchInterval = 1.0;
	float lineMargin = 2.5f;          // the line blocked this close to the point still counts (its cover)
	float minDistance = 4.0f;
	float maxPath = 12.0f;            // metres it walks for a line

	// Out of fighting range of the believed position (cover.pursuitRangeFraction
	// of the attack range; the target moved, or gave itself away somewhere
	// else): it moves up to a spot inside that range that sees it.
	float closeInBandFraction = 0.6f; // the spot is between this share of the fighting range and all of it
	float closeInMaxPath = 40.0f;
};

struct CombatSearchTuning
{
	double timeout = 9.0;             // seconds since the target was declared lost
	float checkDistance = 1.5f;       // metres from the estimate that count as checked
	double lookAroundTime = 1.5;
	double watcherWaitTime = 3.0;
	double othersWaitTime = 1.0;
	float laneReachedDistance = 1.5f;
};

struct CombatMeleeTacticsTuning
{
	// Advance on a hiding target
	float standOffDistance = 3.5f;
	float standOffSlack = 0.8f;
	float advanceWalkDistance = 8.0f;
	double pauseTime = 0.8;
	double pauseJitter = 0.8;

	// Rush an open target
	float rushMinDistance = 3.0f;
	float rushMaxDistance = 12.0f;
	float rushSideOffset = 0.8f;

	// Waiting for a melee slot
	float ringRadius = 5.0f;
	double ringUpdateInterval = 1.5;
	float ringRunDistance = 6.0f;

	float takeCoverCrouchDistance = 2.0f; // low cover: crouch-walk the last stretch
};

struct CombatTuningValues
{
	CombatMovementTuning movement;
	CombatShotTuning shot;
	CombatShoveTuning shove;
	CombatFireTuning fire;
	CombatBrainTuning brain;
	CombatHybridTuning hybrid;
	CombatTokenTuning tokens;
	CombatBeliefTuning belief;
	CombatSquadTuning squad;
	CombatCoverTuning cover;
	CombatCoverFightTuning coverFight;
	CombatApproachTuning approach;
	CombatBoundingTuning bounding;
	CombatStrafeTuning strafe;
	CombatKiteTuning kite;
	CombatRegainSightTuning regainSight;
	CombatPressTuning press;
	CombatHoldTuning hold;
	CombatSearchTuning search;
	CombatMeleeTacticsTuning melee;
};

class CombatTuning
{
public:
	static const CombatTuningValues& Get() { return values; }
	static void Set(const CombatTuningValues& newValues) { values = newValues; }
	static void Reset() { values = CombatTuningValues(); }

private:
	static inline CombatTuningValues values;
};
