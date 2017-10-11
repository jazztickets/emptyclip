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
#include <vector2.h>
#include <coord.h>
#include <color.h>
#include <texture.h>
#include <vector>
#include <string>

// Forward Declarations
class _Buffer;
class _Map;

// Object class
class _Object {

	public:

		// Types of objects in the game
		enum ObjectType {
			UNDEFINED=-1,
			PLAYER,
			MONSTER,
			MISCITEM,
			AMMO,
			UPGRADE,
			WEAPON,
			ARMOR,
			COUNT
		};

		_Object();
		virtual ~_Object();

		virtual void Update(double FrameTime) { }
		virtual void Render(double BlendFactor) { }
		virtual void Serialize(_Buffer &Buffer) { }
		void FacePosition(const Vector2 &Cursor);

		void SetActive(bool Value) { this->Active = Value; }
		bool GetActive() const { return Active; }

		void SetName(const std::string &Name) { this->Name = Name; }
		virtual const std::string &GetName() const { return Name; }

		void SetColor(const _Color &Color) { this->Color = Color; }
		const _Color &GetColor() const { return Color; }

		void SetDirection(float Direction) { this->Rotation = Direction; }
		float GetDirection() const { return Rotation; }

		void SetScale(float Value) { this->Scale = Value; }
		float GetScale() const { return Scale; }

		void SetPosition(const Vector2 &Position);
		const Vector2 &GetPosition() const { return Position; }

		void SetWallState(int Value) { this->WallState = Value; }
		int GetWallState() const { return WallState; }

		void SetTileChanged(bool Value) { TileChanged = Value; }
		bool GetTileChanged() const { return TileChanged; }

		virtual std::string GetTypeAsString() const { return "Object"; }
		int GetType() const { return Type; }
		float GetRadius() const { return Radius; }

		void SetMap(_Map *Map) { this->Map = Map; }

	protected:

		// Attributes
		bool Active;
		std::string Name;
		int Type;

		// Map
		_Map *Map;
		bool TileChanged;

		// Collision
		float Radius;
		int WallState;

		// Graphics
		Vector2 Position;
		Vector2 LastPosition;
		Vector2 Direction;
		_Color Color;
		float Rotation;
		float Scale;
		float PositionZ;

};
