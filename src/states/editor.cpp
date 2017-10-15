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
#include <states/editor.h>
#include <framework.h>
#include <graphics.h>
#include <camera.h>
#include <input.h>
#include <font.h>
#include <assets.h>
#include <utils.h>
#include <map.h>
#include <events.h>
#include <menu.h>
#include <animation.h>
#include <config.h>
#include <constants.h>
#include <ui/element.h>
#include <ui/button.h>
#include <ui/textbox.h>
#include <ui/label.h>
#include <ui/style.h>
#include <objects/monster.h>
#include <objects/misc.h>
#include <objects/armor.h>
#include <objects/weapon.h>
#include <objects/player.h>
#include <states/play.h>
#include <sstream>
#include <iostream>
#include <SDL_keycode.h>
#include <SDL_mouse.h>

_EditorState EditorState;

// Input box
const char *InputBoxStrings[EDITINPUT_COUNT] = {
	"Load monster set",
	"Load map",
	"Save map",
	"Set item",
	"Set monster",
	"Set particle",
};

// Input box
const int PaletteSizes[EDITMODE_COUNT] = {
	32,
	64,
	64,
	32,
	48,
	32,
	32,
	32,
};

// Constructor
_EditorState::_EditorState()
:	SavedCameraPosition(ZERO_VECTOR),
	SavedCheckpointIndex(-1),
	MapFilename(""),
	SavedLayer(-1),
	SavedPalette(-1) {

}

void _EditorState::Init() {

	// Load command buttons
	MainFont = Assets.GetFont("menu_buttons");
	CommandElement = Assets.GetElement("editor_command");
	BlockElement = Assets.GetElement("editor_blocks");
	EventElement = Assets.GetElement("editor_events");
	InputBox = Assets.GetTextBox("editor_inputbox");

	// Create button groups
	PaletteElement[0] = Assets.GetElement("editor_palette_block");
	PaletteElement[1] = Assets.GetElement("editor_palette_events");
	PaletteElement[2] = Assets.GetElement("editor_palette_monsters");
	PaletteElement[3] = Assets.GetElement("editor_palette_items");
	PaletteElement[4] = Assets.GetElement("editor_palette_ammo");
	PaletteElement[5] = Assets.GetElement("editor_palette_upgrades");
	PaletteElement[6] = Assets.GetElement("editor_palette_weapons");
	PaletteElement[7] = Assets.GetElement("editor_palette_armors");

	// Assign layer buttons
	LayerButtons[0] = Assets.GetButton("editor_layer_base");
	LayerButtons[1] = Assets.GetButton("editor_layer_floor0");
	LayerButtons[2] = Assets.GetButton("editor_layer_floor1");
	LayerButtons[3] = Assets.GetButton("editor_layer_floor2");
	LayerButtons[4] = Assets.GetButton("editor_layer_flat");
	LayerButtons[5] = Assets.GetButton("editor_layer_wall");
	LayerButtons[6] = Assets.GetButton("editor_layer_fore");

	// Assign palette buttons
	ModeButtons[0] = Assets.GetButton("editor_mode_block");
	ModeButtons[1] = Assets.GetButton("editor_mode_event");
	ModeButtons[2] = Assets.GetButton("editor_mode_mons");
	ModeButtons[3] = Assets.GetButton("editor_mode_item");
	ModeButtons[4] = Assets.GetButton("editor_mode_ammo");
	ModeButtons[5] = Assets.GetButton("editor_mode_mod");
	ModeButtons[6] = Assets.GetButton("editor_mode_weap");
	ModeButtons[7] = Assets.GetButton("editor_mode_arm");

	// Reset state
	ResetEditorState();

	// Create camera
	Camera = new _Camera(ZERO_VECTOR, CAMERA_DISTANCE, CAMERA_EDITOR_DIVISOR);

	// Load level
	if(PlayState.GetFromEditor())
		MapFilename = EDITOR_TESTLEVEL;

	LoadMap(MapFilename, PlayState.GetFromEditor());

	// Set up graphics
	Graphics.ChangeViewport(Graphics.GetScreenWidth() - EDITOR_VIEWPORT_OFFSETX, Graphics.GetScreenHeight() - EDITOR_VIEWPORT_OFFSETY);
	Camera->CalculateFrustum(Graphics.GetAspectRatio());
	Graphics.ShowCursor(true);

	// Adjust UI
	for(int i = 0; i < EDITMODE_COUNT; i++)
		PaletteElement[i]->SetHeight(Graphics.GetViewportHeight() - 30);

	if(SavedLayer != -1)
		ExecuteUpdateLayer(SavedLayer, false);

	if(SavedPalette != -1)
		ExecuteSwitchMode(SavedPalette);

	if(SavedCheckpointIndex != -1)
		CheckpointIndex = SavedCheckpointIndex;
}

void _EditorState::Close() {
	SavedCameraPosition = Camera->GetPosition();
	SavedLayer = CurrentLayer;
	SavedPalette = CurrentPalette;
	SavedCheckpointIndex = CheckpointIndex;

	for(int i = 0; i < EDITMODE_COUNT; i++)
		ClearPalette(i);

	delete Camera;
	delete Map;

	Camera = nullptr;
	Map = nullptr;
}

// Load a level
bool _EditorState::LoadMap(const std::string &File, bool UseSavedCameraPosition) {
	bool Success = false;

	if(Map)
		delete Map;

	try {
		Map = new _Map(File);

		Success = true;
	}
	catch(std::exception &Error) {
		std::cout << Error.what() << std::endl;

		Map = new _Map();
		Map->LoadMonsterSet(MAP_DEFAULTMONSTERSET);
	}

	Map->SetCamera(Camera);
	ResetEditorState();

	// Set camera
	if(UseSavedCameraPosition)
		Camera->ForcePosition(SavedCameraPosition);
	else
		Camera->ForcePosition(Map->GetStartingPositionByCheckpoint(0));

	return Success;
}

void _EditorState::ResetEditorState() {
	WorldCursor = ZERO_VECTOR;
	SelectedBlockIndex = -1;
	SelectedEventIndex = -1;
	SelectedBlock = nullptr;
	SelectedEvent = nullptr;
	ClipboardEvent = nullptr;
	MinZ = 0.0f;
	MaxZ = 0.0f;
	Rotation = 0.0f;
	ScaleX = 1.0f;
	EventActivationPeriod = 0;
	EventActive = 1;
	EventLevel = 0;
	AltTextureIdentifier = "";
	AltTexture = nullptr;
	EditorInput = -1;
	CheckpointIndex = 0;
	ClickedPosition = ZERO_VECTOR;
	CopiedPosition = ZERO_VECTOR;
	MoveDelta = ZERO_VECTOR;
	CurrentLayer = EDITOR_DEFAULT_LAYER;
	CurrentPalette = EDITMODE_BLOCKS;
	GridMode = EDITOR_DEFAULT_GRIDMODE;
	Walkable = true;
	HighlightBlocks = false;
	SelectedObjects.clear();
	SelectedObjectIndices.clear();
	ClipboardObjects.clear();

	IsShiftDown = false;
	IsCtrlDown = false;
	DraggingBox = false;
	BlockCopied = false;
	IsDrawing = false;
	IsMoving = false;
	FinishDrawing = false;
	BlockTextEvent = false;

	WorldCursorIndex.X = 0;
	WorldCursorIndex.Y = 0;
	DrawStart.X = 0;
	DrawStart.Y = 0;
	DrawEnd.X = 0;
	DrawEnd.Y = 0;
	OldStart.X = 0;
	OldStart.Y = 0;
	OldEnd.X = 0;
	OldEnd.Y = 0;
	SavedIndex.X = 0;
	SavedIndex.Y = 0;

	for(int i = 0; i < MAPLAYER_COUNT; i++)
		UndoNumber[i] = 0;

	// Enable default button
	for(int i = 0; i < MAPLAYER_COUNT; i++)
		LayerButtons[i]->SetEnabled(false);

	for(int i = 0; i < EDITMODE_COUNT; i++) {
		ModeButtons[i]->SetEnabled(false);
		Brush[i] = nullptr;
	}

	// Load palettes
	LoadPalettes();
	LayerButtons[CurrentLayer]->SetEnabled(true);
	ModeButtons[CurrentPalette]->SetEnabled(true);
}

// Action handler
bool _EditorState::HandleAction(int InputType, int Action, int Value) {

	return false;
}

