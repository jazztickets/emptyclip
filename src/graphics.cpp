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
#include <graphics.h>
#include <color.h>
#include <texture.h>
#include <stdexcept>
#include <constants.h>
#include <opengl.h>
#include <ui/element.h>
#include <SDL_mouse.h>

typedef void (APIENTRYP PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRYP PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRYP PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void (APIENTRYP PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint *buffers);
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;

_Graphics Graphics;

// Initialize
void _Graphics::Init(int WindowWidth, int WindowHeight, int Vsync, int MSAA, bool Fullscreen) {
	this->ScreenWidth = WindowWidth;
	this->ScreenHeight = WindowHeight;
	FramesPerSecond = 0;
	FramesPerSecond = 0;
	FrameCount = 0;
	FrameRateTimer = 0;
	TriangleCount = 0;
	Context = 0;
	Window = 0;
	Enabled = true;
	LastTextureID = -1;
	LastColor = COLOR_WHITE;
	LastTextureEnabled = true;


	// Set video flags
	Uint32 VideoFlags = SDL_WINDOW_OPENGL;
	if(Fullscreen) {
		VideoFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

		SDL_DisplayMode DisplayMode;
		if(SDL_GetDesktopDisplayMode(0, &DisplayMode) == 0) {
			this->ScreenWidth = DisplayMode.w;
			this->ScreenHeight = DisplayMode.h;
		}
	}

	// Set root element
	Element = new _Element("screen_element", NULL, _Point(0, 0), _Point(this->ScreenWidth, this->ScreenHeight), _Alignment(0, 0), NULL, false);

	// Set up viewport
	ChangeViewport(this->ScreenWidth, this->ScreenHeight);

	// Set opengl attributes
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	if(MSAA > 0) {
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, MSAA);
	}

	// Set video mode
	Window = SDL_CreateWindow(GAME_WINDOWTITLE.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, this->ScreenWidth, this->ScreenHeight, VideoFlags);
	if(Window == NULL)
		throw std::runtime_error("SDL_CreateWindow failed");

	// Set up opengl context
	Context = SDL_GL_CreateContext(Window);
	if(Context == NULL)
		throw std::runtime_error("SDL_GL_CreateContext failed");

	// Set vsync
	SDL_GL_SetSwapInterval(Vsync);

	// Set up OpenGL
	SetupOpenGL();

	// Setup viewport
	ChangeViewport(this->ScreenWidth, this->ScreenHeight);
}

// Shutdown system
void _Graphics::Close() {
	delete Element;

	if(Context) {
		for(int i = 0; i < VBO_COUNT; i++)
			glDeleteBuffers(1, &VertexBuffer[i]);

		SDL_GL_DeleteContext(Context);
		Context = NULL;
	}

	if(Window) {
		SDL_DestroyWindow(Window);
		Window = NULL;
	}
}

// Change the viewport
void _Graphics::ChangeViewport(int Width, int Height) {
	ViewportWidth = Width;
	ViewportHeight = Height;

	// Calculate aspect ratio
	AspectRatio = (float)ViewportWidth / ViewportHeight;
}

// Toggle fullscreen
void _Graphics::ToggleFullScreen() {
	if(SDL_SetWindowFullscreen(Window, SDL_GetWindowFlags(Window) ^ SDL_WINDOW_FULLSCREEN) != 0) {
		// failed
	}
}

// Sets up OpenGL
void _Graphics::SetupOpenGL() {

	// Load extensions
	glGenBuffers = (PFNGLGENBUFFERSPROC)SDL_GL_GetProcAddress("glGenBuffers");
	glBindBuffer = (PFNGLBINDBUFFERPROC)SDL_GL_GetProcAddress("glBindBuffer");
	glBufferData = (PFNGLBUFFERDATAPROC)SDL_GL_GetProcAddress("glBufferData");
	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteBuffers");

	// Default state
	glEnable(GL_TEXTURE_2D);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glCullFace(GL_BACK);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Build vertex buffers
	BuildVertexBuffers();

	// Clear screen
	ClearScreen();
	Flip(0);
}

