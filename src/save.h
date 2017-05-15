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
#include <vector>
#include <string>

// Forward Declarations
class _Player;

// Classes
class _Save {

	public:

		enum SlotType {
			SLOT_0,
			SLOT_1,
			SLOT_2,
			SLOT_3,
			SLOT_4,
			SLOT_5,
			SLOT_6,
			SLOT_7,
			SLOT_8,
			SLOT_9,
			SLOT_TUTORIAL,
			SLOT_TEST,
			SLOT_COUNT,
		};

		_Save();
		~_Save();

		void CreateNewPlayer(int Slot, const std::string &Name, const std::string &ColorIdentifier);
		void DeletePlayer(int Slot);
		void LoadSaves();

		_Player *GetPlayer(int Slot) { return Players[Slot]; }

	private:

		std::string GetConfigPath(int Slot);

		// Players
		std::vector<_Player *> Players;

};

extern _Save Save;