// Key handler
void _EditorState::KeyEvent(const _KeyEvent &KeyEvent) {
	if(IsMoving || IsDrawing || !KeyEvent.Pressed)
		return;

	// See if the user is entering in text
	if(EditorInput != -1) {
		switch(KeyEvent.Key) {
			case SDL_SCANCODE_RETURN: {
				const std::string InputText = InputBox->GetText();
				switch(EditorInput) {
					case EDITINPUT_LOADMONSTERSET:

						if(!Map->LoadMonsterSet(InputText))
							SavedText[EditorInput] = "";
						else {
							SavedText[EditorInput] = InputText;
							LoadMonsterButtons();
						}

						ExecuteDeselect();
					break;
					case EDITINPUT_LOAD: {
						if(InputText == "")
							break;

						if(LoadMap(InputText, false))
							SavedText[EDITINPUT_SAVE] = InputText;

					} break;
					case EDITINPUT_SAVE:
						if(InputText == "" || !Map->SaveLevel(InputText))
							SavedText[EditorInput] = "";
						else {
							SavedText[EditorInput] = InputText;
						}

						ExecuteDeselect();
					break;
					case EDITINPUT_ITEMIDENTIFIER:
					case EDITINPUT_MONSTERIDENTIFIER:
					case EDITINPUT_PARTICLEIDENTIFIER:
						UpdateEventIdentifier(EditorInput, InputText);

						if(!EventSelected())
							SavedText[EditorInput] = InputText;
					break;
				}
				EditorInput = -1;
			} break;
			case SDL_SCANCODE_ESCAPE:
				EditorInput = -1;
			break;
			default:
				InputBox->HandleKeyEvent(KeyEvent);
			break;
		}
	}
	else {

		// Command keys
		switch(KeyEvent.Key) {

			// Exit
			case SDL_SCANCODE_ESCAPE:
				Framework.SetDone(true);
			break;
			case SDL_SCANCODE_F1:
				ExecuteUpdateLayer(0, false);
			break;
			case SDL_SCANCODE_F2:
				ExecuteUpdateLayer(1, false);
			break;
			case SDL_SCANCODE_F3:
				ExecuteUpdateLayer(2, false);
			break;
			case SDL_SCANCODE_F4:
				ExecuteUpdateLayer(3, false);
			break;
			case SDL_SCANCODE_F5:
				ExecuteUpdateLayer(4, false);
			break;
			case SDL_SCANCODE_W:
				ExecuteUpdateLayer(5, false);
			break;
			case SDL_SCANCODE_F:
				ExecuteUpdateLayer(6, false);
			break;
			case SDL_SCANCODE_MINUS:
				ExecuteUpdateCheckpointIndex(-1);
			break;
			case SDL_SCANCODE_EQUALS:
				ExecuteUpdateCheckpointIndex(1);
			break;
			case SDL_SCANCODE_1:
				ExecuteSwitchMode(EDITMODE_BLOCKS);
			break;
			case SDL_SCANCODE_2:
				ExecuteSwitchMode(EDITMODE_EVENTS);
			break;
			case SDL_SCANCODE_3:
				ExecuteSwitchMode(EDITMODE_MONSTERS);
			break;
			case SDL_SCANCODE_4:
				ExecuteSwitchMode(EDITMODE_ITEMS);
			break;
			case SDL_SCANCODE_5:
				ExecuteSwitchMode(EDITMODE_AMMO);
			break;
			case SDL_SCANCODE_6:
				ExecuteSwitchMode(EDITMODE_UPGRADES);
			break;
			case SDL_SCANCODE_7:
				ExecuteSwitchMode(EDITMODE_WEAPONS);
			break;
			case SDL_SCANCODE_8:
				ExecuteSwitchMode(EDITMODE_ARMOR);
			break;
			case SDL_SCANCODE_GRAVE:
				ExecuteDeselect();
			break;
			case SDL_SCANCODE_D:
				ExecuteDelete();
			break;
			case SDL_SCANCODE_C:
				ExecuteCopy();
			break;
			case SDL_SCANCODE_V:
				ExecutePaste(true);
			break;
			case SDL_SCANCODE_G:
				if(IsShiftDown)
					ExecuteUpdateGridMode(-1);
				else
					ExecuteUpdateGridMode(1);
			break;
			case SDL_SCANCODE_B:
				ExecuteHighlightBlocks();
			break;
			case SDL_SCANCODE_A:
				ExecuteWalkable();
			break;
			case SDL_SCANCODE_R:
				ExecuteRotate();
			break;
			case SDL_SCANCODE_M:
				if(IsShiftDown) {
					ExecuteIOCommand(EDITINPUT_MONSTERIDENTIFIER);
					BlockTextEvent = true;
				}
				else
					ExecuteMirror();
			break;
			case SDL_SCANCODE_KP_MINUS:
				ExecuteChangeZ(-0.5f, !IsShiftDown);
			break;
			case SDL_SCANCODE_KP_PLUS:
				ExecuteChangeZ(0.5f, !IsShiftDown);
			break;
			case SDL_SCANCODE_SPACE:
				ExecuteToggleTile();
			break;
			case SDL_SCANCODE_P:
				if(IsShiftDown) {
					ExecuteIOCommand(EDITINPUT_PARTICLEIDENTIFIER);
					BlockTextEvent = true;
				}
			break;
			case SDL_SCANCODE_Z:
				if(IsCtrlDown)
					ExecuteUndo();
			break;
			case SDL_SCANCODE_N:
				if(IsCtrlDown)
					ExecuteClear();
			break;
			case SDL_SCANCODE_I:
				if(IsShiftDown) {
					ExecuteIOCommand(EDITINPUT_ITEMIDENTIFIER);
					BlockTextEvent = true;
				}
			break;
			case SDL_SCANCODE_O:
				ExecuteIOCommand(EDITINPUT_LOADMONSTERSET);
				BlockTextEvent = true;
			break;
			case SDL_SCANCODE_L:
				ExecuteIOCommand(EDITINPUT_LOAD);
				BlockTextEvent = true;
			break;
			case SDL_SCANCODE_S:
				ExecuteIOCommand(EDITINPUT_SAVE);
				BlockTextEvent = true;
			break;
			case SDL_SCANCODE_T:
				ExecuteTest();
			break;
			case SDL_SCANCODE_TAB:
				if(IsShiftDown)
					ExecuteUpdateSelectedPalette(-1);
				else
					ExecuteUpdateSelectedPalette(1);
			break;
			case SDL_SCANCODE_LEFT:
				ExecuteUpdateBlockLimits(0, !IsShiftDown);
			break;
			case SDL_SCANCODE_UP:
				ExecuteUpdateBlockLimits(1, !IsShiftDown);
			break;
			case SDL_SCANCODE_RIGHT:
				ExecuteUpdateBlockLimits(2, !IsShiftDown);
			break;
			case SDL_SCANCODE_DOWN:
				ExecuteUpdateBlockLimits(3, !IsShiftDown);
			break;
		}
	}
}

// Text event handler
void _EditorState::TextEvent(const char *Text) {
	if(EditorInput != -1) {
		if(BlockTextEvent)
			BlockTextEvent = false;
		else
			InputBox->HandleTextEvent(Text);
	}
}

// Mouse handler
void _EditorState::MouseEvent(const _MouseEvent &MouseEvent) {

	if(MouseEvent.Button == SDL_BUTTON_LEFT) {
		CommandElement->HandleInput(MouseEvent.Pressed);
		BlockElement->HandleInput(MouseEvent.Pressed);
	}
	if(MouseEvent.Button == SDL_BUTTON_LEFT || MouseEvent.Button == SDL_BUTTON_RIGHT) {
		EventElement->HandleInput(MouseEvent.Pressed);
		PaletteElement[CurrentPalette]->HandleInput(MouseEvent.Pressed);
	}

	// Handle command group clicks
	_Element *Clicked = CommandElement->GetClickedElement();
	if(Clicked && Clicked->GetID() != -1) {
		ProcessIcons(Clicked->GetID(), MouseEvent.Button == SDL_BUTTON_RIGHT);
	}

	if(CurrentPalette == EDITMODE_BLOCKS) {
		_Element *Clicked = BlockElement->GetClickedElement();
		if(Clicked && Clicked->GetID() != -1) {
			ProcessBlockIcons(Clicked->GetID(), MouseEvent.Button == SDL_BUTTON_RIGHT);
		}
	}

	if(CurrentPalette == EDITMODE_EVENTS) {
		_Element *Clicked = EventElement->GetClickedElement();
		if(Clicked && Clicked->GetID() != -1) {
			ProcessEventIcons(Clicked->GetID(), MouseEvent.Button == SDL_BUTTON_RIGHT);
		}
	}

	// Distinguish between interface and viewport clicks
	if(Input.GetMouse().X < Graphics.GetViewportWidth() && Input.GetMouse().Y < Graphics.GetViewportHeight()) {
		if(MouseEvent.Pressed) {

			// Mouse press
			switch(MouseEvent.Button) {
				case SDL_BUTTON_LEFT:
					if(!IsMoving && !Clicked) {
						switch(CurrentPalette) {
							case EDITMODE_BLOCKS:
							case EDITMODE_EVENTS:

								DeselectBlock();
								DeselectEvent();

								// Save the indices
								SavedIndex = WorldCursorIndex;
								IsDrawing = true;
								FinishDrawing = false;

							break;
							default: {
								_Button *Button = Brush[CurrentPalette];
								if(Button)
									SpawnObject(Map->GetValidPosition(WorldCursor), StateToType(CurrentPalette), Button->GetIdentifier(), IsShiftDown);
							} break;
						}
					}
				break;
				case SDL_BUTTON_RIGHT:

					// Move the camera
					Camera->SetPosition(WorldCursor);

				break;
				case SDL_BUTTON_MIDDLE:
					if(!IsDrawing) {
						switch(CurrentPalette) {
							case EDITMODE_BLOCKS:

								// Get the block
								SelectedBlockIndex = Map->GetSelectedBlock(CurrentLayer, WorldCursorIndex, &SelectedBlock);
								if(BlockSelected()) {

									// Save old states
									OldStart = SelectedBlock->Start;
									OldEnd = SelectedBlock->End;
									SavedIndex = WorldCursorIndex;

									IsMoving = true;
								}

							break;
							case EDITMODE_EVENTS:

								// Get the event
								SelectedEventIndex = Map->GetSelectedEvent(WorldCursorIndex, &SelectedEvent);
								if(EventSelected()) {

									// Save old states
									OldStart = SelectedEvent->GetStart();
									OldEnd = SelectedEvent->GetEnd();
									SavedIndex = WorldCursorIndex;

									// Remove bad tiles
									std::vector<_EventTile> &Tiles = SelectedEvent->GetTiles();
									for(size_t i = 0; i < Tiles.size(); i++) {
										int LayerSize = Map->GetLayerSize(Tiles[i].Layer);
										if(Tiles[i].BlockID != -1 && LayerSize != -1 && Tiles[i].BlockID >= LayerSize) {
											std::cout << "Bad BlockID cleansed in layer" << Tiles[i].Layer << ": " << Tiles[i].BlockID << " vs " << LayerSize << std::endl;
											Tiles[i].BlockID = -1;
										}
									}
									IsMoving = true;
								}
							break;
							default:
								SelectObject();
							break;
						}
					}
				break;
			}


		}
	}
	else {

		// Get button click for palette
		_Button *Button = (_Button *)PaletteElement[CurrentPalette]->GetClickedElement();
		if(Button) {
			ExecuteSelectPalette(Button, MouseEvent.Button == SDL_BUTTON_RIGHT);
		}
	}

	// Mouse Release
	if(!MouseEvent.Pressed) {
		switch(MouseEvent.Button) {
			case SDL_BUTTON_LEFT:
				if(IsDrawing) {
					FinishDrawing = true;
					UndoNumber[CurrentLayer]++;
				}
			break;
			case SDL_BUTTON_MIDDLE:
				if(IsMoving) {
					IsMoving = false;
					for(auto Iterator : SelectedObjects) {
						Iterator->Position = GetMoveDeltaPosition(Iterator->Position);
					}
					MoveDelta = ZERO_VECTOR;
				}

				if(DraggingBox) {
					DraggingBox = false;
					SelectObjects();
				}
			break;
		}
	}
}

