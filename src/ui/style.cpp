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
#include <ui/style.h>

// Constructor
_Style::_Style(const std::string &Identifier, bool HasBackgroundColor, bool HasBorderColor, const _Color &BackgroundColor, const _Color &BorderColor, const _Texture *Texture, const _Color &TextureColor, bool Stretch) {
	this->Identifier = Identifier;
	this->HasBackgroundColor = HasBackgroundColor;
	this->HasBorderColor = HasBorderColor;
	this->BackgroundColor = BackgroundColor;
	this->BorderColor = BorderColor;
	this->Texture = Texture;
	this->TextureColor = TextureColor;
	this->Stretch = Stretch;
}

// Destructor
_Style::~_Style() {
}
