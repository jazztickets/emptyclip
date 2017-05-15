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
#include <objects/ammo.h>
#include <objects/templates.h>

// Constructor
_Ammo::_Ammo(const std::string &Identifier, int Count, const Vector2 &Position, const _AmmoTemplate *Ammo, _Texture *Texture) {
	this->AmmoType = Ammo->AmmoType;
	this->Type = _Object::AMMO;
	this->Identifier = Identifier;
	this->Count = Count;
	this->Name = Ammo->Name;
	this->Texture = Texture;
	this->Color = Ammo->Color;
	this->Position = Position;
}

// Destructor
_Ammo::~_Ammo() {
}

// Convert an ammo type to string
std::string _Ammo::ToString(int Type) {

	switch(Type) {
		case AMMO_NONE:
			return "None";
		break;
		case AMMO_9MM:
			return "9mm";
		break;
		case AMMO_357:
			return ".357";
		break;
		case AMMO_556MM:
			return "5.56mm";
		break;
		case AMMO_762MM:
			return "7.62mm";
		break;
		case AMMO_SHELLS:
			return "Shells";
		break;
		case AMMO_ROCKETS:
			return "Rockets";
		break;
		case AMMO_POWERCELLS:
			return "Power Cells";
		break;
		case AMMO_PLASMACELLS:
			return "Plasma Cells";
		break;
		case AMMO_PSI:
			return "Psi";
		break;
		default:
		break;
	}

	return "";
}
