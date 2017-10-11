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

#include <color.h>
#include <vector2.h>
#include <stdint.h>
#include <string>

class _Texture;

// Types of ammo
enum AmmoType {
	AMMO_NONE,
	AMMO_9MM,
	AMMO_357,
	AMMO_556MM,
	AMMO_762MM,
	AMMO_SHELLS,
	AMMO_ROCKETS,
	AMMO_POWERCELLS,
	AMMO_PLASMACELLS,
	AMMO_PSI,
	AMMO_TYPES
};

// Types of weapons
enum WeaponType {
	WEAPON_MELEE,
	WEAPON_PISTOL,
	WEAPON_SHOTGUN,
	WEAPON_RIFLE,
	WEAPON_HEAVY,
	WEAPON_TYPES
};

// Determines if you can hold down the mouse to fire or not
enum FireRateType {
	FIRERATE_SEMI,
	FIRERATE_AUTO
};

// Types of weapon samples
enum WeaponSampleTypes {
	WEAPON_FIRESAMPLE,
	WEAPON_MISSSAMPLE,
	WEAPON_RICOCHETSAMPLE,
	WEAPON_EMPTYSAMPLE,
	WEAPON_RELOADSAMPLE
};

// Types of weapon particles
enum WeaponParticleTypes {
	WEAPONPARTICLE_FIRE,
	WEAPONPARTICLE_SMOKE,
	WEAPONPARTICLE_RICOCHET,
	WEAPONPARTICLE_BULLETHOLE,
	WEAPONPARTICLE_TYPES
};

// Types of samples
enum SampleTypes {
	SAMPLE_FIRE,
	SAMPLE_TRIGGERDOWN,
	SAMPLE_RICOCHET,
	SAMPLE_EMPTY,
	SAMPLE_RELOAD,
	SAMPLE_HIT,
	SAMPLE_TAKEDAMAGE,
	SAMPLE_DEATH,
	SAMPLE_MOVE,
	SAMPLE_TYPES,
};

// Upgrade component types
enum UpgradeType {
	UPGRADE_CLIP,
	UPGRADE_DAMAGE,
	UPGRADE_ACCURACY,
	UPGRADE_FIREPERIOD,
	UPGRADE_RELOADPERIOD,
	UPGRADE_ATTACKS,
	UPGRADE_TYPES
};

// Skill types
enum SkillTypes {
	SKILL_STRENGTH,
	SKILL_HEALTH,
	SKILL_ACCURACY,
	SKILL_RELOADSPEED,
	SKILL_ATTACKSPEED,
	SKILL_MOVESPEED,
	SKILL_DAMAGERESIST,
	SKILL_MAXINVENTORY,
	SKILL_MAXSTAMINA,
	SKILL_UNUSED2,
	SKILL_MAXUSED = SKILL_UNUSED2,
	SKILL_UNUSED3,
	SKILL_UNUSED4,
	SKILL_UNUSED5,
	SKILL_UNUSED6,
	SKILL_UNUSED7,
	SKILL_UNUSED8,
	SKILL_UNUSED9,
	SKILL_UNUSED10,
	SKILL_COUNT,
};

struct _AmmoTemplate {
	std::string Name, IconIdentifier;
	_Color Color;
	int AmmoType;
};

struct _UpgradeTemplate {
	std::string Name, IconIdentifier;
	_Color Color;
	float Bonus;
	int WeaponType, UpgradeType;
};

struct _MiscItemTemplate {
	std::string Name, IconIdentifier;
	_Color Color;
	int Type, Level;
};

struct _ParticleTemplate {
	Vector2 StartDirection, VelocityScale, TurnSpeed, Size;
	_Color Color;
	const _Texture *Texture;
	int Count;
	float AccelerationScale;
	float AlphaSpeed;
	double Lifetime;
	float ScaleAspect;
	int Type;
};

struct _WeaponParticleTemplate {
	_WeaponParticleTemplate() {
		for(int i = 0; i < WEAPONPARTICLE_TYPES; i++)
			ParticleTemplates[i] = nullptr;
	}

	_ParticleTemplate *ParticleTemplates[WEAPONPARTICLE_TYPES];
};

// Holds information about a weapon
struct _WeaponTemplate {
	_WeaponTemplate()
		:	WeaponParticles(nullptr),
			Name("Fists"),
			MinAccuracy(0.0f),
			MaxAccuracy(90.0f),
			Recoil(0.0f),
			RecoilRegen(0.0f),
			Range(0.5f),
			ZoomScale(15.0f),
			FirePeriod(0.4),
			ReloadPeriod(0.0),
			MinComponents(0),
			MaxComponents(0),
			MinDamage(1),
			MaxDamage(3),
			BulletsShot(1),
			RoundSize(0),
			Type(WEAPON_MELEE),
			AmmoType(AMMO_NONE),
			FireRate(FIRERATE_SEMI) {

		for(int i = 0; i < SAMPLE_TYPES; i++)
			Samples[i] = "";
	}

	_WeaponParticleTemplate *WeaponParticles;
	_Color Color;
	std::string Name, IconIdentifier;
	std::string Samples[SAMPLE_TYPES];

	float MinAccuracy, MaxAccuracy, Recoil, RecoilRegen, Range, ZoomScale;
	double FirePeriod, ReloadPeriod;
	int MinComponents, MaxComponents, MinDamage, MaxDamage, BulletsShot, RoundSize, Type, AmmoType, FireRate;
};

// Holds information about a monster
struct _MonsterTemplate {
	_Color Color;
	_WeaponParticleTemplate *WeaponParticles;
	std::string Name, AnimationIdentifier, SamplesIdentifier, ItemGroupIdentifier;
	float Radius, Scale, MovementSpeed, Accuracy, ViewRange, AttackRange;
	int Level, Health, DamageBlock, CurrentSpeed, MinDamage, MaxDamage, BehaviorType, WeaponType;
	int64_t ExperienceGiven;
	double FirePeriod;
	std::string FireSample, MissSample, RicochetSample, EmptySample, ReloadSample, HitSample, DeathSample;
};

// Holds information about object spawns
struct _ObjectSpawn {
	_ObjectSpawn()
		:	Identifier(""),
			Position(ZERO_VECTOR),
			Type(-1) { }
	_ObjectSpawn(const std::string &Identifier, const Vector2 &Position, int Type)
		:	Identifier(Identifier),
			Position(Position),
			Type(Type) { }

	std::string Identifier;
	Vector2 Position;
	int Type;
};

