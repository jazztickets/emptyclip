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
#include <objects/player.h>
#include <graphics.h>
#include <audio.h>
#include <assets.h>
#include <animation.h>
#include <utils.h>
#include <map.h>
#include <constants.h>
#include <buffer.h>
#include <utils.h>
#include <ui/ui.h>
#include <objects/monster.h>
#include <objects/misc.h>
#include <objects/ammo.h>
#include <objects/upgrade.h>
#include <objects/weapon.h>
#include <objects/armor.h>
#include <fstream>
#include <iostream>
#include <stdexcept>

enum SaveChunkTypes {
	CHUNK_SAVEVERSION,
	CHUNK_PLAYERNAME,
	CHUNK_COLOR,
	CHUNK_MAP,
	CHUNK_PROGRESSION,
	CHUNK_CHECKPOINT,
	CHUNK_EXPERIENCE,
	CHUNK_GOLD,
	CHUNK_HEALTH,
	CHUNK_TIME_PLAYED,
	CHUNK_MONSTER_KILLS,
	CHUNK_SKILLS,
	CHUNK_ITEMS,
};

// Constructor
_Player::_Player(const std::string &SavePath) {
	this->SavePath = SavePath;
	DebugLevel = 0;
	Type = _Object::PLAYER;

	// Set up animations
	LegAnimation = new _Animation();
	WalkingAnimation = PLAYER_ANIMATIONWALKINGONEHAND;
	MeleeAnimation = PLAYER_ANIMATIONMELEE;
	ShootingOnehandAnimation = PLAYER_ANIMATIONSHOOTONEHAND;
	ShootingTwohandAnimation = PLAYER_ANIMATIONSHOOTTWOHAND;
	DyingAnimation = PLAYER_ANIMATIONDYING;

	// Weapon offsets
	WeaponParticleOffset[0] = ZERO_VECTOR;
	WeaponParticleOffset[1] = PLAYER_PISTOLOFFSET;
	for(int i = 2; i < WEAPON_TYPES; i++)
		WeaponParticleOffset[i] = PLAYER_WEAPONOFFSET;

	// Inventory
	for(int i = 0; i < INVENTORY_SIZE; i++)
		Inventory[i] = nullptr;

	// Set animation
	SetTorsoAnimation(Assets.GetAnimation("player_torso"));
	SetLegAnimation(Assets.GetAnimation("player_legs"));

	// Set samples
	AttackSampleTemplateStruct *AttackSample = Assets.GetAttackSampleTemplate("player0");
	for(int i = 0; i < SAMPLE_TYPES; i++) {
		if(AttackSample)
			SetSample(i, AttackSample->Samples[i]);
	}

	Reset();
}

// Destructor
_Player::~_Player() {

	delete LegAnimation;

	DeleteItems();
}

// Resets the player state
void _Player::Reset() {

	// Set stats
	MonsterKills = TimePlayed = 0;
	Radius = PLAYER_RADIUS;
	Name = PLAYER_DEFAULTNAME;
	ColorIdentifier = "white";
	Level = 1;
	Gold = 0;
	Experience = 0;
	ExperienceCurrentLevel = 0;
	ExperienceNextLevel = 0;
	LevelPercentage = 0.0f;
	SkillPointsRemaining = 0;

	for(int i = 0; i < SKILL_COUNT; i++)
		Skills[i] = 0;

	DeleteItems();

	// Reset state
	MapIdentifier = GAME_STARTLEVEL;
	CheckpointIndex = 0;
	Progression = 0;
	Active = true;
	Action = ACTION_IDLE;
	MoveSoundDelay = 0;
	Reloading = SwitchingWeapons = Crouching = Sprinting = AttackRequested = UseRequested = MedkitRequested = false;
	AttackAllowed = true;
	UsePeriod = PLAYER_USEPERIOD;
	ZoomScale = PLAYER_ZOOMSCALE;
	LegDirection = 0.0f;
	MovementSpeed = PLAYER_MOVEMENTSPEED;
	MoveState = MOVE_NONE;
	WeaponSwitchTimer = ReloadTimer = UseTimer = MedkitTimer = 0;
	WeaponSwitchFrom = -1;
	WeaponSwitchTo = -1;
	TimePlayed = 0;
	PlayingTimer = 0;
	Stamina = 100.0f;

	CalculateExperienceStats();
	CalculateLevelPercentage();
	CalculateSkillsRemaining();
	UpdateColor();

	Animation->ChangeReel(PLAYER_ANIMATIONWALKINGONEHAND);
	Animation->SetPlayMode(STOPPED);
	LegAnimation->SetPlayMode(STOPPED);

	RecalculateStats();
	ResetWeaponAnimation();
	StopAudio();

	CurrentHealth = MaxHealth;
}