// Mouse wheel handler
void _EditorState::MouseWheelEvent(int Direction) {

	if(Input.GetMouse().X < Graphics.GetViewportWidth() && Input.GetMouse().Y < Graphics.GetViewportHeight()) {
		float Multiplier = 1.0f * Direction;
		if(IsShiftDown)
			Multiplier = 10.0f * Direction;

		// Zoom
		Camera->UpdateDistance(-Multiplier);
		if(Camera->GetTargetDistance() < 1.0f)
			Camera->SetDistance(1.0f);
	}
	else {
		if(Direction > 0)
			PaletteElement[CurrentPalette]->UpdateChildrenOffset(_Point(0, PaletteSizes[CurrentPalette]));
		else
			PaletteElement[CurrentPalette]->UpdateChildrenOffset(_Point(0, -PaletteSizes[CurrentPalette]));
	}
}

// Update
void _EditorState::Update(double FrameTime) {

	CommandElement->Update(FrameTime, Input.GetMouse());
	BlockElement->Update(FrameTime, Input.GetMouse());
	EventElement->Update(FrameTime, Input.GetMouse());
	PaletteElement[CurrentPalette]->Update(FrameTime, Input.GetMouse());
	if(EditorInput != -1) {
		InputBox->Update(FrameTime, Input.GetMouse());
	}

	// Get modifier key status
	IsShiftDown = Input.ModKeyDown(KMOD_SHIFT) ? true : false;
	IsCtrlDown = Input.ModKeyDown(KMOD_CTRL) ? true : false;

	// Get world cursor
	Camera->ConvertScreenToWorld(Input.GetMouse(), WorldCursor);

	// Get tile indices for later usage
	WorldCursorIndex = Map->GetValidCoord(WorldCursor);

	// Set camera position
	Camera->Update(FrameTime);

	// Drawing a block or event
	if(IsDrawing) {

		// Get start positions
		DrawStart = SavedIndex;

		// Check bounds
		DrawEnd = WorldCursorIndex + 1;

		// Reverse X
		if(DrawEnd.X <= DrawStart.X) {
			std::swap(DrawStart.X, DrawEnd.X);
			DrawStart.X--;
			DrawEnd.X++;
		}

		// Reverse Y
		if(DrawEnd.Y <= DrawStart.Y) {
			std::swap(DrawStart.Y, DrawEnd.Y);
			DrawStart.Y--;
			DrawEnd.Y++;
		}
	}

	// Moving a block or event
	if(IsMoving) {
		_Coord Offset;

		// Get offsets
		Offset = WorldCursorIndex - SavedIndex;

		// Check x bounds
		if(Offset.X + OldStart.X < 0)
			Offset.X = -OldStart.X;
		else if(Offset.X + OldEnd.X >= Map->GetWidth())
			Offset.X = Map->GetWidth() - OldEnd.X - 1;

		// Check y bounds
		if(Offset.Y + OldStart.Y < 0)
			Offset.Y = -OldStart.Y;
		else if(Offset.Y + OldEnd.Y >= Map->GetHeight())
			Offset.Y = Map->GetHeight() - OldEnd.Y - 1;

		// Get start positions
		DrawStart = OldStart + Offset;

		// Check bounds
		DrawEnd.X = OldEnd.X + Offset.X + 1;
		DrawEnd.Y = OldEnd.Y + Offset.Y + 1;

	}

	// Update based on editor state
	switch(CurrentPalette) {
		case EDITMODE_BLOCKS:

			// Finish drawing a block and add it to the list
			if(FinishDrawing) {
				if(Brush[EDITMODE_BLOCKS]) {
					_Block Block;
					Block.Start = DrawStart;
					Block.End = DrawEnd-1;
					Block.MinZ = MinZ;
					Block.MaxZ = MaxZ;
					Block.Texture = Brush[EDITMODE_BLOCKS]->GetStyle()->GetTexture();
					Block.AltTexture = AltTexture;
					Block.TextureIdentifier = Brush[EDITMODE_BLOCKS]->GetIdentifier();
					Block.AltTextureIdentifier = Brush[EDITMODE_BLOCKS]->GetIdentifier();
					Block.Rotation = Rotation;
					Block.ScaleX = ScaleX;
					Block.Wall = (CurrentLayer == EDITOR_WALL_LAYER);
					Block.Walkable = Walkable;

					Map->AddBlock(CurrentLayer, Block);
				}

				FinishDrawing = IsDrawing = false;
			}

			if(IsMoving) {
				SelectedBlock->Start = DrawStart;
				SelectedBlock->End = DrawEnd-1;
			}
		break;
		case EDITMODE_EVENTS:
			if(FinishDrawing) {
				if(Brush[EDITMODE_EVENTS])
					AddEvent(Brush[EDITMODE_EVENTS]->GetID());
				FinishDrawing = IsDrawing = false;
			}

			if(IsMoving) {
				SelectedEvent->SetStart(DrawStart);
				SelectedEvent->SetEnd(DrawEnd-1);
			}
		break;
		default:
			if(IsMoving)
				MoveDelta = WorldCursor - ClickedPosition;
		break;
	}
}

