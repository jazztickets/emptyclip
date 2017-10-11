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
#include <hud.h>
#include <graphics.h>
#include <font.h>
#include <config.h>
#include <assets.h>
#include <actions.h>
#include <ui/label.h>
#include <ui/image.h>
#include <ui/button.h>
#include <objects/entity.h>
#include <objects/player.h>
#include <objects/item.h>
#include <objects/misc.h>
#include <objects/armor.h>
#include <objects/weapon.h>
#include <objects/ammo.h>
#include <objects/upgrade.h>
#include <sstream>
#include <iomanip>
#include <SDL_mouse.h>

// Initialize
_HUD::_HUD(_Player *Player) {
	this->Player = Player;
	LastEntityHit = nullptr;
	DragStart = nullptr;
	CursorItem = CursorOverItem = nullptr;
	CursorSkill = -1;
	CrosshairScale = 0.0f;
	MessageTimer = 0.0;
	MessageBoxTimer = 0.0;
	InventoryOpen = false;

	// Get textures
	Fonts[FONT_TINY] = Assets.GetFont("hud_tiny");
	Fonts[FONT_SMALL] = Assets.GetFont("hud_small");
	Fonts[FONT_MEDIUM] = Assets.GetFont("hud_medium");
	Fonts[FONT_LARGE] = Assets.GetFont("hud_large");
	Fonts[FONT_LARGER] = Assets.GetFont("hud_larger");
	Fonts[FONT_LARGEST] = Assets.GetFont("hud_largest");
	CrosshairID = Assets.GetTexture("hud_crosshair");
	ReloadTexture = Assets.GetTexture("viewport_reload0");
	WeaponSwitchTexture = Assets.GetTexture("viewport_weaponswitch0");

	// Elements
	Labels[LABEL_FPS] = Assets.GetLabel("hud_fps");
	Labels[LABEL_MESSAGE] = Assets.GetLabel("hud_message");
	Labels[LABEL_MESSAGEBOX] = Assets.GetLabel("hud_messagebox_text");

	Elements[ELEMENT_PLAYERINFO] = Assets.GetElement("hud_player_info");
	Labels[LABEL_PLAYERNAME] = Assets.GetLabel("hud_player_name");
	Labels[LABEL_PLAYERLEVEL] = Assets.GetLabel("hud_player_level");
	Labels[LABEL_PLAYERHEALTH] = Assets.GetLabel("hud_player_health");

	Elements[ELEMENT_ENEMYINFO] = Assets.GetElement("hud_enemy_info");
	Labels[LABEL_ENEMYNAME] = Assets.GetLabel("hud_enemy_name");

	Elements[ELEMENT_PLAYERHEALTH] = Assets.GetElement("hud_player_health");
	Images[IMAGE_PLAYERHEALTH] = Assets.GetImage("player_health_full");
	Labels[LABEL_PLAYERHEALTH] = Assets.GetLabel("hud_player_health_text");

	Elements[ELEMENT_PLAYERSTAMINA] = Assets.GetElement("hud_player_stamina");
	Images[IMAGE_PLAYERSTAMINA] = Assets.GetImage("player_stamina_full");

	Images[IMAGE_ENEMYHEALTH] = Assets.GetImage("enemy_health_full");

	Elements[ELEMENT_INDICATOR] = Assets.GetElement("hud_indicator");
	Images[IMAGE_RELOAD] = Assets.GetImage("indicator_progress");
	Labels[LABEL_INDICATOR] = Assets.GetLabel("hud_indicator_text");

	Elements[ELEMENT_EXPERIENCE] = Assets.GetElement("hud_experience");
	Images[IMAGE_EXPERIENCE] = Assets.GetImage("experience_bar_full");
	Labels[LABEL_EXPERIENCE] = Assets.GetLabel("hud_experience_text");

	Elements[ELEMENT_MAINHAND] = Assets.GetElement("hud_mainhand");
	Images[IMAGE_MAINHAND_ICON] = Assets.GetImage("weapon0_icon");
	Labels[LABEL_MAINHAND_AMMO] = Assets.GetLabel("hud_mainhand_ammo");

	Elements[ELEMENT_OFFHAND] = Assets.GetElement("hud_offhand");
	Images[IMAGE_OFFHAND_ICON] = Assets.GetImage("weapon1_icon");
	Labels[LABEL_OFFHAND_AMMO] = Assets.GetLabel("hud_offhand_ammo");

	Elements[ELEMENT_INVENTORY] = Assets.GetElement("inventory");
	Elements[ELEMENT_SKILLS] = Assets.GetElement("skills");
	Labels[LABEL_SKILL_REMAINING] = Assets.GetLabel("hud_skill_remaining_value");
	Labels[LABEL_SKILL0] = Assets.GetLabel("hud_skill0_value");
	Labels[LABEL_SKILL1] = Assets.GetLabel("hud_skill1_value");
	Labels[LABEL_SKILL2] = Assets.GetLabel("hud_skill2_value");
	Labels[LABEL_SKILL3] = Assets.GetLabel("hud_skill3_value");
	Labels[LABEL_SKILL4] = Assets.GetLabel("hud_skill4_value");
	Labels[LABEL_SKILL5] = Assets.GetLabel("hud_skill5_value");
	Labels[LABEL_SKILL6] = Assets.GetLabel("hud_skill6_value");
	Labels[LABEL_SKILL7] = Assets.GetLabel("hud_skill7_value");
	Labels[LABEL_SKILL8] = Assets.GetLabel("hud_skill8_value");

	Labels[LABEL_DAMAGE] = Assets.GetLabel("hud_player_damage_value");
	Labels[LABEL_DAMAGEBLOCK] = Assets.GetLabel("hud_player_damageblock_value");
	Labels[LABEL_DAMAGERESIST] = Assets.GetLabel("hud_player_damageresist_value");
	Labels[LABEL_KILLS] = Assets.GetLabel("hud_player_kills_value");

	Elements[ELEMENT_SKILLINFO] = Assets.GetElement("skill_info");
	Labels[LABEL_SKILLTEXT] = Assets.GetLabel("hud_skill_text");
	Labels[LABEL_SKILL_LEVEL] = Assets.GetLabel("hud_skill_level");
	Labels[LABEL_SKILL_LEVEL_NEXT] = Assets.GetLabel("hud_skill_level_next");

	Elements[ELEMENT_MESSAGE] = Assets.GetElement("hud_messagebox");
}