// Loads information from a file
void _Player::Load() {
	Reset();

	// Open file
	std::ifstream File(SavePath.c_str(), std::ios::in | std::ios::binary);
	if(!File)
		throw std::runtime_error("Cannot load save file: " + SavePath);

	// Read file
	while(!File.eof() && File.peek() != EOF) {

		// Get chunk type
		int Type;
		File.read((char *)&Type, sizeof(Type));

		// Get chunk size
		size_t Size;
		File.read((char *)&Size, sizeof(Size));

		switch(Type) {
			case CHUNK_SAVEVERSION: {
				int SaveVersion;
				File.read((char *)&SaveVersion, sizeof(SaveVersion));

				if(SaveVersion != PLAYER_SAVEVERSION)
					throw std::runtime_error("Save version mismatch");
			} break;
			case CHUNK_PLAYERNAME: {
				char Buffer[1024];
				File.read(Buffer, Size);
				Buffer[Size] = 0;
				Name = Buffer;
				//std::cout << "Name: " << Name << std::endl;
			} break;
			case CHUNK_COLOR: {
				char Buffer[1024];
				File.read(Buffer, Size);
				Buffer[Size] = 0;
				ColorIdentifier = Buffer;
				//std::cout << "ColorIdentifier: " << ColorIdentifier << std::endl;
			} break;
			case CHUNK_MAP: {
				char Buffer[1024];
				File.read(Buffer, Size);
				Buffer[Size] = 0;
				MapIdentifier = Buffer;
				//std::cout << "MapIdentifier: " << MapIdentifier << std::endl;
			} break;
			case CHUNK_CHECKPOINT:
				File.read((char *)&CheckpointIndex, sizeof(CheckpointIndex));
				//std::cout << "CheckpointIndex: " << CheckpointIndex << std::endl;
			break;
			case CHUNK_PROGRESSION:
				File.read((char *)&Progression, sizeof(Progression));
				//std::cout << "Progression: " << Progression << std::endl;
			break;
			case CHUNK_GOLD:
				File.read((char *)&Gold, sizeof(Gold));
				//std::cout << "Gold: " << Gold << std::endl;
			break;
			case CHUNK_EXPERIENCE:
				File.read((char *)&Experience, sizeof(Experience));
				//std::cout << "Experience: " << Experience << std::endl;
			break;
			case CHUNK_HEALTH:
				File.read((char *)&CurrentHealth, sizeof(CurrentHealth));
				if(CurrentHealth <= 0)
					CurrentHealth = 1;
				//std::cout << "CurrentHealth: " << CurrentHealth << std::endl;
			break;
			case CHUNK_TIME_PLAYED: {
				File.read((char *)&TimePlayed, sizeof(TimePlayed));
				//std::cout << "TimePlayed: " << TimePlayed << std::endl;
			} break;
			case CHUNK_MONSTER_KILLS:
				File.read((char *)&MonsterKills, sizeof(MonsterKills));
				//std::cout << "MonsterKills: " << MonsterKills << std::endl;
			break;
			case CHUNK_SKILLS:
				File.read((char *)&Skills, sizeof(Skills));
				//std::cout << "Skills: " << Skills[0] << " " << Skills[1] << " " << Skills[2] << " " << Skills[3] << " " << Skills[4] << std::endl;
			break;
			case CHUNK_ITEMS: {
				_Buffer Buffer(Size);
				File.read(&Buffer[0], Size);
				LoadItems(Buffer);
			} break;
			default:
				//std::cout << "Unknown chunk: " << Type << " size: " << Size << std::endl;
				File.ignore(Size);
			break;
		}
	}

	File.close();
	//std::cout << std::endl;

	CalculateExperienceStats();
	CalculateLevelPercentage();
	CalculateSkillsRemaining();
	UpdateColor();
	RecalculateStats();
	ResetWeaponAnimation();
	UpdateHealth(0);
}

// Saves information to a file
void _Player::Save() {

	// Open file
	std::ofstream File(SavePath.c_str(), std::ios::out | std::ios::binary);
	if(!File.is_open()) {
		throw std::runtime_error("Cannot create save file: " + SavePath);
	}

	WriteChunk(File, CHUNK_SAVEVERSION, (char *)&PLAYER_SAVEVERSION, sizeof(PLAYER_SAVEVERSION));
	WriteChunk(File, CHUNK_PLAYERNAME, Name.c_str(), Name.length());
	WriteChunk(File, CHUNK_COLOR, ColorIdentifier.c_str(), ColorIdentifier.length());
	if(Map) {
		WriteChunk(File, CHUNK_MAP, MapIdentifier.c_str(), MapIdentifier.length());
		WriteChunk(File, CHUNK_CHECKPOINT, (char *)&CheckpointIndex, sizeof(CheckpointIndex));
	}
	WriteChunk(File, CHUNK_PROGRESSION, (char *)&Progression, sizeof(Progression));
	WriteChunk(File, CHUNK_EXPERIENCE, (char *)&Experience, sizeof(Experience));
	WriteChunk(File, CHUNK_GOLD, (char *)&Gold, sizeof(Gold));
	WriteChunk(File, CHUNK_HEALTH, (char *)&CurrentHealth, sizeof(CurrentHealth));
	WriteChunk(File, CHUNK_TIME_PLAYED, (char *)&TimePlayed, sizeof(TimePlayed));
	WriteChunk(File, CHUNK_MONSTER_KILLS, (char *)&MonsterKills, sizeof(MonsterKills));
	WriteChunk(File, CHUNK_SKILLS, (char *)&Skills, sizeof(Skills));

	SaveItems(File);

	File.close();
}

// Loads items from a stream
void _Player::LoadItems(_Buffer &Buffer) {

	// Get inventory size
	int ItemCount = Buffer.Read<int>();
	if(ItemCount > INVENTORY_SIZE) {
		throw std::runtime_error("Too many items");
	}

	// Get items
	for(int i = 0; i < ItemCount; i++) {
		int Slot = Buffer.Read<int>();
		int Type = Buffer.Read<int>();
		//int Quality = Buffer.Read<int>();
		Buffer.Read<int>();
		int Count = Buffer.Read<int>();
		std::string Identifier;

		// Create items
		switch(Type) {
			case _Object::MISCITEM:
				Identifier = Buffer.ReadString();
				Inventory[Slot] = Assets.CreateMiscItem(Identifier, Count, ZERO_VECTOR);
			break;
			case _Object::AMMO:
				Identifier = Buffer.ReadString();
				Inventory[Slot] = Assets.CreateAmmoItem(Identifier, Count, ZERO_VECTOR);
			break;
			case _Object::UPGRADE:
				Identifier = Buffer.ReadString();
				Inventory[Slot] = Assets.CreateUpgradeItem(Identifier, Count, ZERO_VECTOR);
			break;
			case _Object::WEAPON:
				LoadWeapon(Buffer, Count, Slot);
			break;
			case _Object::ARMOR:
				Identifier = Buffer.ReadString();
				Inventory[Slot] = Assets.CreateArmor(Identifier, Count, ZERO_VECTOR);
			break;
		}
	}
}

