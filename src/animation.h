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
#include <string>
#include <vector>

// Forward Declarations
class _Texture;

// Enumerations
enum PlayType {
	PLAYING,
	STOPPED,
	PAUSED
};

enum RepeatType {
	STOP,
	WRAP,
	BOUNCE
};

// Used for storing information about a template reel
struct _ReelTemplate {
	_ReelTemplate() { }

	std::vector<std::string> TextureFiles;
	float PlaybackSpeed;
	int RepeatMode, StartPosition;
};

// Used for storing information about an animation sequence
struct _Reel {
	std::vector<_Texture *> Textures;
	double PlaybackSpeed;
	RepeatType RepeatMode;
	int StartPosition;
};

// Classes
class _Animation {

	public:

		_Animation();
		_Animation &operator=(const _Animation &Animation);
		~_Animation();

		void Update(double FrameTime);
		void AddReel(const _Reel *Reel) { Reels.push_back(Reel); }
		void ChangeReel(int Index);

		void SetFramePeriod(double Value);
		void SetPlaybackSpeedFactor(double Value) { PlaybackSpeed = Reels[CurrentReel]->PlaybackSpeed * Value; }
		void SetPlayMode(int Mode);
		void SetAllowUpdate(bool Value) { AllowUpdate = Value; }

		int GetPlayMode() const { return PlayMode; }
		int GetCurrentReel() const { return CurrentReel; }
		_Texture *GetCurrentFrame() const;
		_Texture *GetStartPositionFrame() const;
		float GetPlaybackSpeed() const { return PlaybackSpeed; }
		const _Reel *GetReel(int Index) { return Reels[Index]; }

	private:

		std::vector<const _Reel *> Reels;
		int PlayMode;
		int PlayDirection;
		int CurrentReel;
		int Position;
		double Timer;
		double PlaybackSpeed;
		bool AllowUpdate;

};
