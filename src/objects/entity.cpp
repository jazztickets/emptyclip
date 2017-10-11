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
#include <objects/entity.h>
#include <graphics.h>
#include <audio.h>
#include <map.h>
#include <animation.h>
#include <objects/monster.h>
#include <random.h>
#include <constants.h>
#include <iostream>

// Constructor
_Entity::_Entity()
:	MoveState(MOVE_NONE),
	MoveSoundTimer(0),
	MoveSoundDelay(0),
	MovementSpeed(0),
	MovementModifier(1.0f),
	PositionChanged(false),
	Stamina(1),
	MaxStamina(1),
	StaminaRegenModifier(1.0f),
	Tired(false),
	Action(ACTION_IDLE),
	WalkingAnimation(ENTITY_ANIMATIONWALKING),
	MeleeAnimation(ENTITY_ANIMATIONATTACK),
	ShootingOnehandAnimation(ENTITY_ANIMATIONATTACK),
	ShootingTwohandAnimation(ENTITY_ANIMATIONATTACK),
	DyingAnimation(ENTITY_ANIMATIONDYING),
	Level(1),
	CurrentHealth(0),
	MaxHealth(0),
	DamageBlock(0),
	DamageResist(0.0f),
	CurrentAccuracy(0),
	MinAccuracy(0),
	MaxAccuracy(0),
	AccuracyModifier(1.0f),
	Recoil(0),
	RecoilRegen(0),
	AttackRange(0),
	FireTimer(0),
	FirePeriod(0),
	BulletsShot(1),
	AttackRequested(false),
	AttackAllowed(true),
	AttackMade(false),
	TriggerDownAudio(nullptr) {

	Animation = new _Animation();
	Map = nullptr;

	for(int i = 0; i < WEAPON_TYPES; i++)
		WeaponParticleOffset[i] = Vector2(0.0f, 0.0f);

	for(int i = 0; i < SAMPLE_TYPES; i++)
		Samples[i] = -1;

}

// Destructor
_Entity::~_Entity() {

	if(Animation)
		delete Animation;

	StopAudio();
}

// Generates a direction (in degrees) and updates the entity's accuracy
float _Entity::GenerateShotDirection() {
	float RandomOffset, NewDirection;

	// Generate the offset
	RandomOffset = Random.GenerateRange(-CurrentAccuracy * AccuracyModifier / 2.0f, CurrentAccuracy * AccuracyModifier / 2.0f);

	// Figure out new direction
	NewDirection = Rotation + RandomOffset;

	// Check bounds
	if(NewDirection < 0.0f)
		NewDirection += 360.0f;
	else if(NewDirection >= 360.0f)
		NewDirection -= 360.0f;

	// Update accuracy based on the weapon's recoil
	CurrentAccuracy += Recoil;
	if(CurrentAccuracy > MaxAccuracy)
		CurrentAccuracy = MaxAccuracy;

	return NewDirection;
}

// Generates damage after defenses
int _Entity::GenerateDamage(int DamageBlock, float DamageResist) {
	int Damage = Random.GenerateRange(MinDamage, MaxDamage);
	Damage -= (int)(Damage * DamageResist);
	Damage -= DamageBlock;

	// Cap the damage
	if(Damage < ENTITY_MINDAMAGEPOINTS)
		Damage = ENTITY_MINDAMAGEPOINTS;

	return Damage;
}

// Sets the movement state of the object
void _Entity::SetMoveState(MoveType State) {

	if(!IsMeleeAttacking())
		MoveState = State;
}

// Starts the attack animation
bool _Entity::StartAttack() {

	// Make sure object is allowed to attack
	if(!CanAttack())
		return false;

	// Check ammo
	if(!HasAmmo())
		return false;

	// Set animation
	if(GetWeaponType() == WEAPON_MELEE) {
		Action = ACTION_STARTMELEE;

		// Melee fire sound
		Audio.Play(new _AudioSource(Audio.GetBuffer(GetSample(SAMPLE_FIRE))), GetPosition());
	}
	else
		Action = ACTION_STARTSHOOT;

	ResetAttackAllowed();
	return true;
}

// Start playing the trigger down audio loop
void _Entity::StartTriggerDownAudio() {

	if(!TriggerDownAudio && GetSample(SAMPLE_TRIGGERDOWN) != "") {
		TriggerDownAudio = new _AudioSource(Audio.GetBuffer(GetSample(SAMPLE_TRIGGERDOWN)), false, true);
		TriggerDownAudio->Play();
	}
}

// Stop all audio associated with entity
void _Entity::StopAudio() {
	delete TriggerDownAudio;
	TriggerDownAudio = nullptr;
}

