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
#include <ui/element.h>
#include <color.h>

// Forward Declarations
class _Texture;

// Classes
class _Image : public _Element {

	public:

		_Image(const std::string &Identifier, _Element *Parent, const _Point &Offset, const _Point &Size, const _Alignment &Alignment, const _Texture *Texture, const _Color &Color, bool Stretch);
		~_Image();

		void SetColor(const _Color &Color) { this->Color = Color; }
		const _Color &GetColor() const { return Color; }

		void SetTexture(const _Texture *Texture);
		const _Texture *GetTexture() const;

		void Render() const;

	private:

		_Color Color;
		const _Texture *Texture;
		bool Stretch;

};
