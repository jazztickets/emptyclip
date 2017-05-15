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
#include <assets.h>
#include <font.h>
#include <texture.h>
#include <audio.h>
#include <random.h>
#include <utils.h>
#include <animation.h>
#include <ui/style.h>
#include <ui/element.h>
#include <ui/label.h>
#include <ui/button.h>
#include <ui/image.h>
#include <ui/textbox.h>
#include <objects/monster.h>
#include <objects/particle.h>
#include <objects/player.h>
#include <objects/weapon.h>
#include <objects/armor.h>
#include <objects/misc.h>
#include <objects/upgrade.h>
#include <objects/ammo.h>
#include <constants.h>
#include <stdexcept>

_Assets Assets;

// Initialize
void _Assets::Init(const std::string &AssetPath) {
	this->AssetPath = AssetPath;

	LoadStringTable(ASSETS_STRINGS);
	LoadLevels();
	LoadSkills();
	LoadFonts(ASSETS_FONTDATA);
	LoadTextures(ASSETS_TEXTURES_MAIN);
	LoadTextures(ASSETS_TEXTURES_MAP);
	LoadTextures(ASSETS_TEXTURES_EDITOR);
	LoadColorTable(ASSETS_COLORS);
	LoadSamples(ASSETS_SAMPLEDATA, ASSETS_SAMPLES);
	LoadAttackSampleTable(ASSETS_ATTACK_SAMPLES);
	LoadParticleTable(ASSETS_PARTICLES);
	LoadWeaponParticles(ASSETS_WEAPONPARTICLES);
	LoadStyles(ASSETS_STYLES);
	LoadElements(ASSETS_ELEMENTS);
	LoadImages(ASSETS_IMAGES);
	LoadButtons(ASSETS_BUTTONS);
	LoadTextBoxes(ASSETS_TEXTBOXES);
	LoadLabels(ASSETS_LABELS);
	LoadReelTable(ASSETS_REELS);
	LoadAnimationTable(ASSETS_ANIMATIONS);
	LoadMiscItemTable(ASSETS_MISCITEMS);
	LoadUpgradeTable(ASSETS_UPGRADES);
	LoadAmmoTable(ASSETS_AMMO);
	LoadWeaponTable(ASSETS_WEAPONS);
	LoadArmorTable(ASSETS_ARMOR);
	LoadItemDropTable(ASSETS_ITEMDROPDATA);
	LoadMonsterTable(ASSETS_MONSTERS);

	LoadAnimation("player_torso", ASSETS_PLAYERTEXTURES);
	LoadAnimation("player_legs", ASSETS_PLAYERTEXTURES);

	BlankWeaponParticle = _WeaponParticleTemplate();
}

// Shutdown
void _Assets::Close() {

	UnloadTextures();
	UnloadMonsterSet();
	UnloadAnimation("player_torso");
	UnloadAnimation("player_legs");
	UnloadStyles();
	UnloadElements();
	UnloadFonts();
}

// Loads the strings
void _Assets::LoadStringTable(const std::string &Filename) {

	std::string Identifier, Text;

	// Load file
	std::ifstream InputFile((AssetPath + Filename).c_str(), std::ios::in);
	if(!InputFile) {
		throw std::runtime_error("Error loading: " + Filename);
	}

	// Ignore the first line
	InputFile.ignore(1024, '\n');

	// Read the file
	while(!InputFile.eof() && InputFile.peek() != EOF) {

		Identifier = GetTSVText(InputFile);
		Text = GetTSVText(InputFile);

		// Check for duplicates
		if(IsStringLoaded(Identifier)) {
			throw std::runtime_error("LoadStringTable - Duplicate entry: " + Identifier);
		}

		StringTable.insert(make_pair(Identifier, Text));
	}
	InputFile.close();
}

// Loads the fonts
void _Assets::LoadFonts(const std::string &Filename) {

	// Load file
	std::ifstream InputFile((AssetPath + Filename).c_str(), std::ios::in);
	if(!InputFile) {
		throw std::runtime_error("Error loading: " + Filename);
	}

	// Ignore the first line
	InputFile.ignore(1024, '\n');

	// Read the file
	while(!InputFile.eof() && InputFile.peek() != EOF) {
		std::string Identifier = GetTSVText(InputFile);
		std::string FontFile = GetTSVText(InputFile);

		int Size;
		InputFile >> Size;

		InputFile.ignore(1024, '\n');

		// Load font
		_Font *Font = new _Font(AssetPath + ASSETS_FONTS + FontFile, Size);

		// Check for duplicates
		if(IsFontLoaded(Identifier)) {
			throw std::runtime_error("LoadFonts - Duplicate entry: " + Identifier);
		}

		Fonts.insert(make_pair(Identifier, Font));
	}

	InputFile.close();
}

// Loads the level table
void _Assets::LoadLevels() {
	LevelStruct Level;

	// Load file
	std::ifstream InputFile((AssetPath + ASSETS_LEVELS).c_str(), std::ios::in);
	if(!InputFile) {
		throw std::runtime_error("Error loading: " + ASSETS_LEVELS);
	}

	Levels.clear();

	// Load the data
	InputFile.ignore(1024, '\n');
	for(int i = 0; i < GAME_MAX_LEVEL; i++) {
		if(InputFile.eof()) {
			throw std::runtime_error("LoadLevels - Premature end of file");
		}

		InputFile >> Level.Experience >> Level.HealthBonus >> Level.DefenseBonus >> Level.SkillPoints;

		Levels.push_back(Level);
	}
}

// Loads the skill table
void _Assets::LoadSkills() {
	SkillStruct Skill;

	// Load file
	std::ifstream InputFile((AssetPath + ASSETS_SKILLS).c_str(), std::ios::in);
	if(!InputFile) {
		throw std::runtime_error("LoadSkills: Cannot open " + ASSETS_SKILLS);
	}

	Skills.clear();

	// Load the data
	InputFile.ignore(1024, '\n');
	for(int i = 0; i < GAME_SKILLLEVELS+1; i++) {
		if(InputFile.eof()) {
			throw std::runtime_error("Premature end of file" + ASSETS_SKILLS);
		}

		for(int i = 0; i < SKILL_COUNT; i++)
			InputFile >> Skill.Data[i];

		Skills.push_back(Skill);
	}
}

// Loads the color table
void _Assets::LoadColorTable(const std::string &Filename) {
	_Color Color;
	std::string Identifier;

	// Load file
	std::ifstream InputFile((AssetPath + Filename).c_str(), std::ios::in);
	if(!InputFile) {
		throw std::runtime_error("Error loading: " + Filename);
	}

	// Remove previous data
	UnloadColorTable();

	// Read the file
	InputFile.ignore(1024, '\n');
	while(!InputFile.eof() && InputFile.peek() != EOF) {

		Identifier = GetTSVText(InputFile);
		InputFile >> Color.Red >> Color.Green >> Color.Blue >> Color.Alpha;
		InputFile.ignore(1024, '\n');

		// Check for duplicates
		if(IsColorLoaded(Identifier)) {
			throw std::runtime_error(std::string(__FUNCTION__) + " - Duplicate entry: " + Identifier);
		}

		ColorTable.insert(make_pair(Identifier, Color));
	}
	InputFile.close();
}

// Loads the reels table
void _Assets::LoadReelTable(const std::string &Filename) {
	_ReelTemplate ReelTemplate;
	std::string TextureFile, Identifier;

	// Load file
	std::ifstream InputFile((AssetPath + Filename).c_str(), std::ios::in);
	if(!InputFile) {
		throw std::runtime_error("Error loading: " + Filename);
	}

	// Remove previous data
	UnloadReelTable();

	// Read the file
	InputFile.ignore(1024, '\n');
	while(!InputFile.eof() && InputFile.peek() != EOF) {

		Identifier = GetTSVText(InputFile);
		InputFile >> ReelTemplate.PlaybackSpeed >> ReelTemplate.RepeatMode >> ReelTemplate.StartPosition;
		ReelTemplate.TextureFiles.clear();

		// Get textures
		bool EndOfLine = false;
		while(!EndOfLine && InputFile.peek() != EOF) {
			TextureFile = GetTSVText(InputFile, &EndOfLine);

			if(TextureFile != "")
				ReelTemplate.TextureFiles.push_back(TextureFile);
		}

		// Check for duplicates
		if(IsReelLoaded(Identifier)) {
			throw std::runtime_error(std::string(__FUNCTION__) + " - Duplicate entry: " + Identifier);
		}

		ReelTable.insert(make_pair(Identifier, ReelTemplate));
	}
	InputFile.close();
}

