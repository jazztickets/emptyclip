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

// Forward Declarations
class _Point;

// Camera class
class _Camera {

	public:

		_Camera(const Vector2 &Position, float Distance, float UpdateDivisor);
		~_Camera();

		// Updates
		void CalculateFrustum(float AspectRatio);
		void Set3DProjection(double BlendFactor) const;
		void Update(double FrameTime);
		void ConvertScreenToWorld(const _Point &Point, Vector2 &WorldPosition);
		void ConvertWorldToScreen(const Vector2 &WorldPosition, _Point &Point);

		bool IsCircleInView(const Vector2 &Position, float Radius) const;
		bool IsAABBInView(const float *Bounds) const;

		void UpdatePosition(const Vector2 &UpdatePosition) { this->TargetPosition += UpdatePosition; }
		void ForcePosition(const Vector2 &Position) { this->LastPosition = this->Position = Position; this->TargetPosition = Position; }

		void UpdateDistance(float Update) { this->TargetDistance += Update; }
		void ForceDistance(float Distance) { this->Distance = LastDistance = TargetDistance = Distance; }

		void SetPosition(const Vector2 &Position) { this->TargetPosition = Position; }
		const Vector2 &GetPosition() const { return Position; }

		void SetDistance(float Distance) { this->TargetDistance = Distance; }
		float GetDistance() const { return Distance; }
		float GetTargetDistance() const { return TargetDistance; }

		void SetFovy(float Fovy) { this->Fovy = Fovy; }
		float GetFovy() const { return Fovy; }

		const float *GetAABB() const { return AABB; }

	private:

		Vector2 LastPosition, Position, TargetPosition;
		float LastDistance, Distance, TargetDistance;
		float Fovy;
		float UpdateDivisor;

		float Frustum[2];
		float Near, Far;

		float AABB[4];
};