// Shut down
_HUD::~_HUD() {
}

// Set inventory state
void _HUD::SetInventoryOpen(bool Value) {
	if(InventoryOpen == Value)
		return;

	InventoryOpen = Value;
	if(InventoryOpen) {

	}
	else {
		DragStart = nullptr;
		CursorItem = CursorOverItem = nullptr;
	}

	Graphics.ShowCursor(InventoryOpen);
}

// Handle mouse events
void _HUD::MouseEvent(const _MouseEvent &MouseEvent) {
	if(!GetInventoryOpen())
		return;

	_Element *HitElement;

	HitElement = Elements[ELEMENT_INVENTORY]->GetHitElement();
	if(MouseEvent.Button == SDL_BUTTON_LEFT) {

		// Start dragging an item
		if(MouseEvent.Pressed) {
			if(HitElement && HitElement->GetID() >= 0 && Player->CanDropItem()) {
				DragStart = HitElement;
				CursorItem = Player->GetInventory(DragStart->GetID());
				ClickOffset = MouseEvent.Position - HitElement->GetBounds().GetMidPoint();
			}
		}
		else {

			// Was dragging an item
			if(CursorItem) {

				// Dropped outside the inventory
				if(!HitElement) {
					Player->DropItem(DragStart->GetID());
				}
				else if(HitElement->GetID() >= 0) {
					Player->SwapInventory(DragStart->GetID(), HitElement->GetID());
				}
			}

			// Swap inventory
			CursorItem = nullptr;
			DragStart = nullptr;
		}

		//if(HitElement)
		//	printf("%d %s\n", HitElement->GetID(), HitElement->GetIdentifier().c_str());
	}
	else if(MouseEvent.Button == SDL_BUTTON_RIGHT) {
		if(MouseEvent.Pressed) {
			if(HitElement && HitElement->GetID() >= 0) {
				Player->UseMedkit(HitElement->GetID());
				CursorOverItem = nullptr;
			}
		}
	}

	HitElement = Elements[ELEMENT_SKILLS]->GetHitElement();
	if(MouseEvent.Pressed && MouseEvent.Button == SDL_BUTTON_LEFT) {
		if(HitElement && HitElement->GetID() >= 0) {
			Player->UpdateSkill(HitElement->GetID(), 1);
		}
	}
}

// Update phase
void _HUD::Update(double FrameTime, float Radius) {
	LastEntityHitTimer += FrameTime;
	CursorOverItem = nullptr;
	CursorSkill = -1;

	// Update crosshair
	CrosshairScale += (Radius - CrosshairScale) / HUD_CROSSHAIRDIVISOR;
	if(CrosshairScale < HUD_MINCROSSHAIRSCALE)
		CrosshairScale = HUD_MINCROSSHAIRSCALE;

	// Update inventory
	if(GetInventoryOpen()) {
		Graphics.ShowCursor(true);
		Elements[ELEMENT_INVENTORY]->Update(FrameTime, Input.GetMouse());
		Elements[ELEMENT_SKILLS]->Update(FrameTime, Input.GetMouse());

		_Element *HitElement;
		HitElement = Elements[ELEMENT_INVENTORY]->GetHitElement();
		if(HitElement && HitElement->GetID() >= 0)
			CursorOverItem = Player->GetInventory(HitElement->GetID());

		HitElement = Elements[ELEMENT_SKILLS]->GetHitElement();
		if(HitElement && HitElement->GetID() >= 0)
			UpdateSkillInfo(HitElement->GetID(), Input.GetMouse().X, Input.GetMouse().Y);
	}
	else
		Graphics.ShowCursor(false);

	// Update health display
	if(LastEntityHit != nullptr && (LastEntityHitTimer > HUD_ENTITYHEALTHDISPLAYPERIOD || !LastEntityHit->GetActive())) {
		LastEntityHit = nullptr;
	}

	if(MessageTimer > 0.0) {
		MessageTimer -= FrameTime;
	}

	if(MessageBoxTimer > 0.0) {
		MessageBoxTimer -= FrameTime;
	}
}