// Builds the vertex buffer objects
void _Graphics::BuildVertexBuffers() {

	// Circle
	{
		float Triangles[GRAPHICS_CIRCLE_VERTICES * 2];

		// Get vertices
		for(int i = 0; i < GRAPHICS_CIRCLE_VERTICES; i++) {
			float Radians = ((float)i / GRAPHICS_CIRCLE_VERTICES) * (M_PI * 2);
			Triangles[i * 2] = cos(Radians);
			Triangles[i * 2 + 1] = sin(Radians);
		}

		VertexBuffer[VBO_CIRCLE] = CreateVBO(Triangles, sizeof(Triangles));
	}

	// Textured 2D Quad
	{
		// Vertex data for quad
		float Triangles[] = {
			-0.5f,  0.5f, 0.0f, 1.0f,
			 0.5f,  0.5f, 1.0f, 1.0f,
			-0.5f, -0.5f, 0.0f, 0.0f,
			 0.5f, -0.5f, 1.0f, 0.0f,
		};

		VertexBuffer[VBO_QUAD] = CreateVBO(Triangles, sizeof(Triangles));
	}

	// Cube
	{

		float Triangles[] = {

			// Top
			1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
			1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,

			// Front
			1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,

			// Left
			0.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,

			// Back
			0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
			1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
			1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f,

			// Right
			1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
		};

		VertexBuffer[VBO_CUBE] = CreateVBO(Triangles, sizeof(Triangles));
	}
}

// Create vertex buffer and return id
GLuint _Graphics::CreateVBO(float *Triangles, GLuint Size) {

	GLuint BufferID;
	glGenBuffers(1, &BufferID);
	glBindBuffer(GL_ARRAY_BUFFER, BufferID);
	glBufferData(GL_ARRAY_BUFFER, Size, Triangles, GL_STATIC_DRAW);

	return BufferID;
}

// Enable state for VBO
void _Graphics::EnableVBO(int Type) {

	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer[Type]);

	switch(Type) {
		case VBO_CUBE:
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glEnableClientState(GL_NORMAL_ARRAY);
			glVertexPointer(3, GL_FLOAT, sizeof(float) * 8, 0);
			glTexCoordPointer(2, GL_FLOAT, sizeof(float) * 8, (GLvoid *)(sizeof(float) * 3));
			glNormalPointer(GL_FLOAT, sizeof(float) * 8, (GLvoid *)(sizeof(float) * 5));
		break;
		case VBO_QUAD:
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glVertexPointer(2, GL_FLOAT, sizeof(float) * 4, 0);
			glTexCoordPointer(2, GL_FLOAT, sizeof(float) * 4, (GLvoid *)(sizeof(float) * 2));
		break;
		case VBO_CIRCLE:
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(2, GL_FLOAT, sizeof(float) * 2, 0);
		break;
	}
}

// Disable state for VBO
void _Graphics::DisableVBO(int Type) {

	switch(Type) {
		case VBO_CUBE:
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisableClientState(GL_NORMAL_ARRAY);
		break;
		case VBO_QUAD:
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		break;
		case VBO_CIRCLE:
			glDisableClientState(GL_VERTEX_ARRAY);
		break;
	}
}

// Clears the screen
void _Graphics::ClearScreen() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

// Set up modelview matrix
void _Graphics::Setup3DViewport() {
	glViewport(0, ScreenHeight - ViewportHeight, ViewportWidth, ViewportHeight);
}

// Sets up the projection matrix for drawing 2D objects
void _Graphics::Setup2DProjectionMatrix() {

	// Set viewport
	glViewport(0, 0, ScreenWidth, ScreenHeight);

	// Set projection matrix and frustum
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, ScreenWidth, ScreenHeight, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
}

// Fade the screen
void _Graphics::FadeScreen(float Amount) {
	Graphics.DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), _Color(0.0f, 0.0f, 0.0f, Amount), true);
}

