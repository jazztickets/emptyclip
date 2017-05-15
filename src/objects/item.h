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
#include <objects/object.h>

// Classes
class _Item : public _Object {

	public:

		_Item();
		virtual ~_Item();

		void Serialize(_Buffer &Buffer) override;
		void Render(double BlendFactor) override;

		int UpdateCount(int Amount) { Count += Amount; return Count; }
		bool CanStack() { return !(Type == _Object::WEAPON || Type == _Object::ARMOR); }

		void SetLevel(int Level) { this->Level = Level; }
		int GetLevel() const { return Level; }

		void SetCount(int Count) { this->Count = Count; }
		int GetCount() const { return Count; }

		void SetIdentifier(const std::string &Identifier) { this->Identifier = Identifier; }
		std::string GetIdentifier() const { return Identifier; }

		const _Texture *GetTexture() const { return Texture; }

	protected:

		std::string Identifier;
		int Level;
		int Count;
		_Texture *Texture;
};