// Loads the animation table
void _Assets::LoadAnimationTable(const std::string &Filename) {
	AnimationTemplateStruct AnimationTemplate;
	std::string Identifier, ReelIdentifier;

	// Load file
	std::ifstream InputFile((AssetPath + Filename).c_str(), std::ios::in);
	if(!InputFile) {
		throw std::runtime_error("Error loading: " + Filename);
	}

	// Remove previous data
	UnloadAnimationTable();

	// Read the file
	InputFile.ignore(1024, '\n');
	while(!InputFile.eof() && InputFile.peek() != EOF) {

		Identifier = GetTSVText(InputFile);
		AnimationTemplate.Identifiers.clear();

		// Get textures
		bool EndOfLine = false;
		while(!EndOfLine && InputFile.peek() != EOF) {
			ReelIdentifier = GetTSVText(InputFile, &EndOfLine);

			if(ReelIdentifier != "")
				AnimationTemplate.Identifiers.push_back(ReelIdentifier);
		}

		// Check for duplicates
		if(IsAnimationLoaded(Identifier)) {
			throw std::runtime_error(std::string(__FUNCTION__) + " - Duplicate entry: " + Identifier);
		}

		AnimationTable.insert(make_pair(Identifier, AnimationTemplate));
	}

	InputFile.close();
}

// Load textures
void _Assets::LoadTextures(const std::string &Filename) {

	// Load file
	std::ifstream InputFile((AssetPath + Filename).c_str(), std::ios::in);
	if(!InputFile) {
		throw std::runtime_error("Error loading: " + Filename);
	}

	// Read the file
	InputFile.ignore(1024, '\n');
	while(!InputFile.eof() && InputFile.peek() != EOF) {

		std::string Identifier = GetTSVText(InputFile);
		std::string TextureFile = GetTSVText(InputFile);
		int Group;
		bool Repeat, MipMaps;
		InputFile >> Group >> Repeat >> MipMaps;
		InputFile.ignore(1024, '\n');

		// Load texture
		std::string Path = AssetPath + ASSETS_TEXTURE_PATH + TextureFile;
		_Texture *Texture = new _Texture(Path, Group, Repeat, MipMaps);
		if(!Texture) {
			throw std::runtime_error(std::string(__FUNCTION__) + " - Error loading: " + Path);
		}

		// Check for duplicates
		if(IsTextureLoaded(Identifier)) {
			throw std::runtime_error(std::string(__FUNCTION__) + " - Duplicate entry: " + Identifier);
		}

		Textures.insert(make_pair(Identifier, Texture));
	}

	InputFile.close();
}

// Loads the sample table
void _Assets::LoadSamples(const std::string &Filename, const std::string &SamplePath) {
	std::string Identifier, SampleFile;
	float Volume;
	int Limit;

	// Load file
	std::ifstream InputFile((AssetPath + Filename).c_str(), std::ios::in);
	if(!InputFile) {
		throw std::runtime_error("Error loading: " + Filename);
	}

	// Read the file
	InputFile.ignore(1024, '\n');
	while(!InputFile.eof() && InputFile.peek() != EOF) {

		Identifier = GetTSVText(InputFile);
		SampleFile = GetTSVText(InputFile);
		InputFile >> Volume >> Limit;
		InputFile.ignore(1024, '\n');

		// Load sample file
		std::string Path = AssetPath + ASSETS_SAMPLES + SampleFile;
		if(!Audio.LoadBuffer(Identifier, Path, Volume, Limit)) {
			throw std::runtime_error("Error loading: " + Path);
		}
	}

	InputFile.close();
}

// Loads the attack samples table
void _Assets::LoadAttackSampleTable(const std::string &Filename) {
	AttackSampleTemplateStruct SampleTemplate;
	std::string Identifier;

	// Load file
	std::ifstream InputFile((AssetPath + Filename).c_str(), std::ios::in);
	if(!InputFile) {
		throw std::runtime_error("Error loading: " + Filename);
	}

	// Remove previous data
	UnloadAttackSampleTable();

	// Ignore the first line
	InputFile.ignore(1024, '\n');

	while(!InputFile.eof() && InputFile.peek() != EOF) {
		Identifier = GetTSVText(InputFile);
		for(int i = 0; i < SAMPLE_TYPES; i++)
			SampleTemplate.Samples[i] = GetTSVText(InputFile);

		// Check for duplicates
		if(IsAttackSampleLoaded(Identifier)) {
			throw std::runtime_error(std::string(__FUNCTION__) + " - Duplicate entry: " + Identifier);
		}

		AttackSampleTable.insert(make_pair(Identifier, SampleTemplate));
	}

	InputFile.close();
}

// Loads the particle table
void _Assets::LoadParticleTable(const std::string &Filename) {
	_ParticleTemplate Particle;
	std::string Identifier;

	// Load file
	std::ifstream InputFile((AssetPath + Filename).c_str(), std::ios::in);
	if(!InputFile) {
		throw std::runtime_error("Error loading: " + Filename);
	}

	// Remove previous data
	UnloadParticleTable();

	// Read the file
	InputFile.ignore(1024, '\n');
	while(!InputFile.eof() && InputFile.peek() != EOF) {

		Identifier = GetTSVText(InputFile);
		std::string TextureIdentifier = GetTSVText(InputFile);
		std::string ColorIdentifier = GetTSVText(InputFile);

		InputFile 	>> Particle.Type >> Particle.Count >> Particle.Lifetime >> Particle.StartDirection[0] >> Particle.StartDirection[1] >> Particle.TurnSpeed[0]
					>> Particle.TurnSpeed[1] >> Particle.VelocityScale[0] >> Particle.VelocityScale[1] >> Particle.AccelerationScale
					>> Particle.Size[0] >> Particle.Size[1] >> Particle.ScaleAspect >> Particle.AlphaSpeed;
		InputFile.ignore(1024, '\n');

		// Check for duplicates
		if(IsParticleLoaded(Identifier)) {
			throw std::runtime_error(std::string(__FUNCTION__) + " - Duplicate entry: " + Identifier);
		}

		// Get texture
		Particle.Texture = Assets.GetTexture(TextureIdentifier);
		if(!Particle.Texture)
			throw std::runtime_error("Unable to find texture: " + TextureIdentifier);

		// Set color
		Particle.Color = GetColor(ColorIdentifier);

		ParticleTable.insert(make_pair(Identifier, Particle));
	}

	InputFile.close();
}

// Loads the weapon particles
void _Assets::LoadWeaponParticles(const std::string &Filename) {
	_WeaponParticleTemplate WeaponParticle;
	std::string Identifier, ParticleIdentifier;

	// Load file
	std::ifstream InputFile((AssetPath + Filename).c_str(), std::ios::in);
	if(!InputFile) {
		throw std::runtime_error("Error loading: " + Filename);
	}

	// Remove previous data
	UnloadWeaponParticleTable();

	// Read the file
	InputFile.ignore(1024, '\n');
	while(!InputFile.eof() && InputFile.peek() != EOF) {

		Identifier = GetTSVText(InputFile);
		for(int i = 0; i < WEAPONPARTICLE_TYPES; i++) {
			ParticleIdentifier = GetTSVText(InputFile);

			if(ParticleIdentifier != "" && !IsParticleLoaded(ParticleIdentifier)) {
				throw std::runtime_error(std::string(__FUNCTION__) + " - Cannot find particle: " + ParticleIdentifier);
			}
			else if(ParticleIdentifier == "")
				WeaponParticle.ParticleTemplates[i] = NULL;
			else
				WeaponParticle.ParticleTemplates[i] = GetParticleTemplate(ParticleIdentifier);

		}

		WeaponParticleTable.insert(make_pair(Identifier, WeaponParticle));
	}

	InputFile.close();
}

