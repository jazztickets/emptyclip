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
#include <objects/object.h>
#include <constants.h>

// Constructor
_Object::_Object()
:	Active(true),
	Name(""),
	Type(UNDEFINED),
	Map(nullptr),
	TileChanged(false),
	Radius(0.25f),
	WallState(0),
	Position(ZERO_VECTOR),
	LastPosition(ZERO_VECTOR),
	Direction(0.0, 1.0f),
	Color(COLOR_WHITE),
	Rotation(0.0f),
	Scale(1.0f),
	PositionZ(OBJECT_Z)	{

}

// Destructor
_Object::~_Object() {
}

// Calculates the angle from a slope
void _Object::FacePosition(const Vector2 &Cursor) {

	Direction.X = Cursor.X - Position.X;
	Direction.Y = Cursor.Y - Position.Y;
	if(Direction.X == 0 && Direction.Y == 0.0f)
		Direction.Y = 1.0f;
	Direction.Normalize();

	Rotation = atan2(Direction.Y, Direction.X) * DEGREES_IN_RADIAN + 90.0f;
	if(Rotation < 0.0f)
		Rotation += 360.0f;
}

// Force position of object
void _Object::SetPosition(const Vector2 &Position) {
	this->LastPosition = this->Position = Position;
}