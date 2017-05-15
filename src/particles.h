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
#include <list>

// Forward Declarations
class _Camera;
class _Particle;
struct _ParticleTemplate;

struct _ParticleSpawn {
	_ParticleSpawn(const _ParticleTemplate *Template, const Vector2 &Position, float PositionZ, float RotationAdjust)
		:	Template(Template),
			Position(Position),
			PositionZ(PositionZ),
			RotationAdjust(RotationAdjust) { }

	const _ParticleTemplate *Template;
	Vector2 Position;
	float PositionZ;
	float RotationAdjust;
};

// Manages all the objects
class _Particles {

	public:

		enum ParticleTypes {
			NORMAL,
			FLOOR_DECALS,
			WALL_DECALS,
			COUNT,
		};

		_Particles();
		~_Particles();

		// Updates
		void Update(double FrameTime);
		void Render(int Type);

		// Management
		void Add(_Particle *Particle);
		void Create(const _ParticleSpawn &Spawn);
		void Clear();

		void SetCamera(const _Camera *Camera) { this->Camera = Camera; }
		const _Camera *GetCamera() const { return Camera; }

	private:

		// Objects
		std::list<_Particle *> Particles;

		// Rendering
		std::list<_Particle *> RenderList[COUNT];

		// Graphics
		const _Camera *Camera;
};
