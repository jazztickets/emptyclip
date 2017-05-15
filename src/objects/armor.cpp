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
#include <objects/armor.h>

// Constructor
_Armor::_Armor(const std::string &Identifier, int Count, const Vector2 &Position, const _ArmorTemplate *Armor, _Texture *Texture) {
	this->Type = _Object::ARMOR;
	this->Identifier = Identifier;
	this->Count = Count;
	this->Texture = Texture;
	this->Position = Position;

	this->Defense = Armor->Defense;
	this->StrengthRequirement = Armor->StrengthRequirement;
	this->Name = Armor->Name;
	this->Color = Armor->Color;
}

// Destructor
_Armor::~_Armor() {
}
