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
#include <font.h>
#include <texture.h>
#include <queue>
#include <stdexcept>
#include <graphics.h>

// Get next power of two
inline unsigned int GetNextPowerOf2(unsigned int Value) {
	--Value;
	Value |= Value >> 1;
	Value |= Value >> 2;
	Value |= Value >> 4;
	Value |= Value >> 8;
	Value |= Value >> 16;
	return ++Value;
}

// Struct used when sorting glyphs by height
struct CharacterSortStruct {
	FT_UInt Character;
	int Height;
};

// Comparison operator for priority queue
bool operator>(const CharacterSortStruct &A, const CharacterSortStruct &B) {
	return A.Height < B.Height;
}

// Constructor
_Font::_Font() {
	Texture = nullptr;
}

// Load a font file
_Font::_Font(const std::string &FontFile, int FontSize, int TextureWidth) {
	Texture = nullptr;
	HasKerning = false;
	MaxHeight = 0.0f;

	// Zero out glyphs
	for(int i = 0; i < 256; i++) {
		Glyphs[i].Left = 0.0f;
		Glyphs[i].Top = 0.0f;
		Glyphs[i].Right = 0.0f;
		Glyphs[i].Bottom = 0.0f;
		Glyphs[i].Width = 0.0f;
		Glyphs[i].Height = 0.0f;
		Glyphs[i].Advance = 0.0f;
		Glyphs[i].OffsetX = 0.0f;
		Glyphs[i].OffsetY = 0.0f;
	}

	// Initialize library
	if(FT_Init_FreeType(&Library) != 0) {
		throw std::runtime_error("Error initializing FreeType");
	}

	// Load the font
	if(FT_New_Face(Library, FontFile.c_str(), 0, &Face) != 0) {
		throw std::runtime_error("Error loading font file: " + FontFile);
	}

	// Set font size
	if(FT_Set_Pixel_Sizes(Face, 0, FontSize)) {
		throw std::runtime_error("Error setting pixel size");
	}

	HasKerning = !!FT_HAS_KERNING(Face);
	LoadFlags = FT_LOAD_RENDER;

	// Characters to create
	std::string Characters;
	for(int i = 32; i < 127; i++)
		Characters.push_back(char(i));

	// Sort characters by size
	std::string SortedCharacters;
	SortCharacters(Face, Characters, SortedCharacters);

	// Create the OpenGL texture and populate GlyphUVs
	CreateFontTexture(SortedCharacters, TextureWidth);
}

// Destructor
_Font::~_Font() {

	// Close face
	FT_Done_Face(Face);

	// Close freetype
	FT_Done_FreeType(Library);

	// Free OpenGL texture
	delete Texture;
}

// Sorts characters by vertical size
void _Font::SortCharacters(FT_Face &Face, const std::string &Characters, std::string &SortedCharacters) {

	// Build queue
	std::priority_queue<int, std::vector<CharacterSortStruct>, std::greater<CharacterSortStruct> > CharacterList;
	CharacterSortStruct Character;
	for(size_t i = 0; i < Characters.size(); i++) {

		// Load a character
		FT_Load_Char(Face, Characters[i], LoadFlags);
		FT_GlyphSlot &Glyph = Face->glyph;

		// Add character to the list
		Character.Character = Characters[i];
		Character.Height = Glyph->bitmap.rows;
		CharacterList.push(Character);
	}

	// Build sorted string
	while(!CharacterList.empty()) {
		const CharacterSortStruct &Character = CharacterList.top();
		if(Character.Height > MaxHeight)
			MaxHeight = Character.Height;
		SortedCharacters.push_back(Character.Character);
		CharacterList.pop();
	}
}