// Loads weapons from a stream
void _Player::LoadWeapon(_Buffer &Buffer, int Count, int InventoryIndex) {

	// Get weapons
	std::string Identifier = Buffer.ReadString();
	int Ammo = Buffer.Read<int>();
	int MaxComponents = Buffer.Read<int>();

	// Create weapon
	_Weapon *Weapon = Assets.CreateWeapon(Identifier, Count, ZERO_VECTOR, false);
	Weapon->SetMaxComponents(MaxComponents);
	LoadUpgrades(Buffer, Weapon);
	Weapon->RecalculateStats();
	Weapon->SetAmmo(Ammo);

	Inventory[InventoryIndex] = Weapon;
}

// Loads upgrade components from a stream
void _Player::LoadUpgrades(_Buffer &Buffer, _Weapon *Weapon) {

	// Get size header
	int Components = Buffer.Read<int>();

	// Read data
	for(int i = 0; i < Components; i++) {
		std::string Identifier = Buffer.ReadString();
		_Upgrade *Upgrade = Assets.CreateUpgradeItem(Identifier, 1, ZERO_VECTOR);
		if(!Weapon->AddComponent(Upgrade))
			delete Upgrade;
	}
}

// Saves items to a stream
void _Player::SaveItems(std::ofstream &File) {

	// Buffer
	int ItemCount = 0;
	for(int i = 0; i < INVENTORY_SIZE; i++) {
		if(HasInventory(i))
			ItemCount++;
	}

	// Write item count
	_Buffer Buffer;
	Buffer.Write<int>(ItemCount);

	// Write items
	for(int i = 0; i < INVENTORY_SIZE; i++) {
		if(HasInventory(i)) {
			Buffer.Write(i);
			Buffer.Write(Inventory[i]->GetType());
			Buffer.Write(Inventory[i]->GetQuality());
			Buffer.Write(Inventory[i]->GetCount());
			Inventory[i]->Serialize(Buffer);
		}
	}

	// Write chunk
	WriteChunk(File, CHUNK_ITEMS, &Buffer[0], Buffer.GetCurrentSize());
}

// Deletes the item objects
void _Player::DeleteItems() {
	for(int i = 0; i < INVENTORY_SIZE; i++) {
		delete Inventory[i];
		Inventory[i] = nullptr;
	}
}

// Updates the entity's states
void _Player::Update(double FrameTime) {
	_Entity::Update(FrameTime);

	PlayingTimer += FrameTime;
	WeaponSwitchTimer += FrameTime;
	ReloadTimer += FrameTime;
	UseTimer += FrameTime;
	MedkitTimer += FrameTime;

	if(PlayingTimer > 1.0) {
		TimePlayed++;
		PlayingTimer -= 1.0;
	}

	// Update stamina
	if(!IsDying())
		Stamina += PLAYER_STAMINAREGEN * StaminaRegenModifier;

	if(Stamina > MaxStamina)
	   Stamina = MaxStamina;
	if(Tired && Stamina > PLAYER_TIREDTHRESHOLD)
		Tired = false;

	// Check timer to see if the object can attack
	if(!AttackAllowed && FireTimer >= FirePeriod) {
		AttackAllowed = true;
	}

	// Update states
	UpdateAnimation(FrameTime);
	UpdateRecoil();
	UpdateReloading();
	UpdateWeaponSwitch();

	// Stop trigger down audio
	if(TriggerDownAudio && (!GetAttackRequested() || !HasAmmo() || IsDying() || IsSwitchingWeapons() || IsReloading())) {
		StopAudio();
	}

	// Make an attack
	if(GetAttackRequested()) {
		StartAttack();
		SetAttackRequested(false);
	}

	// Use a medkit
	if(GetMedkitRequested()) {
		UseMedkit(FindMiscItem(MISCITEM_MEDKIT));
		SetMedkitRequested(false);
	}

	Move();

	if(Stamina > 0.0f && PositionChanged && IsSprinting()) {
		Stamina -= PLAYER_SPRINTSTAMINA;
		if(Stamina < 0.0f) {
			Stamina = 0.0f;
			SetSprinting(false);
			Tired = true;
		}
	}

	if(TriggerDownAudio)
		TriggerDownAudio->SetPosition(Position);
}

// Updates the leg's animation and direction
void _Player::UpdateAnimation(double FrameTime) {
	::_Entity::UpdateAnimation(FrameTime);

	LegAnimation->Update(FrameTime);

	switch(MoveState) {
		case MOVE_FORWARD:
			AdjustLegDirection(0);
		break;
		case MOVE_BACKWARD:
			AdjustLegDirection(180);
		break;
		case MOVE_LEFT:
			AdjustLegDirection(270);
		break;
		case MOVE_RIGHT:
			AdjustLegDirection(90);
		break;
		case MOVE_FORWARDLEFT:
			AdjustLegDirection(315);
		break;
		case MOVE_FORWARDRIGHT:
			AdjustLegDirection(45);
		break;
		case MOVE_BACKWARDLEFT:
			AdjustLegDirection(225);
		break;
		case MOVE_BACKWARDRIGHT:
			AdjustLegDirection(135);
		break;
		default:
		break;
	}

}

// Determines how much to move the leg direction
void _Player::AdjustLegDirection(float Destination) {
	float Distance, Adjust;

	Distance = Destination - LegDirection;

	// Get deltas
	if(Distance < -180.0f)
		Adjust = -(Distance + 180.0f) / PLAYER_LEGCHANGEFACTOR;
	else if(Distance > 180.0f)
		Adjust = -(Distance - 180.0f) / PLAYER_LEGCHANGEFACTOR;
	else
		Adjust = Distance / PLAYER_LEGCHANGEFACTOR;

	// Update leg
	if(std::abs(Adjust) < 0.1f)
		LegDirection = Destination;
	else
		LegDirection += Adjust;

	// Cap direction
	if(LegDirection < 0.0f)
		LegDirection += 360.0f;
	else if(LegDirection >= 360.0f)
		LegDirection -= 360.0f;
}