// Loads the monsters table
void _Assets::LoadMonsterTable(const std::string &Filename) {
	_MonsterTemplate Monster;
	std::string Identifier, ColorName, WeaponParticlesIdentifier;

	// Load file
	std::ifstream InputFile((AssetPath + Filename).c_str(), std::ios::in);
	if(!InputFile) {
		throw std::runtime_error("Error loading: " + Filename);
	}

	// Remove previous data
	UnloadMonsterTable();

	// Read the file
	InputFile.ignore(1024, '\n');
	while(!InputFile.eof() && InputFile.peek() != EOF) {

		Identifier = GetTSVText(InputFile);
		Monster.Name = GetTSVText(InputFile);
		Monster.AnimationIdentifier = GetTSVText(InputFile);
		WeaponParticlesIdentifier = GetTSVText(InputFile);
		Monster.SamplesIdentifier = GetTSVText(InputFile);
		Monster.ItemGroupIdentifier = GetTSVText(InputFile);
		ColorName = GetTSVText(InputFile);
		InputFile 	>> Monster.Level >> Monster.Health >> Monster.Defense >> Monster.BehaviorType >> Monster.ViewRange >> Monster.ExperienceGiven
					>> Monster.MovementSpeed >> Monster.Radius >> Monster.Scale >> Monster.CurrentSpeed >> Monster.Accuracy
					>> Monster.AttackRange >> Monster.MinDamage >> Monster.MaxDamage >> Monster.FirePeriod >> Monster.WeaponType;
		InputFile.ignore(1024, '\n');

		// Set color
		if(ColorName == "")
			Monster.Color = COLOR_WHITE;
		else {

			if(!IsColorLoaded(ColorName)) {
				throw std::runtime_error(std::string(__FUNCTION__) + " - Cannot find color: " + ColorName);
			}
			Monster.Color = GetColor(ColorName);
		}

		// Check for item group
		if(Monster.ItemGroupIdentifier != "" && !IsItemGroupLoaded(Monster.ItemGroupIdentifier)) {
			throw std::runtime_error(std::string(__FUNCTION__) + " - Cannot find item group: " + Monster.ItemGroupIdentifier + " in " + Identifier);
		}

		// Set particles
		if(IsWeaponParticleTemplateLoaded(WeaponParticlesIdentifier))
			Monster.WeaponParticles = GetWeaponParticleTemplate(WeaponParticlesIdentifier);
		else
			Monster.WeaponParticles = &BlankWeaponParticle;

		// Check for duplicates
		if(IsMonsterLoaded(Identifier)) {
			throw std::runtime_error(std::string(__FUNCTION__) + " - Duplicate entry: " + Identifier);
		}

		MonsterTable.insert(make_pair(Identifier, Monster));
	}

	InputFile.close();
}

// Loads the misc item table
void _Assets::LoadMiscItemTable(const std::string &Filename) {
	_MiscItemTemplate MiscItem;
	std::string Identifier, ColorName;

	// Load file
	std::ifstream InputFile((AssetPath + Filename).c_str(), std::ios::in);
	if(!InputFile) {
		throw std::runtime_error("Error loading: " + Filename);
	}

	// Remove previous data
	UnloadMiscItemTable();

	// Read the file
	InputFile.ignore(1024, '\n');
	while(!InputFile.eof() && InputFile.peek() != EOF) {

		Identifier = GetTSVText(InputFile);
		MiscItem.Name = GetTSVText(InputFile);
		MiscItem.IconIdentifier = GetTSVText(InputFile);
		ColorName = GetTSVText(InputFile);
		InputFile >> MiscItem.Type >> MiscItem.Level;
		InputFile.ignore(1024, '\n');

		// Check for loaded textures
		if(!IsTextureLoaded(MiscItem.IconIdentifier)) {
			throw std::runtime_error(std::string(__FUNCTION__) + " - Cannot find texture: " + MiscItem.IconIdentifier);
		}

		// Set color
		if(ColorName == "")
			MiscItem.Color = COLOR_WHITE;
		else {

			if(!IsColorLoaded(ColorName)) {
				throw std::runtime_error(std::string(__FUNCTION__) + " - Cannot find color: " + ColorName);
			}
			MiscItem.Color = GetColor(ColorName);
		}

		// Check for duplicates
		if(IsMiscItemLoaded(Identifier)) {
			throw std::runtime_error(std::string(__FUNCTION__) + " - Duplicate entry: " + Identifier);
		}

		MiscItemTable.insert(make_pair(Identifier, MiscItem));
	}

	InputFile.close();
}

// Loads the upgrade table
void _Assets::LoadUpgradeTable(const std::string &Filename) {
	_UpgradeTemplate Upgrade;
	std::string Identifier, ColorName;

	// Load file
	std::ifstream InputFile((AssetPath + Filename).c_str(), std::ios::in);
	if(!InputFile) {
		throw std::runtime_error("Error loading: " + Filename);
	}

	// Remove previous data
	UnloadUpgradeTable();

	// Read the file
	InputFile.ignore(1024, '\n');
	while(!InputFile.eof() && InputFile.peek() != EOF) {

		Identifier = GetTSVText(InputFile);
		Upgrade.Name = GetTSVText(InputFile);
		Upgrade.IconIdentifier = GetTSVText(InputFile);
		ColorName = GetTSVText(InputFile);
		InputFile >> Upgrade.UpgradeType >> Upgrade.WeaponType >> Upgrade.Bonus;
		InputFile.ignore(1024, '\n');

		// Check for loaded textures
		if(!IsTextureLoaded(Upgrade.IconIdentifier)) {
			throw std::runtime_error(std::string(__FUNCTION__) + " - Cannot find texture: " + Upgrade.IconIdentifier);
		}

		// Set color
		if(ColorName == "")
			Upgrade.Color = COLOR_WHITE;
		else {

			if(!IsColorLoaded(ColorName)) {
				throw std::runtime_error(std::string(__FUNCTION__) + " - Cannot find color: " + ColorName);
			}
			Upgrade.Color = GetColor(ColorName);
		}

		// Check for duplicates
		if(IsUpgradeLoaded(Identifier)) {
			throw std::runtime_error(std::string(__FUNCTION__) + " - Duplicate entry: " + Identifier);
		}

		UpgradeTable.insert(make_pair(Identifier, Upgrade));
	}
	InputFile.close();
}

// Loads the ammo table
void _Assets::LoadAmmoTable(const std::string &Filename) {
	_AmmoTemplate Ammo;
	std::string Identifier, ColorName;

	// Load file
	std::ifstream InputFile((AssetPath + Filename).c_str(), std::ios::in);
	if(!InputFile) {
		throw std::runtime_error("Error loading: " + Filename);
	}

	// Remove previous data
	UnloadAmmoTable();

	// Read the file
	InputFile.ignore(1024, '\n');
	while(!InputFile.eof() && InputFile.peek() != EOF) {

		Identifier = GetTSVText(InputFile);
		Ammo.Name = GetTSVText(InputFile);
		Ammo.IconIdentifier = GetTSVText(InputFile);
		ColorName = GetTSVText(InputFile);
		InputFile >> Ammo.AmmoType;
		InputFile.ignore(1024, '\n');

		// Check for loaded textures
		if(!IsTextureLoaded(Ammo.IconIdentifier)) {
			throw std::runtime_error(std::string(__FUNCTION__) + " - Texture not found: " + Ammo.IconIdentifier);
		}

		// Set color
		if(ColorName == "")
			Ammo.Color = COLOR_WHITE;
		else {

			if(!IsColorLoaded(ColorName)) {
				throw std::runtime_error(std::string(__FUNCTION__) + " - Cannot find color: " + ColorName);
			}
			Ammo.Color = GetColor(ColorName);
		}

		// Check for duplicates
		if(IsAmmoLoaded(Identifier)) {
			throw std::runtime_error(std::string(__FUNCTION__) + " - Duplicate entry: " + Identifier);
		}

		AmmoTypeIdentifiers[Ammo.AmmoType] = Identifier;
		AmmoTable.insert(make_pair(Identifier, Ammo));
	}
	InputFile.close();
}

