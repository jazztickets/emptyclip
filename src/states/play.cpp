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
#include <states/play.h>
#include <graphics.h>
#include <framework.h>
#include <menu.h>
#include <camera.h>
#include <constants.h>
#include <assets.h>
#include <hud.h>
#include <map.h>
#include <events.h>
#include <audio.h>
#include <config.h>
#include <actions.h>
#include <utils.h>
#include <particles.h>
#include <objects/entity.h>
#include <objects/player.h>
#include <objects/monster.h>
#include <objects/particle.h>
#include <objects/misc.h>
#include <objects/ammo.h>
#include <objects/upgrade.h>
#include <objects/weapon.h>
#include <objects/armor.h>
#include <states/editor.h>
#include <states/null.h>
#include <stdexcept>
#include <fstream>
#include <iostream>

_PlayState PlayState;

// Constructor
_PlayState::_PlayState() {
	Player = nullptr;
	Level = "";
	TestMode = false;
	FromEditor = false;
}

// Load level and set up objects
void _PlayState::Init() {
	CursorItem = nullptr;
	PreviousCursorItem = nullptr;
	LastLightEvent = nullptr;
	SaveGameTimer = 0;

	// Check for player
	if(TestMode) {
		Player = Save.GetPlayer(_Save::SLOT_TEST);
	}

	// Bad player
	if(!Player)
		throw std::runtime_error("Player is nullptr");

	// Set checkpoint from editor
	if(FromEditor)
		Player->SetCheckpointIndex(CheckpointIndex);

	// Check for level override
	if(Level == "")
		Level = Player->GetMapIdentifier();

	// Load level
	Map = new _Map(Level);
	Map->Init();
	Player->SetMap(Map);
	Player->SetMapIdentifier(Map->GetFilename());

	// Set starting states
	Player->SetPosition(Map->GetStartingPositionByCheckpoint(Player->GetCheckpointIndex()));
	Player->SetTileChanged(true);
	Map->AddObjectToGrid(Player, GRID_PLAYER);

	// Get monster and item list
	std::vector<_ObjectSpawn *> Objects = Map->GetObjectsList();

	// Spawn objects
	for(size_t i = 0; i < Objects.size(); i++) {
		SpawnObject(Objects[i]);
	}

	// Initialize hud
	HUD = new _HUD(Player);

	// Set up graphics
	Camera = new _Camera(Player->GetPosition(), CAMERA_DISTANCE, CAMERA_DIVISOR);
	Map->SetCamera(Camera);

	Particles = new _Particles();
	Particles->SetCamera(Camera);

	Graphics.ChangeViewport(Graphics.GetScreenWidth(), Graphics.GetScreenHeight());
	Camera->CalculateFrustum(Graphics.GetAspectRatio());
	Graphics.ShowCursor(false);

	Actions.ResetState();
}

// Close map
void _PlayState::Close() {

	DeleteMonsters();
	DeleteActiveEvents();

	Player->StopAudio();

	delete Particles;
	delete Camera;
	delete Map;
	delete HUD;
}

// Action handler
bool _PlayState::HandleAction(int InputType, int Action, int Value) {
	if(!Player || IsPaused())
		return false;

	if(Value) {
		if(!Player->IsDying()) {
			switch(Action) {
				case _Actions::INVENTORY:
					HUD->SetInventoryOpen(!HUD->GetInventoryOpen());
					Player->SetCrouching(false);
					Player->SetSprinting(false);
				break;
				case _Actions::FIRE:
					if(!HUD->GetInventoryOpen()) {
						if(Player->CanAttack() && !Player->HasAmmo())
							Audio.Play(new _AudioSource(Audio.GetBuffer(Player->GetSample(SAMPLE_EMPTY))), Player->GetPosition());

						if(Player->GetFireRate() == FIRERATE_SEMI)
							Player->SetAttackRequested(true);
					}
				break;
				case _Actions::RELOAD:
					if(!HUD->IsDragging()) {
						if(Player->IsReloading())
							Player->CancelReloading();
						else
							Player->StartReloading();
					}
				break;
				case _Actions::WEAPONSWITCH:
					if(!HUD->IsDragging())
						Player->StartWeaponSwitch(INVENTORY_MAINHAND, INVENTORY_OFFHAND);
				break;
				case _Actions::MEDKIT:
					Player->SetMedkitRequested(true);
				break;
			}
		}
		else {
			if(Action == _Actions::USE)
				RestartFromDeath();
		}
	}

	return false;
}