// Update the entity
void _Entity::Update(double FrameTime) {
	LastPosition = Position;

	MoveSoundTimer += FrameTime;
	FireTimer += FrameTime;
}

// Updates the animation
void _Entity::UpdateAnimation(double FrameTime) {

	// Check action
	switch(Action) {
		case ACTION_IDLE:
			if(!PositionChanged) {
				Animation->SetPlayMode(STOPPED);
				SetLegAnimationPlayMode(STOPPED);
			}
			else {
				Action = ACTION_MOVING;
				Animation->ChangeReel(WalkingAnimation);
				Animation->SetPlayMode(PLAYING);
				SetLegAnimationPlayMode(PLAYING);
				SetAnimationPlaybackSpeedFactor();
			}
		break;
		case ACTION_MOVING:
			if(!PositionChanged) {
				Action = ACTION_IDLE;
				Animation->SetPlayMode(STOPPED);
				SetLegAnimationPlayMode(STOPPED);
			}
		break;
		case ACTION_STARTMELEE:
			Action = ACTION_MELEE;
			Animation->ChangeReel(MeleeAnimation);
			if(Type == _Object::PLAYER)
				Animation->SetFramePeriod(FirePeriod);
			Animation->SetPlayMode(PLAYING);
			SetLegAnimationPlayMode(STOPPED);
			MoveState = MOVE_NONE;
		break;
		case ACTION_MELEE:
			if(Animation->GetPlayMode() == STOPPED) {
				Animation->ChangeReel(WalkingAnimation);
				SetAnimationPlaybackSpeedFactor();
				Action = ACTION_IDLE;
				AttackMade = true;
			}
		break;
		case ACTION_STARTSHOOT:
			Action = ACTION_SHOOT;
			if(GetWeaponType() == WEAPON_PISTOL)
				Animation->ChangeReel(ShootingOnehandAnimation);
			else
				Animation->ChangeReel(ShootingTwohandAnimation);

			Animation->SetPlayMode(PLAYING);
			AttackMade = true;
		break;
		case ACTION_SHOOT:
			if(Animation->GetPlayMode() == STOPPED) {
				Action = ACTION_IDLE;
				Animation->ChangeReel(WalkingAnimation);
				SetAnimationPlaybackSpeedFactor();
			}

			if(PositionChanged)
				SetLegAnimationPlayMode(PLAYING);
			else
				SetLegAnimationPlayMode(STOPPED);
		break;
		case ACTION_STARTDEATH:
			Action = ACTION_DYING;
			Animation->ChangeReel(DyingAnimation);
			Animation->SetPlayMode(PLAYING);
			SetLegAnimationPlayMode(STOPPED);
			MoveState = MOVE_NONE;
			IncurDeathPenalty();
		break;
		case ACTION_DYING:
			if(Animation->GetPlayMode() == STOPPED)
				Active = false;
		break;
	}

	Animation->Update(FrameTime);
}

// Updates the entity's accuracy according to the weapon's recoil
void _Entity::UpdateRecoil() {

	// Update accuracy based on the weapon's recoil
	CurrentAccuracy -= RecoilRegen;

	// Check bounds
	if(CurrentAccuracy < MinAccuracy)
		CurrentAccuracy = MinAccuracy;
}