// Loads the weapon table
void _Assets::LoadWeaponTable(const std::string &Filename) {

	AttackSampleTemplateStruct *AttackSample;
	std::string Identifier, ColorName, SamplesIdentifier, WeaponParticlesIdentifier;

	// Load file
	std::ifstream InputFile((AssetPath + Filename).c_str(), std::ios::in);
	if(!InputFile) {
		throw std::runtime_error("Error loading: " + Filename);
	}

	// Remove previous data
	UnloadWeaponTable();

	// Read the file
	InputFile.ignore(1024, '\n');
	while(!InputFile.eof() && InputFile.peek() != EOF) {
		_WeaponTemplate Weapon;

		Identifier = GetTSVText(InputFile);
		Weapon.Name = GetTSVText(InputFile);
		Weapon.IconIdentifier = GetTSVText(InputFile);
		SamplesIdentifier = GetTSVText(InputFile);
		WeaponParticlesIdentifier = GetTSVText(InputFile);
		ColorName = GetTSVText(InputFile);
		InputFile 	>> Weapon.Type >> Weapon.ZoomScale >> Weapon.MinAccuracy >> Weapon.MaxAccuracy >> Weapon.Recoil >> Weapon.RecoilRegen >> Weapon.Range
					>> Weapon.FireRate >> Weapon.FirePeriod >> Weapon.ReloadPeriod >> Weapon.MinComponents >> Weapon.MaxComponents
					>> Weapon.MinDamage >> Weapon.MaxDamage	>> Weapon.BulletsShot >> Weapon.RoundSize >> Weapon.AmmoType;
		InputFile.ignore(1024, '\n');

		// Check for loaded textures
		if(!IsTextureLoaded(Weapon.IconIdentifier)) {
			throw std::runtime_error(std::string(__FUNCTION__) + " - Texture not found: " + Weapon.IconIdentifier);
		}

		// Set color
		if(ColorName == "")
			Weapon.Color = COLOR_WHITE;
		else {

			if(!IsColorLoaded(ColorName)) {
				throw std::runtime_error(std::string(__FUNCTION__) + " - Cannot find color: " + ColorName);
			}
			Weapon.Color = GetColor(ColorName);
		}

		// Check for attack sample
		if(!IsAttackSampleLoaded(SamplesIdentifier)) {
			throw std::runtime_error(std::string(__FUNCTION__) + " - Cannot find sample: " + SamplesIdentifier);
		}

		// Set samples
		AttackSample = GetAttackSampleTemplate(SamplesIdentifier);
		for(int i = 0; i < SAMPLE_TYPES; i++) {
			if(AttackSample)
				Weapon.Samples[i] = AttackSample->Samples[i];
		}

		// Set particles
		if(IsWeaponParticleTemplateLoaded(WeaponParticlesIdentifier))
			Weapon.WeaponParticles = GetWeaponParticleTemplate(WeaponParticlesIdentifier);
		else
			Weapon.WeaponParticles = &BlankWeaponParticle;

		// Check for duplicates
		if(IsWeaponLoaded(Identifier)) {
			throw std::runtime_error(std::string(__FUNCTION__) + " - Duplicate entry: " + Identifier);
		}

		WeaponTable.insert(make_pair(Identifier, Weapon));
	}
	InputFile.close();
}

// Loads the armor table
void _Assets::LoadArmorTable(const std::string &Filename) {
	_ArmorTemplate Armor;
	std::string Identifier, ColorName;

	// Load file
	std::ifstream InputFile((AssetPath + Filename).c_str(), std::ios::in);
	if(!InputFile) {
		throw std::runtime_error("Error loading: " + Filename);
	}

	// Remove previous data
	UnloadArmorTable();

	// Read the file
	InputFile.ignore(1024, '\n');
	while(!InputFile.eof() && InputFile.peek() != EOF) {

		Identifier = GetTSVText(InputFile);
		Armor.Name = GetTSVText(InputFile);
		Armor.IconIdentifier = GetTSVText(InputFile);
		ColorName = GetTSVText(InputFile);
		InputFile >> Armor.Defense >> Armor.StrengthRequirement;
		InputFile.ignore(1024, '\n');

		// Check for loaded textures
		if(!IsTextureLoaded(Armor.IconIdentifier)) {
			throw std::runtime_error(std::string(__FUNCTION__) + " - Texture not found: " + Armor.IconIdentifier);
		}

		// Set color
		if(ColorName == "")
			Armor.Color = COLOR_WHITE;
		else {

			if(!IsColorLoaded(ColorName)) {
				throw std::runtime_error(std::string(__FUNCTION__) + " - Cannot find color: " + ColorName);
			}
			Armor.Color = GetColor(ColorName);
		}

		// Check for duplicates
		if(IsArmorLoaded(Identifier)) {
			throw std::runtime_error(std::string(__FUNCTION__) + " - Duplicate entry: " + Identifier);
		}

		ArmorTable.insert(make_pair(Identifier, Armor));
	}
	InputFile.close();
}

// Load item drop table
void _Assets::LoadItemDropTable(const std::string &Filename) {

	// Load file
	std::ifstream InputFile((AssetPath + Filename).c_str(), std::ios::in);
	if(!InputFile) {
		throw std::runtime_error("Error loading: " + Filename);
	}

	// Remove previous data
	UnloadItemGroupTable();

	// Skip first two fields
	GetTSVText(InputFile);
	GetTSVText(InputFile);

	// Get item drop names first
	bool EndOfLine = false;
	int ItemDrops = 0;
	std::vector<std::string> ItemDropNames;
	while(!EndOfLine && InputFile.peek() != EOF) {
		std::string DropName = GetTSVText(InputFile, &EndOfLine);

		if(DropName != "") {
			ItemDropNames.push_back(DropName);

			auto ItemGroupTableIterator = ItemGroupTable.find(DropName);
			if(ItemGroupTableIterator == ItemGroupTable.end()) {
				_ItemGroup ItemGroup;
				ItemGroup.Total = 0;
				ItemGroup.Quantity = 1;
				ItemGroupTable[DropName] = ItemGroup;
			}
			ItemDrops++;
		}
	}

	// Read rest of data
	while(!InputFile.eof() && InputFile.peek() != EOF) {
		ItemGroupEntryStruct ItemGroupEntry;

		InputFile >> ItemGroupEntry.Type;
		InputFile.ignore(1, '\t');
		ItemGroupEntry.ItemIdentifier = GetTSVText(InputFile);

		// See if items exist
		switch(ItemGroupEntry.Type) {
			case -1:
			break;
			case _Object::MISCITEM:
				if(!IsMiscItemLoaded(ItemGroupEntry.ItemIdentifier))
					throw std::runtime_error(std::string(__FUNCTION__) + " - Cannot find: " + ItemGroupEntry.ItemIdentifier + " in " + Filename);
			break;
			case _Object::AMMO:
				if(!IsAmmoLoaded(ItemGroupEntry.ItemIdentifier))
					throw std::runtime_error(std::string(__FUNCTION__) + " - Cannot find: " + ItemGroupEntry.ItemIdentifier + " in " + Filename);
			break;
			case _Object::UPGRADE:
				if(!IsUpgradeLoaded(ItemGroupEntry.ItemIdentifier))
					throw std::runtime_error(std::string(__FUNCTION__) + " - Cannot find: " + ItemGroupEntry.ItemIdentifier + " in " + Filename);
			break;
			case _Object::WEAPON:
				if(!IsWeaponLoaded(ItemGroupEntry.ItemIdentifier))
					throw std::runtime_error(std::string(__FUNCTION__) + " - Cannot find: " + ItemGroupEntry.ItemIdentifier + " in " + Filename);
			break;
			case _Object::ARMOR:
				if(!IsArmorLoaded(ItemGroupEntry.ItemIdentifier))
					throw std::runtime_error(std::string(__FUNCTION__) + " - Cannot find: " + ItemGroupEntry.ItemIdentifier + " in " + Filename);
			break;
			default:
				throw std::runtime_error(std::string(__FUNCTION__) + " - Bad item type: " + ItemGroupEntry.ItemIdentifier + " in " + Filename);
			break;
		}

		// Add counts to item groups
		for(int i = 0; i < ItemDrops; i++) {
			InputFile >> ItemGroupEntry.Count;

			if(ItemGroupEntry.Count > 0) {
				ItemGroupTable[ItemDropNames[i]].Total += ItemGroupEntry.Count;
				ItemGroupEntry.Count = ItemGroupTable[ItemDropNames[i]].Total;
				ItemGroupTable[ItemDropNames[i]].Entries.push_back(ItemGroupEntry);
			}
		}

		InputFile.ignore(1024, '\n');
	}

	InputFile.close();
}

