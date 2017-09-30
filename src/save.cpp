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
#include <save.h>
#include <config.h>
#include <filesystem.h>
#include <objects/player.h>
#include <cstdlib>
#include <sstream>

_Save Save;

// Constructor
_Save::_Save() {
	Players.insert(Players.begin(), SLOT_COUNT, (_Player *)0);
}

// Destructor
_Save::~_Save() {
	for(size_t i = 0; i < Players.size(); i++) {
		delete Players[i];
	}
}

// Get a save path for a slot
std::string _Save::GetConfigPath(int Slot) {
	if(Slot < 0 || Slot >= SLOT_COUNT)
		return "";

	std::stringstream Buffer;
	Buffer << Config.GetConfigPath() << (Slot + 1) << ".save";
	return Buffer.str();
}

// Create new player
void _Save::CreateNewPlayer(int Slot, const std::string &Name, const std::string &ColorIdentifier) {
	if(Slot < 0 || Slot >= SLOT_COUNT)
		return;

	Players[Slot] = new _Player(GetConfigPath(Slot));
	Players[Slot]->SetName(Name);
	Players[Slot]->SetColorIdentifier(ColorIdentifier);

	Players[Slot]->Save();
}

// Deletes a player
void _Save::DeletePlayer(int Slot) {
	if(Slot < 0 || Slot >= SLOT_COUNT)
		return;

	// Build save name
	remove(GetConfigPath(Slot).c_str());

	delete Players[Slot];
	Players[Slot] = NULL;
}

// Load save files
void _Save::LoadSaves() {

	// Clear previous saves
	for(size_t i = 0; i < Players.size(); i++) {
		delete Players[i];
		Players[i] = NULL;
	}

	// Load test files
	try {
		Players[SLOT_TUTORIAL] = new _Player(Config.GetConfigPath() + "tutorial.save");
		Players[SLOT_TEST] = new _Player(Config.GetConfigPath() + "test.save");
		Players[SLOT_TEST]->Load();
	}
	catch(std::exception &Error) {
	}

	// Get directory contents
	std::vector<std::string> Contents;
	_FileSystem::GetFiles(Config.GetConfigPath(), Contents);

	// Load slots with player names
	for(size_t i = 0; i < Contents.size(); i++) {
		size_t Extension = Contents[i].find(".save");
		std::string SlotIndexString = Contents[i].substr(0, Extension);
		int SlotIndex = atoi(SlotIndexString.c_str()) - 1;
		if(SlotIndex >= 0 && SlotIndex <= SLOT_9) {
			try {
				Players[SlotIndex] = new _Player(Config.GetConfigPath() + Contents[i]);
				Players[SlotIndex]->Load();
			}
			catch(std::exception &Error) {
			}
		}
	}
}
