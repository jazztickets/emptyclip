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
#include <objects/monster.h>
#include <objects/templates.h>
#include <objects/player.h>
#include <map.h>
#include <animation.h>
#include <random.h>

int PersonalityBaseBehaviors[PERSONALITY_COUNT] = {
	MONSTER_ATTACK | MONSTER_INVESTIGATE,
	MONSTER_RETREAT,
	MONSTER_ATTACK | MONSTER_WANDER | MONSTER_INVESTIGATE,
	MONSTER_ATTACK | MONSTER_INVESTIGATE,
	MONSTER_ATTACK,
	MONSTER_ATTACK | MONSTER_INVESTIGATE,
	MONSTER_WANDER | MONSTER_ATTACK,
	MONSTER_ATTACK,
	MONSTER_ATTACK | MONSTER_LOOK | MONSTER_INVESTIGATE,
	MONSTER_SEARCH | MONSTER_RETREAT,
	MONSTER_STOP
};

// Constructor
_Monster::_Monster()
:	_Entity() {

	Type = _Object::MONSTER;
}

// Destructor
_Monster::~_Monster() {
}

// Constructor
_Monster::_Monster(_MonsterTemplate *Monster, _Animation *Animation, const Vector2 &Position)
:	_Entity() {

	Type = _Object::MONSTER;

	// Monster stats
	Name = Monster->Name;
	Color = Monster->Color;
	MovementSpeed = Monster->MovementSpeed / (Monster->CurrentSpeed / 16.6666f);
	Radius = Monster->Radius;
	Scale = Monster->Scale;
	MinAccuracy = Monster->Accuracy;
	MaxAccuracy = Monster->Accuracy;
	Recoil = 0;
	RecoilRegen = 0;
	AttackRange = Monster->AttackRange;
	AttackRange *= AttackRange;
	Level = Monster->Level;
	CurrentHealth = MaxHealth = Monster->Health;
	Defense = Monster->Defense;
	ViewRangeFront = Monster->ViewRange;
	ViewRangeSide = Monster->ViewRange * MONSTER_SIDERANGE;
	ViewRangeBack = Monster->ViewRange * MONSTER_BACKRANGE;
	ExperienceGiven = Monster->ExperienceGiven;
	ItemGroupIdentifier = Monster->ItemGroupIdentifier;
	MinDamage = Monster->MinDamage;
	MaxDamage = Monster->MaxDamage;
	FirePeriod = Monster->FirePeriod;
	WeaponType = Monster->WeaponType;
	this->Position = LastPosition = Position;
	*this->Animation = *Animation;
	WeaponParticles = Monster->WeaponParticles;
	MoveSoundDelay = 1000;
	if(Animation && Animation->GetReel(0))
		MoveSoundDelay = Animation->GetPlaybackSpeed() * Animation->GetReel(0)->Textures.size();

	ViewRangeFront *= ViewRangeFront;
	ViewRangeSide *= ViewRangeSide;
	ViewRangeBack *= ViewRangeBack;
	PersonalityType = Monster->BehaviorType;
	if(PersonalityType != PERSONALITY_TREASURE)
		Rotation = Random.GenerateRange(0.0f, 359.0f);

	WeaponParticleOffset[0] = ZERO_VECTOR;
	for(int i = 1; i < WEAPON_TYPES; i++)
		WeaponParticleOffset[i] = MONSTER_WEAPONOFFSET * Scale;

	// Temp
	AITimer = 0;
	WaitTime = 0;
	BaseBehavior = PersonalityBaseBehaviors[PersonalityType];
	CurrentActions = 0;

	MoveDirection = Vector2(0, 0);

	ReturnPosition = Vector2(-1.0f, -1.0f);
}