// Loads a monster set
void _Assets::LoadMonsterSet(const std::string &Filename) {

	// Load file
	std::ifstream InputFile((AssetPath + Filename).c_str(), std::ios::in);
	if(!InputFile)
		throw std::runtime_error("Error loading: " + Filename);

	UnloadMonsterSet();

	// Read the file
	std::string Identifier;
	while(!InputFile.eof() && InputFile.peek() != EOF) {
		InputFile >> Identifier;
		InputFile.ignore(1024, '\n');

		if(!IsMonsterLoaded(Identifier))
			throw std::runtime_error("Cannot find monster: " + Identifier);

		MonsterSet.push_back(Identifier);
	}
	InputFile.close();

	// Load the animation textures
	LoadMonsterAnimation();
}

// Loads the reel from the given identifier
void _Assets::LoadReel(const std::string &Identifier, const std::string &Path) {
	_Reel Reel;
	std::string FilePath;
	_Texture *Texture;

	auto ReelTableIterator = ReelTable.find(Identifier);
	if(ReelTableIterator != ReelTable.end()) {

		auto ReelIterator = Reels.find(Identifier);
		if(ReelIterator == Reels.end()) {

			Reel.StartPosition = ReelTableIterator->second.StartPosition;
			Reel.RepeatMode = static_cast<RepeatType>(ReelTableIterator->second.RepeatMode);
			Reel.PlaybackSpeed = ReelTableIterator->second.PlaybackSpeed;

			for(int i = 0; i < static_cast<int>(ReelTableIterator->second.TextureFiles.size()); i++) {
				FilePath = AssetPath + Path + ReelTableIterator->second.TextureFiles[i];
				Texture = new _Texture(FilePath, 0, false, true);
				if(!Texture) {
					throw std::runtime_error("Error loading: " + FilePath);
				}

				Reel.Textures.push_back(Texture);
			}

			Reels.insert(make_pair(Identifier, Reel));
		}
	}
}

// Loads the reels for an animation
void _Assets::LoadAnimation(const std::string &Identifier, const std::string &Path) {

	auto AnimationTableIterator = AnimationTable.find(Identifier);
	if(AnimationTableIterator != AnimationTable.end()) {

		// Check if animation has already been loaded
		auto AnimationIterator = Animations.find(Identifier);
		if(AnimationIterator == Animations.end()) {

			_Animation *Animation = new _Animation();

			// Load reels
			for(int i = 0; i < static_cast<int>(AnimationTableIterator->second.Identifiers.size()); i++) {
				LoadReel(AnimationTableIterator->second.Identifiers[i], Path);

				Animation->AddReel(GetReel(AnimationTableIterator->second.Identifiers[i]));
			}

			Animation->ChangeReel(0);
			Animations.insert(make_pair(Identifier, Animation));
		}
	}
}

// Loads the monster aniimation
void _Assets::LoadMonsterAnimation() {

	for(size_t i = 0; i < MonsterSet.size(); i++) {
		LoadAnimation(GetMonsterTemplate(MonsterSet[i])->AnimationIdentifier, ASSETS_MONSTERTEXTURES);
	}
}

// Loads the styles
void _Assets::LoadStyles(const std::string &Filename) {

	// Load file
	std::ifstream InputFile((AssetPath + Filename).c_str(), std::ios::in);
	if(!InputFile) {
		throw std::runtime_error("Error loading: " + Filename);
	}

	// Read the file
	InputFile.ignore(1024, '\n');
	while(!InputFile.eof() && InputFile.peek() != EOF) {

		std::string Identifier = GetTSVText(InputFile);
		std::string BackgroundColorIdentifier = GetTSVText(InputFile);
		std::string BorderColorIdentifier = GetTSVText(InputFile);
		std::string TextureIdentifier = GetTSVText(InputFile);
		std::string TextureColorIdentifier = GetTSVText(InputFile);

		bool Stretch;
		InputFile >> Stretch;
		InputFile.ignore(1024, '\n');

		// Get colors
		_Color BackgroundColor = GetColor(BackgroundColorIdentifier);
		_Color BorderColor = GetColor(BorderColorIdentifier);
		_Color TextureColor = GetColor(TextureColorIdentifier);

		// Get textures
		_Texture *Texture = GetTexture(TextureIdentifier);

		// Create
		_Style *Style = new _Style(Identifier, BackgroundColorIdentifier != "", BorderColorIdentifier != "", BackgroundColor, BorderColor, Texture, TextureColor, Stretch);

		// Check for duplicates
		if(GetStyle(Identifier)) {
			throw std::runtime_error("Duplicate style identifier: " + Identifier);
		}

		Styles.insert(make_pair(Identifier, Style));
	}

	InputFile.close();
}

// Loads the ui elements
void _Assets::LoadElements(const std::string &Filename) {

	// Load file
	std::ifstream InputFile((AssetPath + Filename).c_str(), std::ios::in);
	if(!InputFile) {
		throw std::runtime_error("Error loading: " + Filename);
	}

	// Read the file
	InputFile.ignore(1024, '\n');
	while(!InputFile.eof() && InputFile.peek() != EOF) {

		std::string Identifier = GetTSVText(InputFile);
		std::string ParentIdentifier = GetTSVText(InputFile);
		std::string StyleIdentifier = GetTSVText(InputFile);

		_Point Offset, Size;
		_Alignment Alignment;
		bool MaskOutside;
		InputFile >> Offset.X >> Offset.Y >> Size.X >> Size.Y >> Alignment.Horizontal >> Alignment.Vertical >> MaskOutside;
		InputFile.ignore(1024, '\n');

		// Look for parent
		_Element *ParentElement = NULL;
		if(ParentIdentifier != "") {
			ParentElement = GetElement(ParentIdentifier);
			if(!ParentElement) {
				throw std::runtime_error("Parent element not found: " + ParentIdentifier);
			}
		}

		// Get style
		_Style *Style = NULL;
		if(StyleIdentifier != "") {
			Style = GetStyle(StyleIdentifier);
			if(!Style) {
				throw std::runtime_error("Unable to find style: " + StyleIdentifier);
			}
		}

		// Create
		_Element *Element = new _Element(Identifier, ParentElement, Offset, Size, Alignment, Style, MaskOutside);

		// Check for duplicates
		if(GetElement(Identifier)) {
			throw std::runtime_error("Duplicate element identifier: " + Identifier);
		}

		// Add as child for parent
		if(ParentElement) {
			ParentElement->AddChild(Element);
		}

		Elements.insert(make_pair(Identifier, Element));
	}

	InputFile.close();
}

// Loads labels elements
void _Assets::LoadLabels(const std::string &Filename) {

	// Load file
	std::ifstream InputFile((AssetPath + Filename).c_str(), std::ios::in);
	if(!InputFile) {
		throw std::runtime_error("Error loading: " + Filename);
	}

	// Read the file
	InputFile.ignore(1024, '\n');
	while(!InputFile.eof() && InputFile.peek() != EOF) {

		std::string Identifier = GetTSVText(InputFile);
		std::string ParentIdentifier = GetTSVText(InputFile);
		std::string FontIdentifier = GetTSVText(InputFile);
		std::string ColorIdentifier = GetTSVText(InputFile);
		std::string Text = GetTSVText(InputFile);

		_Point Offset, Size;
		_Alignment Alignment;
		InputFile >> Offset.X >> Offset.Y >> Size.X >> Size.Y >> Alignment.Horizontal >> Alignment.Vertical;
		InputFile.ignore(1024, '\n');

		// Look for parent
		_Element *ParentElement = NULL;
		if(ParentIdentifier != "") {
			ParentElement = GetElement(ParentIdentifier);
			if(!ParentElement) {
				throw std::runtime_error("Parent element not found: " + ParentIdentifier);
			}
		}

		// Get font
		_Font *Font = GetFont(FontIdentifier);
		if(!Font) {
			throw std::runtime_error("Unable to find font: " + FontIdentifier);
		}

		// Get color
		_Color Color = GetColor(ColorIdentifier);

		// Create
		_Label *Element = new _Label(Identifier, ParentElement, Offset, Size, Alignment, Font, Color, Text);

		// Check for duplicates
		if(GetElement(Identifier)) {
			throw std::runtime_error("Duplicate element identifier: " + Identifier);
		}

		// Add as child for parent
		if(ParentElement) {
			ParentElement->AddChild(Element);
		}

		Elements.insert(make_pair(Identifier, Element));
	}

	InputFile.close();
}