// Key handler
void _PlayState::KeyEvent(const _KeyEvent &KeyEvent) {
	if(IsPaused()) {
		Player->StopAudio();
		Menu.KeyEvent(KeyEvent);
		return;
	}

	if(KeyEvent.Pressed) {
		switch(KeyEvent.Key) {
			case SDL_SCANCODE_ESCAPE:
				if(Player->IsDead()) {
					RestartFromDeath();
				}
				else if(!Player->IsDying()) {
					if(TestMode) {
						if(FromEditor)
							Framework.ChangeState(&EditorState);
						else
							Framework.SetDone(true);

						Player->Save();
					}
					else
						Menu.InitInGame();
				}
			break;
			case SDL_SCANCODE_F1:
				Menu.InitInGame();
			break;
			case SDL_SCANCODE_GRAVE:
				//WorldCursor.Print();
				//IsFiring = !IsFiring;
				//Audio.Play(new _AudioSource(Audio.GetBuffer("player_hit0")), WorldCursor);
				//HUD->ShowTextMessage("CHECKPOINT REACHED", 5.0f);
				//_ParticleSpawn
				//Particles->Create(_ParticleSpawn(Assets.GetParticleTemplate("tracer0"), WorldCursor, OBJECT_Z, Player->GetDirection()));
			break;
		}
	}
}

// Mouse handler
void _PlayState::MouseEvent(const _MouseEvent &MouseEvent) {
	HUD->MouseEvent(MouseEvent);

	if(IsPaused())
		Menu.MouseEvent(MouseEvent);
}

// Update
void _PlayState::Update(double FrameTime) {
	if(IsPaused()) {
		Menu.Update(FrameTime);
		Graphics.ShowCursor(true);
		if(HUD)
			HUD->SetCursorOverItem(nullptr);

		return;
	}

	SaveGameTimer += FrameTime;

	// Update world cursor
	Camera->ConvertScreenToWorld(Input.GetMouse(), WorldCursor);

	// Handle input
	if(!Player->IsDying()) {

		// Turn character to face the world cursor
		Player->FacePosition(WorldCursor);

		// Move types
		if(Actions.GetState(_Actions::UP) && Actions.GetState(_Actions::LEFT))
			Player->SetMoveState(MOVE_FORWARDLEFT);
		else if(Actions.GetState(_Actions::UP) && Actions.GetState(_Actions::RIGHT))
			Player->SetMoveState(MOVE_FORWARDRIGHT);
		else if(Actions.GetState(_Actions::DOWN) && Actions.GetState(_Actions::LEFT))
			Player->SetMoveState(MOVE_BACKWARDLEFT);
		else if(Actions.GetState(_Actions::DOWN) && Actions.GetState(_Actions::RIGHT))
			Player->SetMoveState(MOVE_BACKWARDRIGHT);
		else if(Actions.GetState(_Actions::LEFT))
			Player->SetMoveState(MOVE_LEFT);
		else if(Actions.GetState(_Actions::RIGHT))
			Player->SetMoveState(MOVE_RIGHT);
		else if(Actions.GetState(_Actions::UP))
			Player->SetMoveState(MOVE_FORWARD);
		else if(Actions.GetState(_Actions::DOWN))
			Player->SetMoveState(MOVE_BACKWARD);
		else
			Player->SetMoveState(MOVE_NONE);

		// Attack or aim
		if(!HUD->GetInventoryOpen()) {
			if(Player->GetFireRate() == FIRERATE_AUTO && Actions.GetState(_Actions::FIRE)) {

				// Attack
				Player->SetAttackRequested(true);
			}

			// Aim
			Player->SetCrouching(Actions.GetState(_Actions::AIM));
			Player->SetSprinting(Actions.GetState(_Actions::SPRINT));
		}

		Player->SetUseRequested(Actions.GetState(_Actions::USE));
	}
	else {
		HUD->SetInventoryOpen(false);
	}

	// Update the player's states
	Player->Update(FrameTime);

	// Check for events
	if(Player->GetTileChanged()) {
		CheckEvents(Player);
	}
	Player->SetTileChanged(false);

	// Pickup up an object
	if(Player->GetUseRequested()) {
		UseObject();

		Player->SetUseRequested(false);
	}

	// Update objects
	Map->Update(FrameTime);
	UpdateMonsters(FrameTime);
	Particles->Update(FrameTime);
	Map->AddRenderList(Player, 1);

	// Update events
	UpdateEvents(FrameTime);

	// Apply the damage
	IsFiring = false;
	if(Player->GetAttackMade()) {
		IsFiring = true;

		// Execute the attack
		EntityAttack(Player, GRID_MONSTER);
	}

	// Update camera
	Camera->SetPosition(Player->GetPosition());

	// Get zoom state
	if(Player->IsCrouching()) {
		if(Map->IsVisible(Player->GetPosition(), WorldCursor)) {
			Camera->UpdatePosition((WorldCursor - Player->GetPosition()) / Player->GetZoomScale());
		}
		else {
			Vector2 Direction = WorldCursor - Player->GetPosition();

			Vector2 NewPosition;
			Map->CheckBulletCollisions(Player->GetPosition(), Direction, nullptr, &NewPosition, 0, false);
			Camera->UpdatePosition((NewPosition - Player->GetPosition()) / Player->GetZoomScale());
		}
		Camera->SetDistance(CAMERA_DISTANCE_AIMED);
	}
	else
		Camera->SetDistance(CAMERA_DISTANCE);

	Camera->Update(FrameTime);

	// Get item at cursor
	PreviousCursorItem = CursorItem;
	CursorItem = static_cast<_Item *>(Map->CheckCollisionsInGrid(WorldCursor, 0.05f, GRID_ITEM, nullptr));
	if(CursorItem && CursorItem == PreviousCursorItem)
		CursorItemTimer += FrameTime;
	else
		CursorItemTimer = 0;

	// Update the HUD
	HUD->Update(FrameTime, Player->GetCrosshairRadius(WorldCursor));

	// Set cursor item
	if(CursorItem && !HUD->GetCursorOverItem() && (HUD->GetInventoryOpen() || CursorItemTimer > HUD_CURSOR_ITEM_WAIT))
		HUD->SetCursorOverItem(CursorItem);

	Audio.SetPosition(Player->GetPosition());
}