// Draw centered image in screen space
void _Graphics::DrawImage(const _Point &CenterPoint, const _Texture *Texture, const _Color &Color) {
	SetTextureEnabled(true);
	SetTextureID(Texture->GetID());
	SetColor(Color);

	float HalfWidth = Texture->GetWidth() / 2.0f;
	float HalfHeight = Texture->GetHeight() / 2.0f;

	glBegin(GL_TRIANGLE_STRIP);

		// Top right
		glTexCoord2f(1, 0.0f);
		glVertex2f(CenterPoint.X + HalfWidth, CenterPoint.Y - HalfHeight);

		// Top left
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(CenterPoint.X - HalfWidth, CenterPoint.Y - HalfHeight);

		// Bottom right
		glTexCoord2f(1, 1);
		glVertex2f(CenterPoint.X + HalfWidth, CenterPoint.Y + HalfHeight);

		// Bottom left
		glTexCoord2f(0.0f, 1);
		glVertex2f(CenterPoint.X - HalfWidth, CenterPoint.Y + HalfHeight);

	glEnd();

	TriangleCount += 2;
}

// Draw image in screen space
void _Graphics::DrawImage(const _Bounds &Bounds, const _Texture *Texture, const _Color &Color, bool Stretch) {
	SetTextureEnabled(true);
	SetColor(Color);
	SetTextureID(Texture->GetID());

	// Get s and t
	float S, T;
	if(Stretch) {
		S = T = 1;
	}
	else {
		S = (Bounds.End.X - Bounds.Start.X) / (float)(Texture->GetWidth());
		T = (Bounds.End.Y - Bounds.Start.Y) / (float)(Texture->GetHeight());
	}

	glBegin(GL_TRIANGLE_STRIP);

		// Top right
		glTexCoord2f(S, 0.0f);
		glVertex2f(Bounds.End.X, Bounds.Start.Y);

		// Top left
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(Bounds.Start.X, Bounds.Start.Y);

		// Bottom right
		glTexCoord2f(S, T);
		glVertex2f(Bounds.End.X, Bounds.End.Y);

		// Bottom left
		glTexCoord2f(0.0f, T);
		glVertex2f(Bounds.Start.X, Bounds.End.Y);

	glEnd();

	TriangleCount += 2;
}

// Draw rectangle in screen space
void _Graphics::DrawRectangle(const _Bounds &Bounds, const _Color &Color, bool Filled) {
	SetTextureEnabled(false);

	// Set alpha
	SetColor(Color);

	if(Filled)
		glBegin(GL_QUADS);
	else
		glBegin(GL_LINE_LOOP);

	// Top left
	glVertex2f(Bounds.Start.X+1, Bounds.Start.Y);

	// Top right
	glVertex2f(Bounds.End.X, Bounds.Start.Y);

	// Bottom right
	glVertex2f(Bounds.End.X, Bounds.End.Y-1);

	// Bottom left
	glVertex2f(Bounds.Start.X+1, Bounds.End.Y-1);

	glEnd();

	TriangleCount += 2;
}

// Draw stencil mask
void _Graphics::DrawMask(const _Bounds &Bounds) {

	// Enable stencil
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	glStencilMask(0x01);

	// Write 1 to stencil buffer
	glStencilFunc(GL_ALWAYS, 0x01, 0x01);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	glBegin(GL_TRIANGLE_STRIP);
		glVertex2f(Bounds.End.X, Bounds.Start.Y);
		glVertex2f(Bounds.Start.X, Bounds.Start.Y);
		glVertex2f(Bounds.End.X, Bounds.End.Y);
		glVertex2f(Bounds.Start.X, Bounds.End.Y);
	glEnd();

	// Then draw element only where stencil is 1
	glStencilFunc(GL_EQUAL, 0x01, 0x01);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	TriangleCount += 2;
}

// Draw 3d sprite
void _Graphics::DrawTexture(float X, float Y, float Z, const _Texture *Texture, const _Color &Color, float Rotation, float ScaleX, float ScaleY) {
	SetTextureEnabled(true);
	SetColor(Color);
	SetTextureID(Texture->GetID());

	glPushMatrix();

		// Apply translation, rotation, and scale transforms
		glTranslatef(X, Y, Z);
		if(Rotation != 0.0f)
			glRotatef(Rotation, 0.0f, 0.0f, 1.0f);

		// Set scale
		glScalef(ScaleX, ScaleY, 1.0f);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glPopMatrix();

	TriangleCount += 2;
}

