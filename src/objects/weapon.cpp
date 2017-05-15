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
#include <objects/weapon.h>
#include <objects/upgrade.h>
#include <objects/particle.h>
#include <random.h>
#include <buffer.h>

// Constructor
_Weapon::_Weapon(const std::string &Identifier, int Count, const Vector2 &Position, const _WeaponTemplate *Weapon, _Texture *Texture, bool Generate)
:	Ammo(Weapon->RoundSize),
	RoundSize(Weapon->RoundSize),
	MinDamage(Weapon->MinDamage),
	MaxDamage(Weapon->MaxDamage),
	MinAccuracy(Weapon->MinAccuracy),
	MaxAccuracy(Weapon->MaxAccuracy),
	FirePeriod(Weapon->FirePeriod),
	ReloadPeriod(Weapon->ReloadPeriod),
	BulletsShot(Weapon->BulletsShot) {

	Stats = *Weapon;
	if(Generate)
		this->MaxComponents = Random.GenerateRange(Weapon->MinComponents, Weapon->MaxComponents);
	else
		this->MaxComponents = Weapon->MinComponents;

	this->Type = _Object::WEAPON;
	this->Identifier = Identifier;
	this->Texture = Texture;
	this->Color = Weapon->Color;
	this->Position = Position;

	RecalculateStats();
}

// Destructor
_Weapon::~_Weapon() {

	for(size_t i = 0; i < Upgrades.size(); i++)
		delete Upgrades[i];
}

// Serialize weapon for saving
void _Weapon::Serialize(_Buffer &Buffer) {

	// Write weapons
	Buffer.WriteString(Identifier.c_str());

	// Ammo
	Buffer.Write(Ammo);

	// Max upgrades
	Buffer.Write(MaxComponents);

	// Upgrades
	Buffer.Write<int>(Upgrades.size());
	for(size_t i = 0; i < Upgrades.size(); i++) {
		Upgrades[i]->Serialize(Buffer);
	}
}

// Reduces ammo by 1
void _Weapon::ReduceAmmo() {

	Ammo--;
	if(Ammo < 0)
		Ammo = 0;
}

// Adds ammo to the gun
void _Weapon::SetAmmo(int Value) {

	Ammo = Value;
	if(Ammo > RoundSize)
		Ammo = RoundSize;
}

// Recalculates the weapon stats
void _Weapon::RecalculateStats() {

	for(int i = 0; i < UPGRADE_TYPES; i++)
		Bonus[i] = 0.0f;

	// Sum bonuses
	for(size_t i = 0; i < Upgrades.size(); i++)
		Bonus[Upgrades[i]->GetUpgradeType()] += Upgrades[i]->GetBonus();

	// Set stats
	RoundSize = static_cast<int>(Stats.RoundSize * (1.0f + Bonus[UPGRADE_CLIP]));
	MinDamage = static_cast<int>(Stats.MinDamage * (1.0f + Bonus[UPGRADE_DAMAGE]));
	MaxDamage = static_cast<int>(Stats.MaxDamage * (1.0f + Bonus[UPGRADE_DAMAGE]));
	MinAccuracy = Stats.MinAccuracy / (1.0f + Bonus[UPGRADE_ACCURACY]);
	MaxAccuracy = Stats.MaxAccuracy / (1.0f + Bonus[UPGRADE_ACCURACY]);
	FirePeriod = Stats.FirePeriod / (1.0f + Bonus[UPGRADE_FIREPERIOD]);
	ReloadPeriod = Stats.ReloadPeriod / (1.0f + Bonus[UPGRADE_RELOADPERIOD]);
	BulletsShot = Stats.BulletsShot + static_cast<int>(Bonus[UPGRADE_ATTACKS]);

	SetAmmo(Ammo);
}

// Adds a component to the weapon
bool _Weapon::AddComponent(_Upgrade *Upgrade) {

	if(GetComponents() < MaxComponents &&
		!(Upgrade->GetWeaponType() != -1 && Upgrade->GetWeaponType() != Stats.Type) &&
		!(Upgrade->GetUpgradeType() == UPGRADE_CLIP && Stats.RoundSize == 0)) {

		Upgrades.push_back(Upgrade);
		RecalculateStats();
		return true;
	}

	return false;
}

// Converts the weapon's type into a string
std::string _Weapon::ToString(int Type) {

	switch(Type) {
		case WEAPON_MELEE:
			return "Melee";
		break;
		case WEAPON_PISTOL:
			return "Pistol";
		break;
		case WEAPON_SHOTGUN:
			return "Shotgun";
		break;
		case WEAPON_RIFLE:
			return "Rifle";
		break;
		case WEAPON_HEAVY:
			return "Heavy";
		break;
		default:
		break;
	}

	return "";
}

_Upgrade *_Weapon::GetUpgrade(int Index) const { return Upgrades[Index]; }
_ParticleTemplate *_Weapon::GetWeaponParticle(int Index) { return Stats.WeaponParticles->ParticleTemplates[Index]; }
