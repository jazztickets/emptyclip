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
#include <config.h>
#include <filesystem.h>
#include <constants.h>
#include <actions.h>
#include <input.h>
#include <sstream>
#include <fstream>
#include <SDL_filesystem.h>

// Globals
_Config Config;

// Initializes the config system
void _Config::Init(const std::string &ConfigFile) {

	// Create config path
	char *PrefPath = SDL_GetPrefPath("", "emptyclip");
	if(PrefPath) {
		ConfigPath = PrefPath;
		SDL_free(PrefPath);
	}
	else {
		throw std::runtime_error("Cannot create config path!");
	}

	this->ConfigFile = ConfigPath + ConfigFile;

	// Load defaults
	SetDefaults();

	// Load config
	Load();
}

// Closes the config system
void _Config::Close() {
}

// Set defaults
void _Config::SetDefaults() {

	Version = 1;
	WindowWidth = DEFAULT_WINDOW_WIDTH;
	WindowHeight = DEFAULT_WINDOW_HEIGHT;
	MSAA = 0;
	Aniso = 0;
	Fullscreen = DEFAULT_FULLSCREEN;
	Vsync = DEFAULT_VSYNC;
	MaxFPS = DEFAULT_MAXFPS;
	AudioEnabled = DEFAULT_AUDIOENABLED;

	SoundVolume = 1.0f;
	MusicVolume = 1.0f;

	LoadDefaultInputBindings();
}

// Load default key bindings
void _Config::LoadDefaultInputBindings() {

	// Clear mappings
	for(int i = 0; i < _Input::INPUT_COUNT; i++)
		Actions.ClearMappings(i);

	Actions.AddInputMap(_Input::KEYBOARD, DEFAULT_KEYUP, _Actions::UP);
	Actions.AddInputMap(_Input::KEYBOARD, DEFAULT_KEYDOWN, _Actions::DOWN);
	Actions.AddInputMap(_Input::KEYBOARD, DEFAULT_KEYLEFT, _Actions::LEFT);
	Actions.AddInputMap(_Input::KEYBOARD, DEFAULT_KEYRIGHT, _Actions::RIGHT);
	Actions.AddInputMap(_Input::KEYBOARD, DEFAULT_KEYSPRINT, _Actions::SPRINT);
	Actions.AddInputMap(_Input::KEYBOARD, DEFAULT_KEYUSE, _Actions::USE);
	Actions.AddInputMap(_Input::KEYBOARD, DEFAULT_KEYINVENTORY, _Actions::INVENTORY);
	Actions.AddInputMap(_Input::MOUSE_BUTTON, DEFAULT_BUTTONFIRE, _Actions::FIRE);
	Actions.AddInputMap(_Input::MOUSE_BUTTON, DEFAULT_BUTTONAIM, _Actions::AIM);
	Actions.AddInputMap(_Input::KEYBOARD, DEFAULT_KEYRELOAD, _Actions::RELOAD);
	Actions.AddInputMap(_Input::KEYBOARD, DEFAULT_KEYWEAPONSWITCH, _Actions::WEAPONSWITCH);
	Actions.AddInputMap(_Input::KEYBOARD, DEFAULT_KEYMEDKIT, _Actions::MEDKIT);
	Actions.AddInputMap(_Input::KEYBOARD, DEFAULT_KEYWEAPON1, _Actions::WEAPON1);
	Actions.AddInputMap(_Input::KEYBOARD, DEFAULT_KEYWEAPON2, _Actions::WEAPON2);
	Actions.AddInputMap(_Input::KEYBOARD, DEFAULT_KEYWEAPON3, _Actions::WEAPON3);
	Actions.AddInputMap(_Input::KEYBOARD, DEFAULT_KEYWEAPON4, _Actions::WEAPON4);
}

// Load the config file
void _Config::Load() {

	// Open file
	std::ifstream In(ConfigFile.c_str());
	if(!In.is_open()) {
		Save();
		return;
	}

	// Read data into map
	Map.clear();
	char Buffer[256];
	while(In) {

		In.getline(Buffer, 256);
		if(In.good()) {
			std::string Line(Buffer);
			std::size_t Pos = Line.find_first_of('=');
			if(Pos != std::string::npos) {
				std::string Field = Line.substr(0, Pos);
				std::string Value = Line.substr(Pos+1, Line.size());

				Map[Field] = Value;
			}
		}
	}
	In.close();

	// Read version
	int ReadVersion = 0;
	GetValue("version", ReadVersion);
	if(ReadVersion != Version) {
		std::rename(ConfigFile.c_str(), (ConfigFile + "." + std::to_string(ReadVersion)).c_str());
		Save();
		return;
	}

	// Read config
	GetValue("window_width", WindowWidth);
	GetValue("window_height", WindowHeight);
	GetValue("fullscreen", Fullscreen);
	GetValue("vsync", Vsync);
	GetValue("max_fps", MaxFPS);
	GetValue("aniso", Aniso);
	GetValue("msaa", MSAA);
	GetValue("audio_enabled", AudioEnabled);
	GetValue("sound_volume", SoundVolume);
	GetValue("music_volume", MusicVolume);

	// Load bindings
	for(int i = 0; i < _Actions::COUNT; i++) {
		std::ostringstream Buffer;
		Buffer << "action_" << i;

		// Get input key/button
		std::string InputString;
		GetValue(Buffer.str(), InputString);

		// Clear out current map
		Actions.ClearAllMappingsForAction(i);

		// Skip empty
		if(!InputString.size())
			continue;

		// Parse input bind
		int InputType, Input;
		char Dummy;
		std::stringstream Stream(InputString);
		Stream >> InputType >> Dummy >> Input;
		Actions.AddInputMap(InputType, Input, i);
	}
}

// Save variables to the config file
void _Config::Save() {

	std::ofstream Out(ConfigFile.c_str());
	if(!Out.is_open()) {
		return;
	}

	// Write variables
	Out << "version=" << Version << std::endl;
	Out << "window_width=" << WindowWidth << std::endl;
	Out << "window_height=" << WindowHeight << std::endl;
	Out << "fullscreen=" << Fullscreen << std::endl;
	Out << "vsync=" << Vsync << std::endl;
	Out << "max_fps=" << MaxFPS << std::endl;
	Out << "msaa=" << MSAA << std::endl;
	Out << "aniso=" << Aniso << std::endl;
	Out << "audio_enabled=" << AudioEnabled << std::endl;
	Out << "sound_volume=" << SoundVolume << std::endl;
	Out << "music_volume=" << MusicVolume << std::endl;

	// Write out input map
	for(int i = 0; i < _Actions::COUNT; i++) {
		Out << "action_" << i << "=";
		for(int j = 0; j < _Input::INPUT_COUNT; j++) {
			int Input = Actions.GetInputForAction(j, i);
			if(Input != -1) {
				Out << j << "_" << Input;
				break;
			}
		}
		Out << std::endl;
	}

	Out.close();
}