// Render the state
void _PlayState::Render(double BlendFactor) {
	if(IsPaused())
		BlendFactor = 0;

	// Setup the viewing matrix
	Graphics.Setup3DViewport();
	Camera->Set3DProjection(BlendFactor);
	Graphics.EnableDepthTest();

	// Draw the floor
	Map->RenderFloors();

	Graphics.SetDepthMask(false);

	// Enable VBOs
	Graphics.EnableVBO(VBO_QUAD);
	Graphics.DisableDepthTest();

	// Draw floor decals
	Particles->Render(_Particles::FLOOR_DECALS);

	// Draw objects
	Map->RenderObjects(BlendFactor);

	// Disable VBOs
	Graphics.EnableDepthTest();
	Graphics.DisableVBO(VBO_QUAD);
	Graphics.SetDepthMask(true);

	// Draw the walls
	Map->RenderWalls();

	Graphics.SetDepthMask(false);

	Graphics.EnableVBO(VBO_QUAD);

	// Draw wall decals
	Particles->Render(_Particles::WALL_DECALS);

	// Draw particles
	Graphics.EnableParticleBlending();
	Particles->Render(_Particles::NORMAL);
	Graphics.DisableParticleBlending();
	Graphics.DisableVBO(VBO_QUAD);

	Graphics.SetDepthMask(true);

	// Draw the foreground tiles
	Map->RenderForeground();
	Map->RenderLights(Player->GetPosition());

	// Draw the crosshair
	if(!Player->IsDying()) {
		HUD->RenderCrosshair(WorldCursor);
	}

	/*
	// Temp
	if(0) {
		Graphics.DisableDepthTest();
		// Draw melee stuff (temp)
		Graphics.DrawCircle(Player->GetPositionX(), Player->GetPositionY(), 0.2f, Player->GetWeaponRange(), COLOR_WHITE);
		Vector2 t1, t2;
		t1 = Player->GetPosition() + Vector2(Player->GetDirection() - Player->GetMaxAccuracy() / 2 ) * Player->GetWeaponRange();
		t2 = Player->GetPosition() + Vector2(Player->GetDirection() + Player->GetMaxAccuracy() / 2 ) * Player->GetWeaponRange();
		glBegin(GL_LINES);
		glVertex2f(Player->GetPositionX(), Player->GetPositionY());
		glVertex2f(t1[0], t1[1]);
		glEnd();
		glBegin(GL_LINES);
		glVertex2f(Player->GetPositionX(), Player->GetPositionY());
		glVertex2f(t2[0], t2[1]);
		glEnd();
		Graphics.EnableDepthTest();
	}
	*/

	// Setup OpenGL for drawing the HUD
	Graphics.Setup2DProjectionMatrix();
	/*
	_Coord Start(Camera->GetAABB()[0], Camera->GetAABB()[1]);
	_Coord End(Camera->GetAABB()[2], Camera->GetAABB()[3]);

	for(int X = Start.X; X < End.X; X++) {
		for(int Y = Start.Y; Y < End.Y; Y++) {
			if(X > 0 && Y > 0) {
				_Point P;
				Camera->ConvertWorldToScreen(Vector2(X-0.5f, Y-0.5f), P);
				std::ostringstream Buffer;
				size_t Count = 0;
				std::list<_Event *> &Events = Map->GetEventList(_Coord(X, Y));
				for(auto Event : Events) {
					if(Event->GetActive())
						Count++;

				}
				Buffer << Count << "/" << Events.size();
				Assets.GetFont("hud_tiny")->DrawText(Buffer.str(), P.X, P.Y);
				Buffer.str("");
			}
		}
	}*/

	HUD->Render();

	if(IsPaused() || (Player && Player->IsDead()))
		Graphics.DrawRectangle(0, 0, Graphics.GetScreenWidth(), Graphics.GetScreenHeight(), _Color(0, 0, 0, GAME_PAUSE_FADEAMOUNT), true);

	// Draw in-game menu
	if(IsPaused()) {
		Menu.Render();
	}
	else if(Player && Player->IsDead()) {
		Graphics.ShowCursor(1);
		HUD->RenderDeathScreen();
	}

	Graphics.SetDepthMask(true);
}

