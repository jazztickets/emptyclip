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

// Point struct
struct _Point {
	_Point() { }
	_Point(int X, int Y) : X(X), Y(Y) { }
	void Clear() { X = Y = 0; }

	void operator+=(const _Point &Point) { X += Point.X; Y += Point.Y; }
	void operator-=(const _Point &Point) { X -= Point.X; Y -= Point.Y; }
	_Point operator+(const _Point &Point) const { return _Point(X + Point.X, Y + Point.Y); }
	_Point operator-(const _Point &Point) const { return _Point(X - Point.X, Y - Point.Y); }
	_Point operator*(const float Multiplier) const { return _Point(X * Multiplier, Y * Multiplier); }
	_Point operator/(const float Divisor) const { return _Point(X / Divisor, Y / Divisor); }
	bool operator>(const _Point &Point) const { return X > Point.X && Y > Point.Y; }
	bool operator>=(const _Point &Point) const { return X >= Point.X && Y >= Point.Y; }
	bool operator<(const _Point &Point) const { return X < Point.X && Y < Point.Y; }
	bool operator<=(const _Point &Point) const { return X <= Point.X && Y <= Point.Y; }

	int X, Y;
};

// Bounds struct
struct _Bounds {
	_Bounds() { }
	_Bounds(const _Point &Start, const _Point &End) : Start(Start), End(End) { }
	void Clear() { Start.Clear(); End.Clear(); }
	_Point GetMidPoint() const { return (Start + End) / 2.0f; }
	_Point GetSize() const { return End - Start; }
	bool PointInside(const _Point &Point) const { return Point >= Start && Point < End; }

	_Point Start;
	_Point End;
};

// Alignment struct
struct _Alignment {

	enum HorizontalAlignment {
		LEFT,
		CENTER,
		RIGHT,
	};

	enum VerticalAlignment {
		TOP,
		MIDDLE,
		BOTTOM,
		BASELINE,
	};

	_Alignment() { }
	_Alignment(int Horizontal, int Vertical) : Horizontal(Horizontal), Vertical(Vertical) { }
	void Clear() { Horizontal = LEFT; Vertical = TOP; }

	int Horizontal, Vertical;
};

const _Alignment LEFT_TOP         = _Alignment(_Alignment::LEFT, _Alignment::TOP);
const _Alignment CENTER_MIDDLE    = _Alignment(_Alignment::CENTER, _Alignment::MIDDLE);
const _Alignment LEFT_BASELINE    = _Alignment(_Alignment::LEFT, _Alignment::BASELINE);
const _Alignment RIGHT_BASELINE   = _Alignment(_Alignment::RIGHT, _Alignment::BASELINE);
const _Alignment CENTER_BASELINE  = _Alignment(_Alignment::CENTER, _Alignment::BASELINE);