// Draw phase
void _HUD::Render() {

	// FPS
	std::ostringstream Buffer;
	Buffer << Graphics.GetFramesPerSecond() << " FPS";
	Labels[LABEL_FPS]->SetText(Buffer.str());
	Labels[LABEL_FPS]->Render();
	Buffer.str("");

	// Message
	if(MessageTimer > 0.0) {
		if(MessageTimer < 1.0)
			Labels[LABEL_MESSAGE]->SetFade(MessageTimer);

		Labels[LABEL_MESSAGE]->Render();
	}

	// Message Box
	if(MessageBoxTimer > 0.0) {
		if(MessageBoxTimer < 1.0)
			Elements[ELEMENT_MESSAGE]->SetFade(MessageBoxTimer);

		Elements[ELEMENT_MESSAGE]->Render();
	}

	// Draw enemy health
	if(LastEntityHit != nullptr) {
		Labels[LABEL_ENEMYNAME]->SetText(LastEntityHit->GetName());
		Images[IMAGE_ENEMYHEALTH]->SetWidth(Elements[ELEMENT_ENEMYINFO]->GetSize().X * LastEntityHit->GetHealthPercentage());
		Elements[ELEMENT_ENEMYINFO]->Render();
	}

	// Draw stamina
	Images[IMAGE_PLAYERSTAMINA]->SetWidth(Elements[ELEMENT_PLAYERSTAMINA]->GetSize().X * Player->GetStaminaPercentage());
	if(Player->GetTired())
		Images[IMAGE_PLAYERSTAMINA]->SetColor(_Color(1.0f, 0.5f, 0.0f));
	else
		Images[IMAGE_PLAYERSTAMINA]->SetColor(_Color(1.0f, 1.0f, 1.0f));

	if(Player->GetStaminaPercentage() < 1.0f)
		Elements[ELEMENT_PLAYERSTAMINA]->Render();

	// Draw player health
	Buffer << Player->GetHealth() << "/" << Player->GetMaxHealth();
	Labels[LABEL_PLAYERHEALTH]->SetText(Buffer.str());
	Buffer.str("");

	Images[IMAGE_PLAYERHEALTH]->SetWidth(Elements[ELEMENT_PLAYERHEALTH]->GetSize().X * Player->GetHealthPercentage());
	Elements[ELEMENT_PLAYERHEALTH]->Render();

	// Draw experience bar
	Buffer << Player->GetExperience() << " / " << Player->GetExperienceNextLevel() << " XP";
	Labels[LABEL_EXPERIENCE]->SetText(Buffer.str());
	Buffer.str("");
	Images[IMAGE_EXPERIENCE]->SetWidth(Elements[ELEMENT_EXPERIENCE]->GetSize().X * Player->GetLevelPercentage());
	Elements[ELEMENT_EXPERIENCE]->Render();

	// Draw player name and level
	Labels[LABEL_PLAYERNAME]->SetText(Player->GetName());
	Buffer << "Level " << Player->GetLevel();
	Labels[LABEL_PLAYERLEVEL]->SetText(Buffer.str());
	Buffer.str("");
	Elements[ELEMENT_PLAYERINFO]->Render();

	// Reload indicator
	if(Player->IsReloading()) {
		DrawIndicator("Reload", Player->GetReloadPercent(), ReloadTexture);
	}
	else if(!Player->HasAmmo() && !Player->IsSwitchingWeapons() && Player->GetMainHand() && Player->GetMainHand()->GetRoundSize() > 0)
		DrawIndicator("Reload");

	// Weapon switch indicator
	if(Player->IsSwitchingWeapons()) {
		DrawIndicator("Switching Weapons", Player->GetWeaponSwitchPercent(), WeaponSwitchTexture);
	}

	// Draw weapons
	DrawHUDWeapon(Player->GetMainHand(), Elements[ELEMENT_MAINHAND], Images[IMAGE_MAINHAND_ICON], Labels[LABEL_MAINHAND_AMMO]);
	DrawHUDWeapon(Player->GetOffHand(), Elements[ELEMENT_OFFHAND], Images[IMAGE_OFFHAND_ICON], Labels[LABEL_OFFHAND_AMMO]);

	// Draw character screen
	RenderCharacterScreen();

	if(CursorOverItem && CursorItem != CursorOverItem) {
		RenderItemInfo(CursorOverItem, Input.GetMouse().X, Input.GetMouse().Y);
		if(CursorOverItem->GetType() == _Object::WEAPON && CursorOverItem != Player->GetMainHand())
			RenderItemInfo(Player->GetMainHand(), -100, Graphics.GetScreenHeight()/2);
		else if(CursorOverItem->GetType() == _Object::ARMOR && CursorOverItem != Player->GetArmor())
			RenderItemInfo(Player->GetArmor(), -100, Graphics.GetScreenHeight()/2);
	}
}

