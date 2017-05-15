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
#include <objectmanager.h>
#include <objects/object.h>
#include <camera.h>

// Constructor
_ObjectManager::_ObjectManager() {
}

// Destructor
_ObjectManager::~_ObjectManager() {

	ClearObjects();
}

// Updates all objects
void _ObjectManager::Update(double FrameTime, const _Camera *Camera) {
	ItemRenderList[0].clear();
	ItemRenderList[1].clear();
	ItemRenderList[2].clear();

	// Update objects
	for(auto Iterator = Objects.begin(); Iterator != Objects.end(); ) {
		_Object *Object = *Iterator;

		// Update the object
		Object->Update(FrameTime);

		// Delete old objects
		if(!Object->GetActive()) {

			// Delete object
			delete Object;
			Iterator = Objects.erase(Iterator);
		}
		else {

			if(Camera->IsCircleInView(Object->GetPosition(), Object->GetScale())) {

				// Add object to render list
				switch(Object->GetType()) {
					case _Object::MISCITEM:
					case _Object::AMMO:
					case _Object::UPGRADE:
					case _Object::WEAPON:
					case _Object::ARMOR:
						ItemRenderList[0].push_back(Object);
					break;
				}
			}

			++Iterator;
		}
	}
}

// Render objects
void _ObjectManager::Render(double BlendFactor) {

	// Draw items
	for(auto Iterator : ItemRenderList[0])
		Iterator->Render(BlendFactor);

	// Draw player
	for(auto Iterator : ItemRenderList[1])
		Iterator->Render(BlendFactor);

	// Draw monsters
	for(auto Iterator : ItemRenderList[2])
		Iterator->Render(BlendFactor);
}

// Deletes all of the objects
void _ObjectManager::ClearObjects() {

	// Delete objects
	for(auto Iterator : Objects)
		delete Iterator;

	Objects.clear();
	ItemRenderList[0].clear();
	ItemRenderList[1].clear();
	ItemRenderList[2].clear();
}

// Adds an object to the manager
void _ObjectManager::AddObject(_Object *Object) {
	Objects.push_back(Object);
}

// Remove an object from the update list
void _ObjectManager::RemoveObject(_Object *Object) {

	for(auto Iterator = Objects.begin(); Iterator != Objects.end(); ++Iterator) {
		if(*Iterator == Object) {
			Objects.erase(Iterator);
			break;
		}
	}
}

void _ObjectManager::AddRenderList(_Object *Object, int Layer) {
	ItemRenderList[Layer].push_back(Object);
}
