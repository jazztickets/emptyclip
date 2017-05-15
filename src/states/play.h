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

#include <state.h>
#include <vector2.h>
#include <color.h>
#include <list>

// Forward Declarations
class _Font;
class _HUD;
class _Map;
class _Event;
class _Entity;
class _Monster;
class _Player;
class _Item;
class _Particles;
class _Camera;
struct _ObjectSpawn;
struct _ParticleTemplate;
struct _EventTile;

// Types of entity attack outcomes
enum CollisionType {
	HIT_NONE,
	HIT_WALL,
	HIT_OBJECT
};

// Holds information about a hit entity
struct HitStruct {
	HitStruct() { }
	HitStruct(_Entity *Object, const Vector2 &Position, int Type)
		:	Object(Object),
		    Position(Position),
		    Type(Type) { }

	_Entity *Object;
	Vector2 Position;
	int Type;
};

// Play state
class _PlayState : public _State {

	public:

		// Setup
		_PlayState();
		void Init();
		void Close();

		// Input
		bool HandleAction(int InputType, int Action, int Value);
		void KeyEvent(const _KeyEvent &KeyEvent);
		void MouseEvent(const _MouseEvent &MouseEvent);

		// Update
		void Update(double FrameTime);
		void Render(double BlendFactor);

		void SetLevel(const std::string &Level) { this->Level = Level; }
		void SetTestMode(bool Value) { TestMode = Value; }
		void SetFromEditor(bool Value) { FromEditor = Value; }
		void SetCheckpointIndex(int Value) { CheckpointIndex = Value; }
		bool GetFromEditor() const { return FromEditor; }

		void GenerateBulletEffects(_Entity *Attacker, const int Type, const Vector2 &Position);

		void SetPlayer(_Player *Player) { this->Player = Player; }
		_Player *GetPlayer() { return Player; }

	protected:

		bool IsPaused();
		void RestartFromDeath();

		void DeleteMonsters();
		void DeleteActiveEvents();

		void UpdateMonsters(double FrameTime);
		void CheckEvents(const _Entity *Entity);
		void UpdateEvents(double FrameTime);

		void SpawnObject(_ObjectSpawn *ObjectSpawn, bool GenerateStats=false);
		void AddMonster(_Monster *Monster);
		void RemoveMonster(_Monster *Monster);
		void CreateItemDrop(const _Entity *Entity);
		void EntityAttack(_Entity *Attacker, int GridType);
		void PickupObject();
		void UseObject();

		// Parameters
		std::string Level;
		bool TestMode, FromEditor;
		int CheckpointIndex;

		// Game
		double CursorItemTimer, SaveGameTimer;

		// Map
		_Map *Map;
		_Event *LastLightEvent;

		// Entities
		_Player *Player;
		std::list<_Entity *> Monsters;
		std::list<_Event *> ActiveEvents;

		// HUD
		_HUD *HUD;
		_Item *CursorItem, *PreviousCursorItem;

		// Particles
		_Particles *Particles;
		bool IsFiring;

		// Camera
		_Camera *Camera;
		Vector2 WorldCursor;
};

extern _PlayState PlayState;
