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

struct _Color {

	_Color() { }
	_Color(float Red, float Green, float Blue)
		:   Red(Red),
		    Green(Green),
		    Blue(Blue),
		    Alpha(1.0f) { }
	_Color(float Red, float Green, float Blue, float Alpha)
		:   Red(Red),
		    Green(Green),
		    Blue(Blue),
		    Alpha(Alpha) { }

	_Color operator+(const _Color &Color) const {
		return _Color(Red + Color.Red, Green + Color.Green, Blue + Color.Blue, Alpha + Color.Alpha);
	}

	_Color operator*(const float &Value) const {
		return _Color(Red * Value, Green * Value, Blue * Value, Alpha * Value);
	}

	bool operator!=(const _Color &Color) const {
		return !(Red == Color.Red && Green == Color.Green && Blue == Color.Blue && Alpha == Color.Alpha);
	}

	float Red, Green, Blue, Alpha;
};

const _Color COLOR_WHITE    = _Color(1.0f, 1.0f, 1.0f, 1.0f);
const _Color COLOR_TWHITE   = _Color(1.0f, 1.0f, 1.0f, 0.5f);
const _Color COLOR_DARK     = _Color(0.3f, 0.3f, 0.3f, 1.0f);
const _Color COLOR_TGRAY    = _Color(1.0f, 1.0f, 1.0f, 0.2f);
const _Color COLOR_RED      = _Color(1.0f, 0.0f, 0.0f, 1.0f);
const _Color COLOR_GREEN    = _Color(0.0f, 1.0f, 0.0f, 1.0f);
const _Color COLOR_BLUE     = _Color(0.0f, 0.0f, 1.0f, 1.0f);
const _Color COLOR_YELLOW   = _Color(1.0f, 1.0f, 0.0f, 1.0f);
const _Color COLOR_MAGENTA  = _Color(1.0f, 0.0f, 1.0f, 1.0f);
const _Color COLOR_CYAN     = _Color(0.0f, 1.0f, 1.0f, 1.0f);