// Draws the player
void _Player::Render(double BlendFactor) {
	Vector2 DrawPosition(Position * BlendFactor + LastPosition * (1.0 - BlendFactor));

	Graphics.DrawTexture(DrawPosition[0], DrawPosition[1], PositionZ, LegAnimation->GetCurrentFrame(), Color, LegDirection, Scale, Scale);
	Graphics.DrawTexture(DrawPosition[0], DrawPosition[1], PositionZ + 0.01f, Animation->GetCurrentFrame(), COLOR_WHITE, Rotation, Scale, Scale);
}

// Draws the player in screen space
void _Player::Render2D(const _Point &Position) {
	Graphics.DrawTexture(Position.X, Position.Y, 0, LegAnimation->GetCurrentFrame(), Color, Rotation, LegAnimation->GetCurrentFrame()->GetWidth(), LegAnimation->GetCurrentFrame()->GetHeight());
	Graphics.DrawTexture(Position.X, Position.Y, 0 + 0.01f, Animation->GetCurrentFrame(), COLOR_WHITE, Rotation, Animation->GetCurrentFrame()->GetWidth(), Animation->GetCurrentFrame()->GetHeight());
}

// Updates the player's experience, leveling up if needed
void _Player::UpdateExperience(int64_t ExperienceGained) {
	Experience = Assets.GetValidExperience(Experience + ExperienceGained);

	// Check if enough experience has been reached for a new level.
	if(Experience >= ExperienceNextLevel) {
		UpdateLevel();
	}

	CalculateLevelPercentage();
}

// Advance the player levels
void _Player::UpdateLevel() {
	int OldLevel = Level;

	// Get new level
	CalculateExperienceStats();

	// Get the number of skill points to spend
	CalculateSkillsRemaining();

	// Reset stats
	RecalculateStats();

	// Update current health
	UpdateHealth(Assets.GetLevelHealth(Level) - Assets.GetLevelHealth(OldLevel));
}

// Updates a skill
void _Player::UpdateSkill(int Index, int Value) {
	int TentativeSum = Value;

	for(int i = 0; i < SKILL_COUNT; i++)
		TentativeSum += Skills[i];

	// Check to make sure skill points don't exceed level
	if(TentativeSum > Assets.GetSkillPointsRemaining(Level))
		return;

	Skills[Index] = Assets.GetValidSkill(Skills[Index] + Value);
	CalculateSkillsRemaining();

	// Update player stats
	RecalculateStats();
}

// Calculates the level and experience variables
void _Player::CalculateExperienceStats() {
	Level = Assets.GetLevel(Experience);
	ExperienceCurrentLevel = Assets.GetExperienceForLevel(Level);
	ExperienceNextLevel = Assets.GetExperienceForLevel(Level + 1);
}

// Calculates the number of skills points remaining
void _Player::CalculateSkillsRemaining() {
	SkillPointsRemaining = Assets.GetSkillPointsRemaining(Level) - SpentSkillPoints();
}

// Calculates the percentage to the player's next level
void _Player::CalculateLevelPercentage() {
	LevelPercentage = static_cast<float>(Experience - ExperienceCurrentLevel) / static_cast<float>(ExperienceNextLevel - ExperienceCurrentLevel);
}

// Returns the number of skill points the player has spent
int _Player::SpentSkillPoints() const {
	int Sum = 0;
	for(int i = 0; i < SKILL_COUNT; i++)
		Sum += Skills[i];

	return Sum;
}

// Adds an item to the player's possession, returns 0 on full, return 2 on combine
int _Player::AddItem(_Item *Item) {

	switch(Item->GetType()) {
		case _Object::WEAPON:
			if(!HasMainHand()) {
				SetMainHand(static_cast<_Weapon *>(Item));
				RecalculateStats();
				ResetWeaponAnimation();
				return 1;
			}
			else if(!HasOffHand()) {
				SetOffHand(static_cast<_Weapon *>(Item));
				return 1;
			}
			else
				return AddInventory(Item);
		break;
		case _Object::ARMOR: {
			_Armor *ArmorItem = static_cast<_Armor *>(Item);
			if(!HasArmor() && Assets.GetSkill(Skills[SKILL_STRENGTH], SKILL_STRENGTH) >= ArmorItem->GetStrengthRequirement()) {
				SetArmor(ArmorItem);
				RecalculateStats();
				return 1;
			}
			else {
				return AddInventory(ArmorItem);
			}
		} break;
		default:
			return AddInventory(Item);
		break;
	}

	return 0;
}

// Drops an item from the player's inventory
void _Player::DropItem(int Slot) {
	if(!CanDropItem() || Slot < 0 || Slot >= INVENTORY_SIZE)
		return;

	// Get item
	_Item *Item = Inventory[Slot];
	if(!Item)
		return;

	// Remove item from inventory
	Inventory[Slot] = nullptr;

	// Check if the item was equipped
	if(Slot < INVENTORY_BAGSTART) {
		RecalculateStats();
		ResetWeaponAnimation();
	}

	// Add item to map
	Item->SetPosition(GetPosition() + GenerateRandomPointInCircle(PLAYER_RADIUS));
	Map->AddItem(Item);
}

// Equip an item
bool _Player::CanEquipItem(_Item *Item, int Slot) {
	if(!Item)
		return true;

	switch(Slot) {
		case INVENTORY_ARMOR: {
			if(Item->GetType() != _Object::ARMOR)
				return false;

			_Armor *Armor = (_Armor *)Item;
			return Assets.GetSkill(Skills[SKILL_STRENGTH], SKILL_STRENGTH) >= Armor->GetStrengthRequirement();
		} break;
		case INVENTORY_MAINHAND:
		case INVENTORY_OFFHAND:
			return Item->GetType() == _Object::WEAPON;
		break;
	}

	return false;
}

