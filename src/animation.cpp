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
#include <animation.h>

// Constructor
_Animation::_Animation()
:	PlayMode(STOPPED),
	PlayDirection(1),
	CurrentReel(0),
	Position(0),
	Timer(0),
	AllowUpdate(false) {

}

// Destructor
_Animation::~_Animation() {
}

// Assignment
_Animation &_Animation::operator=(const _Animation &Animation) {
	Reels = Animation.Reels;
	PlayMode = Animation.PlayMode;
	CurrentReel = Animation.CurrentReel;
	Position = Animation.Position;
	PlayDirection = Animation.PlayDirection;
	Timer = 0;
	PlaybackSpeed = Reels[CurrentReel]->PlaybackSpeed;

	return *this;
}

// Changes animation reels
void _Animation::ChangeReel(int Index) {
	CurrentReel = Index;
	Position = Reels[Index]->StartPosition;
	PlaybackSpeed = Reels[Index]->PlaybackSpeed;
	AllowUpdate = false;
	PlayDirection = 1;

	Timer = 0;
}

// Updates the animation
void _Animation::Update(double FrameTime) {
	Timer += FrameTime;

	if(!AllowUpdate && Timer >= PlaybackSpeed)
		AllowUpdate = true;

	// Update position
	int ReelSize = (int)Reels[CurrentReel]->Textures.size();
	if(ReelSize <= 1) {
		if(AllowUpdate)
			PlayMode = STOPPED;
	}
	else if(AllowUpdate && PlayMode == PLAYING) {
		Position += PlayDirection;

		// If the position goes past the last frame
		if(Position > ReelSize-1) {
			switch(Reels[CurrentReel]->RepeatMode) {
				case STOP:
					Position = ReelSize-1;
					PlayMode = STOPPED;
				break;
				case WRAP:
					Position = 0;
				break;
				case BOUNCE:
					Position = ReelSize-2;
					PlayDirection = -1;
				break;
			}
		}
		else if(Position < 0) {
			switch(Reels[CurrentReel]->RepeatMode) {
				case BOUNCE:
					Position = 1;
					PlayDirection = 1;
				break;
				default:
				break;
			}
		}

		// Update state
		AllowUpdate = false;
		Timer = 0;
	}
}

// Sets the play mode and updates the position if necessary
void _Animation::SetPlayMode(int Mode) {

	// If the last animation was stopped, reset position
	if(PlayMode == STOPPED || Mode == STOPPED)
		Position = Reels[CurrentReel]->StartPosition;

	// If resuming, restart timer
	if((PlayMode == STOPPED || PlayMode == PAUSED) && Mode == PLAYING)
		Timer = 0;

	PlayMode = Mode;
}

// Set playback speed
void _Animation::SetFramePeriod(double Value) {
	PlaybackSpeed = Value / Reels[CurrentReel]->Textures.size();
}

// Return texture of start frame
_Texture *_Animation::GetStartPositionFrame() const {
	return Reels[CurrentReel]->Textures[Reels[CurrentReel]->StartPosition];
}

// Return current frame
_Texture *_Animation::GetCurrentFrame() const {
	return Reels[CurrentReel]->Textures[Position];
}