// Draws the crosshair
void _HUD::RenderCrosshair(const Vector2 &Position) {
	if(InventoryOpen)
		return;

	Graphics.DisableDepthTest();

	Graphics.EnableVBO(VBO_CIRCLE);
	Graphics.DrawCircle(Position[0], Position[1], 0, CrosshairScale, COLOR_WHITE);
	Graphics.DisableVBO(VBO_CIRCLE);

	Graphics.EnableVBO(VBO_QUAD);
	Graphics.DrawTexture(Position[0], Position[1], 0, CrosshairID, COLOR_WHITE, 0, 1.0f, 1.0f);
	Graphics.DisableVBO(VBO_QUAD);

	Graphics.EnableDepthTest();
}

// Draws a box and text
void _HUD::DrawIndicator(const std::string &String, float Percent, _Texture *Texture) {

	// Set text
	Labels[LABEL_INDICATOR]->SetText(String);
	Graphics.DrawRectangle(Elements[ELEMENT_INDICATOR]->GetBounds(), COLOR_TGRAY);

	// Set progress size
	Images[IMAGE_RELOAD]->SetTexture(Texture);
	Images[IMAGE_RELOAD]->SetWidth(Elements[ELEMENT_INDICATOR]->GetSize().X * Percent);
	Elements[ELEMENT_INDICATOR]->Render();
}

// Draw the weapons on the HUD
void _HUD::DrawHUDWeapon(const _Weapon *Weapon, _Element *Element, _Image *Image, _Label *Label) {
	if(!Weapon)
		return;

	Image->SetTexture(Weapon->GetTexture());
	Image->SetColor(Weapon->GetColor());
	if(Weapon->GetRoundSize()) {
		std::ostringstream Buffer;
		Buffer << Weapon->GetAmmo() << "/" << Weapon->GetRoundSize();
		Label->SetText(Buffer.str());
	}
	else
		Label->SetText("");

	Element->Render();
}

// Sets the last entity hit object
void _HUD::SetLastEntityHit(_Entity *Entity) {

	LastEntityHit = Entity;
	LastEntityHitTimer = 0;
}

// Draw the inventory and character screen
void _HUD::RenderCharacterScreen() {
	if(!InventoryOpen)
		return;

	// Draw the inventory background
	Elements[ELEMENT_INVENTORY]->Render();

	// Set skill labels
	std::ostringstream Buffer;
	Buffer << Player->GetSkillPointsRemaining();
	Labels[LABEL_SKILL_REMAINING]->SetText(Buffer.str());
	Buffer.str("");

	for(int i = 0; i < SKILL_MAXUSED; i++) {
		Buffer << Player->GetSkill(i);
		Labels[LABEL_SKILL0 + i]->SetText(Buffer.str());
		Buffer.str("");
	}

	Buffer << Player->GetMinDamage() << " - " << Player->GetMaxDamage();
	Labels[LABEL_DAMAGE]->SetText(Buffer.str());
	Buffer.str("");

	Buffer << Player->GetDamageBlock();
	Labels[LABEL_DAMAGEBLOCK]->SetText(Buffer.str());
	Buffer.str("");

	Buffer << int(100 * Player->GetDamageResist() + 0.5f) << "%";
	Labels[LABEL_DAMAGERESIST]->SetText(Buffer.str());
	Buffer.str("");

	Buffer << Player->GetMonsterKills();
	Labels[LABEL_KILLS]->SetText(Buffer.str());
	Buffer.str("");

	Elements[ELEMENT_SKILLS]->Render();

	// Draw inventory
	for(int i = INVENTORY_ARMOR; i < INVENTORY_BAGEND; i++) {
		if(Player->HasInventory(i)) {
			if(Player->GetInventory(i) != CursorItem) {
				_Button *Button = (_Button *)Elements[ELEMENT_INVENTORY]->GetChildren()[i];
				if(Button) {
					Graphics.DrawImage(Button->GetBounds().GetMidPoint(), Player->GetInventory(i)->GetTexture(), Player->GetInventory(i)->GetColor());
					if(i >= INVENTORY_BAGSTART && Player->GetInventory(i)->CanStack()) {
						DrawItemCount(Player->GetInventory(i), Button->GetBounds().End.X - 2, Button->GetBounds().End.Y - 2);
					}
				}
			}
		}
	}

	// Draw cursor item
	if(CursorItem) {
		_Point Position(Input.GetMouse() - ClickOffset);
		Graphics.DrawImage(Position, CursorItem->GetTexture(), CursorItem->GetColor());
		if(CursorItem->CanStack())
			DrawItemCount(CursorItem, Position.X + 22, Position.Y + 22);
	}

	// Draw cursor skill
	if(CursorSkill != -1)
		Elements[ELEMENT_SKILLINFO]->Render();
}

// Draw the item count text
void _HUD::DrawItemCount(_Item *Item, int X, int Y) {
	std::ostringstream Buffer;
	Buffer << Item->GetCount();
	Fonts[FONT_TINY]->DrawText(Buffer.str(), X, Y, COLOR_WHITE, RIGHT_BASELINE);
	Buffer.str("");
}