// Loads image elements
void _Assets::LoadImages(const std::string &Filename) {

	// Load file
	std::ifstream InputFile((AssetPath + Filename).c_str(), std::ios::in);
	if(!InputFile) {
		throw std::runtime_error("Error loading: " + Filename);
	}

	// Read the file
	InputFile.ignore(1024, '\n');
	while(!InputFile.eof() && InputFile.peek() != EOF) {

		std::string Identifier = GetTSVText(InputFile);
		std::string ParentIdentifier = GetTSVText(InputFile);
		std::string TextureIdentifier = GetTSVText(InputFile);
		std::string ColorIdentifier = GetTSVText(InputFile);

		_Point Offset, Size;
		_Alignment Alignment;
		int Stretch;
		InputFile >> Offset.X >> Offset.Y >> Size.X >> Size.Y >> Alignment.Horizontal >> Alignment.Vertical >> Stretch;
		InputFile.ignore(1024, '\n');

		// Look for parent
		_Element *ParentElement = NULL;
		if(ParentIdentifier != "") {
			ParentElement = GetElement(ParentIdentifier);
			if(!ParentElement) {
				throw std::runtime_error("Parent element not found: " + ParentIdentifier);
			}
		}

		// Get texture
		_Texture *Texture = GetTexture(TextureIdentifier);

		// Get color
		_Color Color = GetColor(ColorIdentifier);

		// Create
		_Image *Element = new _Image(Identifier, ParentElement, Offset, Size, Alignment, Texture, Color, Stretch);

		// Check for duplicates
		if(GetElement(Identifier)) {
			throw std::runtime_error("Duplicate element identifier: " + Identifier);
		}

		// Add as child for parent
		if(ParentElement) {
			ParentElement->AddChild(Element);
		}

		Elements.insert(make_pair(Identifier, Element));
	}

	InputFile.close();
}

// Loads button elements
void _Assets::LoadButtons(const std::string &Filename) {

	// Load file
	std::ifstream InputFile((AssetPath + Filename).c_str(), std::ios::in);
	if(!InputFile) {
		throw std::runtime_error("Error loading: " + Filename);
	}

	// Read the file
	InputFile.ignore(1024, '\n');
	while(!InputFile.eof() && InputFile.peek() != EOF) {

		std::string Identifier = GetTSVText(InputFile);
		std::string ParentIdentifier = GetTSVText(InputFile);
		std::string StyleIdentifier = GetTSVText(InputFile);
		std::string HoverStyleIdentifier = GetTSVText(InputFile);

		_Point Offset, Size;
		_Alignment Alignment;
		InputFile >> Offset.X >> Offset.Y >> Size.X >> Size.Y >> Alignment.Horizontal >> Alignment.Vertical;
		InputFile.ignore(1024, '\n');

		// Look for parent
		_Element *ParentElement = NULL;
		if(ParentIdentifier != "") {
			ParentElement = GetElement(ParentIdentifier);
			if(!ParentElement) {
				throw std::runtime_error("Parent element not found: " + ParentIdentifier);
			}
		}

		// Get style
		_Style *Style = GetStyle(StyleIdentifier);
		_Style *HoverStyle = GetStyle(HoverStyleIdentifier);

		// Create
		_Button *Element = new _Button(Identifier, ParentElement, Offset, Size, Alignment, Style, HoverStyle);

		// Check for duplicates
		if(GetElement(Identifier)) {
			throw std::runtime_error("Duplicate element identifier: " + Identifier);
		}

		// Add as child for parent
		if(ParentElement) {
			ParentElement->AddChild(Element);
		}

		Elements.insert(make_pair(Identifier, Element));
	}

	InputFile.close();
}

// Loads textbox elements
void _Assets::LoadTextBoxes(const std::string &Filename) {

	// Load file
	std::ifstream InputFile((AssetPath + Filename).c_str(), std::ios::in);
	if(!InputFile) {
		throw std::runtime_error("Error loading: " + Filename);
	}

	// Read the file
	InputFile.ignore(1024, '\n');
	while(!InputFile.eof() && InputFile.peek() != EOF) {

		std::string Identifier = GetTSVText(InputFile);
		std::string ParentIdentifier = GetTSVText(InputFile);
		std::string StyleIdentifier = GetTSVText(InputFile);
		std::string FontIdentifier = GetTSVText(InputFile);

		_Point Offset, Size;
		_Alignment Alignment;
		int MaxLength;
		InputFile >> Offset.X >> Offset.Y >> Size.X >> Size.Y >> Alignment.Horizontal >> Alignment.Vertical >> MaxLength;
		InputFile.ignore(1024, '\n');

		// Look for parent
		_Element *ParentElement = NULL;
		if(ParentIdentifier != "") {
			ParentElement = GetElement(ParentIdentifier);
			if(!ParentElement) {
				throw std::runtime_error("Parent element not found: " + ParentIdentifier);
			}
		}

		// Get style
		_Style *Style = GetStyle(StyleIdentifier);

		// Get font
		_Font *Font = GetFont(FontIdentifier);
		if(!Font) {
			throw std::runtime_error("Unable to find font: " + FontIdentifier);
		}

		// Create
		_TextBox *Element = new _TextBox(Identifier, ParentElement, Offset, Size, Alignment, Style, Font, MaxLength);

		// Check for duplicates
		if(GetElement(Identifier)) {
			throw std::runtime_error("Duplicate element identifier: " + Identifier);
		}

		// Add as child for parent
		if(ParentElement) {
			ParentElement->AddChild(Element);
		}

		Elements.insert(make_pair(Identifier, Element));
	}

	InputFile.close();
}

// Frees memory and textures used by a reel
void _Assets::UnloadReel(const std::string &Identifier) {

	auto ReelIterator = Reels.find(Identifier);
	if(ReelIterator != Reels.end()) {
		for(int i = 0; i < static_cast<int>(ReelIterator->second.Textures.size()); i++)
			delete ReelIterator->second.Textures[i];

		Reels.erase(ReelIterator);
	}
}

// Frees memory and textures used by an animation
void _Assets::UnloadAnimation(const std::string &Identifier) {

	auto AnimationIterator = Animations.find(Identifier);
	if(AnimationIterator != Animations.end()) {

		// Unload reels
		auto AnimationTableIterator = AnimationTable.find(Identifier);
		if(AnimationTableIterator != AnimationTable.end()) {

			for(size_t i = 0; i < AnimationTableIterator->second.Identifiers.size(); i++)
				UnloadReel(AnimationTableIterator->second.Identifiers[i]);

		}

		delete AnimationIterator->second;
		Animations.erase(AnimationIterator);
	}
}

// Frees memory and textures used by monsters
void _Assets::UnloadMonsterAnimation() {

	for(const auto &Monster : MonsterTable)
		UnloadAnimation(Monster.second.AnimationIdentifier);
}

// Free styles
void _Assets::UnloadStyles() {

	for(const auto &Style : Styles)
		delete Style.second;

	Styles.clear();
}

// Free ui elements
void _Assets::UnloadElements() {

	for(const auto &Element : Elements)
		delete Element.second;

	Elements.clear();
}

// Frees memory used by textures
void _Assets::UnloadTextures() {

	for(const auto &Texture : Textures)
		delete Texture.second;

	Textures.clear();
}

// Frees memory used by the monster set
void _Assets::UnloadMonsterSet() {
	UnloadMonsterAnimation();

	MonsterSet.clear();
}

// Frees memory used by samples
void _Assets::UnloadSamples() {
}

// Frees memory used by fonts
void _Assets::UnloadFonts() {

	for(const auto &Font : Fonts)
		delete Font.second;

	Fonts.clear();
}

// Returns the valid amount of experience
int64_t _Assets::GetValidExperience(int64_t Experience) {

	if(Experience < 0)
		return 0;
	else if(Experience > Levels[GAME_MAX_LEVEL-1].Experience)
		return Levels[GAME_MAX_LEVEL-1].Experience;

	return Experience;
}

