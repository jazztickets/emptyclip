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

// Includes
#include <string>
#include <vector2.h>
#include <SDL_keycode.h>

//     Config
const  int          DEFAULT_WINDOW_WIDTH           =  1440;
const  int          DEFAULT_WINDOW_HEIGHT          =  900;
const  int          DEFAULT_FULLSCREEN             =  1;
const  int          DEFAULT_AUDIOENABLED           =  1;
const  int          DEFAULT_VSYNC                  =  1;
const  double       DEFAULT_MAXFPS                 =  180.0;
const  int          DEFAULT_KEYUP                  =  SDL_SCANCODE_E;
const  int          DEFAULT_KEYDOWN                =  SDL_SCANCODE_D;
const  int          DEFAULT_KEYLEFT                =  SDL_SCANCODE_S;
const  int          DEFAULT_KEYRIGHT               =  SDL_SCANCODE_F;
const  int          DEFAULT_KEYSPRINT              =  SDL_SCANCODE_A;
const  int          DEFAULT_KEYUSE                 =  SDL_SCANCODE_SPACE;
const  int          DEFAULT_KEYINVENTORY           =  SDL_SCANCODE_C;
const  int          DEFAULT_BUTTONFIRE             =  1;
const  int          DEFAULT_BUTTONAIM              =  3;
const  int          DEFAULT_KEYRELOAD              =  SDL_SCANCODE_R;
const  int          DEFAULT_KEYWEAPONSWITCH        =  SDL_SCANCODE_W;
const  int          DEFAULT_KEYMEDKIT              =  SDL_SCANCODE_Q;
const  int          DEFAULT_KEYWEAPON1             =  SDL_SCANCODE_1;
const  int          DEFAULT_KEYWEAPON2             =  SDL_SCANCODE_2;
const  int          DEFAULT_KEYWEAPON3             =  SDL_SCANCODE_3;
const  int          DEFAULT_KEYWEAPON4             =  SDL_SCANCODE_4;
//     Game
const  std::string  GAME_WINDOWTITLE               =  "Empty Clip";
const  double       GAME_FPS                       =  60.0;
const  double       GAME_TIMESTEP                  =  1.0/GAME_FPS;
const  float        GAME_PAUSE_FADEAMOUNT          =  0.7f;
const  std::string  GAME_STARTLEVEL                =  "start0.map";
const  std::string  GAME_FIRSTLEVEL                =  "mansion0.map";
const  std::string  GAME_TUTORIALLEVEL             =  "tutorial0.map";
const  int          GAME_MAX_LEVEL                 =  100;
const  int          GAME_SKILLLEVELS               =  20;
//     Camera
const  float        CAMERA_DISTANCE                =  6.5f;
const  float        CAMERA_DISTANCE_AIMED          =  7.0f;
const  float        CAMERA_DIVISOR                 =  20.0f;
const  float        CAMERA_EDITOR_DIVISOR          =  5.0f;
const  float        CAMERA_FOVY                    =  90.0f;
const  float        CAMERA_NEAR                    =  0.1f;
const  float        CAMERA_FAR                     =  500.0f;
//     Graphics
const  int          GRAPHICS_CIRCLE_VERTICES       =  32;
//     Weapons
const  double       WEAPON_MINFIREPERIOD           =  0.017;
//     Audio
const  float        MAX_AUDIO_DISTANCE             =  30.0f;
const  float        MAX_AUDIO_DISTANCE_SQUARED     =  MAX_AUDIO_DISTANCE*MAX_AUDIO_DISTANCE;
//     Entities
const  float        ENTITY_MOVESOUNDDELAYFACTOR    =  0.02625f;
const  int          ENTITY_MINDAMAGEPOINTS         =  1;
//     Player
const  int          PLAYER_SAVEVERSION             =  1;
const  std::string  PLAYER_DEFAULTNAME             =  "Jackson";
const  float        PLAYER_RADIUS                  =  0.35f;
const  double       PLAYER_MEDKITPERIOD            =  0.5;
const  float        PLAYER_LEGCHANGEFACTOR         =  5.0f;
const  float        PLAYER_MOVEMENTSPEED           =  0.075f;
const  float        PLAYER_BACKWARDSPEED           =  0.5f;
const  float        PLAYER_CROUCHINGSPEEDFACTOR    =  0.333333f;
const  float        PLAYER_SPRINTINGSPEEDFACTOR    =  1.65f;
const  Vector2      PLAYER_PISTOLOFFSET            =  Vector2(36.0f/64.0f-0.5f,-0.5f);
const  Vector2      PLAYER_WEAPONOFFSET            =  Vector2(30.0f/64.0f-0.5f,-0.5f);
const  float        PLAYER_RECOILSKILLFACTOR       =  0.5f;
const  float        PLAYER_RECOILREGENSKILLFACTOR  =  1.0f;
const  float        PLAYER_STAMINAREGEN            =  0.001f;
const  float        PLAYER_SPRINTSTAMINA           =  0.005f;
const  float        PLAYER_TIREDTHRESHOLD          =  0.3f;
const  float        PLAYER_ZOOMSCALE               =  15.0f;
const  float        PLAYER_MAXACCURACY             =  170.0f;
const  double       PLAYER_WEAPONSWITCHPERIOD      =  0.5;
const  double       PLAYER_USEPERIOD               =  0.2;
const  int          INVENTORY_BAGSIZE              =  16;
//     Items
const  float        ITEM_SCALE                     =  0.5f;
const  float        ITEM_Z                         =  0.05f;
const  int          ITEM_MEDKIT_HEALTH             =  50;
//     Objects
const  float        OBJECT_Z                       =  0.3f;
//     Map
const  int          MAP_FILEVERSION                =  1;
const  std::string  MAP_DEFAULTMONSTERSET          =  "tutorial0";
const  float        MAP_MINZ                       =  0.0f;
const  float        MAP_FLATZ                      =  1.0f;
const  float        MAP_WALLZ                      =  2.0f;
const  float        MAP_FOREGROUNDZ                =  3.0f;
const  float        MAP_LAYEROFFSET                =  0.01f;
const  int          MAP_WIDTH                      =  100;
const  int          MAP_HEIGHT                     =  100;
const  float        MAP_EPSILON                    =  0.0001f;
//     Editor
const  std::string  EDITOR_TESTLEVEL               =  "test.map";
const  float        EDITOR_OBJECTRADIUS            =  0.4f;
const  double       EDITOR_PERIODADJUST            =  0.1;
const  int          EDITOR_DEFAULT_LAYER           =  1;
const  int          EDITOR_DEFAULT_GRIDMODE        =  5;
const  int          EDITOR_VIEWPORT_OFFSETX        =  224;
const  int          EDITOR_VIEWPORT_OFFSETY        =  168;
const  int          EDITOR_PALETTE_SELECTEDSIZE    =  32;
const  int          EDITOR_WALL_LAYER              =  5;
//     Menu
const  float        MENU_ACCEPTINPUT_FADE          =  0.7f;
const  double       MENU_DOUBLECLICK_TIME          =  0.250;
const  double       MENU_CURSOR_PERIOD             =  0.5;
//     HUD
const  double       HUD_ENTITYHEALTHDISPLAYPERIOD  =  5.0;
const  double       HUD_CURSOR_ITEM_WAIT           =  0.5;
const  float        HUD_CROSSHAIRDIVISOR           =  5.0f;
const  float        HUD_MINCROSSHAIRSCALE          =  0.0f;
const  double       HUD_KEYMESSAGETIME             =  3.0;
const  double       HUD_CHECKPOINTTIME             =  5.0;
const  std::string  HUD_CHECKPOINTMESSAGE          =  "CHECKPOINT REACHED";
const  double       HUD_INVENTORYFULLTIME          =  2.0;
const  std::string  HUD_INVENTORYFULLMESSAGE       =  "INVENTORY FULL";
const  double       HUD_KEYUSEDTIME                =  2.0;
const  std::string  HUD_KEYUSEDMESSAGE             =  "KEY USED";
//     Assets
const  std::string  ASSETS_FONTS                   =  "fonts/";
const  std::string  ASSETS_ITEMGROUPS              =  "tables/itemgroups/";
const  std::string  ASSETS_MONSTERSETS             =  "maps/monstersets/";
const  std::string  ASSETS_PLAYERTEXTURES          =  "textures/player/";
const  std::string  ASSETS_MONSTERTEXTURES         =  "textures/monsters/";
const  std::string  ASSETS_TEXTURE_PATH            =  "textures/";
const  std::string  ASSETS_MUSIC                   =  "music/";
const  std::string  ASSETS_SAMPLES                 =  "sounds/";
const  std::string  ASSETS_ATTACK_SAMPLES          =  "tables/sounds/attack.tsv";
const  std::string  ASSETS_SAMPLEDATA              =  "tables/sounds/samples.tsv";
const  std::string  ASSETS_FONTDATA                =  "tables/fonts.tsv";
const  std::string  ASSETS_MAPS                    =  "maps/";
const  std::string  ASSETS_LEVELS                  =  "tables/levels.tsv";
const  std::string  ASSETS_SKILLS                  =  "tables/skills.tsv";
const  std::string  ASSETS_STRINGS                 =  "tables/strings.tsv";
const  std::string  ASSETS_COLORS                  =  "tables/colors.tsv";
const  std::string  ASSETS_REELS                   =  "tables/reels.tsv";
const  std::string  ASSETS_MONSTERS                =  "tables/monsters.tsv";
const  std::string  ASSETS_ANIMATIONS              =  "tables/animation.tsv";
const  std::string  ASSETS_PARTICLES               =  "tables/particles.tsv";
const  std::string  ASSETS_WEAPONPARTICLES         =  "tables/weaponparticles.tsv";
const  std::string  ASSETS_MISCITEMS               =  "tables/items.tsv";
const  std::string  ASSETS_UPGRADES                =  "tables/upgrades.tsv";
const  std::string  ASSETS_AMMO                    =  "tables/ammo.tsv";
const  std::string  ASSETS_WEAPONS                 =  "tables/weapons.tsv";
const  std::string  ASSETS_ARMOR                   =  "tables/armor.tsv";
const  std::string  ASSETS_ITEMDROPDATA            =  "tables/itemdrops.tsv";
const  std::string  ASSETS_TEXTURES_MAIN           =  "tables/textures/main.tsv";
const  std::string  ASSETS_TEXTURES_MAP            =  "tables/textures/map.tsv";
const  std::string  ASSETS_TEXTURES_EDITOR         =  "tables/textures/editor.tsv";
const  std::string  ASSETS_STYLES                  =  "tables/ui/styles.tsv";
const  std::string  ASSETS_ELEMENTS                =  "tables/ui/elements.tsv";
const  std::string  ASSETS_LABELS                  =  "tables/ui/labels.tsv";
const  std::string  ASSETS_IMAGES                  =  "tables/ui/images.tsv";
const  std::string  ASSETS_BUTTONS                 =  "tables/ui/buttons.tsv";
const  std::string  ASSETS_TEXTBOXES               =  "tables/ui/textboxes.tsv";