// Draw the item popup window
void _HUD::RenderItemInfo(_Item *Item, int DrawX, int DrawY) {
	if(!Item)
		return;

	int PadX = 8;
	int Width;
	int Height;

	// TODO cleanup
	if(Item->GetType() == _Object::WEAPON) {
		Width = 245;
		Height = 375;
	}
	else if(Item->GetType() == _Object::ARMOR) {
		Width = 220;
		Height = 170;
	}
	else {
		Width = 150;
		Height = 100;
	}

	// Get title width
	_TextBounds TextBounds;
	Fonts[FONT_LARGE]->GetStringDimensions(Item->GetName(), TextBounds);
	Width = std::max(Width, TextBounds.Width) + 20;

	int MinPadding = 5;
	int MinX = 5;
	int WindowOffsetX = 20;

	// Get current equipment
	_Weapon *MainHand = Player->GetMainHand();
	_Armor *EquippedArmor = Player->GetArmor();
	if((Item->GetType() == _Object::WEAPON && MainHand && Item != MainHand) || (Item->GetType() == _Object::ARMOR && EquippedArmor && Item != EquippedArmor))
		MinX += Width;

	DrawX += WindowOffsetX;
	DrawY -= Height/2;
	if(DrawX < MinX)
		DrawX = MinX;
	if(DrawY < MinPadding)
		DrawY = MinPadding;
	if(DrawX > Graphics.GetScreenWidth() - MinPadding - Width)
		DrawX = Graphics.GetScreenWidth() - MinPadding - Width;
	if(DrawY > Graphics.GetScreenHeight() - MinPadding - Height)
		DrawY = Graphics.GetScreenHeight() - MinPadding - Height;
	Graphics.DrawRectangle(DrawX, DrawY, DrawX + Width, DrawY + Height, _Color(0, 0, 0, 0.8f), true);

	DrawY += 25;
	DrawX += Width/2;
	Fonts[FONT_LARGE]->DrawText(Item->GetName(), DrawX, DrawY, COLOR_WHITE, CENTER_BASELINE);

	DrawY += 16;
	Fonts[FONT_SMALL]->DrawText(Item->GetTypeAsString(), DrawX, DrawY, COLOR_WHITE, CENTER_BASELINE);

	DrawY += 10;
	switch(Item->GetType()) {
		case _Object::WEAPON: {
			std::ostringstream Buffer;
			_Weapon *Weapon = (_Weapon *)Item;
			_Color TextColor;

			// Damage
			TextColor = COLOR_WHITE;
			if(MainHand) {
				if(Weapon->GetAverageDamage() > MainHand->GetAverageDamage())
					TextColor = COLOR_GREEN;
				else if(Weapon->GetAverageDamage() < MainHand->GetAverageDamage())
					TextColor = COLOR_RED;
			}
			DrawY += 20;
			Buffer << Weapon->GetMinDamage() << " - " << Weapon->GetMaxDamage();
			Fonts[FONT_MEDIUM]->DrawText("Damage", DrawX - PadX, DrawY, COLOR_WHITE, RIGHT_BASELINE);
			Fonts[FONT_MEDIUM]->DrawText(Buffer.str(), DrawX + PadX, DrawY, TextColor, LEFT_BASELINE);
			Buffer.str("");

			// Clip size
			if(Weapon->GetRoundSize()) {
				TextColor = COLOR_WHITE;
				if(MainHand) {
					if(Weapon->GetRoundSize() > MainHand->GetRoundSize())
						TextColor = COLOR_GREEN;
					else if(Weapon->GetRoundSize() < MainHand->GetRoundSize())
						TextColor = COLOR_RED;
				}
				DrawY += 20;
				Buffer << Weapon->GetAmmo() << "/" << Weapon->GetRoundSize();
				Fonts[FONT_MEDIUM]->DrawText("Rounds", DrawX - PadX, DrawY, COLOR_WHITE, RIGHT_BASELINE);
				Fonts[FONT_MEDIUM]->DrawText(Buffer.str(), DrawX + PadX, DrawY, TextColor, LEFT_BASELINE);
				Buffer.str("");
			}

			// Attacks
			if(Weapon->GetBulletsShot() > 1) {
				TextColor = COLOR_WHITE;
				if(MainHand) {
					if(Weapon->GetBulletsShot() > MainHand->GetBulletsShot())
						TextColor = COLOR_GREEN;
					else if(Weapon->GetBulletsShot() < MainHand->GetBulletsShot())
						TextColor = COLOR_RED;
				}

				DrawY += 20;
				Buffer << Weapon->GetBulletsShot();
				std::string AttackCountText;
				if(Weapon->GetWeaponType() == WEAPON_MELEE)
					AttackCountText = "Attacks/Swing";
				else
					AttackCountText = "Bullets/Shot";
				Fonts[FONT_MEDIUM]->DrawText(AttackCountText, DrawX - PadX, DrawY, COLOR_WHITE, RIGHT_BASELINE);
				Fonts[FONT_MEDIUM]->DrawText(Buffer.str(), DrawX + PadX, DrawY, TextColor, LEFT_BASELINE);
				Buffer.str("");
			}

			// Fire rate
			if(Weapon->GetFirePeriod()) {
				TextColor = COLOR_WHITE;
				if(MainHand) {
					if(Weapon->GetFirePeriod() < MainHand->GetFirePeriod())
						TextColor = COLOR_GREEN;
					else if(Weapon->GetFirePeriod() > MainHand->GetFirePeriod())
						TextColor = COLOR_RED;
				}

				DrawY += 20;
				Buffer << std::setprecision(3) << 1 / Weapon->GetFirePeriod() << "/s";
				std::string AttackCountText;
				if(Weapon->GetWeaponType() == WEAPON_MELEE)
					AttackCountText = "Attack Rate";
				else
					AttackCountText = "Fire Rate";
				Fonts[FONT_MEDIUM]->DrawText(AttackCountText, DrawX - PadX, DrawY, COLOR_WHITE, RIGHT_BASELINE);
				Fonts[FONT_MEDIUM]->DrawText(Buffer.str(), DrawX + PadX, DrawY, TextColor, LEFT_BASELINE);
				Buffer.str("");
				Buffer << std::setprecision(6);
			}

			// Weapon Spread
			TextColor = COLOR_WHITE;
			if(MainHand && MainHand->IsMelee() == Weapon->IsMelee()) {
				if(Weapon->GetAverageAccuracy() < MainHand->GetAverageAccuracy()) {

					// Less is worse for melee
					if(Weapon->GetWeaponType() == WEAPON_MELEE)
						TextColor = COLOR_RED;
					else
						TextColor = COLOR_GREEN;
				}
				else if(Weapon->GetAverageAccuracy() > MainHand->GetAverageAccuracy()) {

					// Bigger is better for melee
					if(Weapon->GetWeaponType() == WEAPON_MELEE)
						TextColor = COLOR_GREEN;
					else
						TextColor = COLOR_RED;
				}
			}
			DrawY += 20;
			if(Weapon->GetWeaponType() == WEAPON_MELEE) {
				Buffer << Weapon->GetMaxAccuracy() << " degrees";
				Fonts[FONT_MEDIUM]->DrawText("Swing Arc", DrawX - PadX, DrawY, COLOR_WHITE, RIGHT_BASELINE);
			}
			else {
				Buffer << (int)(Weapon->GetMinAccuracy() + 0.5f) << " - " << (int)(Weapon->GetMaxAccuracy() + 0.5f);
				Fonts[FONT_MEDIUM]->DrawText("Spread", DrawX - PadX, DrawY, COLOR_WHITE, RIGHT_BASELINE);
			}
			Fonts[FONT_MEDIUM]->DrawText(Buffer.str(), DrawX + PadX, DrawY, TextColor, LEFT_BASELINE);
			Buffer.str("");

			// Reload speed
			if(Weapon->GetReloadPeriod() > 1) {
				TextColor = COLOR_WHITE;
				if(MainHand) {
					if(Weapon->GetReloadPeriod() < MainHand->GetReloadPeriod())
						TextColor = COLOR_GREEN;
					else if(Weapon->GetReloadPeriod() > MainHand->GetReloadPeriod())
						TextColor = COLOR_RED;
				}

				DrawY += 20;
				Buffer << Weapon->GetReloadPeriod() << "s";
				std::string AttackCountText;
				Fonts[FONT_MEDIUM]->DrawText("Reload Time", DrawX - PadX, DrawY, COLOR_WHITE, RIGHT_BASELINE);
				Fonts[FONT_MEDIUM]->DrawText(Buffer.str(), DrawX + PadX, DrawY, TextColor, LEFT_BASELINE);
				Buffer.str("");
			}

			// Ammo type
			if(Weapon->GetAmmoType()) {
				DrawY += 20;
				Buffer << _Ammo::ToString(Weapon->GetAmmoType());
				Fonts[FONT_MEDIUM]->DrawText("Ammo Type", DrawX  - PadX, DrawY, COLOR_WHITE, RIGHT_BASELINE);
				Fonts[FONT_MEDIUM]->DrawText(Buffer.str(), DrawX + PadX, DrawY);
				Buffer.str("");
			}

			// Components
			if(Weapon->GetMaxComponents() >= 1) {
				TextColor = COLOR_WHITE;
				if(MainHand) {
					if(Weapon->GetMaxComponents() > MainHand->GetMaxComponents())
						TextColor = COLOR_GREEN;
					else if(Weapon->GetMaxComponents() < MainHand->GetMaxComponents())
						TextColor = COLOR_RED;
				}

				DrawY += 20;
				Buffer << Weapon->GetComponents() << "/" << Weapon->GetMaxComponents();
				Fonts[FONT_MEDIUM]->DrawText("Components", DrawX - PadX, DrawY, COLOR_WHITE, RIGHT_BASELINE);
				Fonts[FONT_MEDIUM]->DrawText(Buffer.str(), DrawX + PadX, DrawY, TextColor, LEFT_BASELINE);
				Buffer.str("");
			}

			// Bonuses
			TextColor = COLOR_WHITE;
			bool First = true;
			for(int i = 0; i < UPGRADE_TYPES; i++) {
				if(Weapon->GetBonus(i)) {
					if(First)
						DrawY += 10;
					DrawY += 20;
					Buffer << "+" << Weapon->GetBonus(i) * 100.0f << "% " << _Upgrade::ToString(i, Weapon->GetWeaponType());
					Fonts[FONT_MEDIUM]->DrawText(Buffer.str(), DrawX, DrawY, TextColor, CENTER_BASELINE);
					Buffer.str("");

					First = false;
				}
			}
		} break;
		case _Object::ARMOR: {
			_Armor *Armor = (_Armor *)Item;
			std::ostringstream Buffer;
			_Color TextColor;

			DrawX += 40;

			// Strength required
			TextColor = COLOR_WHITE;
			if(EquippedArmor) {
				if(Armor->GetStrengthRequirement() < EquippedArmor->GetStrengthRequirement())
					TextColor = COLOR_GREEN;
				else if(Armor->GetStrengthRequirement() > EquippedArmor->GetStrengthRequirement())
					TextColor = COLOR_RED;
			}
			DrawY += 20;
			Buffer << Armor->GetStrengthRequirement();
			Fonts[FONT_MEDIUM]->DrawText("Strength Required", DrawX - PadX, DrawY, COLOR_WHITE, RIGHT_BASELINE);
			Fonts[FONT_MEDIUM]->DrawText(Buffer.str(), DrawX + PadX, DrawY, TextColor, LEFT_BASELINE);
			Buffer.str("");

			// DamageBlock
			TextColor = COLOR_WHITE;
			if(EquippedArmor) {
				if(Armor->GetDamageBlock() > EquippedArmor->GetDamageBlock())
					TextColor = COLOR_GREEN;
				else if(Armor->GetDamageBlock() < EquippedArmor->GetDamageBlock())
					TextColor = COLOR_RED;
			}

			DrawY += 20;
			Buffer << Armor->GetDamageBlock();
			Fonts[FONT_MEDIUM]->DrawText("Damage Block", DrawX - PadX, DrawY, COLOR_WHITE, RIGHT_BASELINE);
			Fonts[FONT_MEDIUM]->DrawText(Buffer.str(), DrawX + PadX, DrawY, TextColor, LEFT_BASELINE);
			Buffer.str("");
		} break;
		case _Object::MISCITEM: {
			_MiscItem *MiscItem = (_MiscItem *)Item;
			if(MiscItem->GetMiscItemType() == MISCITEM_MEDKIT) {
				std::ostringstream Buffer;

				// Heal amount
				DrawY += 20;
				Buffer << "+" << Player->GetMedkitHealAmount(MiscItem->GetLevel()) << " HP";
				Fonts[FONT_MEDIUM]->DrawText(Buffer.str(), DrawX, DrawY, COLOR_GREEN, CENTER_BASELINE);
			}
		} break;
		case _Object::UPGRADE: {
			_Upgrade *Upgrade = (_Upgrade *)Item;
			std::ostringstream Buffer;

			// Bonus
			DrawY += 20;
			if(Upgrade->GetUpgradeType() == UPGRADE_ATTACKS)
				Buffer << "+" << (int)(Upgrade->GetBonus()) << " Attack Count";
			else
				Buffer << "+" << (int)(Upgrade->GetBonus() * 100.0f + 0.5f) << "% " << _Upgrade::ToString(Upgrade->GetUpgradeType(), -1);
			Fonts[FONT_MEDIUM]->DrawText(Buffer.str(), DrawX, DrawY, COLOR_WHITE, CENTER_BASELINE);
		} break;
	}
}