// Returns the level given the experience number
int _Assets::GetLevel(int64_t Experience) {

	// Degenerate case
	if(Experience <= 0)
		return 1;
	else if(Experience >= Levels[GAME_MAX_LEVEL-1].Experience)
		return GAME_MAX_LEVEL;

	// Perform linear search through array
	for(int i = 1; i < GAME_MAX_LEVEL; i++) {
		if(Experience < Levels[i].Experience)
			return i;
	}

	return 1;
}

// Returns the total experience required for a level
int64_t _Assets::GetExperienceForLevel(int Level) {

	// Degenerate case
	if(Level <= 0)
		return Levels[0].Experience;
	else if(Level > GAME_MAX_LEVEL)
		return 0;

	return  Levels[Level-1].Experience;
}

// Returns a skill value in a valid range
int _Assets::GetValidSkill(int Level) {
	if(Level < 0)
		return 0;
	else if(Level >= GAME_SKILLLEVELS)
		return GAME_SKILLLEVELS;

	return Level;
}

// Creates a monster
_Monster *_Assets::CreateMonster(const std::string &Identifier, const Vector2 &Position) {
	_MonsterTemplate *MonsterTemplate;
	AttackSampleTemplateStruct *AttackSample;

	MonsterTemplate = GetMonsterTemplate(Identifier);
	AttackSample = GetAttackSampleTemplate(MonsterTemplate->SamplesIdentifier);

	// Creates a monster
	_Monster *Monster = new _Monster(MonsterTemplate, GetAnimation(MonsterTemplate->AnimationIdentifier), Position);
	for(int i = 0; i < SAMPLE_TYPES; i++)
		Monster->SetSample(i, AttackSample->Samples[i]);

	return Monster;
}

// Creates a misc item
_MiscItem *_Assets::CreateMiscItem(const std::string &Identifier, int Count, const Vector2 &Position) {
	_MiscItemTemplate *MiscItemTemplate = GetMiscItemTemplate(Identifier);

	return new _MiscItem(Identifier, Count, Position, MiscItemTemplate, GetTexture(MiscItemTemplate->IconIdentifier));
}

// Creates ammo
_Ammo *_Assets::CreateAmmoItem(const std::string &Identifier, int Count, const Vector2 &Position) {
	_AmmoTemplate *AmmoTemplate = GetAmmoTemplate(Identifier);

	return new _Ammo(Identifier, Count, Position, AmmoTemplate, GetTexture(AmmoTemplate->IconIdentifier));
}

// Creates ammo from an ammo type
_Ammo *_Assets::CreateAmmoItem(int Type) {
	return CreateAmmoItem(AmmoTypeIdentifiers[Type], 1, ZERO_VECTOR);
}

// Creates a weapon
_Weapon *_Assets::CreateWeapon(const std::string &Identifier, int Count, const Vector2 &Position, bool Generate) {
	_WeaponTemplate *WeaponTemplate = GetWeaponTemplate(Identifier);
	_Weapon *Weapon = new _Weapon(Identifier, Count, Position, WeaponTemplate, GetTexture(WeaponTemplate->IconIdentifier), Generate);

	return Weapon;
}

// Creates an upgrade item
_Upgrade *_Assets::CreateUpgradeItem(const std::string &Identifier, int Count, const Vector2 &Position) {
	_UpgradeTemplate *UpgradeTemplate = GetUpgradeTemplate(Identifier);

	return new _Upgrade(Identifier, Count, Position, UpgradeTemplate, GetTexture(UpgradeTemplate->IconIdentifier));
}

// Creates armor
_Armor *_Assets::CreateArmor(const std::string &Identifier, int Count, const Vector2 &Position) {
	_ArmorTemplate *ArmorTemplate = GetArmorTemplate(Identifier);

	return new _Armor(Identifier, Count, Position, ArmorTemplate, GetTexture(ArmorTemplate->IconIdentifier));
}

// Returns a random item identifier from an item group
void _Assets::GetRandomDrop(const _ItemGroup *ItemGroup, _ObjectSpawn *ObjectSpawn) {
	ObjectSpawn->Type = -1;

	// Get item group
	size_t ItemGroupSize = ItemGroup->Entries.size();
	if(ItemGroupSize == 0)
		return;

	// Get total
	if(ItemGroup->Total <= 0.0f)
		return;

	// Generate roll
	float RandomNumber = Random.GenerateRange(0.0f, ItemGroup->Total);

	// Get item
	for(size_t i = 0; i < ItemGroupSize; i++) {
		if(RandomNumber <= ItemGroup->Entries[i].Count) {
			ObjectSpawn->Type = ItemGroup->Entries[i].Type;
			ObjectSpawn->Identifier = ItemGroup->Entries[i].ItemIdentifier;
			return;
		}
	}
}

// Generates a list of monster icons
 void _Assets::GetEventList(std::vector<_Brush> &Icons) {

	Icons.push_back(_Brush("door", "Door", GetTexture("editor_eventdoor"), COLOR_WHITE));
	Icons.push_back(_Brush("wswitch", "Wall Switch", GetTexture("editor_eventwswitch"), COLOR_WHITE));
	Icons.push_back(_Brush("spawn", "Spawn", GetTexture("editor_eventspawn"), COLOR_WHITE));
	Icons.push_back(_Brush("check", "Checkpoint", GetTexture("editor_eventcheck"), COLOR_WHITE));
	Icons.push_back(_Brush("end", "End of Level", GetTexture("editor_eventend"), COLOR_WHITE));
	Icons.push_back(_Brush("text", "Event Message", GetTexture("editor_eventtext"), COLOR_WHITE));
	Icons.push_back(_Brush("sound", "Event Sound", GetTexture("editor_eventsound"), COLOR_WHITE));
	Icons.push_back(_Brush("fswitch", "Floor Switch", GetTexture("editor_eventfswitch"), COLOR_WHITE));
	Icons.push_back(_Brush("enable", "Event Enabler", GetTexture("editor_eventenable"), COLOR_WHITE));
	Icons.push_back(_Brush("tele", "Teleporter", GetTexture("editor_eventtele"), COLOR_WHITE));
	Icons.push_back(_Brush("light", "Lights", GetTexture("editor_eventlight"), COLOR_WHITE));
}

// Generates a list of monster icons
void _Assets::GetMonsterList(std::vector<_Brush> &Icons) {
	_MonsterTemplate *Monster;

	for(size_t i = 0; i < MonsterSet.size(); i++) {
		if(!IsMonsterLoaded(MonsterSet[i])) {
			throw std::runtime_error("_Database::GetMonsterList - Cannot find monster: " + MonsterSet[i]);
		}
		else {
			Monster = GetMonsterTemplate(MonsterSet[i]);
			Icons.push_back(_Brush(MonsterSet[i], Monster->Name, Animations[Monster->AnimationIdentifier]->GetStartPositionFrame(), Monster->Color));
		}
	}
}

// Generates a list of misc item icons
void _Assets::GetItemList(std::vector<_Brush> &Icons) {
	for(const auto &MiscItem : MiscItemTable)
		Icons.push_back(_Brush(MiscItem.first, MiscItem.second.Name, Textures[MiscItem.second.IconIdentifier], MiscItem.second.Color));
}

// Generates a list of upgrade icons
void _Assets::GetUpgradeList(std::vector<_Brush> &Icons) {
	for(const auto &Upgrade : UpgradeTable)
		Icons.push_back(_Brush(Upgrade.first, Upgrade.second.Name, Textures[Upgrade.second.IconIdentifier], Upgrade.second.Color));
}

// Generates a list of ammo icons
void _Assets::GetAmmoList(std::vector<_Brush> &Icons) {
	for(const auto &Ammo : AmmoTable)
		Icons.push_back(_Brush(Ammo.first, Ammo.second.Name, Textures[Ammo.second.IconIdentifier], Ammo.second.Color));
}

// Generates a list of weapon icons
void _Assets::GetWeaponList(std::vector<_Brush> &Icons) {
	for(const auto &Weapon : WeaponTable)
		Icons.push_back(_Brush(Weapon.first, Weapon.second.Name, Textures[Weapon.second.IconIdentifier], Weapon.second.Color));
}