// Restart the level after death
void _PlayState::RestartFromDeath() {
	try {
		Player->Load();
	}
	catch(std::exception &Error) {
	}

	Framework.ChangeState(&PlayState);
}

// Fires a gun or swings a weapon
void _PlayState::EntityAttack(_Entity *Attacker, int GridType) {

	// Check for ammo
	if(!Attacker->HasAmmo())
		return;

	// Reduce ammo
	Attacker->ReduceAmmo();

	// Weapon type specific code
	int WeaponType = Attacker->GetWeaponType();
	HitStruct HitInformation;

	// Play fire sound and generate fire/smoke particles
	if(WeaponType != WEAPON_MELEE) {
		GenerateBulletEffects(Attacker, -1, HitInformation.Position);
		Audio.Play(new _AudioSource(Audio.GetBuffer(Attacker->GetSample(SAMPLE_FIRE))), Attacker->GetPosition());
	}

	Attacker->StartTriggerDownAudio();

	// For each bullet that the weapon fires
	bool PlayedHitWallSound = false;
	for(int i = 0; i < Attacker->GetBulletsShot(); i++) {
		HitInformation.Type = HIT_NONE;

		// Check weapon type
		if(WeaponType == WEAPON_MELEE) {

			HitInformation.Object = Map->CheckMeleeCollisions(Attacker, Vector2(Attacker->GetDirection()), GridType);
			if(HitInformation.Object != nullptr) {
				HitInformation.Type = HIT_OBJECT;
				HitInformation.Position = HitInformation.Object->GetPosition();
			}
		}
		else {

			// Get bullet direction
			float ShotDirection = Attacker->GenerateShotDirection();

			// Check distance to the wall
			Map->CheckBulletCollisions(Attacker->GetPosition(), Vector2(ShotDirection), &HitInformation.Object, &HitInformation.Position, GridType, true);
			if(HitInformation.Object != nullptr)
				HitInformation.Type = HIT_OBJECT;
			else
				HitInformation.Type = HIT_WALL;

			_ParticleTemplate *Template = Assets.GetParticleTemplate("tracer0");
			Vector2 ParticleStart = Attacker->GetPosition() + (Vector2(0, -Template->Size[1] * 0.5f) + Attacker->GetWeaponOffset(Attacker->GetWeaponType())).RotateVector(ShotDirection);

			float Distance = (HitInformation.Position - Attacker->GetPosition()).Magnitude() - Template->Size[1];

			_Particle *Tracer = new _Particle(_ParticleSpawn(Template, ParticleStart, OBJECT_Z, ShotDirection));
			Tracer->SetLifetime(Distance * Template->VelocityScale.Y * GAME_FPS);
			Particles->Add(Tracer);
		}

		// Generate particle effects and reduce health
		switch(HitInformation.Type) {
			case HIT_NONE:
			break;
			case HIT_WALL:
				if(!PlayedHitWallSound) {
					Audio.Play(new _AudioSource(Audio.GetBuffer(Attacker->GetSample(SAMPLE_RICOCHET))), HitInformation.Position);
					PlayedHitWallSound = true;
				}

				GenerateBulletEffects(Attacker, HIT_WALL, HitInformation.Position);
			break;
			case HIT_OBJECT:
				GenerateBulletEffects(Attacker, HIT_OBJECT, HitInformation.Position);

				// Deal damage
				int Damage = Attacker->GenerateDamage(HitInformation.Object->GetDamageBlock(), HitInformation.Object->GetDamageResist());

				HitInformation.Object->UpdateHealth(-Damage);
				if(HitInformation.Object->IsDying()) {
					Attacker->UpdateExperience(HitInformation.Object->GetExperienceGiven());
					CreateItemDrop(HitInformation.Object);

					// Dying sound
					Audio.Play(new _AudioSource(Audio.GetBuffer(HitInformation.Object->GetSample(SAMPLE_DEATH))), HitInformation.Position);

					if(Attacker->GetType() == _Object::PLAYER)
						Attacker->UpdateKillCount(1);
				}

				// Weapon hit sound
				Audio.Play(new _AudioSource(Audio.GetBuffer(Attacker->GetSample(SAMPLE_HIT))), HitInformation.Position);

				// Entity hit sound
				Audio.Play(new _AudioSource(Audio.GetBuffer(HitInformation.Object->GetSample(SAMPLE_TAKEDAMAGE))), HitInformation.Position);

				// Set HUD last hit object
				if(HitInformation.Object->GetType() == _Object::MONSTER)
					HUD->SetLastEntityHit(HitInformation.Object);
			break;
		}
	}

	Attacker->SetAttackMade(false);
}

