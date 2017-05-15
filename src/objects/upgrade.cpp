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
#include <objects/upgrade.h>
#include <objects/templates.h>

// Constructor
_Upgrade::_Upgrade(const std::string &Identifier, int Count, const Vector2 &Position, const _UpgradeTemplate *Upgrade, _Texture *Texture) {
	this->Type = _Object::UPGRADE;
	this->Identifier = Identifier;
	this->Count = Count;
	this->Position = Position;
	this->Texture = Texture;

	this->Name = Upgrade->Name;
	this->Color = Upgrade->Color;
	this->WeaponType = Upgrade->WeaponType;
	this->UpgradeType = Upgrade->UpgradeType;
	this->Bonus = Upgrade->Bonus;
}

// Destructor
_Upgrade::~_Upgrade() {
}

// Convert an upgrade type to string
std::string _Upgrade::ToString(int Type, int WeaponType) {

	switch(Type) {
		case UPGRADE_CLIP:
			return "Round Size";
		break;
		case UPGRADE_DAMAGE:
			return "Damage";
		break;
		case UPGRADE_ACCURACY:
			return "Accuracy";
		break;
		case UPGRADE_FIREPERIOD:
			if(WeaponType == WEAPON_MELEE)
				return "Attack Rate";
			else
				return "Fire Rate";
		break;
		case UPGRADE_RELOADPERIOD:
			return "Reload Speed";
		break;
		case UPGRADE_ATTACKS:
			if(WeaponType == WEAPON_MELEE)
				return "Attack Count";
			else
				return "Bullets/Shot";
		break;
	}

	return "";
}