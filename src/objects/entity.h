/******************************************************************************
* Empty Clip
* Copyright (C) 2015  Alan Witkowski
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/
#pragma once

// Libraries
#include <objects/object.h>
#include <objects/templates.h>
#include <list>

// Forward Declarations
struct _ParticleTemplate;
class _AudioSource;
class _Map;
class _Animation;

// Used to determine what direction an entity wants to go
enum MoveType {
	MOVE_NONE,
	MOVE_FORWARD,
	MOVE_LEFT,
	MOVE_RIGHT,
	MOVE_BACKWARD,
	MOVE_FORWARDRIGHT,
	MOVE_FORWARDLEFT,
	MOVE_BACKWARDRIGHT,
	MOVE_BACKWARDLEFT,
	MOVE_GOAL,
	MOVE_DIRECTION
};

// Used for animation
enum ActionType {
	ACTION_IDLE,
	ACTION_MOVING,
	ACTION_STARTMELEE,
	ACTION_MELEE,
	ACTION_STARTSHOOT,
	ACTION_SHOOT,
	ACTION_STARTDEATH,
	ACTION_DYING
};

// Indices into the animation array
enum EntityAnimationTypes {
	ENTITY_ANIMATIONWALKING,
	ENTITY_ANIMATIONATTACK,
	ENTITY_ANIMATIONDYING,
};

// Classes
class _Entity : public _Object {

	public:

		_Entity();
		~_Entity();

		virtual void Update(double FrameTime) override;
		virtual void Render(double BlendFactor) override;

		void Move();

		bool StartAttack();
		float GenerateShotDirection();
		void ResetAttackAllowed() { AttackAllowed = false; FireTimer = 0; }
		bool IsMeleeAttacking() const { return Action == ACTION_MELEE || Action == ACTION_STARTMELEE; }

		virtual bool CanAttack() const { return AttackAllowed && !IsMeleeAttacking() && !IsDying(); }
		virtual void ReduceAmmo() { }
		virtual bool HasAmmo() const { return true; }

		virtual int64_t ExperienceGiven() const { return 0; }
		virtual void UpdateExperience(int64_t ExperienceGained) { }
		virtual void UpdateKillCount(int Value) { }
		virtual void UpdateAnimation(double FrameTime);

		void UpdateMaxHealth(int Adjust);
		void UpdateHealth(int Adjust);
		virtual void UpdateSpeed(float Factor) {}
		int GenerateDamage(int DamageBlock, float DamageResist);
		bool IsDying() const { return Action == ACTION_DYING || Action == ACTION_STARTDEATH; }
		bool IsDead() const { return Action == ACTION_DYING && !Active; }

		void SetAttackMade(bool Value) { AttackMade = Value; }
		void SetAction(const ActionType Type) { Action = Type; }
		void SetMoveState(MoveType State);
		void SetAttackRequested(bool Attack) { AttackRequested = Attack; }
		void SetSample(int Type, const std::string &Sample) { Samples[Type] = Sample; };

		ActionType GetAction() const { return Action; }
		bool GetAttackMade() const { return AttackMade; }
		MoveType GetMoveState() const { return MoveState; }
		int GetDamageBlock() const { return DamageBlock; }
		float GetDamageResist() const { return DamageResist; }
		int GetMaxHealth() const { return MaxHealth; }
		int GetHealth() const { return CurrentHealth; }
		float GetHealthPercentage() const { return (float)CurrentHealth / MaxHealth; }
		float GetStaminaPercentage() const { return Stamina / MaxStamina; }
		bool GetAttackRequested() const { return AttackRequested; }
		int GetWeaponType() const { return WeaponType; }
		Vector2 GetWeaponOffset(int Type) const { return WeaponParticleOffset[Type]; }
		float GetWeaponRange() const { return AttackRange; }
		float GetCurrentAccuracy() const { return CurrentAccuracy; }
		float GetMaxAccuracy() const { return MaxAccuracy; }
		int GetMinDamage() const { return MinDamage; }
		int GetMaxDamage() const { return MaxDamage; }
		int GetBulletsShot() const { return BulletsShot; }
		int GetLevel() const { return Level; }
		bool GetTired() const { return Tired; }
		virtual Vector2 GetGoal() const;
		virtual int64_t GetExperienceGiven() const { return 0; }
		virtual std::string GetItemGroupIdentifier() const { return ""; }
		virtual const _ParticleTemplate *GetWeaponParticle(int Index) const { return nullptr; }

		void AddGoal(const Vector2 &Goal) { Goals.push_front(Goal); }
		void PopGoal() { if(!Goals.empty()) Goals.pop_front(); }

		void SetAnimation(_Animation *Animation) { this->Animation = Animation; }
		_Animation *GetAnimation() const { return Animation; }

		virtual const std::string &GetSample(int Type) const { return Samples[Type]; };
		Vector2 WallInPath(const Vector2 &Delta) const;

		void SetChangedPosition(bool Value) { PositionChanged = Value; }
		void StartTriggerDownAudio();
		void StopAudio();

	protected:

		virtual void IncurDeathPenalty() { }
		virtual void SetLegAnimationPlayMode(int Type) { }
		virtual void SetAnimationPlaybackSpeedFactor() { }
		void UpdateRecoil();

		// Graphics
		_Animation *Animation;
		Vector2 WeaponParticleOffset[WEAPON_TYPES];

		// Movement
		MoveType MoveState;
		double MoveSoundTimer, MoveSoundDelay;
		float MovementSpeed, MovementModifier;
		Vector2 MoveDirection;
		bool PositionChanged;
		float Stamina, MaxStamina, StaminaRegenModifier;
		bool Tired;

		std::list<Vector2> Goals;

		// States
		ActionType Action;
		int WalkingAnimation, MeleeAnimation, ShootingOnehandAnimation, ShootingTwohandAnimation, DyingAnimation;

		// Stats
		int Level, CurrentHealth, MaxHealth, DamageBlock;
		float DamageResist;

		// Attacking attributes
		float CurrentAccuracy, MinAccuracy, MaxAccuracy, AccuracyModifier, Recoil, RecoilRegen, AttackRange;
		double FireTimer, FirePeriod;
		int MinDamage, MaxDamage, BulletsShot, WeaponType;
		bool AttackRequested, AttackAllowed, AttackMade;
		_AudioSource *TriggerDownAudio;

		std::string Samples[SAMPLE_TYPES];
};
