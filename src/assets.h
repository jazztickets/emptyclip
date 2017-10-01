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
#include <vector2.h>
#include <objects/templates.h>
#include <string>
#include <map>
#include <vector>

// Forward Declarations
class _Style;
class _Font;
class _Element;
class _Label;
class _Image;
class _Button;
class _TextBox;
class _Texture;
class _Animation;
class _Particle;
class _Entity;
class _Player;
class _Monster;
class _Weapon;
class _Armor;
class _MiscItem;
class _Upgrade;
class _Ammo;
struct _Reel;
struct _ReelTemplate;
struct _ParticleTemplate;
struct _MonsterTemplate;
struct _MiscItemTemplate;
struct _UpgradeTemplate;
struct _ArmorTemplate;

// Stores information about a collection of reel identifiers
struct AnimationTemplateStruct {
	AnimationTemplateStruct() { }

	std::vector<std::string> Identifiers;
};

// Stores information about a collection of sound samples used for attacking
struct AttackSampleTemplateStruct {
	AttackSampleTemplateStruct() { }

	std::string Samples[SAMPLE_TYPES];
};

// Used for the map editor
struct _Brush {
	_Brush() { }
	_Brush(const std::string &Identifier, const std::string &Text, _Texture *Texture, const _Color &Color)
		:	Identifier(Identifier),
			Text(Text),
			Texture(Texture),
			Color(Color) { }

	std::string Identifier, Text;
	_Texture *Texture;
	_Color Color;
};

// Used for level information
struct LevelStruct {
	int64_t Experience;
	int HealthBonus;
	int DamageBlockBonus;
	int SkillPoints;
};

// Holds skill information
struct SkillStruct {
	float Data[SKILL_COUNT];
};

// A single entry for an item group
struct ItemGroupEntryStruct {
	ItemGroupEntryStruct() { }
	ItemGroupEntryStruct(const std::string &ItemIdentifier, float Count, int Type)
		:	ItemIdentifier(ItemIdentifier),
		    Count(Count),
		    Type(Type) { }

	std::string ItemIdentifier;
	float Count;
	int Type;
};

// Item group information
struct _ItemGroup {
	_ItemGroup() { }

	std::vector<ItemGroupEntryStruct> Entries;
	int Quantity;
	float Total;
};

// Classes
class _Assets {

	public:

		void Init(const std::string &DatabasePath);
		void Close();

		bool Initialize();
		void LoadStringTable(const std::string &Filename);
		void LoadColorTable(const std::string &Filename);
		void LoadReelTable(const std::string &Filename);
		void LoadAnimationTable(const std::string &Filename);
		void LoadAttackSampleTable(const std::string &Filename);
		void LoadMonsterTable(const std::string &Filename);
		void LoadParticleTable(const std::string &Filename);
		void LoadTextures(const std::string &Filename);
		void LoadSamples(const std::string &Filename, const std::string &SamplePath);
		void LoadMiscItemTable(const std::string &Filename);
		void LoadUpgradeTable(const std::string &Filename);
		void LoadAmmoTable(const std::string &Filename);
		void LoadWeaponTable(const std::string &Filename);
		void LoadArmorTable(const std::string &Filename);
		void LoadItemDropTable(const std::string &Filename);

		void LoadFonts(const std::string &Filename);
		void LoadMonsterSet(const std::string &Filename);
		void LoadReel(const std::string &Identifier, const std::string &Path);
		void LoadAnimation(const std::string &Identifier, const std::string &Path);
		void LoadWeaponParticles(const std::string &Filename);
		void LoadMonsterAnimation();
		void LoadStyles(const std::string &Filename);
		void LoadElements(const std::string &Filename);
		void LoadLabels(const std::string &Filename);
		void LoadImages(const std::string &Filename);
		void LoadButtons(const std::string &Filename);
		void LoadTextBoxes(const std::string &Filename);

		bool IsStringLoaded(const std::string &Identifier);
		bool IsColorLoaded(const std::string &Identifier);
		bool IsTextureLoaded(const std::string &Identifier);
		bool IsAttackSampleLoaded(const std::string &Identifier);
		bool IsParticleLoaded(const std::string &Identifier);
		bool IsWeaponParticleTemplateLoaded(const std::string &Identifier);
		bool IsReelLoaded(const std::string &Identifier);
		bool IsAnimationLoaded(const std::string &Identifier);
		bool IsFontLoaded(const std::string &Identifier);
		bool IsMonsterLoaded(const std::string &Identifier);
		bool IsMiscItemLoaded(const std::string &Identifier);
		bool IsUpgradeLoaded(const std::string &Identifier);
		bool IsAmmoLoaded(const std::string &Identifier);
		bool IsWeaponLoaded(const std::string &Identifier);
		bool IsArmorLoaded(const std::string &Identifier);
		bool IsItemGroupLoaded(const std::string &Identifier);

		void UnloadStringTable();
		void UnloadColorTable();
		void UnloadReelTable();
		void UnloadAnimationTable();
		void UnloadAttackSampleTable();
		void UnloadParticleTable();
		void UnloadWeaponParticleTable();
		void UnloadMonsterTable();
		void UnloadMiscItemTable();
		void UnloadUpgradeTable();
		void UnloadAmmoTable();
		void UnloadWeaponTable();
		void UnloadArmorTable();
		void UnloadItemGroupTable();

		void UnloadTextures();
		void UnloadMonsterSet();
		void UnloadSamples();
		void UnloadFonts();
		void UnloadReel(const std::string &Identifier);
		void UnloadAnimation(const std::string &Identifier);
		void UnloadMonsterAnimation();
		void UnloadStyles();
		void UnloadElements();

