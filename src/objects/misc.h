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

// Enumerations
enum MiscItemType {
	MISCITEM_MEDKIT,
	MISCITEM_OTHER,
	MISCITEM_COUNT,
};

// Forward Declarations
struct _MiscItemTemplate;

// Classes
class _MiscItem : public _Item {

	public:

		_MiscItem(const std::string &Identifier, int Count, const Vector2 &Position, const _MiscItemTemplate *MiscItem, _Texture *Texture);
		~_MiscItem();

		void SetMiscItemType(int MiscItemType) { this->MiscItemType = MiscItemType; }
		int GetMiscItemType() const { return MiscItemType; }

		virtual std::string GetTypeAsString() const override;

	protected:

		int MiscItemType;
};
