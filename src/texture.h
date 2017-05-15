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

#include <opengl.h>
#include <string>

// Classes
class _Texture {

	public:

		enum GroupType {
			MAIN,
			EDITOR,
			MAP,
		};

		_Texture();
		_Texture(const std::string &FilePath, int Group, bool Repeat, bool Mipmaps);
		_Texture(unsigned char *Data, int Width, int Height, int InternalFormat, int Format);
		~_Texture();

		const std::string &GetName() const { return Name; }
		int GetGroup() const { return Group; }
		GLuint GetID() const { return ID; }
		int GetWidth() const { return Width; }
		int GetHeight() const { return Height; }

	private:

		// Info
		std::string Name;
		int Group;
		GLuint ID;

		// Dimensions
		int Width;
		int Height;
};
