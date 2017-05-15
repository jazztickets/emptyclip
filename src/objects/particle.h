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
#include <color.h>
#include <algorithm>

// Forward Declarations
class _Texture;
struct _ParticleSpawn;

// Classes
class _Particle {

	public:

		_Particle(const _ParticleSpawn &Spawn);
		~_Particle();

		void Update(double FrameTime);
		void Render();

		bool IsDeleted() const { return Deleted; }

		void SetType(int Type) { this->Type = Type; }
		int GetType() const { return Type; }

		void SetScale(const Vector2 &Scale) { this->Scale = Scale; }
		float GetRadius() const { return std::max(Scale.X, Scale.Y); }

		void SetPosition(const Vector2 &Position) { this->Position = Position; }
		const Vector2 &GetPosition() const { return Position; }

		void SetLifetime(double Lifetime) { this->Lifetime = Lifetime; }
		double GetLifetime() const { return Lifetime; }

		void SetVelocity(const Vector2 &Velocity) { this->Velocity = Velocity; }
		const Vector2 &GetVelocity() const { return Velocity; }

	private:

		// Attributes
		int Type;
		double Lifetime;
		bool Deleted;

		// Graphics
		const _Texture *Texture;
		_Color Color;
		Vector2 Scale;
		float Rotation, AlphaSpeed, PositionZ, ScaleAspect;

		// Physics
		Vector2 Position, Velocity, Acceleration;
		float TurnSpeed;
};