// Swap inventory
void _Player::SwapInventory(int SlotFrom, int SlotTo) {
	if(SlotFrom == SlotTo)
		return;

	bool CanSwap = false;

	// Check for simple swap
	if(IsBagIndex(SlotFrom) && IsBagIndex(SlotTo)) {
		CanSwap = true;
	}
	// Equipment swap
	else if((IsEquipmentIndex(SlotFrom) && IsBagIndex(SlotTo)) || (IsEquipmentIndex(SlotTo) && IsBagIndex(SlotFrom)) || (IsEquipmentIndex(SlotTo) && IsEquipmentIndex(SlotFrom))) {
		if(IsEquipmentIndex(SlotTo)) {

			// Try component
			if(!AddComponent(SlotFrom, SlotTo)) {
				CanSwap = CanEquipItem(Inventory[SlotFrom], SlotTo);
			}
		}
		else
			CanSwap = CanEquipItem(Inventory[SlotTo], SlotFrom);
	}

	if(CanSwap) {

		if(SlotTo == INVENTORY_MAINHAND || (IsHandIndex(SlotFrom) && IsHandIndex(SlotTo))) {
			StartWeaponSwitch(SlotFrom, SlotTo);
		}
		else {

			// Try to combine items
			int CombineResult = CombineItems(Inventory[SlotFrom], Inventory[SlotTo]);
			if(CombineResult == 0) {
				_Item *Temp = Inventory[SlotFrom];
				Inventory[SlotFrom] = Inventory[SlotTo];
				Inventory[SlotTo] = Temp;
			}
			else if(CombineResult == 2) {
				delete Inventory[SlotFrom];
				Inventory[SlotFrom] = nullptr;
			}
		}

		RecalculateStats();
		ResetWeaponAnimation();
	}
}

// Attempts to combine two items and deletes FromItem if successful
// Return 0 when item can't be combined
// Return 1 when item was combined but still has count left
// Return 2 when item was combined and fromitem needs deletion
int _Player::CombineItems(_Item *FromItem, _Item *ToItem) {
	if(FromItem && ToItem && FromItem->CanStack() && ToItem->CanStack() && FromItem->GetIdentifier() == ToItem->GetIdentifier()) {
		ToItem->UpdateCount(FromItem->GetCount());
		if(ToItem->GetCount() > GetInventoryMaxStack()) {
			FromItem->SetCount(ToItem->GetCount() - GetInventoryMaxStack());
			ToItem->SetCount(GetInventoryMaxStack());
			return 1;
		}
		else {
			return 2;
		}
	}

	return 0;
}

// Add an item to the inventory
// return 0 on inventory full
// return 1 on added item
// return 2 on added item and combined
int _Player::AddInventory(_Item *Item) {

	// Search for an existing item or empty slot
	int EmptySlot = -1;
	for(int i = INVENTORY_BAGSTART; i < INVENTORY_BAGEND; i++) {
		if(CombineItems(Item, Inventory[i]) == 2) {
			return 2;
		}

		if(Inventory[i] == nullptr && EmptySlot == -1)
			EmptySlot = i;
	}

	// Add item to empty slot
	if(EmptySlot != -1) {
		Inventory[EmptySlot] = Item;
		return 1;
	}

	return 0;
}

// Add an upgrade to a weapon
bool _Player::AddComponent(int FromIndex, int ToIndex) {
	_Weapon *Weapon = nullptr;

	if(!HasInventory(FromIndex) || Inventory[FromIndex]->GetType() != _Object::UPGRADE)
		return false;

	if(ToIndex == INVENTORY_MAINHAND && HasMainHand())
		Weapon = GetMainHand();
	else if(ToIndex == INVENTORY_OFFHAND && HasOffHand())
		Weapon = GetOffHand();
	else
		return false;

	if(Weapon->AddComponent(static_cast<_Upgrade *>(Inventory[FromIndex]))) {
		ConsumeInventory(FromIndex, false);
		RecalculateStats();
		return true;
	}

	return false;
}

// Calculates the radius of the crosshair
float _Player::GetCrosshairRadius(const Vector2 &Cursor) {
	float Distance, Accuracy;

	// Check bounds
	Accuracy = CurrentAccuracy * AccuracyModifier;
	if(Accuracy < 0.0f)
		Accuracy = 0.0f;
	else if(Accuracy > PLAYER_MAXACCURACY)
		Accuracy = PLAYER_MAXACCURACY;

	// Get distance to cursor
	Distance = (Cursor - Position).Magnitude();

	return tan((Accuracy * 0.5f) / DEGREES_IN_RADIAN) * Distance;
}

// Determines what type of ammo is required by the weapon the player is using
int _Player::GetWeaponAmmoType() const {
	if(!HasMainHand())
		return AMMO_NONE;

	return GetMainHand()->GetAmmoType();
}

// Determines what type of ammo an item in the inventory is
int _Player::GetInventoryAmmoType(int Index) const {
	if(HasInventory(Index) && Inventory[Index]->GetType() == _Object::AMMO)
		return static_cast<_Ammo *>(Inventory[Index])->GetAmmoType();

	return -1;
}

// Checks if the item is the right ammo for the player's mainhand weapon
bool _Player::IsRightClip(const _Item *Item) const {
	if(Item->GetType() == _Object::AMMO) {
		const _Ammo *Ammo = static_cast<const _Ammo *>(Item);
		if(Ammo->GetAmmoType() == GetWeaponAmmoType())
			return true;
	}

	return false;
}

// Checks if the player's weapon has ammo
bool _Player::HasAmmo() const {
	if(!HasMainHand() || GetMainHand()->GetAmmoType() == AMMO_NONE)
		return true;

	return GetMainHand()->GetAmmo() > 0;
}

