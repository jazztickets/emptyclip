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
#include <color.h>
#include <string>

// Forward Declarations
class _Texture;

// Classes
class _Style {

	public:

		_Style(const std::string &Identifier, bool HasBackgroundColor, bool HasBorderColor, const _Color &BackgroundColor, const _Color &BorderColor, const _Texture *Texture, const _Color &TextureColor, bool Stretch);
		~_Style();

		void SetIdentifier(const std::string &Identifier) { this->Identifier = Identifier; }
		const std::string &GetIdentifier() const { return Identifier; }

		void SetHasBackgroundColor(bool HasBackgroundColor) { this->HasBackgroundColor = HasBackgroundColor; }
		bool GetHasBackgroundColor() const { return HasBackgroundColor; }

		void SetHasBorderColor(bool HasBorderColor) { this->HasBorderColor = HasBorderColor; }
		bool GetHasBorderColor() const { return HasBorderColor; }

		void SetBackgroundColor(const _Color &BackgroundColor) { this->BackgroundColor = BackgroundColor; }
		const _Color &GetBackgroundColor() const { return BackgroundColor; }

		void SetBorderColor(const _Color &BorderColor) { this->BorderColor = BorderColor; }
		const _Color &GetBorderColor() const { return BorderColor; }

		void SetTexture(_Texture *Texture) { this->Texture = Texture; }
		const _Texture *GetTexture() const { return Texture; }

		void SetTextureColor(const _Color &TextureColor) { this->TextureColor = TextureColor; }
		const _Color &GetTextureColor() const { return TextureColor; }

		void SetStretch(bool Stretch) { this->Stretch = Stretch; }
		bool GetStretch() const { return Stretch; }

	private:

		std::string Identifier;

		_Color BackgroundColor, BorderColor;
		bool HasBackgroundColor, HasBorderColor;

		const _Texture *Texture;
		_Color TextureColor;

		bool Stretch;
};
