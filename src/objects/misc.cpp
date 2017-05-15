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
#include <objects/misc.h>
#include <objects/templates.h>

// Constructor
_MiscItem::_MiscItem(const std::string &Identifier, int Count, const Vector2 &Position, const _MiscItemTemplate *MiscItem, _Texture *Texture) {
	this->Identifier = Identifier;
	this->Count = Count;
	this->Texture = Texture;
	this->Type = _Object::MISCITEM;
	this->Position = Position;

	this->Name = MiscItem->Name;
	this->MiscItemType = MiscItem->Type;
	this->Color = MiscItem->Color;
	this->Level = MiscItem->Level;
}

// Destructor
_MiscItem::~_MiscItem() {
}

// Get type as string
std::string _MiscItem::GetTypeAsString() const {

	switch(MiscItemType) {
		case MISCITEM_MEDKIT:
			return "Medkit";
		break;
		case MISCITEM_OTHER:
			return "Key";
		break;
	}

	return "";
}