// Updates the entity's states
void _Monster::Update(double FrameTime, _Player *Player) {
	_Entity::Update(FrameTime);

	BehaviorTime += FrameTime;
	AITimer += FrameTime;

	if(Player->IsDying())
		return;

	// Check timer to see if the object can move
	if(!AttackAllowed && FireTimer >= FirePeriod)
		AttackAllowed = true;

	// Update accuracy
	UpdateRecoil();

	// Update animation
	UpdateAnimation(FrameTime);

	bool PlayerVisible = IsVisible(Player->GetPosition());
	bool PlayerVisibleWithBounds = false;
	if(PlayerVisible)
		PlayerVisibleWithBounds = Map->IsVisibleWithBounds(Position, Player->GetPosition(), Radius);

	if(Player->IsDying()) {
		PlayerVisible = PlayerVisibleWithBounds = false;
		Goals.clear();
	}

	if(!(CurrentActions & AI_LOOKING) && PersonalityType != PERSONALITY_TREASURE && !IsDying()) {
		if(GetMoveState() == MOVE_DIRECTION)//CurrentActions & AI_WANDERING || CurrentActions & (AI_EXPLORING | AI_RETREATING))
			FacePosition(MoveDirection+Position);
		else if(PlayerVisibleWithBounds)
			FacePosition(Player->GetPosition());
		else if(MoveState == MOVE_GOAL)
			FacePosition(GetGoal());
	}

	// Move the monster
	if(!IsDying() && !Player->IsDying()) {

		CheckGoal();
		//if(AITimer >= WaitTime || PathFailed)
		{
			//	AITimer = 0;
			//	WaitTime = Random.GenerateRange(0.1f, 0.6f);

			int Actions = GetBehavior();

			// Don't do anything
			if(Actions & MONSTER_STOP) {
				CurrentActions |= AI_STOPPED;
				SetMoveState(MOVE_NONE);
			}
			else
				CurrentActions &= ~AI_STOPPED;

			// If player is seen, go to his/her last seen position, and if the player can't
			// be seen anymore, return to original position.
			if(Actions & MONSTER_INVESTIGATE) {
				// Check if the investigate behavior is done.
				if(Investigate(Player, PlayerVisibleWithBounds)) {
					// Remove the investigate behavior
					BehaviorList.pop_front();

					CurrentActions &= ~(AI_INVESTIGATING | AI_FOLLOWING_PLAYER | AI_FOLLOWING_PATH);

					switch(PersonalityType) {
						case PERSONALITY_AGGRESSOR:
						case PERSONALITY_KAMIKAZE:
							// Seek out the player
							BehaviorList.push_front(MONSTER_SEARCH | MONSTER_INVESTIGATE | MONSTER_ATTACK);
							break;

						case PERSONALITY_CURIOUS:
							// Since player was seen, explore instead of wander
							BehaviorList.push_front(MONSTER_EXPLORE | MONSTER_INVESTIGATE | MONSTER_ATTACK);
							break;

					}

					BehaviorTime = 0;
				}
			}
			else
				CurrentActions &= ~AI_INVESTIGATING;

			// Wander
			if(Actions & MONSTER_WANDER) {
				Wander();
			}
			else
				CurrentActions &= ~AI_WANDERING;

			// Explore
			if(Actions & MONSTER_EXPLORE) {
				Explore();
			}
			else
				CurrentActions &= ~AI_EXPLORING;

			// Follow path
			if(Actions & MONSTER_MOVE) {
				if(MovePath()) {
					BehaviorList.pop_front();
					ReturnPosition[0] = ReturnPosition[1] = -1;
					CurrentActions &= ~(AI_MOVING | AI_FOLLOWING_PATH);
					SetMoveState(MOVE_NONE);
				}
			}
			else
				CurrentActions &= ~AI_MOVING;

			// Retreat
			if(Actions & MONSTER_RETREAT) {
				Retreat(Player, PlayerVisibleWithBounds);
			}
			else
				CurrentActions &= ~AI_RETREATING;

			// Attack player
			if(Actions & MONSTER_ATTACK) {
				// Make sure player is in range, and in sight
				if(PlayerVisible && InRange(Player->GetPosition())) {
					bool DoAttack = true;
					bool Retreat = false;

					switch(PersonalityType) {
						case PERSONALITY_GUARD:
							if(Player->IsDying())
								DoAttack = false;
							if(CurrentHealth < MaxHealth*0.1) {
								Retreat = true;
								DoAttack = false;
							}
							break;
						case PERSONALITY_IMMOVABLE:
						case PERSONALITY_KAMIKAZE:
							break;
						case PERSONALITY_CURIOUS:
							if(Player->IsDying())
								DoAttack = false;
							if(CurrentHealth < MaxHealth*0.4) {
								Retreat = true;
								DoAttack = false;
							}
							break;
						case PERSONALITY_LEMMING:
							break;
						case PERSONALITY_AGGRESSOR:
						case PERSONALITY_LOOKOUT:
							if(Player->IsDying())
								DoAttack = false;
							if(CurrentHealth < MaxHealth*0.6) {
								DoAttack = false;
							}
							break;
						case PERSONALITY_MINDLESS:
							if(Random.GenerateRange(0, 50) == 0)
								DoAttack = true;
							break;
					}

					// Attack player
					if(DoAttack) {
						FacePosition(Player->GetPosition());
						StartAttack();
						CurrentActions |= AI_ATTACKING;
					}
					else if(PersonalityType == PERSONALITY_AGGRESSOR) {
						FacePosition(Player->GetPosition());
						StartAttack();
					}
					else {
						BehaviorList.pop_front();

						if(Retreat)
							BehaviorList.push_front(MONSTER_RETREAT);
						CurrentActions &= ~AI_ATTACKING;
					}
				}
				else
					CurrentActions &= ~AI_ATTACKING;
			}
			else
				CurrentActions &= ~AI_ATTACKING;

			// Follow the player
			if(Actions & MONSTER_FOLLOW)
			{
				Follow(Player, PlayerVisible);
			}
			else
				CurrentActions &= ~AI_FOLLOWING_PLAYER;

			// Look in random direction
			if(Actions & MONSTER_LOOK) {
				Look();
			}
			else
				CurrentActions &= ~AI_LOOKING;
		}

		if(!(CurrentActions & AI_ATTACKING))
			Move();
	}
}