// Places an item into the player's inventory
void _PlayState::PickupObject() {
	_Item *HitItem;

	// Loop through the objects
	_TileBounds TB;
	Map->GetTileBounds(Vector2(0.8, 0.8), 0.5, TB);
	HitItem = (_Item *)Map->CheckCollisionsInGrid(Player->GetPosition(), Player->GetRadius(), GRID_ITEM, nullptr);

	if(HitItem != nullptr) {
		int AddResult = Player->AddItem(HitItem);
		if(AddResult) {
			Map->RemoveItem(HitItem);
			if(AddResult == 2) {
				delete HitItem;
				CursorItemTimer = 0;
			}
			Player->ResetUseTimer();
		}
		else {
			HUD->ShowTextMessage(HUD_INVENTORYFULLMESSAGE, HUD_INVENTORYFULLTIME);
		}
	}
}

// Processes the use key to open doors and hit switches
void _PlayState::UseObject() {
	if(Player->CanUse()) {

		// Pick up an item if available
		if(Player->CanPickup())
			PickupObject();

		// Open a door if possible
		_Coord Position;
		Map->GetAdjacentTile(Player->GetPosition(), Player->GetDirection(), Position);

		// Check for events
		std::list<_Event *> &Events = Map->GetEventList(Position);
		for(auto Event : Events) {

			// Check for doors or switches
			if(Event->GetActive() && (Event->GetType() == EVENT_DOOR || Event->GetType() == EVENT_WSWITCH) && Map->CanChangeMapState(Event)) {

				// Check for key in inventory and use it
				if(Event->GetItemIdentifier() != "") {
					int ItemIndex = Player->FindItem(Event->GetItemIdentifier());
					if(ItemIndex == -1) {
						if(Assets.IsMiscItemLoaded(Event->GetItemIdentifier()))
							HUD->ShowMessageBox("You need a " + Assets.GetMiscItemTemplate(Event->GetItemIdentifier())->Name, HUD_KEYMESSAGETIME);

						return;
					}

					if(Player->UseItem(ItemIndex, true)) {
						HUD->ShowTextMessage("KEY USED", 2.0f);
					}
				}

				// Change map
				Map->ChangeMapState(Event);

				// Decrement level
				if(Event->GetLevel() > 0) {
					Event->Decrement();
					if(Event->GetLevel() == 0)
						Event->SetActive(false);
				}

				Player->ResetUseTimer();
			}
		}
	}
}