// Render the state
void _EditorState::Render(double BlendFactor) {

	// Setup 3D transformation
	Graphics.Setup3DViewport();
	Camera->Set3DProjection(BlendFactor);
	Graphics.EnableDepthTest();

	// Draw floors
	Map->RenderFloors();

	// Draw tentative block
	if(IsDrawing) {
		if(Brush[CurrentPalette]) {
			if(CurrentPalette == EDITMODE_EVENTS) {
				Graphics.DisableDepthTest();
				Graphics.DrawRepeatable((float)DrawStart.X, (float)DrawStart.Y, MAP_LAYEROFFSET, (float)DrawEnd.X, (float)DrawEnd.Y, MAP_LAYEROFFSET, Brush[CurrentPalette]->GetStyle()->GetTexture(), 0, 1.0f);
				Graphics.EnableDepthTest();
			}
			else {
				if(CurrentLayer == MAPLAYER_FORE)
					Graphics.DrawRepeatable((float)DrawStart.X, (float)DrawStart.Y, (float)MaxZ + MAP_LAYEROFFSET * CurrentLayer, (float)DrawEnd.X, (float)DrawEnd.Y, (float)MaxZ + MAP_LAYEROFFSET * CurrentLayer, Brush[CurrentPalette]->GetStyle()->GetTexture(), Rotation, ScaleX);
				else if(CurrentLayer == MAPLAYER_FLAT) {
					Graphics.EnableVBO(VBO_CUBE);
					Graphics.DrawWall((float)DrawStart.X, (float)DrawStart.Y, (float)MinZ, (float)DrawEnd.X - DrawStart.X, (float)DrawEnd.Y - DrawStart.Y, MaxZ - MinZ, Rotation, Brush[CurrentPalette]->GetStyle()->GetTexture());
					Graphics.DisableVBO(VBO_CUBE);
				}
				else {
					if(MaxZ == MinZ) {
						Graphics.DrawRepeatable((float)DrawStart.X, (float)DrawStart.Y, (float)MinZ + MAP_LAYEROFFSET * CurrentLayer, (float)DrawEnd.X, (float)DrawEnd.Y, (float)MinZ + MAP_LAYEROFFSET * CurrentLayer, Brush[CurrentPalette]->GetStyle()->GetTexture(), Rotation, ScaleX);
					}
					else {
						Graphics.EnableVBO(VBO_CUBE);
						Graphics.DrawCube((float)DrawStart.X, (float)DrawStart.Y, (float)MinZ, (float)DrawEnd.X - DrawStart.X, (float)DrawEnd.Y - DrawStart.Y, (float)MaxZ - MinZ, Brush[CurrentPalette]->GetStyle()->GetTexture());
						Graphics.DisableVBO(VBO_CUBE);
					}
				}
			}
		}
	}

	// Draw objects
	Graphics.SetDepthMask(false);
	Graphics.EnableVBO(VBO_QUAD);
	const std::vector<_ObjectSpawn *> &Objects = Map->GetObjectsList();
	for(size_t i = 0; i < Objects.size(); i++) {
		DrawObject(0.0f, 0.0f, Objects[i], 1.0f);
	}
	Graphics.DisableVBO(VBO_QUAD);

	// Outline selected item
	Graphics.EnableVBO(VBO_CIRCLE);
	for(auto Iterator : SelectedObjects) {
		Vector2 Position = GetMoveDeltaPosition(Iterator->Position);
		Graphics.DrawCircle(Position[0], Position[1], ITEM_Z + 0.05f, EDITOR_OBJECTRADIUS, COLOR_WHITE);
	}
	Graphics.DisableVBO(VBO_CIRCLE);

	// Draw faded items while moving
	Graphics.EnableVBO(VBO_QUAD);
	for(auto Iterator : SelectedObjects) {
		DrawObject(MoveDelta[0], MoveDelta[1], Iterator, 0.5f);
	}
	Graphics.DisableVBO(VBO_QUAD);
	Graphics.SetDepthMask(true);

	// Draw walls
	Map->RenderWalls();

	// Draw the foreground tiles
	Map->RenderForeground();

	// Draw the events
	Map->RenderEvents(EventTextures);

	Graphics.DisableDepthTest();

	// Draw map boundaries
	Graphics.DrawRectangle(-0.01f, -0.01f, Map->GetWidth() + 0.01f, Map->GetHeight() + 0.01f, COLOR_RED);

	// Draw grid
	Map->RenderGrid(GridMode);

	// Outline the blocks
	if(HighlightBlocks)
		Map->HighlightBlocks(CurrentLayer);

	// Outline selected block
	if(BlockSelected())
		Graphics.DrawRectangle((float)SelectedBlock->Start.X, (float)SelectedBlock->Start.Y, (float)SelectedBlock->End.X + 1.0f, (float)SelectedBlock->End.Y + 1.0f, COLOR_WHITE);

	// Outline selected event
	if(EventSelected()) {
		Graphics.DrawRectangle((float)SelectedEvent->GetStart().X + 0.02f, (float)SelectedEvent->GetStart().Y + 0.02f, (float)SelectedEvent->GetEnd().X + 0.98f, (float)SelectedEvent->GetEnd().Y + 0.98f, COLOR_CYAN);

		// Outline affected tiles and blocks
		const std::vector<_EventTile> &Tiles = SelectedEvent->GetTiles();
		for(size_t i = 0; i < Tiles.size(); i++) {
			Graphics.DrawRectangle(Tiles[i].Coord.X + 0.2f, Tiles[i].Coord.Y + 0.2f, Tiles[i].Coord.X + 0.8f, Tiles[i].Coord.Y + 0.8f, COLOR_RED);

			if(Tiles[i].BlockID != -1) {
				if(SelectedEvent->GetType() == EVENT_ENABLE) {
					const _Event *Event = Map->GetEvent(Tiles[i].BlockID);
					Graphics.DrawRectangle((float)Event->GetStart().X, (float)Event->GetStart().Y, (float)Event->GetEnd().X + 1.0f, (float)Event->GetEnd().Y + 1.0f, COLOR_YELLOW);
				}
				else {
					const _Block *Block = Map->GetBlock(Tiles[i].Layer, Tiles[i].BlockID);
					Graphics.DrawRectangle((float)Block->Start.X, (float)Block->Start.Y, (float)Block->End.X + 1.0f, (float)Block->End.Y + 1.0f, COLOR_GREEN);
				}
			}
		}
	}

	// Dragging a box around object
	if(DraggingBox)
		Graphics.DrawRectangle(ClickedPosition[0], ClickedPosition[1], WorldCursor[0], WorldCursor[1], COLOR_WHITE);

	// Draw a block
	if(IsDrawing)
		Graphics.DrawRectangle((float)DrawStart.X, (float)DrawStart.Y, (float)DrawEnd.X, (float)DrawEnd.Y, COLOR_GREEN);

	Graphics.EnableDepthTest();

	// Setup 2D transformation
	Graphics.Setup2DProjectionMatrix();
	std::ostringstream Buffer;

	// Draw viewport outline
	Graphics.DrawRectangle(0, 0, Graphics.GetViewportWidth(), Graphics.GetViewportHeight(), COLOR_DARK);

	// Draw text
	if(EditorInput != -1) {
		InputBox->Render();
	}

	// Draw filename
	Buffer << Map->GetFilename();
	MainFont->DrawText(Buffer.str(), 25, 25);
	Buffer.str("");

	// Draw cursor position
	int X = 16;
	int Y = Graphics.GetScreenHeight() - 25;
	Buffer << std::fixed << WorldCursor[0] << ", " << WorldCursor[1];
	MainFont->DrawText(Buffer.str(), X, Y);
	Buffer.str("");

	// Draw FPS
	X = Graphics.GetViewportWidth() - 45;
	Y = 25;
	Buffer << Graphics.GetFramesPerSecond() << " FPS";
	MainFont->DrawText(Buffer.str(), X, Y, COLOR_WHITE, RIGHT_BASELINE);
	Buffer.str("");

	// Draw selection count
	Buffer << SelectedObjects.size() << " selected";
	MainFont->DrawText(Buffer.str(), X, Y + 20, COLOR_WHITE, RIGHT_BASELINE);
	Buffer.str("");

	// Draw checkpoint info
	X = Graphics.GetViewportWidth() - 45;
	Y = Graphics.GetViewportHeight() - 40;
	Buffer << CheckpointIndex;
	MainFont->DrawText("Checkpoint:", X, Y, COLOR_WHITE, RIGHT_BASELINE);
	MainFont->DrawText(Buffer.str(), X + 5, Y);
	Buffer.str("");

	// Draw grid size
	Y += 20;
	Buffer << GridMode;
	MainFont->DrawText("Grid:", X, Y, COLOR_WHITE, RIGHT_BASELINE);
	MainFont->DrawText(Buffer.str(), X + 5, Y);
	Buffer.str("");

	// Draw command buttons
	CommandElement->Render();
	if(CurrentPalette == EDITMODE_BLOCKS) {
		BlockElement->Render();
	}
	else if(CurrentPalette == EDITMODE_EVENTS) {
		EventElement->Render();
	}

	// Draw current brush
	DrawBrush();

	// Draw Palette
	PaletteElement[CurrentPalette]->Render();

	Graphics.SetDepthMask(true);
}

// Load palette buttons
void _EditorState::LoadPalettes() {
	std::vector<_Brush> Icons;

	// Load map textures
	Assets.GetTextureList(Icons, _Texture::MAP);
	LoadPaletteButtons(Icons, EDITMODE_BLOCKS);
	Icons.clear();

	// Load events
	Assets.GetEventList(Icons);
	LoadPaletteButtons(Icons, EDITMODE_EVENTS);
	for(size_t i = 0; i < Icons.size(); i++)
		EventTextures.push_back(Icons[i].Texture);
	Icons.clear();

	// Load monsters
	LoadMonsterButtons();

	// Load items
	Assets.GetItemList(Icons);
	LoadPaletteButtons(Icons, EDITMODE_ITEMS);
	Icons.clear();

	// Load ammo
	Assets.GetAmmoList(Icons);
	LoadPaletteButtons(Icons, EDITMODE_AMMO);
	Icons.clear();

	// Load upgrades
	Assets.GetUpgradeList(Icons);
	LoadPaletteButtons(Icons, EDITMODE_UPGRADES);
	Icons.clear();

	// Load weapons
	Assets.GetWeaponList(Icons);
	LoadPaletteButtons(Icons, EDITMODE_WEAPONS);
	Icons.clear();

	// Load armor
	Assets.GetArmorList(Icons);
	LoadPaletteButtons(Icons, EDITMODE_ARMOR);
	Icons.clear();
}

// Free memory used by palette
void _EditorState::ClearPalette(int Type) {
	std::vector<_Element *> &Children = PaletteElement[Type]->GetChildren();
	for(size_t i = 0; i < Children.size(); i++) {
		delete Children[i]->GetStyle();
		delete Children[i];
	}
	Children.clear();
}

// Loads the palette buttons from the map's monster set
void _EditorState::LoadMonsterButtons() {
	std::vector<_Brush> Icons;
	Assets.GetMonsterList(Icons);
	LoadPaletteButtons(Icons, EDITMODE_MONSTERS);
}

// Loads the palette
void _EditorState::LoadPaletteButtons(std::vector<_Brush> &Icons, int Type) {
	ClearPalette(Type);

	// Loop through textures
	_Point Offset(0, 0);
	int Width = PaletteElement[Type]->GetSize().X;
	for(size_t i = 0; i < Icons.size(); i++) {
		PaletteElement[Type]->AddChild(new _Button(
			Icons[i].Identifier,
			PaletteElement[Type],
			Offset,
			_Point(PaletteSizes[Type], PaletteSizes[Type]),
			LEFT_TOP,
			new _Style(Icons[i].Text, false, false, COLOR_WHITE, COLOR_WHITE, Icons[i].Texture, Icons[i].Color, true),
			Assets.GetStyle("editor_selected0")));

		Offset.X += PaletteSizes[Type];
		if(Offset.X > Width - PaletteSizes[Type]) {
			Offset.Y += PaletteSizes[Type];
			Offset.X = 0;
		}
	}
}