// Checks if the player has ammo for the current weapon
bool _Player::HasClips() const {

	// Search inventory for clip
	for(int i = INVENTORY_BAGSTART; i < INVENTORY_BAGEND; i++) {
		if(HasInventory(i) && IsRightClip(Inventory[i])) {
			return true;
		}
	}

	return false;
}

// Reduces the weapons ammo by one
void _Player::ReduceAmmo() {
	if(!HasMainHand())
		return;

	GetMainHand()->ReduceAmmo();
}

// Uses an item from the player's inventory, return true if a key was used
bool _Player::UseItem(int Index, bool Event) {

	if(Index >= INVENTORY_BAGSTART && Index < INVENTORY_BAGEND && HasInventory(Index)) {

		_MiscItem *MiscItem;
		switch(Inventory[Index]->GetType()) {
			case _Object::MISCITEM:
				MiscItem = (_MiscItem *)Inventory[Index];
				switch(MiscItem->GetMiscItemType()) {
					case MISCITEM_MEDKIT:
						UseMedkit(Index);
					break;
					case MISCITEM_OTHER:
						if(Event) {
							ConsumeInventory(Index);
							return true;
						}
					break;
				}
			break;
		}
	}

	return false;
}

// Uses a medkit if one is available
bool _Player::UseMedkit(int Index) {
	if(CanUseMedkit() && HasInventory(Index) && Inventory[Index]->GetType() == _Object::MISCITEM) {
		_MiscItem *MiscItem = static_cast<_MiscItem *>(Inventory[Index]);
		if(MiscItem->GetMiscItemType() == MISCITEM_MEDKIT) {

			int Amount = GetMedkitHealAmount(MiscItem->GetLevel());
			UpdateHealth(Amount);
			ConsumeInventory(Index);
			MedkitTimer = 0;

			if(DebugLevel > 1) {
				std::cout << "***UseMedkit***\n";
				std::cout << "Amount: " << Amount << '\n' << std::endl;
			}
			return true;
		}
	}
	return false;
}

// Returns the amount of health given from a medkit
int _Player::GetMedkitHealAmount(int MedkitLevel) const {

	return MedkitLevel * ITEM_MEDKIT_HEALTH;
}

// Searches for an misc item and returns the index
int _Player::FindMiscItem(int ItemType) {
	_MiscItem *MiscItem;

	for(int i = INVENTORY_BAGSTART; i < INVENTORY_BAGEND; i++) {
		if(HasInventory(i) && Inventory[i]->GetType() == _Object::MISCITEM) {
			MiscItem = static_cast<_MiscItem *>(Inventory[i]);

			if(MiscItem->GetMiscItemType() == ItemType)
				return i;
		}
	}

	return -1;
}

// Searchs the inventory for a certain item
int _Player::FindItem(const std::string &Identifier) {

	for(int i = INVENTORY_BAGSTART; i < INVENTORY_BAGEND; i++) {
		if(HasInventory(i) && Inventory[i]->GetIdentifier() == Identifier) {
			return i;
		}
	}

	return -1;
}

// Begins the reloading process
void _Player::StartReloading() {

	// Test conditions
	if(!CanReload())
		return;

	Audio.Play(new _AudioSource(Audio.GetBuffer(GetSample(SAMPLE_RELOAD)), true));

	// Start timer
	ReloadTimer = 0;
	Reloading = true;
}

// Cancel the reload process
void _Player::CancelReloading() {

	Reloading = false;
}

// Begins the weapon switch process
void _Player::StartWeaponSwitch(int SlotFrom, int SlotTo) {

	// Test conditions
	if(!CanSwitchWeapons())
		return;

	// Test for empty hands
	if(IsHandIndex(SlotFrom) && IsHandIndex(SlotTo) && !GetMainHand() && !GetOffHand())
		return;

	// Start timer
	WeaponSwitchFrom = SlotFrom;
	WeaponSwitchTo = SlotTo;
	WeaponSwitchTimer = 0;
	SwitchingWeapons = true;
}

// Reloads the weapon when the timer goes off
void _Player::UpdateReloading() {

	// Check the timer
	if(Reloading && (ReloadTimer > ReloadPeriod)) {
		Reloading = false;

		// Check weapon type
		if(CanReload()) {

			// Search inventory for clip
			for(int i = INVENTORY_BAGSTART; i < INVENTORY_BAGEND; i++) {
				if(HasInventory(i) && IsRightClip(Inventory[i])) {
					GetMainHand()->SetAmmo(GetMainHand()->GetRoundSize());
					ConsumeInventory(i);

					// Update accuracy
					ResetAccuracy(true);
					ResetWeaponAnimation();
					return;
				}
			}
		}
	}
}

// Switches the weapon when the timer goes off
void _Player::UpdateWeaponSwitch() {

	// Check the timer
	if(SwitchingWeapons && (WeaponSwitchTimer > WeaponSwitchPeriod)) {
		SwitchingWeapons = false;

		// Check weapon type
		if(CanSwitchWeapons()) {
			_Item *Temp = Inventory[WeaponSwitchFrom];
			Inventory[WeaponSwitchFrom] = Inventory[WeaponSwitchTo];
			Inventory[WeaponSwitchTo] = Temp;

			RecalculateStats();
			ResetWeaponAnimation();
		}
	}
}

// Updates the states for crouching and running
void _Player::UpdateSpeed(float Factor) {

	if(Crouching) {
		MovementModifier = PLAYER_CROUCHINGSPEEDFACTOR;
	}
	else if(Sprinting) {
		MovementModifier = PLAYER_SPRINTINGSPEEDFACTOR;
	}
	else {
		MovementModifier = 1.0f;
	}
	MovementModifier *= Factor;

	MoveSoundDelay = ENTITY_MOVESOUNDDELAYFACTOR / (MovementSpeed * MovementModifier);
	LegAnimation->SetPlaybackSpeedFactor(1.0f / MovementModifier);
	if(Animation->GetCurrentReel() == PLAYER_ANIMATIONWALKINGONEHAND || Animation->GetCurrentReel() == PLAYER_ANIMATIONWALKINGTWOHAND)
		SetAnimationPlaybackSpeedFactor();
}

