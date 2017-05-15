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
#include <objects/item.h>
#include <objects/templates.h>
#include <vector>
#include <string>

// Forward Declarations
class _Upgrade;
struct _ParticleTemplate;

// Classes
class _Weapon : public _Item {

	public:

		_Weapon(const std::string &Identifier, int Count, const Vector2 &Position, const _WeaponTemplate *Weapon, _Texture *Texture, bool Generate);
		~_Weapon();

		void Serialize(_Buffer &Buffer) override;

		void RecalculateStats();
		bool AddComponent(_Upgrade *Upgrade);

		void SetAmmo(int Value);
		void SetMaxComponents(int Value) { MaxComponents = Value; }
		void SetSample(int Type, int Sample) { Stats.Samples[Type] = Sample; };
		void ReduceAmmo();

		const std::string &GetName() const override { return Stats.Name; }
		int GetFireRate() const { return Stats.FireRate; }
		float GetRecoil() const { return Stats.Recoil; }
		float GetRecoilRegen() const { return Stats.RecoilRegen; }
		float GetRange() const { return Stats.Range; }
		float GetZoomScale() const { return Stats.ZoomScale; }
		int GetWeaponType() const { return Stats.Type; }
		int GetAmmoType() const { return Stats.AmmoType; }
		int GetBulletsShot() const { return BulletsShot;}
		double GetFirePeriod() const { return FirePeriod; }
		double GetReloadPeriod() const { return ReloadPeriod; }
		int GetMinDamage() const { return MinDamage; }
		int GetMaxDamage() const { return MaxDamage; }
		float GetAverageDamage() const { return (MinDamage + MaxDamage) / 2.0f; }
		float GetMinAccuracy() const { return MinAccuracy; }
		float GetMaxAccuracy() const { return MaxAccuracy; }
		float GetAverageAccuracy() const { return (MinAccuracy + MaxAccuracy) / 2.0f; }
		int GetRoundSize() const { return RoundSize; }
		int GetAmmo() const { return Ammo; }
		int GetMaxComponents() const { return MaxComponents; }
		int GetComponents() const { return static_cast<int>(Upgrades.size()); }
		const std::string &GetSample(int Type) const { return Stats.Samples[Type]; };
		float GetBonus(int Index) const { return Bonus[Index]; }
		_Upgrade *GetUpgrade(int Index) const;
		_ParticleTemplate *GetWeaponParticle(int Index);

		bool IsMelee() const { return GetWeaponType() == WEAPON_MELEE; }
		virtual std::string GetTypeAsString() const override { return ToString(Stats.Type) + " class weapon"; }
		static std::string ToString(int Type);

	protected:

		std::vector<_Upgrade *> Upgrades;
		_WeaponTemplate Stats;
		int Ammo, RoundSize;
		int MinDamage, MaxDamage;
		float MinAccuracy, MaxAccuracy, Bonus[UPGRADE_TYPES];
		double FirePeriod, ReloadPeriod;
		int MaxComponents, BulletsShot;
};