bool _Monster::VisiblePath(const Vector2 &Goal) {
	bool Visible = IsVisible(Goal);
	if(Visible) {
		SetMoveState(MOVE_DIRECTION);
		MoveDirection = Goal - Position;
		if(MoveDirection[0] != 0 || MoveDirection[1] != 0)
			MoveDirection = MoveDirection.UnitVector();

		Goals.clear();

		return true;
	}

	return false;
}

// Turns the monster to face its goal
void _Monster::UpdateDirection() {

	FacePosition(GetGoal());
}

bool _Monster::Passed(const Vector2 &Pos) {
	return (Position == Pos);
}

bool _Monster::InRange(const Vector2 &Pos) {
	return ((Position - Pos).MagnitudeSquared() <= AttackRange);
}

bool _Monster::IsVisible(const Vector2 &TargetPosition) {
	float PlayerDirection = atan2(TargetPosition[1] - Position[1], TargetPosition[0] - Position[0]) * DEGREES_IN_RADIAN + 90;
	if(PlayerDirection < 0) PlayerDirection += 360;

	// 65 - 100 right side
	// 260 - 295 left side
	// 295 - 65 front
	// 100 - 260 back
	float DegreesDifference = PlayerDirection - Rotation;
	if(DegreesDifference < 0) DegreesDifference += 360;

	float Distance = (Position - TargetPosition).MagnitudeSquared();
	bool InViewRange = false;
	if(DegreesDifference < 65 || DegreesDifference > 295) {
		if(Distance <= ViewRangeFront)
			InViewRange = true;
	}
	else if(DegreesDifference > 100 && DegreesDifference < 260) {
		if(Distance <= ViewRangeBack)
			InViewRange = true;
	}
	else {
		if(Distance <= ViewRangeSide)
			InViewRange = true;
	}

	if(InViewRange && Map->IsVisible(Position, TargetPosition))
		return true;

	return false;
}

bool _Monster::Investigate(_Player *Player, bool PlayerVisible) {
	// Find path to player if he/she is visible.
	if(PlayerVisible) {
		VisiblePath(Player->GetPosition());
		if(ReturnPosition[0] < 0)
			ReturnPosition = Position;
		CurrentActions |= AI_INVESTIGATING | AI_FOLLOWING_PLAYER | AI_FOLLOWING_PATH;
	}
	else if(VisiblePath(Player->GetPosition()))
		CurrentActions |= AI_INVESTIGATING | AI_FOLLOWING_PLAYER;
	else
		CurrentActions &= ~AI_FOLLOWING_PLAYER;

	if(CurrentActions & AI_INVESTIGATING) {
		if(GetMoveState() == MOVE_GOAL) {
			// Keep moving along the path...
			CheckGoal();
			// Check if we've reach the end of the path.
			if(Goals.empty())
				return true;
		}
		else if(GetMoveState() == MOVE_DIRECTION)
			return false;
	}

	return false;
}

void _Monster::Wander() {
	if(!(CurrentActions & AI_INVESTIGATING) &&
	        (BehaviorWait.empty() || BehaviorTime >= BehaviorWait.front())) {
		CurrentActions |= AI_WANDERING;
		CurrentActions &= ~(AI_FOLLOWING_PATH | AI_FOLLOWING_PLAYER);

		if(!BehaviorWait.empty())
			BehaviorWait.pop_front();

		BehaviorTime = 0;
		BehaviorWait.push_front(Random.GenerateRange(0.5f, 1.5f));

		int Stop = Random.GenerateRange(0, 4);
		if(!Stop)
			SetMoveState(MOVE_NONE);
		else {
			MoveDirection[0] = Random.GenerateRange(-1.0f, 1.0f);
			MoveDirection[1] = Random.GenerateRange(-1.0f, 1.0f);
			if(WallState) {
				if(WallState & WALL_RIGHT && MoveDirection[0] > 0)
					MoveDirection[0] = 0;
				if(WallState & WALL_LEFT && MoveDirection[0] < 0)
					MoveDirection[0] = 0;
				if(WallState & WALL_TOP && MoveDirection[1] < 0)
					MoveDirection[1] = 0;
				if(WallState & WALL_BOTTOM && MoveDirection[1] > 0)
					MoveDirection[1] = 0;
			}
			if(MoveDirection[0] != 0 || MoveDirection[1] != 0)
				MoveDirection = MoveDirection.UnitVector();

			MoveState = MOVE_DIRECTION;
		}
	}
	else if(CurrentActions & AI_INVESTIGATING)
		CurrentActions &= ~AI_WANDERING;
}

