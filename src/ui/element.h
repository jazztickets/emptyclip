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

// Libraries
#include <ui/ui.h>
#include <string>
#include <vector>

// Forward Declarations
class _Style;
struct _KeyEvent;

// Classes
class _Element {

	public:

		_Element(const std::string &Identifier, _Element *Parent, const _Point &Offset, const _Point &Size, const _Alignment &Alignment, const _Style *Style, bool MaskOutside);
		virtual ~_Element();

		virtual void Update(double FrameTime, const _Point &Mouse);

		virtual void CalculateBounds();
		virtual void Render() const;
		virtual void HandleKeyEvent(const _KeyEvent &KeyEvent);
		virtual void HandleTextEvent(const char *Text);
		virtual void HandleInput(bool Pressed);
		_Element *GetClickedElement();

		void AddChild(_Element *Element) { Children.push_back(Element); Element->SetID(Children.size()-1); }
		std::vector<_Element *> &GetChildren() { return Children; }
		void UpdateChildrenOffset(const _Point &Update) { ChildrenOffset += Update; CalculateChildrenBounds(); }
		virtual void CalculateChildrenBounds();

		void SetDebug(int Debug);
		const _Bounds &GetBounds() const { return Bounds; }

		void SetID(int ID) { this->ID = ID; }
		int GetID() const { return ID; }

		void SetStyle(_Style *Style) { this->Style = Style; }
		const _Style *GetStyle() const { return Style; }

		void SetAlignment(const _Alignment &Alignment) { this->Alignment = Alignment; CalculateBounds(); }
		const _Alignment &GetAlignment() const { return Alignment; }

		void SetIdentifier(const std::string &Identifier) { this->Identifier = Identifier; }
		const std::string &GetIdentifier() const { return Identifier; }

		void SetOffset(const _Point &Offset) { this->Offset = Offset; CalculateBounds(); }
		const _Point &GetOffset() const { return Offset; }

		void SetChildrenOffset(const _Point &ChildrenOffset) { this->ChildrenOffset = ChildrenOffset; CalculateChildrenBounds(); }
		const _Point &GetChildrenOffset() const { return ChildrenOffset; }

		void SetParent(_Element *Parent) { this->Parent = Parent; CalculateBounds(); }
		_Element *GetParent() { return Parent; }

		void SetSize(const _Point &Size) { this->Size = Size; CalculateBounds(); }
		const _Point &GetSize() const { return Size; }

		void SetHitElement(_Element *HitElement) { this->HitElement = HitElement; }
		_Element *GetHitElement() { return HitElement; }

		void SetReleasedElement(_Element *ReleasedElement) { this->ReleasedElement = ReleasedElement; }
		_Element *GetReleasedElement() { return ReleasedElement; }

		void SetUserData(void *UserData) { this->UserData = UserData; }
		void *GetUserData() { return UserData; }

		void SetWidth(int Width) { Size.X = Width; CalculateBounds(); }
		void SetHeight(int Height) { Size.Y = Height; CalculateBounds(); }

		void SetFade(float Fade) { this->Fade = Fade; }
		float GetFade() const { return Fade; }

	protected:

		// Attributes
		std::string Identifier;
		_Element *Parent;
		const _Style *Style;
		_Point ChildrenOffset;
		std::vector<_Element *> Children;
		void *UserData;
		int ID;

		_Point Offset;
		_Point Size;
		_Alignment Alignment;
		_Bounds Bounds;
		bool MaskOutside;

		float Fade;

		// Input
		_Element *HitElement, *PressedElement, *ReleasedElement;

		int Debug;
};