// Moves the object with collision detection
void _Entity::Move() {
	UpdateSpeed(1.0f);
	if(MoveState == MOVE_NONE)
		PositionChanged = false;

	// Make a move
	if(MoveState != MOVE_NONE) {

		// Get direction
		Vector2 Goal = GetGoal();
		Vector2 Delta, NewDirection(0.0, 0.0f);
		switch(MoveState) {
			case MOVE_DIRECTION:
				if(MoveDirection[0] != 0 || MoveDirection[1] != 0) {
					Delta = WallInPath(MoveDirection);
					NewDirection = Delta;
				}
			break;
			case MOVE_GOAL:
				Delta = (Goal - Position);
				if(Delta.MagnitudeSquared() >= 0.01f) {
					Delta = WallInPath(Delta);
					NewDirection = Delta;
				}
				else {
					Position = Goal;
				}
			break;
			case MOVE_FORWARD:
				NewDirection[1] = -1;
			break;
			case MOVE_BACKWARD:
				NewDirection[1] = 1;
			break;
			case MOVE_LEFT:
				NewDirection[0] = -1;
			break;
			case MOVE_RIGHT:
				NewDirection[0] = 1;
			break;
			case MOVE_FORWARDLEFT:
				NewDirection[0] = -M_SQRT1_2;
				NewDirection[1] = -M_SQRT1_2;
			break;
			case MOVE_FORWARDRIGHT:
				NewDirection[0] = M_SQRT1_2;
				NewDirection[1] = -M_SQRT1_2;
			break;
			case MOVE_BACKWARDLEFT:
				NewDirection[0] = -M_SQRT1_2;
				NewDirection[1] = M_SQRT1_2;
			break;
			case MOVE_BACKWARDRIGHT:
				NewDirection[0] = M_SQRT1_2;
				NewDirection[1] = M_SQRT1_2;
			break;
			default:
			break;
		}

		// Moving backwards
		if(NewDirection * Direction < 0)
			UpdateSpeed(PLAYER_BACKWARDSPEED);

		float Speed = MovementSpeed * MovementModifier;
		NewDirection *= Speed;

		// Get a list of entities that the object is colliding with
		std::list<_Entity *> HitEntities;
		Map->CheckEntityCollisionsInGrid(Position, Radius, this, HitEntities);

		// Limit movement
		for(auto Iterator : HitEntities) {
			Vector2 HitObjectDirection = Iterator->Position - Position;

			// Determine if we need to clip the direction
			if(HitObjectDirection * NewDirection > 0) {
				Vector2 DividingLine;

				// Rotate vector
				DividingLine[0] = -HitObjectDirection[1];
				DividingLine[1] = HitObjectDirection[0];
				DividingLine.Normalize();

				// Project the direction onto the dividing line
				NewDirection = DividingLine * (NewDirection * DividingLine);
			}
		}

		// Check collisions with walls and map boundaries
		Vector2 NewPosition;
		Map->CheckCollisions(GetPosition() + NewDirection, GetRadius(), NewPosition);

		// Determine if the object has moved
		if(Position != NewPosition) {

			if(MoveSoundTimer >= MoveSoundDelay) {
				if(Type == _Object::PLAYER)
					Audio.Play(new _AudioSource(Audio.GetBuffer(GetSample(SAMPLE_MOVE)), true));
				else
					Audio.Play(new _AudioSource(Audio.GetBuffer(GetSample(SAMPLE_MOVE))), Position);
				MoveSoundTimer = 0;
			}

			int AltGridType = (Type == _Object::PLAYER) ? GRID_PLAYER : GRID_MONSTER;

			// Update grid and position
			Map->RemoveObjectFromGrid(this, AltGridType);

			// Check for updated tile position
			_Coord LastTilePosition = Map->GetValidCoord(Position);
			_Coord TilePosition = Map->GetValidCoord(NewPosition);
			if(TilePosition != LastTilePosition)
				TileChanged = true;

			Position = NewPosition;

			Map->AddObjectToGrid(this, AltGridType);

			PositionChanged = true;
		}
		else {
			Action = ACTION_IDLE;
			PositionChanged = false;
		}

		// Determine which walls are adjacent to the object
		WallState = Map->GetWallState(Position, Radius);
	}
}

// Draws the object
void _Entity::Render(double BlendFactor) {
	Vector2 DrawPosition(Position * BlendFactor + LastPosition * (1.0f - BlendFactor));

	Graphics.DrawTexture(DrawPosition[0], DrawPosition[1], PositionZ, Animation->GetCurrentFrame(), Color, Rotation, Scale, Scale);
}

// Updates the Entity's maximum health
void _Entity::UpdateMaxHealth(int Adjust) {

	// Update health
	MaxHealth += Adjust;

	// In case we allow decreasing max health, make sure it stays above 0.
	if(MaxHealth < 1)
		MaxHealth = 1;
}

// Updates the Entity's current health
void _Entity::UpdateHealth(int Adjust) {

	// Update health
	CurrentHealth += Adjust;

	// Make sure current health doesn't exceed the maximum
	if(CurrentHealth > MaxHealth)
		CurrentHealth = MaxHealth;

	// Object has died
	if(CurrentHealth < 0)
		CurrentHealth = 0;

	if(CurrentHealth == 0 && !IsDying()) {
		Action = ACTION_STARTDEATH;
	}
}

Vector2 _Entity::GetGoal() const {
	if(Goals.empty())
		return Position;
	else
		return Goals.front();
}

Vector2 _Entity::WallInPath(const Vector2 &Delta) const {
	int WallState = Map->GetWallState(Position, Radius);
	if(!WallState)
		return Delta.UnitVector();

	Vector2 NewDelta = Delta;
	if((WallState & WALL_RIGHT) && NewDelta[0] > 0)
		NewDelta[0] = 0;
	if((WallState & WALL_LEFT) && NewDelta[0] < 0)
		NewDelta[0] = 0;
	if((WallState & WALL_TOP) && NewDelta[1] < 0)
		NewDelta[1] = 0;
	if((WallState & WALL_BOTTOM) && NewDelta[1] > 0)
		NewDelta[1] = 0;

	if(!(NewDelta[0] == 0 && NewDelta[1] == 0))
		NewDelta.Normalize();

	return NewDelta;
}