// Renders all the glyphs to a texture
void _Font::CreateFontTexture(std::string SortedCharacters, int TextureWidth) {
	int X = 0;
	int Y = 0;
	int SpacingX = 1;
	int SpacingY = 1;
	int MaxRows = 0;

	// Determine Glyph UVs and texture height given a texture width
	for(size_t i = 0; i < SortedCharacters.size(); i++) {

		// Load a character
		FT_Load_Char(Face, SortedCharacters[i], LoadFlags);

		// Get glyph
		FT_GlyphSlot &GlyphSlot = Face->glyph;
		FT_Bitmap *Bitmap = &GlyphSlot->bitmap;

		// Get width and height of glyph
		int Width = Bitmap->width + SpacingX;
		int Rows = Bitmap->rows;

		// Start a new line if no room left
		if(X + Width > TextureWidth) {
			X = 0;
			Y += MaxRows + SpacingY;
			MaxRows = 0;
		}

		// Check max values
		if(Rows > MaxRows)
			MaxRows = Rows;

		// Add character to list
		GlyphStruct Glyph;
		Glyph.Left = (float)X;
		Glyph.Top = (float)Y;
		Glyph.Right = (float)X + Bitmap->width;
		Glyph.Bottom = (float)Y + Bitmap->rows;
		Glyph.Width = (float)Bitmap->width;
		Glyph.Height = (float)Bitmap->rows;
		Glyph.Advance = (float)(GlyphSlot->advance.x >> 6);
		Glyph.OffsetX = (float)GlyphSlot->bitmap_left;
		Glyph.OffsetY = (float)GlyphSlot->bitmap_top;
		Glyphs[(FT_Byte)SortedCharacters[i]] = Glyph;

		// Update draw position
		X += Width;
	}

	// Add last line
	Y += MaxRows;

	// Round to next power of 2
	int TextureHeight = (int)GetNextPowerOf2(Y);

	// Create image buffer
	int TextureSize = TextureWidth * TextureHeight;
	unsigned char *Image = new unsigned char[TextureSize];
	memset(Image, 0, TextureSize);

	// Render each glyph to the texture
	for(size_t i = 0; i < SortedCharacters.size(); i++) {
		GlyphStruct &Glyph = Glyphs[(FT_Byte)SortedCharacters[i]];

		// Load a character
		FT_Load_Char(Face, SortedCharacters[i], LoadFlags);

		// Get glyph
		FT_GlyphSlot &GlyphSlot = Face->glyph;
		FT_Bitmap *Bitmap = &GlyphSlot->bitmap;

		// Write character bitmap data
		for(FT_UInt y = 0; y < Bitmap->rows; y++) {

			int DrawY = (int)Glyph.Top + y;
			for(FT_UInt x = 0; x < Bitmap->width; x++) {
				int DrawX = (int)Glyph.Left + x;
				int Destination = DrawX + DrawY * TextureWidth;
				int Source = x + y * Bitmap->pitch;

				// Copy to texture
				Image[Destination] = Bitmap->buffer[Source];
			}
		}

		// Convert Glyph bounding box to UV coords
		Glyph.Left /= (float)TextureWidth;
		Glyph.Top /= (float)TextureHeight;
		Glyph.Right /= (float)TextureWidth;
		Glyph.Bottom /= (float)TextureHeight;
	}

	// Load texture
	Texture = new _Texture(Image, TextureWidth, TextureHeight, GL_ALPHA8, GL_ALPHA);

	delete[] Image;
}

// Draws a string
void _Font::DrawText(const std::string &Text, float X, float Y, const _Color &Color, const _Alignment &Alignment) const {
	Graphics.SetTextureEnabled(true);
	Graphics.SetColor(Color);
	Graphics.SetTextureID(Texture->GetID());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Adjust for alignment
	_TextBounds TextBounds;
	GetStringDimensions(Text, TextBounds);

	// Handle horizontal alignment
	switch(Alignment.Horizontal) {
		case _Alignment::CENTER:
			X -= TextBounds.Width >> 1;
		break;
		case _Alignment::RIGHT:
			X -= TextBounds.Width;
		break;
	}

	// Handle vertical alignment
	switch(Alignment.Vertical) {
		case _Alignment::TOP:
			Y += TextBounds.AboveBase;
		break;
		case _Alignment::MIDDLE:
			Y += (TextBounds.AboveBase - TextBounds.BelowBase) >> 1;
		break;
		case _Alignment::BOTTOM:
			Y -= TextBounds.BelowBase;
		break;
	}

	// Draw string
	float DrawX, DrawY;
	FT_UInt PreviousGlyphIndex = 0;
	for(size_t i = 0; i < Text.size(); i++) {
		FT_UInt GlyphIndex = FT_Get_Char_Index(Face, Text[i]);

		// Handle kerning
		if(HasKerning && i) {
			FT_Vector Delta;
			FT_Get_Kerning(Face, PreviousGlyphIndex, GlyphIndex, FT_KERNING_DEFAULT, &Delta);
			X += (float)(Delta.x >> 6);
		}
		PreviousGlyphIndex = GlyphIndex;

		// Get glyph data
		const GlyphStruct &Glyph = Glyphs[(FT_Byte)Text[i]];
		DrawX = X + Glyph.OffsetX;
		DrawY = Y - Glyph.OffsetY;

		glBegin(GL_QUADS);

			// Top left
			glTexCoord2f(Glyph.Left, Glyph.Top);
			glVertex2f(DrawX, DrawY);

			// Top right
			glTexCoord2f(Glyph.Right, Glyph.Top);
			glVertex2f(DrawX + Glyph.Width, DrawY);

			// Bottom right
			glTexCoord2f(Glyph.Right, Glyph.Bottom);
			glVertex2f(DrawX + Glyph.Width, DrawY + Glyph.Height);

			// Bottom left
			glTexCoord2f(Glyph.Left, Glyph.Bottom);
			glVertex2f(DrawX, DrawY + Glyph.Height);

		glEnd();

		X += Glyph.Advance;
	}
}