// Creates a random item from an entity
void _PlayState::CreateItemDrop(const _Entity *Entity) {

	if(Entity->GetItemGroupIdentifier() != "") {
		_ItemGroup *ItemGroup = Assets.GetItemGroup(Entity->GetItemGroupIdentifier());

		for(int i = 0; i < ItemGroup->Quantity; i++) {

			// Spawn random item
			_ObjectSpawn ObjectSpawn;
			ObjectSpawn.Position = GenerateRandomPointInCircle(PLAYER_RADIUS) + Entity->GetPosition();

			// Roll for drop
			Assets.GetRandomDrop(ItemGroup, &ObjectSpawn);
			SpawnObject(&ObjectSpawn, true);
		}
	}
}

// Updates the monsters
void _PlayState::UpdateMonsters(double FrameTime) {

	// Loop through monsters
	for(auto MonsterIterator = Monsters.begin(); MonsterIterator != Monsters.end();) {
		_Monster *Monster = (_Monster *)*MonsterIterator;

		if(!Monster->GetActive()) {
			Map->RemoveObjectFromGrid(Monster, GRID_MONSTER);
			delete Monster;
			MonsterIterator = Monsters.erase(MonsterIterator);
		}
		else {
			Monster->Update(FrameTime, Player);

			if(Monster->GetAttackMade()) {
				EntityAttack(Monster, GRID_PLAYER);
			}

			if(Camera->IsCircleInView(Monster->GetPosition(), Monster->GetScale())) {
				Map->AddRenderList(Monster, 2);
			}

			++MonsterIterator;
		}
	}
}