// Draws the current brush
void _EditorState::DrawBrush() {

	// Get selected palette
	std::string IconText = "", IconIdentifier = "";
	_Color IconColor = COLOR_WHITE;
	const _Texture *IconTexture = nullptr;
	if(Brush[CurrentPalette]) {
		IconIdentifier = Brush[CurrentPalette]->GetIdentifier();
		IconText = Brush[CurrentPalette]->GetStyle()->GetIdentifier();
		IconTexture = Brush[CurrentPalette]->GetStyle()->GetTexture();
		IconColor = Brush[CurrentPalette]->GetStyle()->GetTextureColor();
	}

	// Edit mode specific text
	float IconRotation = 0;
	float IconScaleX = 1.0f;
	switch(CurrentPalette) {
		case EDITMODE_BLOCKS: {

			// See if there's a selected block
			std::string BlockAltTextureIdentifier;
			float BlockMinZ, BlockMaxZ;
			bool BlockWalkable;
			if(BlockSelected()) {
				IconTexture = SelectedBlock->Texture;
				IconText = SelectedBlock->TextureIdentifier;
				IconRotation = SelectedBlock->Rotation;
				IconScaleX = SelectedBlock->ScaleX;
				BlockMinZ = SelectedBlock->MinZ;
				BlockMaxZ = SelectedBlock->MaxZ;
				BlockWalkable = SelectedBlock->Walkable;
				BlockAltTextureIdentifier = SelectedBlock->AltTextureIdentifier;
			}
			else {
				if(Brush[CurrentPalette])
					IconText = Brush[CurrentPalette]->GetIdentifier();
				IconRotation = Rotation;
				IconScaleX = ScaleX;
				BlockMinZ = MinZ;
				BlockMaxZ = MaxZ;
				BlockWalkable = Walkable;
				BlockAltTextureIdentifier = AltTextureIdentifier;
			}
			IconIdentifier = "";

			int X = (float)Graphics.GetViewportWidth() + 100;
			int Y = (float)Graphics.GetViewportHeight() + 5;
			std::ostringstream Buffer;
			Buffer << IconRotation;
			MainFont->DrawText("Rotation:", X, Y, COLOR_WHITE, RIGHT_BASELINE);
			MainFont->DrawText(Buffer.str(), X + 5, Y);
			Buffer.str("");

			Buffer << BlockMinZ;
			MainFont->DrawText("Min Z:", X + 85, Y, COLOR_WHITE, RIGHT_BASELINE);
			MainFont->DrawText(Buffer.str(), X + 90, Y);
			Buffer.str("");

			Y += 15;
			Buffer << IconScaleX;
			MainFont->DrawText("ScaleX:", X, Y, COLOR_WHITE, RIGHT_BASELINE);
			MainFont->DrawText(Buffer.str(), X + 5, Y);
			Buffer.str("");

			Buffer << BlockMaxZ;
			MainFont->DrawText("Max Z:", X + 85, Y, COLOR_WHITE, RIGHT_BASELINE);
			MainFont->DrawText(Buffer.str(), X + 90, Y);
			Buffer.str("");

			Y += 15;
			Buffer << BlockWalkable;
			MainFont->DrawText("Walk:", X, Y, COLOR_WHITE, RIGHT_BASELINE);
			MainFont->DrawText(Buffer.str(), X + 5, Y);
			Buffer.str("");

			MainFont->DrawText(BlockAltTextureIdentifier, (float)Graphics.GetViewportWidth() + 112, (float)Graphics.GetViewportHeight() + 145, COLOR_WHITE, CENTER_MIDDLE);
		} break;
		case EDITMODE_EVENTS: {

			// Get object identifiers
			std::string ItemIdentifier, MonsterIdentifier, ParticleIdentifier;
			double ActivationPeriod;
			int Active, Level;
			if(EventSelected()) {
				_Button *Button = (_Button *)PaletteElement[EDITMODE_EVENTS]->GetChildren()[SelectedEvent->GetType()];
				IconTexture = Button->GetStyle()->GetTexture();
				IconIdentifier = Button->GetIdentifier();
				IconText = Button->GetStyle()->GetIdentifier();

				ItemIdentifier = SelectedEvent->GetItemIdentifier();
				MonsterIdentifier = SelectedEvent->GetMonsterIdentifier();
				ParticleIdentifier = SelectedEvent->GetParticleIdentifier();
				Active = SelectedEvent->GetActive();
				Level = SelectedEvent->GetLevel();
				ActivationPeriod = SelectedEvent->GetActivationPeriod();
			}
			else {
				ItemIdentifier = SavedText[EDITINPUT_ITEMIDENTIFIER];
				MonsterIdentifier = SavedText[EDITINPUT_MONSTERIDENTIFIER];
				ParticleIdentifier = SavedText[EDITINPUT_PARTICLEIDENTIFIER];
				Active = EventActive;
				Level = EventLevel;
				ActivationPeriod = EventActivationPeriod;
			}

			int X = Graphics.GetViewportWidth() + 75;
			int Y = Graphics.GetViewportHeight() - 15;

			std::ostringstream Buffer;
			Buffer << Active;
			MainFont->DrawText("Active:", X, Y, COLOR_WHITE, RIGHT_BASELINE);
			MainFont->DrawText(Buffer.str(), X + 5, Y);
			Buffer.str("");

			Y += 15;
			MainFont->DrawText("Item:", X, Y, COLOR_WHITE, RIGHT_BASELINE);
			MainFont->DrawText(ItemIdentifier, X + 5, Y);

			Y += 15;
			MainFont->DrawText("Monster:", X, Y, COLOR_WHITE, RIGHT_BASELINE);
			MainFont->DrawText(MonsterIdentifier, X + 5, Y);

			Y += 15;
			MainFont->DrawText("Particle:", X, Y, COLOR_WHITE, RIGHT_BASELINE);
			MainFont->DrawText(ParticleIdentifier, X + 5, Y);

			Y += 15;
			Buffer << Level << ":" << ActivationPeriod;
			MainFont->DrawText("Level:", X, Y, COLOR_WHITE, RIGHT_BASELINE);
			MainFont->DrawText(Buffer.str(), X + 5, Y);
			Buffer.str("");
		} break;
		default:

			// See if there's a selected object
			if(SelectedObjects.size() > 0) {
				auto Iterator = SelectedObjects.begin();
				IconIdentifier = (*Iterator)->Identifier;
				IconText = "";
				IconTexture = nullptr;
			}
		break;
	}

	// Bottom information box
	if(IconText != "")
		MainFont->DrawText(IconText, (float)Graphics.GetViewportWidth() + 112, (float)Graphics.GetViewportHeight() + 130, COLOR_WHITE, CENTER_MIDDLE);

	if(IconIdentifier != "")
		MainFont->DrawText(IconIdentifier, (float)Graphics.GetViewportWidth() + 112, (float)Graphics.GetViewportHeight() + 145, COLOR_WHITE, CENTER_MIDDLE);

	if(IconTexture) {
		Graphics.EnableVBO(VBO_QUAD);
		Graphics.DrawTexture((float)Graphics.GetScreenWidth() - 112, (float)Graphics.GetScreenHeight() - 84, 0.0f, IconTexture, IconColor, IconRotation, IconScaleX * EDITOR_PALETTE_SELECTEDSIZE * 2, EDITOR_PALETTE_SELECTEDSIZE * 2);
	}
}

// Draws an object
void _EditorState::DrawObject(float OffsetX, float OffsetY, const _ObjectSpawn *Object, float Alpha) {
	float Scale = ITEM_SCALE;
	float Depth = ITEM_Z;
	_Color Color;
	_Texture *Texture = nullptr;
	switch(Object->Type) {
		case _Object::MONSTER: {
			_MonsterTemplate *Monster = Assets.GetMonsterTemplate(Object->Identifier);
			Texture = Assets.GetAnimation(Monster->AnimationIdentifier)->GetStartPositionFrame();
			Color = Monster->Color;
			Scale = Monster->Scale;
			Depth = OBJECT_Z;
		} break;
		case _Object::MISCITEM: {
			_MiscItemTemplate *MiscItem = Assets.GetMiscItemTemplate(Object->Identifier);
			Texture = Assets.GetTexture(MiscItem->IconIdentifier);
			Color = MiscItem->Color;
		} break;
		case _Object::AMMO: {
			_AmmoTemplate *Ammo = Assets.GetAmmoTemplate(Object->Identifier);
			Texture = Assets.GetTexture(Ammo->IconIdentifier);
			Color = Ammo->Color;
		} break;
		case _Object::UPGRADE: {
			_UpgradeTemplate *Upgrade = Assets.GetUpgradeTemplate(Object->Identifier);
			Texture = Assets.GetTexture(Upgrade->IconIdentifier);
			Color = Upgrade->Color;
		} break;
		case _Object::WEAPON: {
			_WeaponTemplate *Weapon = Assets.GetWeaponTemplate(Object->Identifier);
			Texture = Assets.GetTexture(Weapon->IconIdentifier);
			Color = Weapon->Color;
		} break;
		case _Object::ARMOR: {
			_ArmorTemplate *Armor = Assets.GetArmorTemplate(Object->Identifier);
			Texture = Assets.GetTexture(Armor->IconIdentifier);
			Color = Armor->Color;
		} break;
	}

	Vector2 DrawPosition(Object->Position[0] + OffsetX, Object->Position[1] + OffsetY);
	if(!Camera->IsCircleInView(DrawPosition, Scale)) {
		return;
	}

	Color.Alpha *= Alpha;
	if(Texture != nullptr)
		Graphics.DrawTexture(DrawPosition[0], DrawPosition[1], Depth, Texture, Color, 0.0f, Scale, Scale);
}

