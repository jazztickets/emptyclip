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

// A coordinate for the map
struct _Coord {
	_Coord() = default;
	_Coord(int X, int Y) : X(X), Y(Y) { }
	_Coord(const Vector2 &Vector) : X(Vector.X), Y(Vector.Y) { }

	bool operator==(const _Coord &Coord) const { return X == Coord.X && Y == Coord.Y; }
	bool operator!=(const _Coord &Coord) const { return !(X == Coord.X && Y == Coord.Y); }
	void operator+=(const _Coord &Coord) { X += Coord.X; Y += Coord.Y; }
	void operator-=(const _Coord &Coord) { X -= Coord.X; Y -= Coord.Y; }
	_Coord operator+(const _Coord &Coord) const { return _Coord(X + Coord.X, Y + Coord.Y); }
	_Coord operator-(const _Coord &Coord) const { return _Coord(X - Coord.X, Y - Coord.Y); }
	_Coord operator+(const int Value) const { return _Coord(X + Value, Y + Value); }
	_Coord operator-(const int Value) const { return _Coord(X - Value, Y - Value); }
	bool operator>(const _Coord &Coord) const { return X > Coord.X && Y > Coord.Y; }
	bool operator>=(const _Coord &Coord) const { return X >= Coord.X && Y >= Coord.Y; }
	bool operator<(const _Coord &Coord) const { return X < Coord.X && Y < Coord.Y; }
	bool operator<=(const _Coord &Coord) const { return X <= Coord.X && Y <= Coord.Y; }

	int X;
	int Y;
};