void _Monster::Explore() {
	if(!(CurrentActions & AI_INVESTIGATING)) {
		CurrentActions |= AI_EXPLORING;
		CurrentActions &= ~(AI_FOLLOWING_PATH | AI_FOLLOWING_PLAYER);

		if(WallState || MoveDirection == Vector2(0, 0)) {
			if((WallState & WALL_RIGHT && MoveDirection[0] > 0) ||
			        (WallState & WALL_LEFT && MoveDirection[0] < 0) ||
			        (WallState & WALL_TOP && MoveDirection[1] < 0) ||
			        (WallState & WALL_BOTTOM && MoveDirection[1] > 0) ||
			        (MoveDirection == Vector2(0, 0))) {
				if(MoveDirection[0] != 0 && (WallState & WALL_LEFT || WallState & WALL_RIGHT))
					MoveDirection[1] = Random.GenerateRange(-1.0f, 1.0f);
				else if(MoveDirection[1] != 0 && (WallState & WALL_TOP || WallState & WALL_BOTTOM))
					MoveDirection[0] = Random.GenerateRange(-1.0f, 1.0f);
				else {
					MoveDirection[0] = Random.GenerateRange(-1.0f, 1.0f);
					MoveDirection[1] = Random.GenerateRange(-1.0f, 1.0f);
				}

				if(WallState & WALL_RIGHT && MoveDirection[0] > 0)
					MoveDirection[0] = 0;
				if(WallState & WALL_LEFT && MoveDirection[0] < 0)
					MoveDirection[0] = 0;
				if(WallState & WALL_TOP && MoveDirection[1] < 0)
					MoveDirection[1] = 0;
				if(WallState & WALL_BOTTOM && MoveDirection[1] > 0)
					MoveDirection[1] = 0;

				if(MoveDirection[0] != 0 || MoveDirection[1] != 0)
					MoveDirection = MoveDirection.UnitVector();
			}
		}

		MoveState = MOVE_DIRECTION;
	}
	else if(CurrentActions & AI_INVESTIGATING)
		CurrentActions &= ~AI_EXPLORING;
}

bool _Monster::MovePath() {
	if(!(CurrentActions & AI_INVESTIGATING)) {
		if(Goals.empty() && ReturnPosition[0] >= 0) {
			VisiblePath(ReturnPosition);
			CurrentActions |= AI_MOVING | AI_FOLLOWING_PATH;
		}

		if(CheckGoal() && Position == ReturnPosition)
			return true;
	}
	else
		CurrentActions &= ~AI_MOVING;

	return false;
}

void _Monster::Look() {
	if(!(CurrentActions & AI_INVESTIGATING)) {
		if(BehaviorWait.empty() || BehaviorTime >= BehaviorWait.front()) {
			CurrentActions |= AI_LOOKING;
			BehaviorList.pop_front();
			if(!BehaviorWait.empty())
				BehaviorWait.pop_front();
			else
				BehaviorWait.push_front(Random.GenerateRange(0.5, 1.5));
			BehaviorTime = 0;
			FacePosition(Position + Vector2(static_cast<float>(Random.GenerateRange(0,360))));
		}
	}
	else
		CurrentActions &= ~AI_LOOKING;
}

void _Monster::Retreat(_Player *Player, bool PlayerVisible) {
	if(PlayerVisible) {
		MoveDirection = Position - Player->GetPosition();
		if(MoveDirection[0] != 0 || MoveDirection[1] != 0)
			MoveDirection = MoveDirection.UnitVector();

		MoveState = MOVE_DIRECTION;

		CurrentActions |= AI_RETREATING;
	}
	else {
		MoveDirection[0] = MoveDirection[1] = 0;
		CurrentActions &= ~AI_RETREATING;
	}
}

void _Monster::Follow(_Player *Player, bool PlayerVisible) {
	if(PlayerVisible)
	{
		VisiblePath(Player->GetPosition());
		CurrentActions |= AI_FOLLOWING_PLAYER;
	}
	CheckGoal();
}

bool _Monster::CheckGoal() {
	if(!Goals.empty()) {
		if(Passed(GetGoal())) {
			PopGoal();
			if(Goals.empty())
				return true;
		}
	}

	return false;
}

const _ParticleTemplate *_Monster::GetWeaponParticle(int Index) const {

	return WeaponParticles->ParticleTemplates[Index];
}