// Converts an editor mode to an object type
int _EditorState::StateToType(int State) {

	switch(State) {
		case EDITMODE_MONSTERS:
			return _Object::MONSTER;
		break;
		case EDITMODE_ITEMS:
			return _Object::MISCITEM;
		break;
		case EDITMODE_AMMO:
			return _Object::AMMO;
		break;
		case EDITMODE_UPGRADES:
			return _Object::UPGRADE;
		break;
		case EDITMODE_WEAPONS:
			return _Object::WEAPON;
		break;
		case EDITMODE_ARMOR:
			return _Object::ARMOR;
		break;
	}

	return _Object::MISCITEM;
}

// Processes clicks on the buttons
void _EditorState::ProcessIcons(int Index, int Type) {

	switch(Index) {
		case ICON_LAYER1:
			ExecuteUpdateLayer(0, false);
		break;
		case ICON_LAYER2:
			ExecuteUpdateLayer(1, false);
		break;
		case ICON_LAYER3:
			ExecuteUpdateLayer(2, false);
		break;
		case ICON_LAYER4:
			ExecuteUpdateLayer(3, false);
		break;
		case ICON_LAYER5:
			ExecuteUpdateLayer(4, false);
		break;
		case ICON_WALL:
			ExecuteUpdateLayer(5, false);
		break;
		case ICON_FORE:
			ExecuteUpdateLayer(6, false);
		break;
		case ICON_DOWN:
			ExecuteShiftLayer(-1);
		break;
		case ICON_UP:
			ExecuteShiftLayer(1);
		break;
		case ICON_BLOCK:
			ExecuteSwitchMode(EDITMODE_BLOCKS);
		break;
		case ICON_EVENT:
			ExecuteSwitchMode(EDITMODE_EVENTS);
		break;
		case ICON_ITEM:
			ExecuteSwitchMode(EDITMODE_ITEMS);
		break;
		case ICON_AMMO:
			ExecuteSwitchMode(EDITMODE_AMMO);
		break;
		case ICON_UPGRADE:
			ExecuteSwitchMode(EDITMODE_UPGRADES);
		break;
		case ICON_MONSTER:
			ExecuteSwitchMode(EDITMODE_MONSTERS);
		break;
		case ICON_WEAPON:
			ExecuteSwitchMode(EDITMODE_WEAPONS);
		break;
		case ICON_ARMOR:
			ExecuteSwitchMode(EDITMODE_ARMOR);
		break;
		case ICON_NONE:
			ExecuteDeselect();
		break;
		case ICON_DELETE:
			ExecuteDelete();
		break;
		case ICON_COPY:
			ExecuteCopy();
		break;
		case ICON_PASTE:
			ExecutePaste(false);
		break;
		case ICON_SHOW:
			ExecuteHighlightBlocks();
		break;
		case ICON_UNDO:
			ExecuteUndo();
		break;
		case ICON_CLEAR:
			ExecuteClear();
		break;
		case ICON_GRID:
			if(IsShiftDown)
				ExecuteUpdateGridMode(-1);
			else
				ExecuteUpdateGridMode(1);
		break;
		case ICON_MSET:
			ExecuteIOCommand(EDITINPUT_LOADMONSTERSET);
		break;
		case ICON_LOAD:
			ExecuteIOCommand(EDITINPUT_LOAD);
		break;
		case ICON_SAVE:
			ExecuteIOCommand(EDITINPUT_SAVE);
		break;
		case ICON_TEST:
			ExecuteTest();
		break;
	}
}

// Processes clicks on the block buttons
void _EditorState::ProcessBlockIcons(int Index, int Type) {

	switch(Index) {
		case ICON_WALK:
			ExecuteWalkable();
		break;
		case ICON_ROTATE:
			ExecuteRotate();
		break;
		case ICON_MIRROR:
			ExecuteMirror();
		break;
		case ICON_RAISE:
			ExecuteChangeZ(0.5f, !IsShiftDown);
		break;
		case ICON_LOWER:
			ExecuteChangeZ(-0.5f, !IsShiftDown);
		break;
	}
}

// Processes clicks on the event buttons
void _EditorState::ProcessEventIcons(int Index, int Type) {

	switch(Index) {
		case ICON_TILE:
		break;
		case ICON_LEVELUP:
			if(Type)
				ExecuteChangeLevel(5);
			else
				ExecuteChangeLevel(1);
		break;
		case ICON_LEVELDOWN:
			if(Type)
				ExecuteChangeLevel(-5);
			else
				ExecuteChangeLevel(-1);
		break;
		case ICON_PERIODUP:
			if(Type)
				ExecuteChangePeriod(EDITOR_PERIODADJUST);
			else
				ExecuteChangePeriod(EDITOR_PERIODADJUST * 10);
		break;
		case ICON_PERIODDOWN:
			if(Type)
				ExecuteChangePeriod(-EDITOR_PERIODADJUST);
			else
				ExecuteChangePeriod(-EDITOR_PERIODADJUST * 10);
		break;
		case ICON_ACTIVE:
			ExecuteChangeActive();
		break;
		case ICON_ITEMIDENTIFIER:
			ExecuteIOCommand(EDITINPUT_ITEMIDENTIFIER);
		break;
		case ICON_MONSTERIDENTIFIER:
			ExecuteIOCommand(EDITINPUT_MONSTERIDENTIFIER);
		break;
		case ICON_PARTICLEIDENTIFIER:
			ExecuteIOCommand(EDITINPUT_PARTICLEIDENTIFIER);
		break;
	}
}

// Adds an object to the list
void _EditorState::SpawnObject(const Vector2 &Position, int Type, const std::string &Identifier, bool Align) {
	Vector2 SpawnPosition;

	if(Align)
		SpawnPosition = AlignToGrid(Position);
	else
		SpawnPosition = Position;

	_ObjectSpawn *Object = new _ObjectSpawn(Identifier, SpawnPosition, Type);
	Map->AddObject(Object);
}

// Adds an event to the list
void _EditorState::AddEvent(int Type) {

	// Get object identifiers
	std::string ItemIdentifier = SavedText[EDITINPUT_ITEMIDENTIFIER];
	std::string MonsterIdentifier = SavedText[EDITINPUT_MONSTERIDENTIFIER];
	std::string ParticleIdentifier = SavedText[EDITINPUT_PARTICLEIDENTIFIER];

	// Get selected object's identifier
	if(ObjectsSelected()) {
		_ObjectSpawn *SelectedObject = *SelectedObjects.begin();
		switch(SelectedObject->Type) {
			case _Object::MONSTER:
				MonsterIdentifier = SelectedObject->Identifier;
			break;
			default:
				ItemIdentifier = SelectedObject->Identifier;
			break;
		}
	}

	int TileLayer = -1;
	_Coord Start = DrawStart;
	_Coord End = DrawEnd - 1;
	std::string EventItemIdentifier = "";
	std::string EventMonsterIdentifier = "";
	std::string EventParticleIdentifier = "";

	// Setup the event
	switch(Type) {
		case EVENT_DOOR:
			EventItemIdentifier = ItemIdentifier;
			TileLayer = MAPLAYER_FLAT;
		break;
		case EVENT_WSWITCH:
			EventItemIdentifier = ItemIdentifier;
			TileLayer = CurrentLayer;
		break;
		case EVENT_SPAWN:
			EventMonsterIdentifier = MonsterIdentifier;
			EventParticleIdentifier = ParticleIdentifier;
			TileLayer = CurrentLayer;
		break;
		default:
		break;
	}

	if(Type != -1) {
		_Event *Event = new _Event(Type, EventActive, Start, End, EventLevel, EventActivationPeriod, EventItemIdentifier, EventMonsterIdentifier, EventParticleIdentifier);
		if(TileLayer != -1) {
			int BlockIndex = Map->GetSelectedBlock(TileLayer, Start);
			Event->AddTile(_EventTile(Start, TileLayer, BlockIndex));
		}

		Map->AddEvent(Event);
	}
}

// Updates the selected event's object identifier
void _EditorState::UpdateEventIdentifier(int Type, const std::string &Identifier) {

	if(EventSelected()) {
		switch(Type) {
			case EDITINPUT_ITEMIDENTIFIER:
				SelectedEvent->SetItemIdentifier(Identifier);
			break;
			case EDITINPUT_MONSTERIDENTIFIER:
				SelectedEvent->SetMonsterIdentifier(Identifier);
			break;
			case EDITINPUT_PARTICLEIDENTIFIER:
				SelectedEvent->SetParticleIdentifier(Identifier);
			break;
		}
	}
}

// Gets the selected event's object identifier
std::string _EditorState::GetEventIdentifier(int Type) {

	if(EventSelected()) {
		switch(Type) {
			case EDITINPUT_ITEMIDENTIFIER:
				return SelectedEvent->GetItemIdentifier();
			break;
			case EDITINPUT_MONSTERIDENTIFIER:
				return SelectedEvent->GetMonsterIdentifier();
			break;
			case EDITINPUT_PARTICLEIDENTIFIER:
				return SelectedEvent->GetParticleIdentifier();
			break;
		}
	}

	return "";
}

// Returns a valid position for the object
Vector2 _EditorState::GetValidObjectPosition(const Vector2 &Position) const {
	Vector2 NewPosition;

	if(Position[0] < 0)
		NewPosition[0] = 0;
	else if(Position[0] >= Map->GetWidth())
		NewPosition[0] = (float)Map->GetWidth();
	else
		NewPosition[0] = Position[0];

	if(Position[1] < 0)
		NewPosition[1] = 0;
	else if(Position[1] >= Map->GetHeight())
		NewPosition[1] = (float)Map->GetHeight();
	else
		NewPosition[1] = Position[1];

	return NewPosition;
}

