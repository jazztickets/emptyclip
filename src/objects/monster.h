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
#include <objects/entity.h>
#include <list>
#include <vector>

// Constants
const Vector2 MONSTER_WEAPONOFFSET 	= Vector2(32.0f / 64.0f - 0.5f, -0.5f);

const float MONSTER_SIDERANGE 		= 0.80f;
const float MONSTER_BACKRANGE 		= 0.50f;

const int MONSTER_STOP				= 0x1;
const int MONSTER_MOVE				= 0x2;
const int MONSTER_RETREAT			= 0x4;
const int MONSTER_INVESTIGATE		= 0x8;
const int MONSTER_EXPLORE			= 0x10;
const int MONSTER_WANDER			= 0x20;
const int MONSTER_SEARCH			= 0x40;
const int MONSTER_ATTACK			= 0x80;
const int MONSTER_FOLLOW			= 0x100;
const int MONSTER_LOOK				= 0x200;

const int AI_STOPPED				= 0x1;
const int AI_FOLLOWING_PLAYER		= 0x2;
const int AI_FOLLOWING_PATH			= 0x4;
const int AI_MOVING					= 0x8;
const int AI_RETREATING				= 0x10;
const int AI_INVESTIGATING			= 0x20;
const int AI_EXPLORING				= 0x40;
const int AI_WANDERING				= 0x80;
const int AI_ATTACKING				= 0x100;
const int AI_LOOKING				= 0x200;

struct _MonsterTemplate;
struct _WeaponParticleTemplate;
class _Player;

enum PersonalityTypes {
	PERSONALITY_AGGRESSOR, 	// 0 Likes to be on the offensive, but will retreat if needed			ATTACK, INVESTIGATE
	PERSONALITY_COWARD, 	// 1 Avoids confrontation as much as possible							RETREAT
	PERSONALITY_CURIOUS, 	// 2 Wanderer, searcher													ATTACK, WANDER, INVESTIGATE
	PERSONALITY_GUARD, 		// 3 Protects their point of origin										ATTACK, INVESTIGATE
	PERSONALITY_IMMOVABLE, 	// 4 Will never move from spot, but will protect it						ATTACK
	PERSONALITY_KAMIKAZE, 	// 5 Extremely aggressive, doesn't care if it dies						ATTACK, INVESTIGATE
	PERSONALITY_MINDLESS, 	// 6 Moves aimlessly, may attack if near player, but may not. basically has no goal	WANDER
	PERSONALITY_LEMMING, 	// 7 Seeks to do whatever the group is doing							ATTACK, GROUP
	PERSONALITY_LOOKOUT, 	// 8 Looks around, pursues player if seen, alerts others				ATTACK, LOOK, INVESTIGATE
	PERSONALITY_SPY, 		// 9 Seeks out player, when found returns to others and then they all go after the player	SEARCH, RETREAT
	PERSONALITY_TREASURE,	// 10 Treasure chest (does nothing)
	PERSONALITY_COUNT,
};

// Classes
class _Monster : public _Entity {

	public:

		_Monster();
		_Monster(_MonsterTemplate *Monster, _Animation *Animation, const Vector2 &Position);
		~_Monster();

		bool CalcPath(const Vector2 &Goal);
		bool VisiblePath(const Vector2 &Goal);

		void UpdateDirection();
		bool Passed(const Vector2 &Pos);
		void Update(double FrameTime, _Player *Player);
		bool IsVisible(const Vector2 &TargetPosition);
		bool InRange(const Vector2 &Pos);

		bool Investigate(_Player *Player, bool PlayerVisible);
		void Patrol();
		void Wander();
		void Explore();
		bool MovePath();
		void Look();
		void Retreat(_Player *Player, bool PlayerVisible);
		void Follow(_Player *Player, bool PlayerVisible);

		bool CheckGoal();

		const _ParticleTemplate *GetWeaponParticle(int Index) const;
		std::string GetItemGroupIdentifier() const { return ItemGroupIdentifier; }
		int64_t GetExperienceGiven() const { return ExperienceGiven; }
		int GetBehavior() { if(BehaviorList.empty()) BehaviorList.push_front(BaseBehavior); return BehaviorList.front(); }

		Vector2 ReturnPosition;

	private:

		float AITimer;
		float WaitTime;

		int64_t ExperienceGiven;
		std::string ItemGroupIdentifier;

		int PersonalityType;
		std::list<int> BehaviorList;
		std::list<float> BehaviorWait;
		float BehaviorTime;
		int BaseBehavior;
		int CurrentActions;

		// Viewing ranges
		float ViewRangeFront, ViewRangeSide, ViewRangeBack;
		_WeaponParticleTemplate *WeaponParticles;
};
