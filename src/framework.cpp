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
#include <framework.h>
#include <config.h>
#include <graphics.h>
#include <input.h>
#include <actions.h>
#include <audio.h>
#include <state.h>
#include <framelimit.h>
#include <random.h>
#include <stdexcept>
#include <constants.h>
#include <assets.h>
#include <save.h>
#include <states/null.h>
#include <states/convert.h>
#include <states/play.h>
#include <states/editor.h>
#include <SDL.h>

// Global instance
_Framework Framework;

// Initialize
void _Framework::Init(int ArgumentCount, char **Arguments) {
	RequestedState = nullptr;
	Done = false;
	TimeStepAccumulator = 0.0;
	TimeStep = GAME_TIMESTEP;
	FrameworkState = INIT;
	std::string ModPath = "./";
	State = &NullState;

	bool AudioEnabled = Config.AudioEnabled;
	bool Fullscreen = Config.Fullscreen;
	int ScreenWidth = Config.WindowWidth;
	int ScreenHeight = Config.WindowHeight;
	int MSAA = Config.MSAA;
	int Vsync = Config.Vsync;

	// Process arguments
	std::string Token;
	int TokensRemaining;
	for(int i = 1; i < ArgumentCount; i++) {
		Token = std::string(Arguments[i]);
		TokensRemaining = ArgumentCount - i - 1;

		if(Token == "-fullscreen") {
			Fullscreen = true;
		}
		else if(Token == "-window") {
			Fullscreen = false;
		}
		else if(Token == "-w" && TokensRemaining > 0) {
			ScreenWidth = atoi(Arguments[++i]);
		}
		else if(Token == "-h" && TokensRemaining > 0) {
			ScreenHeight = atoi(Arguments[++i]);
		}
		else if(Token == "-vsync" && TokensRemaining > 0) {
			Vsync = atoi(Arguments[++i]);
		}
		else if(Token == "-msaa" && TokensRemaining > 0) {
			MSAA = atoi(Arguments[++i]);
		}
		else if(Token == "-editor") {
			State = &EditorState;
			if(TokensRemaining && Arguments[i+1][0] != '-')
				EditorState.SetMapFilename(Arguments[++i]);
		}
		else if(Token == "-convert" && TokensRemaining > 0) {
			State = &ConvertState;
			ConvertState.SetParam1(Arguments[++i]);
		}
		else if(Token == "-level" && TokensRemaining > 0) {
			PlayState.SetLevel(Arguments[++i]);
			PlayState.SetTestMode(true);

			State = &PlayState;
		}
		else if(Token == "-noaudio") {
			AudioEnabled = false;
		}
		else if(Token == "-mod" && TokensRemaining > 0) {
			ModPath = Arguments[++i];
			if(ModPath.substr(ModPath.size()-1, 1) != "/")
				ModPath += "/";
		}
	}

	// Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		throw std::runtime_error("Failed to initialize SDL");
	}

	// Set up subsystems
	Graphics.Init(ScreenWidth, ScreenHeight, Vsync, MSAA, Fullscreen);
	Audio.Init(AudioEnabled);
	Audio.SetGain(Config.SoundVolume);

	FrameLimit = new _FrameLimit(Config.MaxFPS);
	Timer = SDL_GetPerformanceCounter();
	Random.SetSeed(SDL_GetPerformanceCounter());

	// Load assets
	Assets.Init(ModPath);
	Actions.LoadActionNames();
	Save.LoadSaves();
}

// Shutdown
void _Framework::Close() {

	// Close the current state
	if(State)
		State->Close();

	Assets.Close();
	delete FrameLimit;

	Audio.Close();
	Graphics.Close();
	SDL_Quit();
}

// Update input
void _Framework::Update() {

	// Get frame time
	double FrameTime = (SDL_GetPerformanceCounter() - Timer) / (double)SDL_GetPerformanceFrequency();
	Timer = SDL_GetPerformanceCounter();

	// Get events from SDL
	SDL_PumpEvents();
	Input.Update(FrameTime);

	// Loop through events
	SDL_Event Event;
	while(SDL_PollEvent(&Event)) {
		switch(Event.type){
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				if(!Event.key.repeat) {
					if(State && FrameworkState == UPDATE) {
						_KeyEvent KeyEvent(Event.key.keysym.scancode, Event.type == SDL_KEYDOWN);
						State->KeyEvent(KeyEvent);
						Actions.InputEvent(_Input::KEYBOARD, Event.key.keysym.scancode, Event.type == SDL_KEYDOWN);
					}

					// Toggle fullscreen
					if(Event.type == SDL_KEYDOWN && (Event.key.keysym.mod & KMOD_ALT) && Event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
						//Graphics.ToggleFullScreen();
					}
				}
				else {
					if(State && FrameworkState == UPDATE && Event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE) {
						_KeyEvent KeyEvent(Event.key.keysym.scancode, Event.type == SDL_KEYDOWN);
						State->KeyEvent(KeyEvent);
					}
				}
			break;
			case SDL_TEXTINPUT:
				if(State)
					State->TextEvent(Event.text.text);
			break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				if(State && FrameworkState == UPDATE) {
					_MouseEvent MouseEvent(_Point(Event.motion.x, Event.motion.y), Event.button.button, Event.type == SDL_MOUSEBUTTONDOWN);
					State->MouseEvent(MouseEvent);
					Actions.InputEvent(_Input::MOUSE_BUTTON, Event.button.button, Event.type == SDL_MOUSEBUTTONDOWN);
				}
			break;
			case SDL_MOUSEWHEEL:
				if(State)
					State->MouseWheelEvent(Event.wheel.y);
			break;
			case SDL_QUIT:
				Done = true;
			break;
		}
	}

	switch(FrameworkState) {
		case INIT: {
			if(State) {
				State->Init();
				FrameworkState = UPDATE;
			}
			else
				Done = true;
		} break;
		case UPDATE: {
			TimeStepAccumulator += FrameTime;
			while(TimeStepAccumulator >= TimeStep) {
				State->Update(TimeStep);
				TimeStepAccumulator -= TimeStep;
			}
			State->Render(TimeStepAccumulator / TimeStep);
			//printf("%f\n", TimeStepAccumulator);
		} break;
		case CLOSE: {
			if(State)
				State->Close();

			State = RequestedState;
			FrameworkState = INIT;
		} break;
	}

	Audio.Update(FrameTime);
	Graphics.Flip(FrameTime);
	if(!Config.Vsync)
		FrameLimit->Update();
}

// Change states
void _Framework::ChangeState(_State *RequestedState) {
	this->RequestedState = RequestedState;
	FrameworkState = CLOSE;
}