// Checks for the player triggering events
void _PlayState::CheckEvents(const _Entity *Entity) {
	_Coord Position = Map->GetValidCoord(Entity->GetPosition());

	// Check for events triggered by walking
	std::list<_Event *> &Events = Map->GetEventList(Position);
	for(auto Event : Events) {

		// Perform action
		if(Event->GetActive()) {
			switch(Event->GetType()) {
				case EVENT_SPAWN:
					if(Assets.IsMonsterLoaded(Event->GetMonsterIdentifier())) {
						Event->StartTimer();
						ActiveEvents.push_back(Event);
					}
					Event->SetActive(false);
				break;
				case EVENT_CHECK:
					switch(Map->GetMapType()) {
						case MAPTYPE_SINGLE:
							if(Event->GetLevel() > Player->GetCheckpointIndex()) {
								Player->SetCheckpointIndex(Event->GetLevel());
								HUD->ShowTextMessage(HUD_CHECKPOINTMESSAGE, HUD_CHECKPOINTTIME);
								Player->Save();
								SaveGameTimer = 0;
							}
							Event->SetActive(false);
						break;
						case MAPTYPE_TUTORIAL:
							if(Event->GetLevel() > Player->GetCheckpointIndex()) {
								Player->SetCheckpointIndex(Event->GetLevel());
								HUD->ShowTextMessage(HUD_CHECKPOINTMESSAGE, HUD_CHECKPOINTTIME);
								Player->Save();
							}
							Event->SetActive(false);
						break;
						case MAPTYPE_ADVENTURE:
							if(Event->GetLevel() != Player->GetCheckpointIndex()) {
								Player->SetCheckpointIndex(Event->GetLevel());
								HUD->ShowTextMessage(HUD_CHECKPOINTMESSAGE, HUD_CHECKPOINTTIME);
								Player->Save();
							}
						break;
						default:
						break;
					}
				break;
				case EVENT_END:
					Level = Event->GetItemIdentifier();

					// End of the game
					if(Level == "") {
						Level = GAME_FIRSTLEVEL;
						Framework.ChangeState(&NullState);
					}
					// Next level
					else
						Framework.ChangeState(&PlayState);

					Player->SetCheckpointIndex(Event->GetLevel());
					Player->SetMapIdentifier(Level);
					Player->Save();
				break;
				case EVENT_TEXT:
					if(Assets.IsStringLoaded(Event->GetItemIdentifier()))
						HUD->ShowMessageBox(Assets.GetString(Event->GetItemIdentifier()), Event->GetActivationPeriod());

					// Message with level of 0 = infinite
					if(Event->GetLevel() != 0)
						Event->SetActive(false);
				break;
				case EVENT_SOUND:
					if(Audio.GetBuffer(Event->GetItemIdentifier())) {
						Event->StartTimer();
						ActiveEvents.push_back(Event);
					}
					Event->SetActive(false);
				break;
				case EVENT_FSWITCH:
				case EVENT_ENABLE:
					Event->StartTimer();
					ActiveEvents.push_back(Event);
					Event->SetActive(false);
				break;
				case EVENT_TELE: {
					if(Event->GetLevel() > 0) {
						Event->Decrement();
						if(Event->GetLevel() == 0)
							Event->SetActive(false);
					}

					const std::vector<_EventTile> &Tiles = Event->GetTiles();
					if(Tiles.size() > 0) {
						Vector2 NewPosition(Tiles[0].Coord.X + 0.5f, Tiles[0].Coord.Y + 0.5f);
						Particles->Create(_ParticleSpawn(Assets.GetParticleTemplate(Event->GetParticleIdentifier()), NewPosition, OBJECT_Z, 0));

						Map->RemoveObjectFromGrid(Player, GRID_PLAYER);
						Player->SetPosition(NewPosition);
						Map->AddObjectToGrid(Player, GRID_PLAYER);
					}
				} break;
				case EVENT_LIGHT: {
					if(LastLightEvent != Event) {
						Map->SetAmbientLight(Assets.GetColor(Event->GetItemIdentifier()));
						Map->SetAmbientLightChangePeriod(Event->GetActivationPeriod());
						Map->SetAmbientLightRadius((float)Event->GetLevel());
						LastLightEvent = Event;
					}
				} break;
				default:
				break;
			}
		}
	}
}

// Update the active events
void _PlayState::UpdateEvents(double FrameTime) {

	// Activate events
	for(auto ActiveEventIterator = ActiveEvents.begin(); ActiveEventIterator != ActiveEvents.end(); ++ActiveEventIterator) {
		_Event *Event = *ActiveEventIterator;
		Event->Update(FrameTime);

		if(Event->TimerExpired()) {
			Vector2 Position;
			bool Decrement = false;
			switch(Event->GetType()) {
				case EVENT_SPAWN: {

					const std::vector<_EventTile> &Tiles = Event->GetTiles();
					for(size_t i = 0; i < Tiles.size(); i++) {
						Position[0] = static_cast<float>(Tiles[i].Coord.X) + 0.5f;
						Position[1] = static_cast<float>(Tiles[i].Coord.Y) + 0.5f;
						AddMonster(Assets.CreateMonster(Event->GetMonsterIdentifier(), Position));
						Particles->Create(_ParticleSpawn(Assets.GetParticleTemplate(Event->GetParticleIdentifier()), Position, OBJECT_Z, 0));
					}

					Decrement = true;
				} break;
				case EVENT_SOUND:
					Audio.Play(new _AudioSource(Audio.GetBuffer(Event->GetItemIdentifier()), true));
					Decrement = true;
				break;
				case EVENT_FSWITCH:
					if(Map->CanChangeMapState(Event)) {
						Map->ChangeMapState(Event);
						Decrement = true;
					}
				break;
				case EVENT_ENABLE: {
					const std::vector<_EventTile> &Tiles = Event->GetTiles();
					for(size_t i = 0; i < Tiles.size(); i++)
						Map->ToggleEventActive(Tiles[i].BlockID);

					Decrement = true;
				} break;
				default:
				break;
			}

			// Decrease the event level
			if(Decrement) {
				Event->StartTimer();
				if(Event->GetLevel() != -1)
					Event->Decrement();

				if(Event->GetLevel() == 0) {
					ActiveEventIterator = ActiveEvents.erase(ActiveEventIterator);
					if(ActiveEventIterator == ActiveEvents.end())
						break;
				}
			}

		}
	}
}