		void SetAssetPath(const std::string &AssetPath) { this->AssetPath = AssetPath; }
		std::string GetAssetPath() const { return AssetPath; }

		int GetLevel(int64_t Experience);
		int64_t GetValidExperience(int64_t Experience);
		int64_t GetExperienceForLevel(int Level);
		int GetLevelHealth(int Level) { return Levels[Level-1].HealthBonus; }
		int GetLevelDamageBlock(int Level) { return Levels[Level-1].DamageBlockBonus; }
		int GetSkillPointsRemaining(int Level) { return Levels[Level-1].SkillPoints; }
		int GetValidSkill(int Level);
		float GetSkill(int Level, int Type) const { return Skills[Level].Data[Type]; }
		float GetSkillPercentImprovement(int Level, int Type) const { return (Skills[Level].Data[Type] - 1.0f) * 100.0f; }

		_Font *GetFont(const std::string &Identifier);
		_Style *GetStyle(const std::string &Identifier);
		_Element *GetElement(const std::string &Identifier);
		_Label *GetLabel(const std::string &Identifier);
		_Image *GetImage(const std::string &Identifier);
		_Button *GetButton(const std::string &Identifier);
		_TextBox *GetTextBox(const std::string &Identifier);
		_Texture *GetTexture(const std::string &Identifier);
		std::string GetString(const std::string &Identifier);
		const _Color &GetColor(const std::string &Identifier);
		_Reel *GetReel(const std::string &Identifier);
		AttackSampleTemplateStruct *GetAttackSampleTemplate(const std::string &Identifier);
		_Animation *GetAnimation(const std::string &Identifier);
		_ParticleTemplate *GetParticleTemplate(const std::string &Identifier);
		_WeaponParticleTemplate *GetWeaponParticleTemplate(const std::string &Identifer);
		_MonsterTemplate *GetMonsterTemplate(const std::string &Identifier);
		_MiscItemTemplate *GetMiscItemTemplate(const std::string &Identifier);
		_UpgradeTemplate *GetUpgradeTemplate(const std::string &Identifier);
		_AmmoTemplate *GetAmmoTemplate(const std::string &Identifier);
		_WeaponTemplate *GetWeaponTemplate(const std::string &Identifier);
		_ArmorTemplate *GetArmorTemplate(const std::string &Identifier);
		_ItemGroup *GetItemGroup(const std::string &Identifier);
		_Monster *CreateMonster(const std::string &Identifier, const Vector2 &Position);
		_MiscItem *CreateMiscItem(const std::string &Identifier, int Count, const Vector2 &Position);
		_Ammo *CreateAmmoItem(const std::string &Identifier, int Count, const Vector2 &Position);
		_Ammo *CreateAmmoItem(int Type);
		_Upgrade *CreateUpgradeItem(const std::string &Identifier, int Count, const Vector2 &Position);
		_Weapon *CreateWeapon(const std::string &Identifier, int Count, const Vector2 &Position, bool Generate);
		_Armor *CreateArmor(const std::string &Identifier, int Count, const Vector2 &Position);
		void GetRandomDrop(const _ItemGroup *ItemGroup, _ObjectSpawn *ObjectSpawn);

		void GetEventList(std::vector<_Brush> &Icons);
		void GetMonsterList(std::vector<_Brush> &Icons);
		void GetItemList(std::vector<_Brush> &Icons);
		void GetUpgradeList(std::vector<_Brush> &Icons);
		void GetAmmoList(std::vector<_Brush> &Icons);
		void GetWeaponList(std::vector<_Brush> &Icons);
		void GetArmorList(std::vector<_Brush> &Icons);

		void GetTextureList(std::vector<_Brush> &TextureList, int Group=-1);

	private:

		void LoadLevels();
		void LoadSkills();

		std::string AssetPath;

		// Tables
		std::map<std::string, std::string> StringTable;
		std::map<std::string, _ReelTemplate> ReelTable;
		std::map<std::string, AnimationTemplateStruct> AnimationTable;
		std::map<std::string, AttackSampleTemplateStruct> AttackSampleTable;
		std::map<std::string, _ParticleTemplate> ParticleTable;
		std::map<std::string, _WeaponParticleTemplate> WeaponParticleTable;
		std::map<std::string, _MonsterTemplate> MonsterTable;
		std::map<std::string, _MiscItemTemplate> MiscItemTable;
		std::map<std::string, _UpgradeTemplate> UpgradeTable;
		std::map<std::string, _AmmoTemplate> AmmoTable;
		std::map<std::string, _WeaponTemplate> WeaponTable;
		std::map<std::string, _ArmorTemplate> ArmorTable;
		std::map<std::string, _ItemGroup> ItemGroupTable;
		std::vector<std::string> MonsterSet;
		std::vector<LevelStruct> Levels;
		std::vector<SkillStruct> Skills;
		std::string AmmoTypeIdentifiers[AMMO_TYPES];

		// Data
		std::map<std::string, _Color> ColorTable;
		std::map<std::string, _Texture *> Textures;
		std::map<std::string, _Reel> Reels;
		std::map<std::string, _Animation *> Animations;
		std::map<std::string, _Style *> Styles;
		std::map<std::string, _Element *> Elements;
		std::map<std::string, _Font *> Fonts;
		_WeaponParticleTemplate BlankWeaponParticle;
};

extern _Assets Assets;