// Draw the skill popup window
void _HUD::UpdateSkillInfo(int Skill, int DrawX, int DrawY) {
	CursorSkill = Skill;

	DrawX -= Elements[ELEMENT_SKILLINFO]->GetSize().X + 15;
	DrawY -= Elements[ELEMENT_SKILLINFO]->GetSize().Y + 15;
	if(DrawX < 10)
		DrawX = 10;
	if(DrawY < 10)
		DrawY = 10;

	// Move window
	Elements[ELEMENT_SKILLINFO]->SetOffset(_Point(DrawX, DrawY));

	// Get skill description
	std::ostringstream Buffer, BufferNext;
	Buffer << std::setprecision(3);
	BufferNext << std::setprecision(3);
	switch(Skill) {
		case SKILL_STRENGTH:
			Labels[LABEL_SKILLTEXT]->SetText("Allows you to equip heavier armor");
			Buffer << "+" << Assets.GetSkill(Player->GetSkill(Skill), Skill) << " Strength";
			BufferNext << "+" << Assets.GetSkill(Assets.GetValidSkill(Player->GetSkill(Skill)+1), Skill) << " Strength";
		break;
		case SKILL_HEALTH:
			Labels[LABEL_SKILLTEXT]->SetText("Increases health");
			Buffer << "+" << Assets.GetSkillPercentImprovement(Player->GetSkill(Skill), Skill) << "% Health";
			BufferNext << "+" << Assets.GetSkillPercentImprovement(Assets.GetValidSkill(Player->GetSkill(Skill)+1), Skill) << "% Health";
		break;
		case SKILL_ACCURACY:
			Labels[LABEL_SKILLTEXT]->SetText("Increases gun accuracy");
			Buffer << "+" << Assets.GetSkillPercentImprovement(Player->GetSkill(Skill), Skill) << "% Accuracy";
			BufferNext << "+" << Assets.GetSkillPercentImprovement(Assets.GetValidSkill(Player->GetSkill(Skill)+1), Skill) << "% Accuracy";
		break;
		case SKILL_RELOADSPEED:
			Labels[LABEL_SKILLTEXT]->SetText("Increases reload speed");
			Buffer << "+" << Assets.GetSkillPercentImprovement(Player->GetSkill(Skill), Skill) << "% Reload Speed";
			BufferNext << "+" << Assets.GetSkillPercentImprovement(Assets.GetValidSkill(Player->GetSkill(Skill)+1), Skill) << "% Reload Speed";
		break;
		case SKILL_ATTACKSPEED:
			Labels[LABEL_SKILLTEXT]->SetText("Increases attack speed");
			Buffer << "+" << Assets.GetSkillPercentImprovement(Player->GetSkill(Skill), Skill) << "% Attack Speed";
			BufferNext << "+" << Assets.GetSkillPercentImprovement(Assets.GetValidSkill(Player->GetSkill(Skill)+1), Skill) << "% Attack Speed";
		break;
		case SKILL_MOVESPEED:
			Labels[LABEL_SKILLTEXT]->SetText("Increases move speed");
			Buffer << "+" << Assets.GetSkillPercentImprovement(Player->GetSkill(Skill), Skill) << "% Move Speed";
			BufferNext << "+" << Assets.GetSkillPercentImprovement(Assets.GetValidSkill(Player->GetSkill(Skill)+1), Skill) << "% Move Speed";
		break;
		case SKILL_DAMAGERESIST:
			Labels[LABEL_SKILLTEXT]->SetText("Damage Resist");
			Buffer << "+" << Assets.GetSkillPercentImprovement(Player->GetSkill(Skill), Skill) << "% Damage Resist";
			BufferNext << "+" << Assets.GetSkillPercentImprovement(Assets.GetValidSkill(Player->GetSkill(Skill)+1), Skill) << "% Damage Resist";
		break;
		case SKILL_MAXINVENTORY:
			Labels[LABEL_SKILLTEXT]->SetText("Increases max inventory stack size");
			Buffer << "+" << Assets.GetSkill(Player->GetSkill(Skill), Skill) << " Stacks";
			BufferNext << "+" << Assets.GetSkill(Assets.GetValidSkill(Player->GetSkill(Skill)+1), Skill) << " Stacks";
		break;
		case SKILL_MAXSTAMINA:
			Labels[LABEL_SKILLTEXT]->SetText("Increases max stamina");
			Buffer << "+" << Assets.GetSkillPercentImprovement(Player->GetSkill(Skill), Skill) << "% Max Stamina";
			BufferNext << "+" << Assets.GetSkillPercentImprovement(Assets.GetValidSkill(Player->GetSkill(Skill)+1), Skill) << "% Max Stamina";
		break;
	}

	// Wrap text
	Labels[LABEL_SKILLTEXT]->SetWrap(Elements[ELEMENT_SKILLINFO]->GetSize().X - 20);

	Labels[LABEL_SKILL_LEVEL]->SetText(Buffer.str());
	if(Player->GetSkill(Skill)+1 > GAME_SKILLLEVELS)
		BufferNext.str("");
	Labels[LABEL_SKILL_LEVEL_NEXT]->SetText(BufferNext.str());
}