// Deletes the monsters
void _PlayState::DeleteMonsters() {

	for(auto Iterator : Monsters)
		if(Iterator)
			delete Iterator;

	Monsters.clear();
}

// Deletes the active events
void _PlayState::DeleteActiveEvents() {

	ActiveEvents.clear();
}

// Spawn an object in the map
void _PlayState::SpawnObject(_ObjectSpawn *ObjectSpawn, bool GenerateStats) {
	switch(ObjectSpawn->Type) {
		case _Object::MONSTER:
			AddMonster(Assets.CreateMonster(ObjectSpawn->Identifier, ObjectSpawn->Position));
		break;
		case _Object::MISCITEM:
			Map->AddItem(Assets.CreateMiscItem(ObjectSpawn->Identifier, 1, ObjectSpawn->Position));
		break;
		case _Object::AMMO:
			Map->AddItem(Assets.CreateAmmoItem(ObjectSpawn->Identifier, 1, ObjectSpawn->Position));
		break;
		case _Object::UPGRADE:
			Map->AddItem(Assets.CreateUpgradeItem(ObjectSpawn->Identifier, 1, ObjectSpawn->Position));
		break;
		case _Object::WEAPON:
			Map->AddItem(Assets.CreateWeapon(ObjectSpawn->Identifier, 1, ObjectSpawn->Position, GenerateStats));
		break;
		case _Object::ARMOR:
			Map->AddItem(Assets.CreateArmor(ObjectSpawn->Identifier, 1, ObjectSpawn->Position));
		break;
	}
}

// Adds a monster to the monster list and collision grid
void _PlayState::AddMonster(_Monster *Monster) {
	Monster->SetMap(Map);

	Monsters.push_back(Monster);
	Map->AddObjectToGrid(Monster, GRID_MONSTER);
}

// Removes a monster from object list and collision grid
void _PlayState::RemoveMonster(_Monster *Monster) {
	for(auto Iterator = Monsters.begin(); Iterator != Monsters.end(); ++Iterator) {
		if(*Iterator == Monster) {
			Monsters.erase(Iterator);
			break;
		}
	}

	Map->RemoveObjectFromGrid(Monster, GRID_MONSTER);
}

void _PlayState::GenerateBulletEffects(_Entity *Attacker, const int Type, const Vector2 &Position) {
	Vector2 ParticlePosition;

	if(Type == -1) {

		// Particle position
		ParticlePosition = Attacker->GetPosition() + Attacker->GetWeaponOffset(Attacker->GetWeaponType()).RotateVector(Attacker->GetDirection());

		// Particles
		Particles->Create(_ParticleSpawn(Attacker->GetWeaponParticle(WEAPONPARTICLE_FIRE), ParticlePosition, OBJECT_Z, Attacker->GetDirection()));
		Particles->Create(_ParticleSpawn(Attacker->GetWeaponParticle(WEAPONPARTICLE_SMOKE), ParticlePosition, OBJECT_Z, 0));
	}
	else if(Type == HIT_WALL) {
		Particles->Create(_ParticleSpawn(Attacker->GetWeaponParticle(WEAPONPARTICLE_RICOCHET), Position, OBJECT_Z, Attacker->GetDirection()));
		Particles->Create(_ParticleSpawn(Attacker->GetWeaponParticle(WEAPONPARTICLE_BULLETHOLE), Position, OBJECT_Z, Attacker->GetDirection()));
	}
	else if(Type == HIT_OBJECT) {
		ParticlePosition = GenerateRandomPointInCircle(0.7f) + Position;

		// Blood
		Particles->Create(_ParticleSpawn(Assets.GetParticleTemplate("bloodspurt0"), Position, OBJECT_Z, Attacker->GetDirection()));
		Particles->Create(_ParticleSpawn(Assets.GetParticleTemplate("blood0"), ParticlePosition, 0.06f, Attacker->GetDirection()));
	}
}

bool _PlayState::IsPaused() { return Menu.GetState() != _Menu::STATE_NONE; }