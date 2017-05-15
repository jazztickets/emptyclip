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
#include <ui/button.h>
#include <ui/label.h>
#include <ui/style.h>
#include <texture.h>
#include <graphics.h>

// Constructor
_Button::_Button(const std::string &Identifier, _Element *Parent, const _Point &Offset, const _Point &Size, const _Alignment &Alignment, const _Style *Style, const _Style *HoverStyle)
:	_Element(Identifier, Parent, Offset, Size, Alignment, Style, false) {

	this->HoverStyle = HoverStyle;
	this->Enabled = false;
}

// Destructor
_Button::~_Button() {
}

// Render the element
void _Button::Render() const {

	if(Style) {
		if(Style->GetTexture()) {
			Graphics.DrawImage(Bounds, Style->GetTexture(), Style->GetTextureColor(), Style->GetStretch());
		}
		else {
			Graphics.DrawRectangle(Bounds, Style->GetBackgroundColor(), true);
			Graphics.DrawRectangle(Bounds, Style->GetBorderColor(), false);
		}
	}

	// Draw hover texture
	if(HoverStyle && (Enabled || HitElement)) {
		if(HoverStyle->GetTexture())
			Graphics.DrawImage(Bounds, HoverStyle->GetTexture(), HoverStyle->GetTextureColor(), Style->GetStretch());
		else {
			if(HoverStyle->GetHasBackgroundColor())
				Graphics.DrawRectangle(Bounds, HoverStyle->GetBackgroundColor(), true);

			if(HoverStyle->GetHasBorderColor())
				Graphics.DrawRectangle(Bounds, HoverStyle->GetBorderColor(), false);
		}
	}

	// Render all children
	for(size_t i = 0; i < Children.size(); i++) {
		Children[i]->Render();
	}
}

// Handle pressed event
void _Button::HandleInput(bool Pressed) {
	if(HitElement) {
	}
}
