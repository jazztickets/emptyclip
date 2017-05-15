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
#include <objects/particle.h>
#include <objects/templates.h>
#include <particles.h>
#include <graphics.h>
#include <random.h>

// Constructor
_Particle::_Particle(const _ParticleSpawn &Spawn)
:	Type(Spawn.Template->Type),
	Lifetime(Spawn.Template->Lifetime),
	Deleted(false),
	Texture(Spawn.Template->Texture),
	Color(Spawn.Template->Color),
	AlphaSpeed(Spawn.Template->AlphaSpeed),
	PositionZ(Spawn.PositionZ),
	ScaleAspect(Spawn.Template->ScaleAspect) {

	// Random
	this->Rotation = Spawn.RotationAdjust + static_cast<float>(Random.GenerateRange(Spawn.Template->StartDirection[0], Spawn.Template->StartDirection[1]));
	this->Velocity = Vector2(this->Rotation) * Random.GenerateRange(Spawn.Template->VelocityScale[0], Spawn.Template->VelocityScale[1]);
	this->Acceleration = Velocity * Spawn.Template->AccelerationScale;
	this->TurnSpeed = Random.GenerateRange(Spawn.Template->TurnSpeed[0], Spawn.Template->TurnSpeed[1]);
	float Size = Random.GenerateRange(Spawn.Template->Size[0], Spawn.Template->Size[1]);
	if(ScaleAspect >= 1.0f) {
		this->Scale.X = Size;
		this->Scale.Y = Size / ScaleAspect;
	}
	else {
		this->Scale.X = Size * ScaleAspect;
		this->Scale.Y = Size;
	}

	//this->Scale *= Size;
	this->Position = Spawn.Position;
}

// Destructor
_Particle::~_Particle() {
}

// Update
void _Particle::Update(double FrameTime) {
	Position += Velocity;
	Velocity += Acceleration;
	Rotation += TurnSpeed;
	Color.Alpha += AlphaSpeed;
	Lifetime -= FrameTime;

	if(Color.Alpha < 0.0f)
		Color.Alpha = 0.0f;

	if(Lifetime < 0)
		Deleted = true;
}

// Render
void _Particle::Render() {

	if(Texture)
		Graphics.DrawTexture(Position.X, Position.Y, PositionZ, Texture, Color, Rotation, Scale.X, Scale.Y);
}