// Determines if an object is part of the selected objects list
bool _EditorState::ObjectInSelectedList(_ObjectSpawn *Object) {

	for(auto Iterator : SelectedObjects) {
		if(Object == Iterator)
			return true;
	}

	return false;
}

// Executes the walkable command
void _EditorState::ExecuteWalkable() {
	if(BlockSelected())
		SelectedBlock->Walkable = !SelectedBlock->Walkable;
	else
		Walkable = !Walkable;
}

// Executes the rotate command
void _EditorState::ExecuteRotate() {
	if(BlockSelected()) {
		SelectedBlock->Rotation += 90;
		if(SelectedBlock->Rotation > 359)
			SelectedBlock->Rotation = 0;
	}
	else {
		Rotation += 90;
		if(Rotation > 359)
			Rotation = 0;
	}
}

// Executes the mirror texture command
void _EditorState::ExecuteMirror() {
	if(BlockSelected())
		SelectedBlock->ScaleX = -SelectedBlock->ScaleX;
	else
		ScaleX = -ScaleX;
}

// Executes the mirror texture command
void _EditorState::ExecuteToggleTile() {

	if(EventSelected()) {
		auto Iterator = SelectedEvent->FindTile(WorldCursorIndex.X, WorldCursorIndex.Y);
		if(Iterator != SelectedEvent->GetTiles().end()) {
			SelectedEvent->RemoveTile(Iterator);
		}
		else {
			switch(SelectedEvent->GetType()) {
				case EVENT_DOOR:
				case EVENT_WSWITCH:
				case EVENT_FSWITCH: {
					_Block *Block;
					int BlockIndex = Map->GetSelectedBlock(CurrentLayer, WorldCursorIndex, &Block);
					SelectedEvent->AddTile(_EventTile(WorldCursorIndex, CurrentLayer, BlockIndex));
				} break;
				case EVENT_ENABLE: {
					_Event *Event;
					int EventIndex = Map->GetSelectedEvent(WorldCursorIndex, &Event);
					SelectedEvent->AddTile(_EventTile(WorldCursorIndex, -1, EventIndex));
				} break;
				default:
					SelectedEvent->AddTile(_EventTile(WorldCursorIndex, -1, -1));
				break;
			}
		}
	}
}

// Executes the undo command
void _EditorState::ExecuteChangeZ(float Change, int Type) {
	if(Type == 0) {
		if(BlockSelected())
			SelectedBlock->MinZ += Change;
		else
			MinZ += Change;
	}
	else {
		if(BlockSelected())
			SelectedBlock->MaxZ += Change;
		else
			MaxZ += Change;
	}
}

// Executes the change level command
void _EditorState::ExecuteChangeLevel(int Change) {
	if(EventSelected()) {
		SelectedEvent->SetLevel(SelectedEvent->GetLevel() + Change);
		if(SelectedEvent->GetLevel() < 0)
			SelectedEvent->SetLevel(0);

	}
	else {
		EventLevel += Change;
		if(EventLevel < 0)
			EventLevel = 0;
	}
}

// Executes the change activation period command
void _EditorState::ExecuteChangePeriod(double Value) {

	if(EventSelected()) {
		SelectedEvent->SetActivationPeriod(SelectedEvent->GetActivationPeriod() + Value);
		if(SelectedEvent->GetActivationPeriod() < 0.0)
			SelectedEvent->SetActivationPeriod(0.0);

	}
	else {
		EventActivationPeriod += Value;
		if(EventActivationPeriod < 0.0)
			EventActivationPeriod = 0.0;
	}
}

// Executes the change active command
void _EditorState::ExecuteChangeActive() {

	if(EventSelected())
		SelectedEvent->SetActive(!SelectedEvent->GetActive());
	else
		EventActive = !EventActive;
}

// Executes the change checkpoint command
void _EditorState::ExecuteUpdateCheckpointIndex(int Value) {
	CheckpointIndex += Value;

	if(CheckpointIndex < 0)
		CheckpointIndex = 0;
}

// Executes the an I/O command
void _EditorState::ExecuteIOCommand(int Type) {
	EditorInput = Type;
	InputBox->SetFocused(true);
	_Label *Label = (_Label *)InputBox->GetChildren()[0];
	Label->SetText(InputBoxStrings[Type]);
	if(Type >= EDITINPUT_ITEMIDENTIFIER && Type <= EDITINPUT_PARTICLEIDENTIFIER && EventSelected())
		InputBox->SetText(GetEventIdentifier(Type));
	else
		InputBox->SetText(SavedText[Type]);
}

// Executes the clear map command
void _EditorState::ExecuteClear() {
	LoadMap("", false);
	SavedText[EDITINPUT_SAVE] = "";
}

// Executes the test command
void _EditorState::ExecuteTest() {

	// TODO catch exception
	Map->SaveLevel(EDITOR_TESTLEVEL);

	ExecuteDeselect();
	ClearClipboard();

	PlayState.SetTestMode(true);
	PlayState.SetFromEditor(true);
	PlayState.SetLevel(EDITOR_TESTLEVEL);
	PlayState.SetCheckpointIndex(CheckpointIndex);
	Framework.ChangeState(&PlayState);
}

// Executes the delete command
void _EditorState::ExecuteDelete() {

	switch(CurrentPalette) {
		case EDITMODE_BLOCKS:
			if(BlockSelected()) {
				Map->RemoveBlock(CurrentLayer, SelectedBlockIndex);
				DeselectBlock();
			}
		break;
		case EDITMODE_EVENTS:
			if(EventSelected()) {
				Map->RemoveEvent(SelectedEventIndex);
				DeselectEvent();
				ClipboardEvent = nullptr;
			}
		break;
		default:
			if(ObjectsSelected()) {
				Map->RemoveObjects(SelectedObjectIndices);
				DeselectObjects();
				ClipboardObjects.clear();
			}
		break;
	}
}

// Executes the copy command
void _EditorState::ExecuteCopy() {

	switch(CurrentPalette) {
		case EDITMODE_BLOCKS:
			if(BlockSelected()) {
				ClipboardBlock = *SelectedBlock;
				DeselectBlock();
				BlockCopied = true;
			}
		break;
		case EDITMODE_EVENTS:
			if(EventSelected()) {
				ClipboardEvent = SelectedEvent;
			}
		break;
		default:
			if(ObjectsSelected()) {
				CopiedPosition = WorldCursor;
				ClipboardObjects = SelectedObjects;
			}
		break;
	}
}

// Executes the paste command
void _EditorState::ExecutePaste(bool Viewport) {
	Vector2 StartPosition;

	if(Viewport)
		StartPosition = WorldCursor;
	else
		StartPosition = Camera->GetPosition();

	switch(CurrentPalette) {
		case EDITMODE_BLOCKS:
			if(BlockCopied) {
				int Width = ClipboardBlock.End.X - ClipboardBlock.Start.X;
				int Height = ClipboardBlock.End.Y - ClipboardBlock.Start.Y;
				ClipboardBlock.Start = Map->GetValidCoord(_Coord(StartPosition));
				ClipboardBlock.End = Map->GetValidCoord(_Coord(StartPosition.X + Width, StartPosition.Y + Height));

				UndoNumber[CurrentLayer]++;
				Map->AddBlock(CurrentLayer, ClipboardBlock);
			}
		break;
		case EDITMODE_EVENTS:
			if(ClipboardEvent != nullptr) {
				DrawStart = Map->GetValidCoord(StartPosition);
				DrawEnd = Map->GetValidCoord(ClipboardEvent->GetEnd() - ClipboardEvent->GetStart() + StartPosition);

				_Event *Event = new _Event(ClipboardEvent->GetType(), ClipboardEvent->GetActive(), DrawStart, DrawEnd,
										ClipboardEvent->GetLevel(), ClipboardEvent->GetActivationPeriod(), ClipboardEvent->GetItemIdentifier(),
										ClipboardEvent->GetMonsterIdentifier(), ClipboardEvent->GetParticleIdentifier());

				Map->AddEvent(Event);
			}
		break;
		default:
			for(auto Iterator : ClipboardObjects) {
				SpawnObject(GetValidObjectPosition(StartPosition - CopiedPosition + Iterator->Position), Iterator->Type, Iterator->Identifier, IsShiftDown);
			}
		break;
	}
}

// Executes the deselect command
void _EditorState::ExecuteDeselect() {
	DeselectBlock();
	DeselectEvent();
	DeselectObjects();
}

// Executes the undo command
void _EditorState::ExecuteUndo() {
	switch(CurrentPalette) {
		case EDITMODE_BLOCKS:
			if(UndoNumber[CurrentLayer] > 0) {
				Map->RemoveLastBlock(CurrentLayer);
				DeselectBlock();
				UndoNumber[CurrentLayer]--;
			}
		break;
	}
}

// Executes the update selected palette command
void _EditorState::ExecuteUpdateSelectedPalette(int Change) {
	std::vector<_Element *> &Children = PaletteElement[CurrentPalette]->GetChildren();
	if(!Brush[CurrentPalette]) {
		Brush[CurrentPalette] = (_Button *)Children[0];
		return;
	}

	int CurrentIndex = Brush[CurrentPalette]->GetID();
	CurrentIndex += Change;
	if(CurrentIndex >= (int)Children.size())
		CurrentIndex = 0;
	else if(CurrentIndex < 0)
		CurrentIndex = Children.size() - 1;

	ExecuteSelectPalette((_Button *)Children[CurrentIndex], 0);
}

