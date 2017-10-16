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
#include <objects/item.h>

// Holds information about armor
struct _ArmorTemplate {
	std::string Name;
	std::string IconIdentifier;
	_Color Color;
	int StrengthRequirement;
	int DamageBlock;
	float DamageResist;
	float MovementSpeed;
};

// Classes
class _Armor : public _Item {

	public:

		_Armor(const std::string &Identifier, int Count, const Vector2 &Position, const _ArmorTemplate *Armor, _Texture *Texture);
		~_Armor();

		int GetStrengthRequirement() const { return StrengthRequirement; }
		int GetDamageBlock() const { return DamageBlock; }
		float GetDamageResist() const { return DamageResist; }
		float GetMovementSpeed() const { return MovementSpeed; }

		virtual std::string GetTypeAsString() const override { return "Armor"; }

	protected:

		int StrengthRequirement;
		int DamageBlock;
		float DamageResist;
		float MovementSpeed;
};