// Generates a list of armor icons
void _Assets::GetArmorList(std::vector<_Brush> &Icons) {
	for(const auto &Armor : ArmorTable)
		Icons.push_back(_Brush(Armor.first, Armor.second.Name, Textures[Armor.second.IconIdentifier], Armor.second.Color));
}

// Get a list of textures
void _Assets::GetTextureList(std::vector<_Brush> &TextureList, int Group) {
	for(const auto &Texture : Textures) {
		if(!Texture.second)
			throw std::runtime_error("Bad texture in textures list");

		if(Texture.second->GetGroup() == Group || Group == -1) {
			TextureList.push_back(_Brush(Texture.first, Texture.second->GetName(), Texture.second, COLOR_WHITE));
		}
	}
}

bool _Assets::IsStringLoaded(const std::string &Identifier) { return StringTable.find(Identifier) != StringTable.end(); }
bool _Assets::IsColorLoaded(const std::string &Identifier) { return ColorTable.find(Identifier) != ColorTable.end(); }
bool _Assets::IsTextureLoaded(const std::string &Identifier) { return Textures.find(Identifier) != Textures.end(); }
bool _Assets::IsAttackSampleLoaded(const std::string &Identifier) { return AttackSampleTable.find(Identifier) != AttackSampleTable.end(); }
bool _Assets::IsParticleLoaded(const std::string &Identifier) { return ParticleTable.find(Identifier) != ParticleTable.end(); }
bool _Assets::IsWeaponParticleTemplateLoaded(const std::string &Identifier) { return WeaponParticleTable.find(Identifier) != WeaponParticleTable.end(); }
bool _Assets::IsReelLoaded(const std::string &Identifier) { return ReelTable.find(Identifier) != ReelTable.end(); }
bool _Assets::IsAnimationLoaded(const std::string &Identifier) { return AnimationTable.find(Identifier) != AnimationTable.end(); }
bool _Assets::IsFontLoaded(const std::string &Identifier) { return Fonts.find(Identifier) != Fonts.end(); }
bool _Assets::IsMonsterLoaded(const std::string &Identifier) { return MonsterTable.find(Identifier) != MonsterTable.end(); }
bool _Assets::IsMiscItemLoaded(const std::string &Identifier) { return MiscItemTable.find(Identifier) != MiscItemTable.end(); }
bool _Assets::IsUpgradeLoaded(const std::string &Identifier) { return UpgradeTable.find(Identifier) != UpgradeTable.end(); }
bool _Assets::IsAmmoLoaded(const std::string &Identifier) { return AmmoTable.find(Identifier) != AmmoTable.end(); }
bool _Assets::IsWeaponLoaded(const std::string &Identifier) { return WeaponTable.find(Identifier) != WeaponTable.end(); }
bool _Assets::IsArmorLoaded(const std::string &Identifier) { return ArmorTable.find(Identifier) != ArmorTable.end(); }
bool _Assets::IsItemGroupLoaded(const std::string &Identifier) { return ItemGroupTable.find(Identifier) != ItemGroupTable.end(); }

void _Assets::UnloadStringTable() { StringTable.clear(); }
void _Assets::UnloadColorTable() { ColorTable.clear(); }
void _Assets::UnloadReelTable() { ReelTable.clear(); }
void _Assets::UnloadAnimationTable() { AnimationTable.clear(); }
void _Assets::UnloadAttackSampleTable() { AttackSampleTable.clear(); }
void _Assets::UnloadParticleTable() { ParticleTable.clear(); }
void _Assets::UnloadWeaponParticleTable() { WeaponParticleTable.clear(); }
void _Assets::UnloadMonsterTable() { MonsterTable.clear(); }
void _Assets::UnloadMiscItemTable() { MiscItemTable.clear(); }
void _Assets::UnloadUpgradeTable() { UpgradeTable.clear(); }
void _Assets::UnloadAmmoTable() { AmmoTable.clear(); }
void _Assets::UnloadWeaponTable() { WeaponTable.clear(); }
void _Assets::UnloadArmorTable() { ArmorTable.clear(); }
void _Assets::UnloadItemGroupTable() { ItemGroupTable.clear(); }

_Font *_Assets::GetFont(const std::string &Identifier) {
	if(Fonts.find(Identifier) == Fonts.end())
		return NULL;

	return Fonts[Identifier];
}
_Texture *_Assets::GetTexture(const std::string &Identifier) {
	if(Textures.find(Identifier) == Textures.end())
		return NULL;

	return Textures[Identifier];
}
std::string _Assets::GetString(const std::string &Identifier) {
	if(StringTable.find(Identifier) == StringTable.end())
		return "";

	return StringTable[Identifier];
}
const _Color &_Assets::GetColor(const std::string &Identifier) {
	if(ColorTable.find(Identifier) == ColorTable.end())
		return COLOR_WHITE;

	return ColorTable[Identifier];
}
_Reel *_Assets::GetReel(const std::string &Identifier) {
	if(Reels.find(Identifier) == Reels.end())
		return NULL;

	return &Reels[Identifier];
}
AttackSampleTemplateStruct *_Assets::GetAttackSampleTemplate(const std::string &Identifier) {
	if(AttackSampleTable.find(Identifier) == AttackSampleTable.end())
		return NULL;

	return &AttackSampleTable[Identifier];
}
_Animation *_Assets::GetAnimation(const std::string &Identifier) {
	if(Animations.find(Identifier) == Animations.end())
		return NULL;

	return Animations[Identifier];
}
_ParticleTemplate *_Assets::GetParticleTemplate(const std::string &Identifier) {
	if(ParticleTable.find(Identifier) == ParticleTable.end())
		return NULL;

	return &ParticleTable[Identifier];
}
_WeaponParticleTemplate *_Assets::GetWeaponParticleTemplate(const std::string &Identifier) {
	if(WeaponParticleTable.find(Identifier) == WeaponParticleTable.end())
		return NULL;

	return &WeaponParticleTable[Identifier];
}
_MonsterTemplate *_Assets::GetMonsterTemplate(const std::string &Identifier) {
	if(MonsterTable.find(Identifier) == MonsterTable.end())
		return NULL;

	return &MonsterTable[Identifier];
}
_MiscItemTemplate *_Assets::GetMiscItemTemplate(const std::string &Identifier) {
	if(MiscItemTable.find(Identifier) == MiscItemTable.end())
		return NULL;

	return &MiscItemTable[Identifier];
}
_UpgradeTemplate *_Assets::GetUpgradeTemplate(const std::string &Identifier) {
	if(UpgradeTable.find(Identifier) == UpgradeTable.end())
		return NULL;

	return &UpgradeTable[Identifier];
}
_AmmoTemplate *_Assets::GetAmmoTemplate(const std::string &Identifier) {
	if(AmmoTable.find(Identifier) == AmmoTable.end())
		return NULL;

	return &AmmoTable[Identifier];
}
_WeaponTemplate *_Assets::GetWeaponTemplate(const std::string &Identifier) {
	if(WeaponTable.find(Identifier) == WeaponTable.end())
		return NULL;

	return &WeaponTable[Identifier];
}
_ArmorTemplate *_Assets::GetArmorTemplate(const std::string &Identifier) {
	if(ArmorTable.find(Identifier) == ArmorTable.end())
		return NULL;

	return &ArmorTable[Identifier];
}
_ItemGroup *_Assets::GetItemGroup(const std::string &Identifier) {
	if(ItemGroupTable.find(Identifier) == ItemGroupTable.end())
		return NULL;

	return &ItemGroupTable[Identifier];
}
_Style *_Assets::GetStyle(const std::string &Identifier) {
	if(Styles.find(Identifier) == Styles.end())
		return NULL;

	return Styles[Identifier];
}
_Element *_Assets::GetElement(const std::string &Identifier) {
	if(Elements.find(Identifier) == Elements.end())
		return NULL;

	return Elements[Identifier];
}
_Label *_Assets::GetLabel(const std::string &Identifier) { return (_Label *)GetElement(Identifier); }
_Image *_Assets::GetImage(const std::string &Identifier) { return (_Image *)GetElement(Identifier); }
_Button *_Assets::GetButton(const std::string &Identifier) { return (_Button *)GetElement(Identifier); }
_TextBox *_Assets::GetTextBox(const std::string &Identifier) { return (_TextBox *)GetElement(Identifier); }
