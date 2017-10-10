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
#include <objects/item.h>
#include <graphics.h>
#include <constants.h>
#include <buffer.h>

// Constructor
_Item::_Item() {
	Texture = nullptr;
	Level = 0;
	Count = 0;
	Quality = 0;
	PositionZ = ITEM_Z;
}

// Destructor
_Item::~_Item() {
}

// Serialize for saving
void _Item::Serialize(_Buffer &Buffer) {
	Buffer.WriteString(Identifier.c_str());
}

// Draws the object
void _Item::Render(double BlendFactor) {

	Graphics.DrawTexture(Position[0], Position[1], PositionZ, Texture, Color, Rotation, ITEM_SCALE, ITEM_SCALE);
}
