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
#include <list>

// Forward Declarations
class _Object;
class _Camera;

// Manages all the objects for a map
class _ObjectManager {

	public:

		_ObjectManager();
		~_ObjectManager();

		// Updates
		void Update(double FrameTime, const _Camera *Camera);
		void Render(double BlendFactor);

		// Management
		void AddObject(_Object *Object);
		void RemoveObject(_Object *Object);
		void ClearObjects();

		void AddRenderList(_Object *Object, int Layer);

	private:

		// Objects
		std::list<_Object *> Objects;

		// Rendering
		std::list<_Object *> ItemRenderList[3];
};