// Draws the font texture
void _Font::DrawFont(float X, float Y) {
	Graphics.SetTextureEnabled(true);
	Graphics.SetTextureID(Texture->GetID());
	Graphics.SetColor(COLOR_WHITE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glBegin(GL_QUADS);

		// Top left
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(X, Y);

		// Top right
		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(X + Texture->GetWidth(), Y);

		// Bottom right
		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(X + Texture->GetWidth(), Y + Texture->GetHeight());

		// Bottom left
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(X, Y + Texture->GetHeight());

	glEnd();
}

// Get width and height of a string
void _Font::GetStringDimensions(const std::string &Text, _TextBounds &TestBounds) const {
	if(Text.size() == 0) {
		TestBounds.Width = 0;
		TestBounds.AboveBase = 0;
		TestBounds.BelowBase = 0;
		return;
	}

	TestBounds.Width = TestBounds.AboveBase = TestBounds.BelowBase = 0;
	const GlyphStruct *Glyph = nullptr;
	for(size_t i = 0; i < Text.size(); i++) {

		// Get glyph data
		Glyph = &Glyphs[(FT_Byte)Text[i]];

		// Update width by advance
		TestBounds.Width += (int)Glyph->Advance;

		// Get number of pixels below baseline
		int BelowBase = (int)(-Glyph->OffsetY + Glyph->Height);
		if(BelowBase > TestBounds.BelowBase)
			TestBounds.BelowBase = BelowBase;

		// Get number of pixels above baseline
		if(Glyph->OffsetY > (int)TestBounds.AboveBase)
			TestBounds.AboveBase = (int)Glyph->OffsetY;
	}

	// Fix last char since it should be using width
	if(Glyph) {
		TestBounds.Width -= (int)Glyph->Advance;
		TestBounds.Width += (int)(Glyph->Width + Glyph->OffsetX);
	}
}

// Break up text into multiple strings based on max width
void _Font::BreakupString(const std::string &Text, float Width, std::vector<std::string> &Strings) const {

	float X = 0;
	FT_UInt PreviousGlyphIndex = 0;
	size_t StartCut = 0;
	size_t LastSpace = std::string::npos;
	for(size_t i = 0; i < Text.size(); i++) {

		// Remember last space position
		if(Text[i] == ' ')
			LastSpace = i;

		FT_UInt GlyphIndex = FT_Get_Char_Index(Face, Text[i]);

		// Handle kerning
		if(HasKerning && i) {
			FT_Vector Delta;
			FT_Get_Kerning(Face, PreviousGlyphIndex, GlyphIndex, FT_KERNING_DEFAULT, &Delta);
			X += (float)(Delta.x >> 6);
		}
		PreviousGlyphIndex = GlyphIndex;

		// Get glyph info
		const GlyphStruct &Glyph = Glyphs[(FT_Byte)Text[i]];
		X += Glyph.Advance;

		// Check for max width
		if(X > Width) {
			size_t Adjust = 0;
			if(LastSpace == std::string::npos)
				LastSpace = i;
			else
				Adjust = 1;

			// Add to list of strings
			Strings.push_back(Text.substr(StartCut, LastSpace - StartCut));
			StartCut = LastSpace+Adjust;
			LastSpace = -1;
			i = StartCut;

			X = 0;
			PreviousGlyphIndex = 0;
		}

	}

	// Add last cut
	Strings.push_back(Text.substr(StartCut, Text.size()));
}