// Draw light
void _Graphics::DrawLight(const Vector2 &Position, const _Texture *Texture, const _Color &Color, float Scale) {
	SetTextureEnabled(true);
	SetTextureID(Texture->GetID());

	glPushMatrix();

		// Apply translation, rotation, and scale transforms
		glTranslatef(Position.X, Position.Y, 0.0f);

		glScalef(Scale, Scale, 1.0f);

		SetColor(Color);

		glBegin(GL_TRIANGLE_STRIP);
		glNormal3f(0.0f, 0.0f, 1.0f);

		// Top right
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(-1.0f, 1.0f);

		// Top left
		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(1.0f, 1.0f);

		// Bottom light
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(-1.0f, -1.0f);

		// Bottom left
		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(1.0f, -1.0f);

	glEnd();

	glPopMatrix();

	TriangleCount += 2;
}

// Draw 3d wall
void _Graphics::DrawCube(float StartX, float StartY, float StartZ, float ScaleX, float ScaleY, float ScaleZ, const _Texture *Texture) {
	SetTextureEnabled(true);
	SetColor(COLOR_WHITE);
	SetTextureID(Texture->GetID());

	glEnable(GL_CULL_FACE);

	glPushMatrix();

		// Position cube
		glTranslatef(StartX, StartY, StartZ);
		glScalef(ScaleX, ScaleY, ScaleZ);

		// Change texture
		glMatrixMode(GL_TEXTURE);

		// Draw top
		glScalef(ScaleX, ScaleY, 1);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glLoadIdentity();

		// Draw front
		glScalef(ScaleX, ScaleZ, 1);
		glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
		glLoadIdentity();

		// Draw left
		glScalef(ScaleY, ScaleZ, 1);
		glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);
		glLoadIdentity();

		// Draw back
		glScalef(ScaleX, ScaleZ, 1);
		glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);
		glLoadIdentity();

		// Draw right
		glScalef(ScaleY, ScaleZ, 1);
		glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);
		glLoadIdentity();

		glMatrixMode(GL_MODELVIEW);

	glPopMatrix();

	glDisable(GL_CULL_FACE);

	TriangleCount += 2*5;
}

// Draw double-sided flat wall
void _Graphics::DrawWall(float StartX, float StartY, float StartZ, float ScaleX, float ScaleY, float ScaleZ, float Rotation, const _Texture *Texture) {
	SetTextureEnabled(true);
	SetTextureID(Texture->GetID());
	SetColor(COLOR_WHITE);

	glPushMatrix();

	if(Rotation == 0) {

		glTranslatef(StartX, StartY + 0.5f, StartZ);
		glScalef(ScaleX, ScaleY, ScaleZ);

		glMatrixMode(GL_TEXTURE);
		glScalef(ScaleX, ScaleZ, 1);
		glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);
	}
	else {

		glTranslatef(StartX + 0.5f, StartY, StartZ);
		glScalef(ScaleX, ScaleY, ScaleZ);

		glMatrixMode(GL_TEXTURE);
		glScalef(ScaleY, ScaleZ, 1);
		glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);
	}

	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	glPopMatrix();

	TriangleCount += 2;
}

// Draw quad with repeated textures
void _Graphics::DrawRepeatable(float StartX, float StartY, float StartZ, float EndX, float EndY, float EndZ, const _Texture *Texture, float Rotation, float ScaleX) {
	SetTextureEnabled(true);
	SetTextureID(Texture->GetID());
	SetColor(COLOR_WHITE);

	// Get textureID and properties
	float Width = EndX - StartX;
	float Height = EndY - StartY;

	// Set texture mode
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();

		// Rotate the texture
		glScalef(ScaleX, 1.0f, 1.0f);
		glRotatef(Rotation, 0.0f, 0.0f, -1.0f);

		glBegin(GL_TRIANGLE_STRIP);
			glNormal3f(0.0f, 0.0f, 1.0f);

			// Top right
			glTexCoord2f(Width, 0.0f);
			glVertex3f(EndX, StartY, StartZ);

			// Top left
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(StartX, StartY, StartZ);

			// Bottom right
			glTexCoord2f(Width, Height);
			glVertex3f(EndX, EndY, EndZ);

			// Bottom left
			glTexCoord2f(0.0f, Height);
			glVertex3f(StartX, EndY, EndZ);

		glEnd();

	glPopMatrix();

	// Restore modelview matrix
	glMatrixMode(GL_MODELVIEW);

	TriangleCount += 2;
}

