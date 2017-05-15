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

// Headers
#include <objects/item.h>

// Forward Declarations
struct _UpgradeTemplate;

// Classes
class _Upgrade : public _Item {

	public:

		_Upgrade(const std::string &Identifier, int Count, const Vector2 &Position, const _UpgradeTemplate *Upgrade, _Texture *Texture);
		~_Upgrade();

		void SetUpgradeType(int UpgradeType) { this->UpgradeType = UpgradeType; }
		void SetWeaponType(int WeaponType) { this->WeaponType = WeaponType; }

		int GetUpgradeType() const { return UpgradeType; }
		int GetWeaponType() const { return WeaponType; }
		float GetBonus() const { return Bonus; }

		virtual std::string GetTypeAsString() const override { return "Upgrade Component"; }
		static std::string ToString(int Type, int WeaponType);

	protected:

		float Bonus;
		int WeaponType, UpgradeType;
};