// Updates the states for crouching
void _Player::SetCrouching(bool State) {

	// Update state
	if(Crouching != State) {
		Crouching = State;
		ResetAccuracy(false);
		UpdateSpeed(1.0f);
	}

	if(Crouching)
		SetSprinting(false);
}

// Update state for sprinting
void _Player::SetSprinting(bool State) {
	if(State && Tired)
		return;

	// Update state
	if(Sprinting != State) {
		Sprinting = State;
		ResetAccuracy(false);
		UpdateSpeed(1.0f);
	}

	if(Sprinting)
		SetCrouching(false);
}

// Resets the accuracy depending on crouching states
void _Player::ResetAccuracy(bool CompleteReset) {

	if(Crouching && !IsMelee()) {
		AccuracyModifier = 0.5f;
	}
	else if(Sprinting && !IsMelee()) {
		AccuracyModifier = 2.0f;
	}
	else {
		AccuracyModifier = 1.0f;

	}
	if(CompleteReset)
		CurrentAccuracy = MinAccuracyNormal;

	MinAccuracy = MinAccuracyNormal;
	MaxAccuracy = MaxAccuracyNormal;
}

// Consume an item from the inventory
void _Player::ConsumeInventory(int Index, bool Delete) {
	if(Index >= INVENTORY_BAGSTART && Index < INVENTORY_BAGEND && Inventory[Index] != nullptr) {
		if(Inventory[Index]->UpdateCount(-1) <= 0) {
			if(Delete)
				delete Inventory[Index];
			Inventory[Index] = nullptr;
		}
	}
}

// Calculates the player's stats from weapons and skills
void _Player::RecalculateStats() {
	_WeaponTemplate Weapon;
	float DamageMultiplier, LevelPercent;

	// See if the player is using a weapon
	if(HasMainHand()) {

		// Get weapon stats
		Weapon.MinAccuracy = GetMainHand()->GetMinAccuracy();
		Weapon.MaxAccuracy = GetMainHand()->GetMaxAccuracy();
		Weapon.Recoil = GetMainHand()->GetRecoil();
		Weapon.RecoilRegen = GetMainHand()->GetRecoilRegen();
		Weapon.Range = GetMainHand()->GetRange();
		Weapon.FireRate = GetMainHand()->GetFireRate();
		Weapon.FirePeriod = GetMainHand()->GetFirePeriod();
		Weapon.MinDamage = GetMainHand()->GetMinDamage();
		Weapon.MaxDamage = GetMainHand()->GetMaxDamage();
		Weapon.ReloadPeriod = GetMainHand()->GetReloadPeriod();
		Weapon.BulletsShot = GetMainHand()->GetBulletsShot();
		Weapon.ZoomScale = GetMainHand()->GetZoomScale();
		WeaponType = GetMainHand()->GetWeaponType();
	}
	else
		WeaponType = WEAPON_MELEE;

	// Apply skills
	if(WeaponType == WEAPON_MELEE) {
		AttackRange = Weapon.Range * 1;
		CurrentAccuracyNormal = MinAccuracyNormal = Weapon.MinAccuracy;
		MaxAccuracyNormal = Weapon.MaxAccuracy;
		DamageMultiplier = 1;
		Recoil = Weapon.Recoil;
		RecoilRegen = Weapon.RecoilRegen;
	}
	else {
		AttackRange = Weapon.Range;
		float AccuracySkillMultiplier = 1.0f / Assets.GetSkill(Skills[SKILL_ACCURACY], SKILL_ACCURACY);
		CurrentAccuracyNormal = MinAccuracyNormal = Weapon.MinAccuracy * AccuracySkillMultiplier;
		MaxAccuracyNormal = Weapon.MaxAccuracy * AccuracySkillMultiplier;
		DamageMultiplier = 1;

		// Recoil bonus is hard coded
		LevelPercent = 0;
		Recoil = Weapon.Recoil - Weapon.Recoil * LevelPercent * PLAYER_RECOILSKILLFACTOR;
		RecoilRegen = Weapon.RecoilRegen + Weapon.RecoilRegen * LevelPercent * PLAYER_RECOILREGENSKILLFACTOR;
	}

	// Set accuracy
	ResetAccuracy(true);

	// Attacking
	FireRate = Weapon.FireRate;
	FirePeriod = Weapon.FirePeriod / Assets.GetSkill(Skills[SKILL_ATTACKSPEED], SKILL_ATTACKSPEED);
	MinDamage = static_cast<int>(Weapon.MinDamage * DamageMultiplier);
	MaxDamage = static_cast<int>(Weapon.MaxDamage * DamageMultiplier);
	ReloadPeriod = Weapon.ReloadPeriod / Assets.GetSkill(Skills[SKILL_RELOADSPEED], SKILL_RELOADSPEED);
	WeaponSwitchPeriod = PLAYER_WEAPONSWITCHPERIOD * 1;
	BulletsShot = Weapon.BulletsShot;
	ZoomScale = Weapon.ZoomScale;

	// Cap fire period
	if(FirePeriod < WEAPON_MINFIREPERIOD)
		FirePeriod = WEAPON_MINFIREPERIOD;

	MovementSpeed = PLAYER_MOVEMENTSPEED * Assets.GetSkill(Skills[SKILL_MOVESPEED], SKILL_MOVESPEED);
	DamageResist = Assets.GetSkill(Skills[SKILL_DAMAGERESIST], SKILL_DAMAGERESIST) - 1.0f;
	MaxHealth = (int)(Assets.GetLevelHealth(Level) * Assets.GetSkill(Skills[SKILL_HEALTH], SKILL_HEALTH));
	MaxStamina = 1.0f * Assets.GetSkill(Skills[SKILL_MAXSTAMINA], SKILL_MAXSTAMINA);
	StaminaRegenModifier = 1.0f * Assets.GetSkill(Skills[SKILL_MAXSTAMINA], SKILL_MAXSTAMINA);

	// Armor
	if(GetArmor()) {
		DamageBlock = Assets.GetLevelDamageBlock(Level) + GetArmor()->GetDamageBlock();
	}
	else {
		DamageBlock = Assets.GetLevelDamageBlock(Level);
	}

	if(DebugLevel > 1) {
		std::cout << "Stats for " << Name << std::endl;
		std::cout << "AttackRange: " << Weapon.Range << " -> " << AttackRange << std::endl;
		std::cout << "MinAccuracyNormal: " << Weapon.MinAccuracy << " -> " << MinAccuracyNormal << std::endl;
		std::cout << "MaxAccuracyNormal: " << Weapon.MaxAccuracy << " -> " << MaxAccuracyNormal << std::endl;
		std::cout << "ZoomScale: " << Weapon.ZoomScale << " -> " << ZoomScale << std::endl;
		std::cout << "DamageMultiplier: " << DamageMultiplier << std::endl;
		std::cout << "Recoil: " << Weapon.Recoil << " -> " << Recoil << std::endl;
		std::cout << "RecoilRegen: " << Weapon.RecoilRegen << " -> " << RecoilRegen << std::endl;
		std::cout << "FireRate: " << Weapon.FireRate << " -> " << FireRate << std::endl;
		std::cout << "FirePeriod: " << Weapon.FirePeriod << " -> " << FirePeriod << std::endl;
		std::cout << "MinDamage: " << Weapon.MinDamage << " -> " << MinDamage << std::endl;
		std::cout << "MaxDamage: " << Weapon.MaxDamage << " -> " << MaxDamage << std::endl;
		std::cout << "BulletsShot: " << Weapon.BulletsShot << " -> " << BulletsShot << std::endl;
		std::cout << "ReloadPeriod: " << Weapon.ReloadPeriod << " -> " << ReloadPeriod << std::endl;
		std::cout << "WeaponSwitchPeriod: " << PLAYER_WEAPONSWITCHPERIOD << " -> " << WeaponSwitchPeriod << std::endl;
		std::cout << "MovementSpeed: " << PLAYER_MOVEMENTSPEED << " -> " << MovementSpeed << std::endl;
		std::cout << "DamageBlock: " << DamageBlock << std::endl;
		std::cout << "DamageResist: " << DamageResist << std::endl;
		std::cout << "MaxStamina: " << MaxStamina << std::endl;
	}

}

