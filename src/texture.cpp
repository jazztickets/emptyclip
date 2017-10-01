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
#include <texture.h>
#include <SDL_image.h>
#include <stdexcept>

// Constructor
_Texture::_Texture()
:	Name(""),
	Group(0),
	ID(0),
	Width(0),
	Height(0) {

}

// Load from file
_Texture::_Texture(const std::string &FilePath, int Group, bool Repeat, bool Mipmaps) {

	// Open png file
	SDL_Surface *Image = IMG_Load(FilePath.c_str());
	if(!Image) {
		throw std::runtime_error("Error loading image: " + FilePath + " with error: " + IMG_GetError());
	}

	this->Name = FilePath;
	this->Group = Group;
	this->Width = Image->w;
	this->Height = Image->h;

	// Determine OpenGL format
	GLint ColorFormat = GL_RGB;
	if(Image->format->BitsPerPixel == 32)
		ColorFormat = GL_RGBA;

	// Create texture and upload to GPU
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);
	if(Repeat) {
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else {
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}

	if(Mipmaps) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		gluBuild2DMipmaps(GL_TEXTURE_2D, ColorFormat, Width, Height, ColorFormat, GL_UNSIGNED_BYTE, Image->pixels);
	}
	else {
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, ColorFormat, Width, Height, 0, ColorFormat, GL_UNSIGNED_BYTE, Image->pixels);
	}

	// Clean up
	SDL_FreeSurface(Image);
}

// Initialize from buffer
_Texture::_Texture(unsigned char *Data, int Width, int Height, GLint InternalFormat, int Format) {

	// Create texture
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);
	glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, Width, Height, 0, Format, GL_UNSIGNED_BYTE, Data);

	this->Width = Width;
	this->Height = Height;
}

// Constructor
_Texture::~_Texture() {
	if(ID)
		glDeleteTextures(1, &ID);
}