// Executes the select palette command
void _EditorState::ExecuteSelectPalette(_Button *Button, int ClickType) {
	if(!Button)
		return;

	if(Button->GetID() == -1) {

		// Deselect alternate texture
		if(ClickType == 1 && CurrentPalette == EDITMODE_BLOCKS) {
			if(BlockSelected()) {
				SelectedBlock->AltTextureIdentifier = "";
				SelectedBlock->AltTexture = nullptr;
			}
			else {
				AltTextureIdentifier = "";
				AltTexture = nullptr;
			}
		}

		return;
	}

	switch(CurrentPalette) {
		case EDITMODE_BLOCKS:
			if(!Button)
				return;

			if(ClickType == 1) {
				if(BlockSelected()) {
					SelectedBlock->AltTextureIdentifier = Button->GetIdentifier();
					SelectedBlock->AltTexture = Button->GetStyle()->GetTexture();
				}
				else {
					AltTextureIdentifier = Button->GetIdentifier();
					AltTexture = Button->GetStyle()->GetTexture();
				}
			}
			else {
				if(BlockSelected()) {
					SelectedBlock->TextureIdentifier = Button->GetIdentifier();
					SelectedBlock->Texture = Button->GetStyle()->GetTexture();
				}
			}
		break;
		case EDITMODE_EVENTS:
			if(!EventSelected()) {
				switch(Button->GetID()) {
					case EVENT_SPAWN:
						SetEventProperties(0, 1, 1, "smoke0");
					break;
					case EVENT_TEXT:
						SetEventProperties(5, 1, 1, "");
					break;
					case EVENT_SOUND:
					case EVENT_FSWITCH:
					case EVENT_ENABLE:
						SetEventProperties(0, 1, 1, "");
					break;
					case EVENT_TELE:
						SetEventProperties(0, 0, 1, "smoke0");
					break;
					case EVENT_LIGHT:
						SetEventProperties(1, 100, 1, "smoke0");
					break;
					default:
						SetEventProperties(0, 0, 1, "");
					break;
				}
			}
		break;
		default:
			if(ClickType == 1 && EventSelected()) {
				switch(CurrentPalette) {
					case EDITMODE_MONSTERS:
						SelectedEvent->SetMonsterIdentifier(Button->GetIdentifier());
						ExecuteSwitchMode(EDITMODE_EVENTS);
					break;
					case EDITMODE_ITEMS:
						SelectedEvent->SetItemIdentifier(Button->GetIdentifier());
						ExecuteSwitchMode(EDITMODE_EVENTS);
					break;
					default:
					break;
				}
			}
		break;
	}

	if(ClickType == 0)
		Brush[CurrentPalette] = Button;
}

// Executes the update grid command
void _EditorState::ExecuteUpdateGridMode(int Change) {
	GridMode += Change;
	if(GridMode > 10)
		GridMode = 0;
	else if(GridMode < 0)
		GridMode = 10;
}

// Executes the highlight command
void _EditorState::ExecuteHighlightBlocks() {
	HighlightBlocks = !HighlightBlocks;

	Assets.GetButton("editor_show")->SetEnabled(HighlightBlocks);
}

// Executes the toggle editor mode
void _EditorState::ExecuteSwitchMode(int State) {

	// Toggle icons
	if(CurrentPalette != State) {
		ModeButtons[CurrentPalette]->SetEnabled(false);
		ModeButtons[State]->SetEnabled(true);

		// Set state
		CurrentPalette = State;
	}
}

// Executes the update layer command
void _EditorState::ExecuteUpdateLayer(int Layer, bool Move) {

	if(CurrentLayer != Layer) {
		if(Move && BlockSelected()) {
			Map->ChangeLayer(CurrentLayer, Layer, SelectedBlockIndex);
			SelectedBlockIndex = Map->GetLastBlock(Layer, &SelectedBlock);

			// Change block properties
			if(Layer == MAPLAYER_WALL) {
				SelectedBlock->Wall = true;
				SelectedBlock->Walkable = false;
			}
			else {
				SelectedBlock->Wall = false;
				SelectedBlock->Walkable = true;
			}
		}
		else
			DeselectBlock();

		if(Layer == MAPLAYER_FLAT) {
			MaxZ = MAP_FLATZ;
			Walkable = false;
		}
		else if(Layer == MAPLAYER_WALL) {
			MaxZ = MAP_WALLZ;
			Walkable = false;
		}
		else if(Layer == MAPLAYER_FORE) {
			MaxZ = MAP_FOREGROUNDZ;
			Walkable = true;
		}
		else {
			MaxZ = 0.0f;
			Walkable = true;
		}
		MinZ = MAP_MINZ;

		// Toggle icons
		LayerButtons[CurrentLayer]->SetEnabled(false);
		LayerButtons[Layer]->SetEnabled(true);
		CurrentLayer = Layer;
	}
}

// Executes the shift layer command
void _EditorState::ExecuteShiftLayer(int Change) {

	// Shift layers
	int NewLayer = CurrentLayer + Change;
	if(NewLayer > MAPLAYER_COUNT - 1)
		NewLayer = MAPLAYER_COUNT - 1;
	else if(NewLayer < 0)
		NewLayer = 0;

	ExecuteUpdateLayer(NewLayer, true);
}

// Executes the update block limit command
void _EditorState::ExecuteUpdateBlockLimits(int Direction, bool Expand) {
	_Coord Start, End;
	bool Change = false;
	if(CurrentPalette == EDITMODE_BLOCKS && BlockSelected()) {
		Start = SelectedBlock->Start;
		End = SelectedBlock->End;
		Change = true;
	}
	else if(CurrentPalette == EDITMODE_EVENTS && EventSelected()) {
		Start = SelectedEvent->GetStart();
		End = SelectedEvent->GetEnd();
		Change = true;
	}

	if(Change) {
		switch(Direction) {
			case 0:
				if(Expand)
					Start.X--;
				else
					End.X--;
			break;
			case 1:
				if(Expand)
					Start.Y--;
				else
					End.Y--;
			break;
			case 2:
				if(Expand)
					End.X++;
				else
					Start.X++;
			break;
			case 3:
				if(Expand)
					End.Y++;
				else
					Start.Y++;
			break;
		}

		// Check limits
		if(Start.X > End.X)
			Start.X = End.X;

		if(Start.Y > End.Y)
			Start.Y = End.Y;

		if(End.X < Start.X)
			End.X = Start.X;

		if(End.Y < Start.Y)
			End.Y = Start.Y;

		if(CurrentPalette == EDITMODE_BLOCKS && BlockSelected()) {
			SelectedBlock->Start = Map->GetValidCoord(Start);
			SelectedBlock->End = Map->GetValidCoord(End);
		}
		else if(CurrentPalette == EDITMODE_EVENTS && EventSelected()) {
			SelectedEvent->SetStart(Map->GetValidCoord(Start));
			SelectedEvent->SetEnd(Map->GetValidCoord(End));
		}
	}
}

// Selects an object
void _EditorState::SelectObject() {
	ClickedPosition = WorldCursor;

	_ObjectSpawn *SelectedObject;
	size_t Index;
	Map->GetSelectedObject(WorldCursor, EDITOR_OBJECTRADIUS * EDITOR_OBJECTRADIUS, &SelectedObject, &Index);
	if(SelectedObject != nullptr) {
		IsMoving = true;

		// Single object selected
		if(!ObjectInSelectedList(SelectedObject)) {
			DeselectObjects();
			SelectedObjects.push_back(SelectedObject);
			SelectedObjectIndices.push_back(Index);
			if(EventSelected()) {
				switch(SelectedObject->Type) {
					case _Object::MONSTER:
						UpdateEventIdentifier(EDITINPUT_MONSTERIDENTIFIER, SelectedObject->Identifier);
					break;
					default:
						UpdateEventIdentifier(EDITINPUT_ITEMIDENTIFIER, SelectedObject->Identifier);
					break;
				}
			}
		}
	}
	else {
		DeselectObjects();
		DraggingBox = true;
	}
}

// Selects objects
void _EditorState::SelectObjects() {
	DeselectObjects();
	Map->GetSelectedObjects(ClickedPosition, WorldCursor, &SelectedObjects, &SelectedObjectIndices);
}

// Aligns an object to the grid
Vector2 _EditorState::AlignToGrid(const Vector2 &Position) const {

	return Vector2((int)Position.X + 0.5f, (int)Position.Y + 0.5f);
}

// Get tentative position
Vector2 _EditorState::GetMoveDeltaPosition(const Vector2 &Position) {
	Vector2 NewPosition;
	if(IsShiftDown)
		NewPosition = AlignToGrid(GetValidObjectPosition(Position + MoveDelta));
	else
		NewPosition = GetValidObjectPosition(Position + MoveDelta);

	return NewPosition;
}

// Sets event properties
void _EditorState::SetEventProperties(double ActivationPeriod, int Level, int Active, const std::string &ParticleIdentifier) {
	EventActivationPeriod = ActivationPeriod;
	EventLevel = Level;
	EventActive = Active;
	SavedText[EDITINPUT_PARTICLEIDENTIFIER] = ParticleIdentifier;
}

// Clears all the objects in the clipboard
void _EditorState::ClearClipboard() {
	BlockCopied = false;
	ClipboardEvent = nullptr;
	ClipboardObjects.clear();
}

void _EditorState::DeselectObjects() {
	SelectedObjects.clear();
	SelectedObjectIndices.clear();
}

bool _EditorState::ObjectsSelected() {
	return SelectedObjects.size() != 0;
}