// Sets the weapon animation for the player
void _Player::ResetWeaponAnimation() {

	if(!IsDying()) {

		// Get walking animation
		switch(GetWeaponType()) {
			case WEAPON_MELEE:
			case WEAPON_PISTOL:
				WalkingAnimation = PLAYER_ANIMATIONWALKINGONEHAND;
			break;
			default:
				WalkingAnimation = PLAYER_ANIMATIONWALKINGTWOHAND;
			break;
		}

		Action = ACTION_IDLE;
		Animation->ChangeReel(WalkingAnimation);
		SetAnimationPlaybackSpeedFactor();
	}
}

// Applies the death penalty
void _Player::IncurDeathPenalty() {

	Reloading = SwitchingWeapons = false;
}

// Returns a sample index
const std::string &_Player::GetSample(int SampleType) const {

	if(SampleType <= SAMPLE_HIT && HasMainHand())
		return GetMainHand()->GetSample(SampleType);
	else
		return _Entity::GetSample(SampleType);
}

// Returns the weapon's particle template
const _ParticleTemplate *_Player::GetWeaponParticle(int Index) const {

	if(HasMainHand())
		return GetMainHand()->GetWeaponParticle(Index);

	return nullptr;
}

// Sets the color string and color of the player
void _Player::UpdateColor() {

	if(Assets.IsColorLoaded(ColorIdentifier))
		Color = Assets.GetColor(ColorIdentifier);
	else
		Color = COLOR_WHITE;
}

int _Player::GetInventoryMaxStack() const { return Assets.GetSkill(Skills[SKILL_MAXINVENTORY], SKILL_MAXINVENTORY) + 1; }
bool _Player::CanUseMedkit() const { return (MedkitTimer > PLAYER_MEDKITPERIOD) && CurrentHealth < MaxHealth; }
bool _Player::CanReload() const { return HasMainHand() && !Reloading && !SwitchingWeapons && !IsMeleeAttacking() && GetMainHand()->GetAmmo() != GetMainHand()->GetRoundSize() && HasClips(); }

bool _Player::IsMelee() const { return GetMainHand() == nullptr || GetMainHand()->GetWeaponType() == WEAPON_MELEE; }

void _Player::SetMainHand(_Weapon *Weapon) { Inventory[INVENTORY_MAINHAND] = Weapon; }
void _Player::SetOffHand(_Weapon *Weapon) { Inventory[INVENTORY_OFFHAND] = Weapon; }
void _Player::SetArmor(_Armor *Armor) { Inventory[INVENTORY_ARMOR] = Armor; }

void _Player::SetTorsoAnimation(const _Animation *Animation) { *this->Animation = *Animation; }
void _Player::SetLegAnimation(const _Animation *Animation) { *this->LegAnimation = *Animation; }
void _Player::SetLegAnimationPlayMode(int Mode) { LegAnimation->SetPlayMode(Mode); }
void _Player::SetAnimationPlaybackSpeedFactor() { Animation->SetPlaybackSpeedFactor(1.0f / MovementModifier); }