// Draw rectangle in 3d space
void _Graphics::DrawRectangle(float StartX, float StartY, float EndX, float EndY, const _Color &Color, bool Filled) {
	SetTextureEnabled(false);
	SetColor(Color);

	if(Filled)
		glBegin(GL_QUADS);
	else
		glBegin(GL_LINE_LOOP);

	// Top left
	glVertex2f(StartX, StartY);

	// Top right
	glVertex2f(EndX, StartY);

	// Bottom right
	glVertex2f(EndX, EndY);

	// Bottom left
	glVertex2f(StartX, EndY);

	glEnd();

	TriangleCount += 2;
}

// Draws line
void _Graphics::DrawLine(float StartX, float StartY, float EndX, float EndY, const _Color &Color, float Z) {
	SetTextureEnabled(false);

	glPushMatrix();

		glTranslatef(0.0f, 0.0f, Z);

		// Set color
		SetColor(Color);

		glBegin(GL_LINES);

			glVertex2f(StartX, StartY);
			glVertex2f(EndX, EndY);

		glEnd();

	glPopMatrix();
}

// Draw circle
void _Graphics::DrawCircle(float X, float Y, float Z, float Radius, const _Color &Color) {
	SetTextureEnabled(false);
	SetColor(Color);

	glPushMatrix();

		// Apply translation and scale transforms
		glTranslatef(X, Y, Z);
		glScalef(Radius, Radius, 0.0f);

		glDrawArrays(GL_LINE_LOOP, 0, GRAPHICS_CIRCLE_VERTICES);

	glPopMatrix();
}

// Draws the frame
void _Graphics::Flip(double FrameTime) {
	if(!Enabled)
		return;

	TriangleCount = 0;

	// Swap buffers
	SDL_GL_SwapWindow(Window);

	// Clear screen
	ClearScreen();

	// Update frame counter
	FrameCount++;
	FrameRateTimer += FrameTime;
	if(FrameRateTimer >= 1.0) {
		FramesPerSecond = FrameCount;
		FrameCount = 0;
		FrameRateTimer -= 1.0;
	}
}

// Set opengl color
void _Graphics::SetColor(const _Color &Color) {
	if(Color != LastColor) {
		glColor4f(Color.Red, Color.Green, Color.Blue, Color.Alpha);
		LastColor = Color;
	}
}

// Enable/disable textures
void _Graphics::SetTextureEnabled(bool Value) {
	if(Value != LastTextureEnabled) {
		if(Value)
			glEnable(GL_TEXTURE_2D);
		else
			glDisable(GL_TEXTURE_2D);

		LastTextureEnabled = Value;
	}
}

// Set texture id
void _Graphics::SetTextureID(GLuint TextureID) {
	if(TextureID != LastTextureID) {
		glBindTexture(GL_TEXTURE_2D, TextureID);
		LastTextureID = TextureID;
	}
}

_Element *_Graphics::GetElement() { return Element; }
void _Graphics::SetDepthMask(bool Value) { glDepthMask(Value); }
void _Graphics::EnableStencilTest() { glEnable(GL_STENCIL_TEST); }
void _Graphics::DisableStencilTest() { glDisable(GL_STENCIL_TEST); }
void _Graphics::EnableDepthTest() { glEnable(GL_DEPTH_TEST); }
void _Graphics::DisableDepthTest() { glDisable(GL_DEPTH_TEST); }
void _Graphics::EnableParticleBlending() { glBlendFunc(GL_SRC_ALPHA, 1); }
void _Graphics::DisableParticleBlending() { glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); }
void _Graphics::ShowCursor(bool Show) { SDL_ShowCursor(Show); }