// Draw death message
void _HUD::RenderDeathScreen() {
	Fonts[FONT_LARGEST]->DrawText("You Died!", Graphics.GetScreenWidth() / 2, Graphics.GetScreenHeight() / 2 - 200, COLOR_WHITE, CENTER_MIDDLE);
	Fonts[FONT_LARGE]->DrawText(std::string("Press [") + Actions.GetInputNameForAction(_Actions::USE) + "] to continue", Graphics.GetScreenWidth() / 2, Graphics.GetScreenHeight() / 2 - 150, COLOR_WHITE, CENTER_MIDDLE);
}

// Show hud message
void _HUD::ShowTextMessage(const std::string &Message, double Time) {
	Labels[LABEL_MESSAGE]->SetText(Message);
	Labels[LABEL_MESSAGE]->SetFade(1.0f);
	MessageTimer = Time;
}

// Show message box
void _HUD::ShowMessageBox(const std::string &Message, double Time) {
	if(MessageBoxTimer > 0.0 && Labels[LABEL_MESSAGEBOX]->GetText() == Message)
		return;

	Labels[LABEL_MESSAGEBOX]->SetText(Message);
	Labels[LABEL_MESSAGEBOX]->SetWrap(Elements[ELEMENT_MESSAGE]->GetSize().X - 25);

	Elements[ELEMENT_MESSAGE]->SetFade(1.0f);
	MessageBoxTimer = Time;
}