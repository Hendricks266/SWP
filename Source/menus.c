//-------------------------------------------------------------------------
/*
Copyright (C) 1997, 2005 - 3D Realms Entertainment

This file is part of Shadow Warrior version 1.2

Shadow Warrior is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1997 - Frank Maddin and Jim Norwood
Prepared for public release: 03/28/2005 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------

// Colors
// 11=Green / 13=Blue / 14=Red / 16=Normal / 17=White /

// Added SetHighres
// Added Press key options
// Added bGameQuit
// Added enabled Music button
// Added Weapon Switching
// Added Mapname in Save/Load menus
// Added Modified for Shareware music
// Added CD Music stuff
// Added glfovscreen
// Added glwidescreen
// Added seperate more menu
// Added resize menu cursor
// Added car hack option
// Added re-position on/off icon
// Added swp menu hints
// Added MouseScale X and Y
// Added small text in 'Kid Mode'
// Added texture filtering and anisotropy
// Added version and by to menu credits
// Added WeaponIcons
// Added Hightile and Model
// Added enabled gs.Talking
// Added gamesgroup
// Added new options menu
// Added adjust pal on small letters
// Added General menu adjustments
// Added Screen resolution menu
// Added Commandline Info to game menu
// Added variable Palette 23 for menu text
// Fixed screensize 8 bit status bar
// Added wangbang stuff
// Added usegoodalpha (Alpha Channel)           enhances background transparency - no clamping
// Added Texture compression
// Added PutStringInfo2 to text.c
// Added RandomMusic
// Added Fix to start new game from usermap
// Added seach for font in lowres file
// Added changed menu palette from 23 to 32
// Added Autosave
// Added NoHrp check
// MinGW / Msys - 4.2.0
// Added Swp Version in Game menu
// Added Force Setup
// Added new MNU_DrawOptionString
// Added alternate minihud
// Version 4238
// Added back usermaps to episodes/multiplay
// Added addons to multiplay
// Added colors to multi display
// Added force weaponswitch off if multiplay
// Version 4.2.4
// Added changed menu palette from 32 to 10 mPal32 interfered with night goggles (pal.h)
// Version 4.2.5
// Added Delete to kill music
// Version 4.2.6
// Moved weaponicons and alt minihud to video
// Version 4.2.7
// Changed gs.AltMiniBar to gs.MiniHudTyp  2
// Changed gs.WeaponIcons to gs.MiniHudTyp 1
// Added underwater tint
// Added MOUSE_X_MAX_VALUE / MOUSE_Y_MAX_VALUE
// Added NoHrp to configs
// Added screencapture to menus
// Added vsync
// Added cd force loop
// Added MNU_BitCheck
// Added MNU_CheckStart
// Changed MNU_DrawOptionString to add shade
// Fixed DeleteAutoSave deleting correct file
// Added -by detection for bypass press(x)
//

#include "build.h"
#include "compat.h"
#include "osd.h"

#include "keys.h"
#include "names2.h"
#include "panel.h"
#include "game.h"
#include "tags.h"
#include "sector.h"
#include "sprite.h"
#include "weapon.h"
#include "player.h"
#include "jsector.h"
#include "control.h"
#include "menus.h"
#include "sw_strs.h"
#include "control.h"
#include "pal.h"

#include "function.h"
#include "gamedefs.h"
#include "net.h"
#include "fx_man.h"
#include "music.h"
#include "text.h"

#include "colormap.h"

static int menucmp(const char* one, const char* two)
{
    if (one == NULL)
        return -1;
    if (two == NULL)
        return -1;

    return Bstrcmp(one, two);
}

//#define PLOCK_VERSION TRUE

extern BOOL cdvalid, enabled;

short TimeLimitTable[9] = {0,3,5,10,15,20,30,45,60};

short QuickLoadNum = -1;
char QuickLoadDescrDialog[32];
BOOL QuickSaveMode = FALSE;
BOOL SavePrompt = FALSE;

extern int32 ScreenMode,ScreenWidth,ScreenHeight,ScreenBPP;
extern BOOL bGrp;
extern int vsync;
extern char svgame[40];
extern short bypass;

BOOL  bGameQuit = FALSE;
short MenuTextColor = 1;
extern BOOL ShellFlag;
extern short mPal;

short NumUserMaps;
short MapCursor = 0;
short MapPosition = 0;
BOOL  bMapmenu = FALSE;
char  UserMaps[1000][32];
char  MyMaps[15][32];

short NumUserMusic;
short MusicCursor = 0;
short MusicPosition = 0;
BOOL  bMusicmenu = FALSE;
char  UserMusic[300][32];
char  MyMusic[15][32];

long  Menutick = 0;
char MultiOption[16];

BOOL MNU_StartMultiGame(void);
BOOL MNU_HrpCheck(MenuItem_p item);
BOOL MNU_BitCheck(MenuItem_p item);
BOOL MNU_CheckMulti(MenuItem *item);
BOOL MNU_CheckMapStatus(MenuItem *item);
BOOL MNU_CheckStart(MenuItem *item);
BOOL MNU_CheckOption(MenuItem *item);
BOOL MNU_HostingCheck(MenuItem *item);
BOOL MNU_DoPlayerName(void);
BOOL MNU_PlayerName(void);
BOOL MNU_DoIpAddress(void);
void MNU_ClientDisplay(void);
void MNU_DrawOptionString(short x, short y, char *string, short shade, short pal);


extern BOOL InMenuLevel, LoadGameOutsideMoveLoop, LoadGameFromDemo;
extern BYTE RedBookSong[40];
extern BOOL ExitLevel, NewGame;
extern short Level, Skill;
extern BOOL MusicInitialized, FxInitialized;
BOOL MNU_CheckUserMap(MenuItem *item);
BOOL MNU_SaveGameCheck(MenuItem_p item);
BOOL MNU_TeamPlayCheck(MenuItem *item);
BOOL MNU_CoopPlayCheck(MenuItem *item);
BOOL MNU_StatCheck(MenuItem *item);
BOOL MNU_LoadGameCheck(MenuItem *item);

BOOL MNU_TenCheck(MenuItem *item);
static BOOL MNU_TryMusicInit(void);
static void MNU_UpLevel(void);

BOOL MNU_LoadSaveDraw(UserCall call, MenuItem * item);
BOOL MNU_LoadSaveMove(UserCall call, MenuItem * item);

BOOL MenuButtonAutoRun = FALSE;
BOOL MenuButtonAutoAim = FALSE;
// misc load-save vars
short LastSaveNum = 99;
char SaveGameDescr[11][80];
char BackupSaveGameDescr[80];
short screen_tile = -1;

BOOL MenuInputMode = FALSE;
SHORT MenuTextShade = 0;
BOOL passwordvalid = FALSE;
BOOL MNU_HurtTeammateCheck(MenuItem *item);
BOOL MNU_TeamPlayChange(MenuItem *item);

extern BOOL bHosting;
extern int32 NoHrp;

extern char PlayerNameArg[40];
extern char IPAddressArg[8][40];
char sPlayerBack[110];

// Font pic table
unsigned short xlatfont[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 32
    2274, 2294, 2274, 2277, 2278, 2280, 2295, 2282, 2283, 2281, 2286, 2297, 2285,       // 45
    2299, 2301, 2271, 2262, 2263, 2264, 2265, 2266, 2267, 2268, 2269, 2270,     // 57
    2292, 2293, 2296, 2287, 2298, 2300, 2275, 2236, 2237, 2238, 2239, 2240,     // 69
    2241, 2242, 2243, 2244, 2245, 2246, 2247, 2248, 2249, 2250, 2251, 2252,     // 81
    2253, 2254, 2255, 2256, 2257, 2258, 2259, 2260, 2261, 2290, 2289, 2291,     // 93
    2279, 2284, 2295, 2236, 2237, 2238, 2239, 2240, 2241, 2242, 2243, 2244,     // 105
    2245, 2246, 2247, 2248, 2249, 2250, 2251, 2252, 2253, 2254, 2255, 2256,     // 117
    2257, 2258, 2259, 2260, 2261, 2282, 2288, 2283, 2272        // 126
    };

int slidersettings [sldr_max] =
    {
    0, SENSE_DEFAULT, FXVOL_DEFAULT, MUSIC_DEFAULT, SCRSIZE_DEFAULT,
        BRIGHTNESS_DEFAULT, BORDERTILE_DEFAULT, GAMETYPE_DEFAULT, NETLEVEL_DEFAULT,
        MONSTERS_DEFAULT, KILLLIMIT_DEFAULT, TIMELIMIT_DEFAULT, PLAYERCOLOR_DEFAULT,
	0,0,	// video mode
    };

short buttonsettings[btn_max];

// EXTERNS ////////////////////////////
#define XDIM    320
#define YDIM    200
extern char DefaultPalette[];
extern BOOL QuitFlag;

void TerminateGame(void);
void ResetKeys(void);

// GLOBALS ////////////////////////////

char playertextbuffer[80];              // Used for various input strings
char playerbuflen = 0;                  // Current length of the string in
                                        // the buffer
char maxtextlen;                        // max length allowed for current

static struct { int xdim,ydim; } validresolutions[MAXVALIDMODES];
static int numvalidresolutions = 0, validbpps[8], numvalidbpps = 3;

extern char UserMapName[80];
extern char CurrMapName[80];
extern char UserAddon[80];

static void UpdateValidModes(int bpp, int fs)
{
	int i, j;

	numvalidresolutions = numvalidbpps = 0;
	for (i=0; i<validmodecnt; i++) {
		if ((validmode[i].fs & 1) != fs) continue;

		for (j=0; j<numvalidbpps; j++)
			if (validbpps[j] == validmode[i].bpp) break;
		if (j==numvalidbpps) validbpps[numvalidbpps++] = validmode[i].bpp;

		if (validmode[i].bpp != bpp) continue;

		validresolutions[numvalidresolutions].xdim = validmode[i].xdim;
		validresolutions[numvalidresolutions].ydim = validmode[i].ydim;
		numvalidresolutions++;
	}
}

static BOOL ApplyModeSettings(MenuItem *item)
{
	int lastx, lasty, lastbpp, lastfs;
	int newx, newy, newbpp, newfs;

	lastx = xdim; lasty = ydim; lastbpp = bpp; lastfs = fullscreen;
	newx   = validresolutions[ slidersettings[sldr_videores] ].xdim;
	newy   = validresolutions[ slidersettings[sldr_videores] ].ydim;
	newbpp = validbpps[ slidersettings[sldr_videobpp] ];
	newfs  = buttonsettings[btn_videofs];

	if (lastx == newx && lasty == newy && lastbpp == newbpp && lastfs == newfs) return FALSE;

    if (newbpp < 16)
    {
        newbpp = 16;
        slidersettings[sldr_videobpp] = 1;
    }

	if (setgamemode(newfs, newx, newy, newbpp))
		setgamemode(lastfs, lastx, lasty, lastbpp);
	else
	{
		ScreenMode = newfs;
		ScreenWidth = newx;
		ScreenHeight = newy;
		ScreenBPP = newbpp;

		SetupAspectRatio();
		SetRedrawScreen(&Player[myconnectindex]);
	}
	if (newbpp > 8)
	    gs.SetHighres = 1;
    else
	    gs.SetHighres = 0;

	return FALSE;
}

MenuItem hacks_i[] =
    {
    {DefButton(btn_asv,       0, "Auto Save Game"),    OPT_XS+55, OPT_LINE(1),  1, m_defshade, 0, NULL, NULL, NULL},
    {DefButton(btn_hid,       0, "Player Weapon"),     OPT_XS+55, OPT_LINE(2),  1, m_defshade, 0, NULL, NULL, NULL},
    {DefButton(btn_drt,       0, "Optional Darts"),    OPT_XS+55, OPT_LINE(3),  1, m_defshade, 0, NULL, NULL, NULL},
    {DefButton(btn_yin,       0, "Shuriken Cursor"),   OPT_XS+55, OPT_LINE(4),  1, m_defshade, 0, NULL, NULL, NULL},

    {DefButton(btn_nja,       0, "Ninja Sliced Hack"), OPT_XS+55, OPT_LINE(6),  1, m_defshade, 0, NULL, NULL, NULL},
    {DefButton(btn_car,       0, "Seppuku Car Hack"),  OPT_XS+55, OPT_LINE(7),  1, m_defshade, 0, NULL, NULL, NULL},

    {DefNone}
    };
MenuGroup game2group = {100, 5, "^Game 2", hacks_i, pic_optionstitl, 0, m_defshade, NULL,NULL, 0};

MenuItem game_i[] =
    {
    {DefButton(btn_crosshair, 0, "Crosshair"),        OPT_XS+55, OPT_LINE(0),  1, m_defshade, 0, NULL, NULL, NULL},
    {DefButton(btn_shadows,   0, "Shadows"),          OPT_XS+55, OPT_LINE(1),  1, m_defshade, 0, NULL, NULL, NULL},
    {DefButton(btn_auto_run,  0, "Auto Run"),         OPT_XS+55, OPT_LINE(2),  1, m_defshade, 0, NULL, NULL, NULL},
    {DefButton(btn_messages,  0, "Messages"),         OPT_XS+55, OPT_LINE(3),  1, m_defshade, 0, NULL, NULL, NULL},
    {DefButton(btn_auto_aim,  0, "Auto-Aiming"),      OPT_XS+55, OPT_LINE(4),  1, m_defshade, 0, NULL, NULL, NULL},
    {DefButton(btn_stats,     0, "Level Stats"),      OPT_XS+55, OPT_LINE(5),  1, m_defshade, 0, NULL, MNU_StatCheck, NULL},
    {DefButton(btn_voxels,    0, "Voxel Sprites"),    OPT_XS+55, OPT_LINE(6),  1, m_defshade, 0, NULL, NULL, NULL},
    {DefButton(btn_wep,       0, "Weapon Switch"),    OPT_XS+55, OPT_LINE(7),  1, m_defshade, 0, NULL, MNU_StatCheck, NULL},
    {DefButton(btn_str,       0, "Use Startup Menu"), OPT_XS+55, OPT_LINE(8),  1, m_defshade, 0, NULL, NULL, NULL},

    {DefLayer(KEYSC_M,       "More...", &game2group), OPT_XS+55, OPT_LINE(10), 1, m_defshade, 0, NULL, NULL, NULL},
    {DefInert(0, NULL), OPT_XSIDE,                               OPT_LINE(10), 0, m_defshade, 0, NULL, NULL, NULL},
    {DefNone}
    };

MenuGroup gamegroup = {100, 5, "^Game", game_i, pic_optionstitl, 0, m_defshade, NULL,NULL, 0};

MenuItem sound_i[] =
{
    {DefButton(btn_playcd, 0,      "CD Music"),     OPT_XS+20, OPT_LINE(0),  1, m_defshade, 0, NULL, NULL, NULL},
    {DefButton(btn_music,  0,      "Midi Music"),   OPT_XS+20, OPT_LINE(1),  1, m_defshade, 0, NULL, NULL, NULL},
    {DefSlider(sldr_musicvolume, 0,"Music Volume"), OPT_XS+20, OPT_LINE(2),  1, m_defshade, 0, NULL, NULL, NULL},
    {DefInert(0, NULL),                             OPT_XSIDE, OPT_LINE(2),  0, m_defshade, 0, NULL, NULL, NULL},

    {DefButton(btn_sound, 0,       "Sounds"),       OPT_XS+20, OPT_LINE(4),  1, m_defshade, 0, NULL, MNU_FxCheck, NULL},
    {DefSlider(sldr_sndfxvolume, 0,"Sound Volume"), OPT_XS+20, OPT_LINE(5),  1, m_defshade, 0, NULL, MNU_FxCheck, NULL},
    {DefInert(0, NULL),                             OPT_XSIDE, OPT_LINE(5),  0, m_defshade, 0, NULL, NULL, NULL},

    {DefButton(btn_rand,       0,  "Random Music"), OPT_XS+20, OPT_LINE(7),  1, m_defshade, 0, NULL, MNU_FxCheck, NULL},
    {DefButton(btn_flipstereo, 0,  "Flip Stereo"),  OPT_XS+20, OPT_LINE(8),  1, m_defshade, 0, NULL, MNU_FxCheck, NULL},
    {DefButton(btn_ambience,   0,  "Ambience"),     OPT_XS+20, OPT_LINE(9),  1, m_defshade, 0, NULL, MNU_FxCheck, NULL},
    {DefButton(btn_talking,    0,  "Talking"),      OPT_XS+20, OPT_LINE(10), 1, m_defshade, 0, NULL, MNU_FxCheck, NULL},

    {DefNone}
};

MenuGroup soundgroup = {110,5,"^Sound",sound_i,pic_optionstitl,0,m_defshade, NULL,NULL, 0};

MenuItem parental_i[] =
    {
    {DefButton(btn_parental, 0, "Kid Mode"), OPT_XS+50, OPT_LINE(1), 1, m_defshade, 0, NULL, NULL, NULL},
    {DefOption(KEYSC_P, "Change Password"),  OPT_XS+50, OPT_LINE(2), 1, m_defshade, 0, MNU_ParentalCustom, NULL, NULL},
    {DefNone}
    };

MenuGroup parentalgroup = {65, 5, "^Kid Mode", parental_i, pic_newgametitl, 0, m_defshade, MNU_DoParentalPassword,NULL,0};

MenuItem screen_i[] =
    {
    {DefSlider(sldr_textcl, 0, "Menu Text"),    OPT_XS+15, OPT_LINE(0), 1, m_defshade, 0, NULL, MNU_HrpCheck, NULL},
    {DefInert(0, NULL),                         OPT_XSIDE, OPT_LINE(0), 0, m_defshade, 0, NULL, NULL, NULL},

    {DefButton(btn_videofs, 0,   "Fullscreen"), OPT_XS+15, OPT_LINE(1), 1, m_defshade, 0, NULL, NULL, NULL},
    {DefButton(btn_videows, 0,   "Widescreen"), OPT_XS+15, OPT_LINE(2), 1, m_defshade, 0, NULL, NULL, NULL},

    {DefButton(btn_hitile, 0, "Highres Tiles"), OPT_XS+15, OPT_LINE(3), 1, m_defshade, 0, NULL, MNU_HrpCheck, NULL},
    {DefButton(btn_models, 0,"Highres Models"), OPT_XS+15, OPT_LINE(4), 1, m_defshade, 0, NULL, MNU_HrpCheck, NULL},
    {DefButton(btn_cacomp, 0,"Cache Compress"), OPT_XS+15, OPT_LINE(5), 1, m_defshade, 0, NULL, MNU_BitCheck, NULL},
    {DefButton(btn_galpha, 0, "Alpha Channel"), OPT_XS+15, OPT_LINE(6), 1, m_defshade, 0, NULL, MNU_BitCheck, NULL},
    {DefButton(btn_vsync,  0, "Vertical Sync"), OPT_XS+15, OPT_LINE(7), 1, m_defshade, 0, NULL, MNU_BitCheck, NULL},

    {DefSlider(sldr_videobpp, 0,     "Colour"), OPT_XS+15, OPT_LINE(9) ,1, m_defshade, 0, NULL, NULL, NULL},
    {DefInert(0, NULL),                         OPT_XSIDE, OPT_LINE(9) ,0, m_defshade, 0, NULL, NULL, NULL},
    {DefSlider(sldr_videores, 0, "Resolution"), OPT_XS+15, OPT_LINE(11),1, m_defshade, 0, NULL, NULL, NULL},
    {DefInert(0, NULL),                         OPT_XSIDE, OPT_LINE(11),0, m_defshade, 0, NULL, NULL, NULL},

    {DefOption(0,            "Apply Settings"), OPT_XS+30, OPT_LINE(13),1, m_defshade, 21, ApplyModeSettings, NULL, NULL},
    {DefInert(0, NULL),                         OPT_XSIDE, OPT_LINE(13),0, m_defshade, 0, NULL, NULL, NULL},
    {DefNone}
    };

MenuGroup screengroup = {65, 5, "^Screen", screen_i, pic_newgametitl, 0, m_defshade, NULL,NULL, 0};
/*
MenuItem video_i[] =
    {
    {DefButton(btn_videows, 0, "Wide Screen"), OPT_XS+15, OPT_LINE(1), 1, m_defshade, 0, NULL, NULL, NULL},
    {DefSlider(sldr_fov,    0, "Screen Fov"),  OPT_XS+15, OPT_LINE(2), 1, m_defshade, 0, NULL, NULL, NULL},
    {DefInert(0, NULL),                        OPT_XSIDE, OPT_LINE(2), 0, m_defshade, 0, NULL, NULL, NULL},
    {DefNone}
    };
MenuGroup videogroup = {65, 5, "^Video", video_i, pic_newgametitl, 0, m_defshade, NULL,NULL, 0};
*/
MenuItem video_i[] =
    {
    {DefSlider(sldr_scrsize, 0, "Screen Size"),        OPT_XS+10, OPT_LINE(0), 1, m_defshade, 0, NULL, NULL, NULL},
    {DefInert(0, NULL),                                OPT_XSIDE, OPT_LINE(0), 0, m_defshade, 0, NULL, NULL, NULL},

    {DefSlider(sldr_fov,     0, "Screen Fov"),         OPT_XS+10, OPT_LINE(1), 1, m_defshade, 0, NULL, NULL, NULL},
    {DefInert(0, NULL),                                OPT_XSIDE, OPT_LINE(1), 0, m_defshade, 0, NULL, NULL, NULL},

    {DefSlider(sldr_bordertile, 0, "Border Tile"),     OPT_XS+10, OPT_LINE(2), 1, m_defshade, 0, NULL, NULL, NULL},
    {DefInert(0, NULL),                                OPT_XSIDE, OPT_LINE(2), 0, m_defshade, 0, NULL, NULL, NULL},

    {DefSlider(sldr_brightness, 0, "Brightness"),      OPT_XS+10, OPT_LINE(3), 1, m_defshade, 0, NULL, NULL, NULL},
    {DefInert(0, NULL),                                OPT_XSIDE, OPT_LINE(3), 0, m_defshade, 0, NULL, NULL, NULL},

    {DefSlider(sldr_tropy, 0, "Anisotropy"),           OPT_XS+10, OPT_LINE(4), 1, m_defshade, 0, NULL, NULL, NULL},
    {DefInert(0, NULL),                                OPT_XSIDE, OPT_LINE(4), 0, m_defshade, 0, NULL, NULL, NULL},

    {DefSlider(sldr_filter, 0, "Texture Filter"),      OPT_XS+10, OPT_LINE(5), 1, m_defshade, 0, NULL, MNU_BitCheck, NULL},
    {DefInert(0, NULL),                                OPT_XSIDE, OPT_LINE(5), 0, m_defshade, 0, NULL, NULL, NULL},

    {DefSlider(sldr_hud,    0, "Mini Hud Type"),       OPT_XS+10, OPT_LINE(6), 1, m_defshade, 0, NULL, NULL, NULL},
    {DefInert(0, NULL),                                OPT_XSIDE, OPT_LINE(6), 0, m_defshade, 0, NULL, NULL, NULL},

    {DefButton(btn_txcach, 0,  "Texture Cache"),       OPT_XS+10, OPT_LINE(7), 1, m_defshade, 0, NULL, MNU_BitCheck, NULL},
    {DefButton(btn_txcomp, 0,  "Texture Compress"),    OPT_XS+10, OPT_LINE(8), 1, m_defshade, 0, NULL, MNU_BitCheck, NULL},

    {DefLayer(KEYSC_S, "Screen Modes", &screengroup),  OPT_XS+10, OPT_LINE(10),1, m_defshade, 0, NULL, NULL, NULL},
    {DefInert(0, NULL),                                OPT_XSIDE, OPT_LINE(10),0, m_defshade, 0, NULL, NULL, NULL},
    {DefNone}
    };
MenuGroup videogroup = {65, 5, "^Video", video_i, pic_newgametitl, 0, m_defshade, NULL,NULL, 0};

MenuItem mouse_i[] =
    {
    {DefSlider(sldr_mouse,  0,     "Mouse Speed"),   OPT_XS+30, OPT_LINE(1), 1, m_defshade, 0, NULL, MNU_MouseCheck, NULL},
    {DefInert(0, NULL),                              OPT_XSIDE, OPT_LINE(1), 0, m_defshade, 0, NULL, NULL, NULL}, // Blank line for mouse
    {DefSlider(sldr_mousex, 0,     "Mouse Scale X"), OPT_XS+30, OPT_LINE(2), 1, m_defshade, 0, NULL, NULL, NULL},
    {DefInert(0, NULL),                              OPT_XSIDE, OPT_LINE(2), 0, m_defshade, 0, NULL, NULL, NULL}, // Blank line for mouse
    {DefSlider(sldr_mousey, 0,     "Mouse Scale Y"), OPT_XS+30, OPT_LINE(3), 1, m_defshade, 0, NULL, NULL, NULL},
    {DefInert(0, NULL),                              OPT_XSIDE, OPT_LINE(3), 0, m_defshade, 0, NULL, NULL, NULL}, // Blank line for mouse

    {DefButton(btn_mouse_aim, 0,   "Mouse Aiming"),  OPT_XS+30, OPT_LINE(5), 1, m_defshade, 0, NULL, MNU_MouseCheck, NULL},
    {DefButton(btn_mouse_invert, 0,"Mouse Invert"),  OPT_XS+30, OPT_LINE(6), 1, m_defshade, 0, NULL, MNU_MouseCheck, NULL},
    {DefNone}
    };

MenuGroup mousegroup = {65, 5, "^Mouse", mouse_i, pic_newgametitl, 0, m_defshade, NULL,NULL, 0};

BOOL MNU_KeySetupCustom(UserCall call, MenuItem *item);
MenuGroup keysetupgroup = {0, 0, NULL, NULL, 0, 0, m_defshade, MNU_KeySetupCustom, NULL, 0};

BOOL MNU_MouseSetupCustom(UserCall call, MenuItem *item);
MenuGroup mousesetupgroup = {0, 0, NULL, NULL, 0, 0, m_defshade, MNU_MouseSetupCustom, NULL, 0};

BOOL MNU_MapGameInfo(UserCall call, MenuItem *item);
BOOL MNU_MapKeyIn(void);
BOOL MNU_MapStartUp(void);

MenuItem skill_i[] =
    {
    {DefOption(KEYSC_E, &SkillNames[0][0]), 45,  54, pic_easy, m_defshade, 0, MNU_StartGame, NULL, NULL},
    {DefOption(KEYSC_N, &SkillNames[1][0]), 45,  70, pic_normal, m_defshade, 0, MNU_StartGame, NULL, NULL},
    {DefOption(KEYSC_H, &SkillNames[2][0]), 45,  86, pic_hard, m_defshade, 0, MNU_StartGame, NULL, NULL},
    {DefOption(KEYSC_I, &SkillNames[3][0]), 45, 102, pic_impossible, m_defshade, 0, MNU_StartGame, NULL, NULL},
    {DefNone}
    };

MenuGroup skillgroup = {100, 5, "^Skill", skill_i, pic_newgametitl, 0, m_defshade, NULL, NULL, 2};

BOOL MNU_GetMapInput(void);

MenuItem maps_i[] =
    {
#define yy(line) (32 + (line * 9))
    {DefLayer(0, &MyMaps[0][0],  &skillgroup), 110, yy(0),  pic_easy, m_defshade, 0, MNU_GetMapInput, NULL, NULL},
    {DefLayer(0, &MyMaps[1][0],  &skillgroup), 110, yy(1),  pic_easy, m_defshade, 0, MNU_GetMapInput, NULL, NULL},
    {DefLayer(0, &MyMaps[2][0],  &skillgroup), 110, yy(2),  pic_easy, m_defshade, 0, MNU_GetMapInput, NULL, NULL},
    {DefLayer(0, &MyMaps[3][0],  &skillgroup), 110, yy(3),  pic_easy, m_defshade, 0, MNU_GetMapInput, NULL, NULL},
    {DefLayer(0, &MyMaps[4][0],  &skillgroup), 110, yy(4),  pic_easy, m_defshade, 0, MNU_GetMapInput, NULL, NULL},
    {DefLayer(0, &MyMaps[5][0],  &skillgroup), 110, yy(5),  pic_easy, m_defshade, 0, MNU_GetMapInput, NULL, NULL},
    {DefLayer(0, &MyMaps[6][0],  &skillgroup), 110, yy(6),  pic_easy, m_defshade, 0, MNU_GetMapInput, NULL, NULL},
    {DefLayer(0, &MyMaps[7][0],  &skillgroup), 110, yy(7),  pic_easy, m_defshade, 0, MNU_GetMapInput, NULL, NULL},
    {DefLayer(0, &MyMaps[8][0],  &skillgroup), 110, yy(8),  pic_easy, m_defshade, 0, MNU_GetMapInput, NULL, NULL},
    {DefLayer(0, &MyMaps[9][0],  &skillgroup), 110, yy(9),  pic_easy, m_defshade, 0, MNU_GetMapInput, NULL, NULL},
    {DefLayer(0, &MyMaps[10][0], &skillgroup), 110, yy(10), pic_easy, m_defshade, 0, MNU_GetMapInput, NULL, NULL},
    {DefLayer(0, &MyMaps[11][0], &skillgroup), 110, yy(11), pic_easy, m_defshade, 0, MNU_GetMapInput, NULL, NULL},
    {DefLayer(0, &MyMaps[12][0], &skillgroup), 110, yy(12), pic_easy, m_defshade, 0, MNU_GetMapInput, NULL, NULL},
    {DefLayer(0, &MyMaps[13][0], &skillgroup), 110, yy(13), pic_easy, m_defshade, 0, MNU_GetMapInput, NULL, NULL},
    {DefLayer(0, &MyMaps[14][0], &skillgroup), 110, yy(14), pic_easy, m_defshade, 0, MNU_GetMapInput, NULL, NULL},
    {DefNone}
    };
MenuGroup usermapgroup = {100, 5, "^User Maps", maps_i, pic_newgametitl, 0, m_defshade, MNU_MapGameInfo, MNU_MapKeyIn, 0};

BOOL MNU_MusicKeyIn(void);
BOOL MNU_SetMusic(void);

MenuItem music_i[] =
    {
    {DefOption(0, &MyMusic[0][0]),  110, yy(0),  pic_easy, m_defshade, 0, MNU_SetMusic, NULL, NULL},
    {DefOption(0, &MyMusic[1][0]),  110, yy(1),  pic_easy, m_defshade, 0, MNU_SetMusic, NULL, NULL},
    {DefOption(0, &MyMusic[2][0]),  110, yy(2),  pic_easy, m_defshade, 0, MNU_SetMusic, NULL, NULL},
    {DefOption(0, &MyMusic[3][0]),  110, yy(3),  pic_easy, m_defshade, 0, MNU_SetMusic, NULL, NULL},
    {DefOption(0, &MyMusic[4][0]),  110, yy(4),  pic_easy, m_defshade, 0, MNU_SetMusic, NULL, NULL},
    {DefOption(0, &MyMusic[5][0]),  110, yy(5),  pic_easy, m_defshade, 0, MNU_SetMusic, NULL, NULL},
    {DefOption(0, &MyMusic[6][0]),  110, yy(6),  pic_easy, m_defshade, 0, MNU_SetMusic, NULL, NULL},
    {DefOption(0, &MyMusic[7][0]),  110, yy(7),  pic_easy, m_defshade, 0, MNU_SetMusic, NULL, NULL},
    {DefOption(0, &MyMusic[8][0]),  110, yy(8),  pic_easy, m_defshade, 0, MNU_SetMusic, NULL, NULL},
    {DefOption(0, &MyMusic[9][0]),  110, yy(9),  pic_easy, m_defshade, 0, MNU_SetMusic, NULL, NULL},
    {DefOption(0, &MyMusic[10][0]), 110, yy(10), pic_easy, m_defshade, 0, MNU_SetMusic, NULL, NULL},
    {DefOption(0, &MyMusic[11][0]), 110, yy(11), pic_easy, m_defshade, 0, MNU_SetMusic, NULL, NULL},
    {DefOption(0, &MyMusic[12][0]), 110, yy(12), pic_easy, m_defshade, 0, MNU_SetMusic, NULL, NULL},
    {DefOption(0, &MyMusic[13][0]), 110, yy(13), pic_easy, m_defshade, 0, MNU_SetMusic, NULL, NULL},
    {DefOption(0, &MyMusic[14][0]), 110, yy(14), pic_easy, m_defshade, 0, MNU_SetMusic, NULL, NULL},
    {DefNone}
    };
MenuGroup usermusicgroup = {100, 5, "^User Music", music_i, pic_newgametitl, 0, m_defshade, MNU_MapGameInfo, MNU_MusicKeyIn, 0};

MenuItem Peer2Peer_i[] =
    {
    {DefOption(0, "Player's  IP"), OPT_XS+20, OPT_LINE(1), 1, m_defshade, 0, MNU_IpAddress,   NULL, NULL},
    {DefOption(0, "Opponent IP"),  OPT_XS+20, OPT_LINE(2), 1, m_defshade, 0, MNU_IpAddress,   NULL, NULL},
    {DefOption(0, "Opponent IP"),  OPT_XS+20, OPT_LINE(3), 1, m_defshade, 0, MNU_IpAddress,   NULL, NULL},
    {DefOption(0, "Opponent IP"),  OPT_XS+20, OPT_LINE(4), 1, m_defshade, 0, MNU_IpAddress,   NULL, NULL},
    {DefOption(0, "Opponent IP"),  OPT_XS+20, OPT_LINE(5), 1, m_defshade, 0, MNU_IpAddress,   NULL, NULL},
    {DefOption(0, "Opponent IP"),  OPT_XS+20, OPT_LINE(6), 1, m_defshade, 0, MNU_IpAddress,   NULL, NULL},
    {DefOption(0, "Opponent IP"),  OPT_XS+20, OPT_LINE(7), 1, m_defshade, 0, MNU_IpAddress,   NULL, NULL},
    {DefOption(0, "Opponent IP"),  OPT_XS+20, OPT_LINE(8), 1, m_defshade, 0, MNU_IpAddress,   NULL, NULL},
    {DefNone}
    };
MenuGroup addressgroup = {65, 5, "^Peer2Peer", Peer2Peer_i, pic_newgametitl, 0, m_defshade, MNU_DoIpAddress,NULL,0};

MenuItem network_i[] =
{
    {DefOption(0, "Player Name"),                       OPT_XS, OPT_LINE(0), 1, m_defshade, 0, MNU_PlayerName, NULL, NULL},

    {DefSlider(sldr_gametype, 0, "Game Type"),          OPT_XS, OPT_LINE(1), 1, m_defshade, 0, NULL, MNU_HostingCheck, NULL},
    {DefInert(0, NULL),                              OPT_XSIDE, OPT_LINE(1), 0, m_defshade, 0, NULL, NULL, NULL},

    {DefSlider(sldr_netlevel, 0, "Level"),              OPT_XS, OPT_LINE(2), 1, m_defshade, 0, NULL, MNU_CheckUserMap, NULL},
    {DefInert(0, NULL),                              OPT_XSIDE, OPT_LINE(2), 0, m_defshade, 0, NULL, MNU_CheckUserMap, NULL}, // -70
    {DefInert(0, " "),                                  OPT_XS, OPT_LINE(3), pic_episode1, m_defshade, 0, NULL, NULL, NULL},

    {DefSlider(sldr_killlimit, 0, "Kill Limit"),        OPT_XS, OPT_LINE(4), 1, m_defshade, 0, NULL, MNU_CoopPlayCheck, NULL},
    {DefInert(0, NULL),               OPT_XSIDE,                OPT_LINE(4), 0, m_defshade, 0, NULL, MNU_CoopPlayCheck, NULL},

    {DefSlider(sldr_timelimit, 0, "Time Limit"),        OPT_XS, OPT_LINE(5), 1, m_defshade, 0, NULL, MNU_CoopPlayCheck, NULL},
    {DefInert(0, NULL),                              OPT_XSIDE, OPT_LINE(5), 0, m_defshade, 0, NULL, MNU_CoopPlayCheck, NULL},

    {DefSlider(sldr_playercolor, 0, "Player Color"),    OPT_XS, OPT_LINE(6), 1, m_defshade, 0, NULL, NULL, NULL},
    {DefInert(0, NULL),                              OPT_XSIDE, OPT_LINE(6), 0, m_defshade, 0, NULL, NULL, NULL},

    {DefSlider(sldr_monsters, 0, "Monsters"),           OPT_XS, OPT_LINE(7), 1, m_defshade, 0, NULL, MNU_HostingCheck, NULL},
    {DefInert(0, NULL),                              OPT_XSIDE, OPT_LINE(7), 0, m_defshade, 0, NULL, NULL, NULL},

    {DefButton(btn_markers,      0, "Markers"),         OPT_XS, OPT_LINE(8), 1, m_defshade, 0, NULL, MNU_HostingCheck, NULL},
    {DefButton(btn_teamplay,     0, "Team Play"),       OPT_XS, OPT_LINE(9), 1, m_defshade, 0, MNU_TeamPlayChange, MNU_TeamPlayCheck, NULL},
    {DefButton(btn_friendlyfire, 0, "Hurt Teammate"),   OPT_XS, OPT_LINE(10),1, m_defshade, 0, NULL, MNU_HurtTeammateCheck, NULL},
    {DefButton(btn_nuke,         0, "Play with Nuke"),  OPT_XS, OPT_LINE(11),1, m_defshade, 0, NULL, MNU_HostingCheck, NULL},
    {DefLayer(0, "Usermaps", &usermapgroup),            OPT_XS, OPT_LINE(12),1, m_defshade, 0, NULL, MNU_CheckMapStatus, NULL},
    {DefInert(0,  UserMapName),                      OPT_XSIDE, OPT_LINE(12),1, m_defshade, 0, NULL, NULL, NULL},

    //{DefOption(0, "Start Game..."),                     OPT_XS, OPT_LINE(14), pic_episode1, m_defshade, 0, MNU_StartMultiGame, MNU_CheckOption, NULL},
    {DefOption(0,  MultiOption),                        OPT_XS, OPT_LINE(14),1, m_defshade, 0, MNU_StartMultiGame, MNU_CheckOption, NULL},

    {DefNone}
};
MenuGroup networkgroup = {50, 5, "^Multiplay", network_i, pic_newgametitl, 0, m_defshade, MNU_DoPlayerName, NULL, 0};

MenuItem options_i[] =
    {
    {DefLayer(KEYSC_G, "^Game  Options",   &gamegroup),      OPT_XS+55,  41, 1, m_defshade,0,NULL, NULL, NULL},
    {DefLayer(KEYSC_S, "^Sound Options",   &soundgroup),     OPT_XS+55,  57, 1, m_defshade,0,NULL, MNU_MusicFxCheck, NULL},
    {DefLayer(KEYSC_V, "^Video Options",   &videogroup),     OPT_XS+55,  73, 1, m_defshade,0,NULL, NULL, NULL},
    {DefLayer(KEYSC_O, "^Mouse Options",   &mousegroup),     OPT_XS+55,  89, 1, m_defshade,0,NULL, MNU_MouseCheck, NULL},
    {DefLayer(KEYSC_M, "^Mouse Setup",     &mousesetupgroup),OPT_XS+55, 105, 1, m_defshade,0,NULL,NULL,NULL},
    {DefLayer(KEYSC_K, "^Keyboard Setup",  &keysetupgroup),  OPT_XS+55, 121, 1, m_defshade,0,NULL,NULL,NULL},
//{DefDisabled(KEYSC_J,"^Joystick Setup",  &keysetupgroup),  OPT_XS+55, 137, 1, m_defshade,0,NULL,NULL,NULL},
    {DefLayer(KEYSC_P, "^Parental Lock",   &parentalgroup),  OPT_XS+55, 137, 1, m_defshade,0,NULL, NULL, NULL},
    {DefLayer(KEYSC_U, "^Select Music",    &usermusicgroup), OPT_XS+55, 153, pic_hard, m_defshade,0,NULL, MNU_CheckStart, NULL},
    {DefNone}
    };
MenuGroup optiongroup = {100, 5, "^Options", options_i, pic_optionstitl, 0, m_defshade, NULL,NULL, 0};

BOOL MNU_ShareMapsCheck(MenuItem_p item);

MenuItem episode_i[] =
    {
    {DefLayer(KEYSC_S, &EpisodeNames[0][0], &skillgroup),   41,  54, pic_episode1, m_defshade, 0, MNU_EpisodeCustom, NULL, NULL},
    {DefLayer(KEYSC_F, &EpisodeNames[1][0], &skillgroup),   41,  87, pic_episode2, m_defshade, 0, MNU_EpisodeCustom, MNU_ShareWareCheck, MNU_ShareWareMessage},
    {DefLayer(KEYSC_U, &EpisodeNames[2][0], &usermapgroup), 41, 120, pic_episode2, m_defshade, 0, MNU_EpisodeCustom, MNU_ShareMapsCheck, MNU_ShareWareMessage},
    {DefNone}
    };

MenuGroup episodegroup = {100, 5, "^Episode", episode_i, pic_newgametitl, 0, m_defshade, MNU_DoEpisodeSelect, NULL, 0};

MenuItem load_i[] =
    {
#define SD_YSTART 26
#define SD_XSTART 5
#define SD_YOFF 13
#define SD_LINE(line) (SD_YSTART + (line * SD_YOFF))
    {DefOption(0, NULL), SD_XSTART, SD_LINE(0), 0, m_defshade, 0, MNU_GetLoadCustom, NULL, NULL},
    {DefOption(0, NULL), SD_XSTART, SD_LINE(1), 0, m_defshade, 0, MNU_GetLoadCustom, NULL, NULL},
    {DefOption(0, NULL), SD_XSTART, SD_LINE(2), 0, m_defshade, 0, MNU_GetLoadCustom, NULL, NULL},
    {DefOption(0, NULL), SD_XSTART, SD_LINE(3), 0, m_defshade, 0, MNU_GetLoadCustom, NULL, NULL},
    {DefOption(0, NULL), SD_XSTART, SD_LINE(4), 0, m_defshade, 0, MNU_GetLoadCustom, NULL, NULL},
    {DefOption(0, NULL), SD_XSTART, SD_LINE(5), 0, m_defshade, 0, MNU_GetLoadCustom, NULL, NULL},
    {DefOption(0, NULL), SD_XSTART, SD_LINE(6), 0, m_defshade, 0, MNU_GetLoadCustom, NULL, NULL},
    {DefOption(0, NULL), SD_XSTART, SD_LINE(7), 0, m_defshade, 0, MNU_GetLoadCustom, NULL, NULL},
    {DefOption(0, NULL), SD_XSTART, SD_LINE(8), 0, m_defshade, 0, MNU_GetLoadCustom, NULL, NULL},
    {DefOption(0, NULL), SD_XSTART, SD_LINE(9), 0, m_defshade, 0, MNU_GetLoadCustom, NULL, NULL},
    {DefOption(0, NULL), SD_XSTART, SD_LINE(10), 0, m_defshade, 0, MNU_GetLoadCustom, NULL, NULL},
    {DefNone}
    };

MenuItem save_i[] =
    {
    {DefOption(0, NULL), SD_XSTART, SD_LINE(0), 0, m_defshade, 0, MNU_GetSaveCustom, NULL, NULL},
    {DefOption(0, NULL), SD_XSTART, SD_LINE(1), 0, m_defshade, 0, MNU_GetSaveCustom, NULL, NULL},
    {DefOption(0, NULL), SD_XSTART, SD_LINE(2), 0, m_defshade, 0, MNU_GetSaveCustom, NULL, NULL},
    {DefOption(0, NULL), SD_XSTART, SD_LINE(3), 0, m_defshade, 0, MNU_GetSaveCustom, NULL, NULL},
    {DefOption(0, NULL), SD_XSTART, SD_LINE(4), 0, m_defshade, 0, MNU_GetSaveCustom, NULL, NULL},
    {DefOption(0, NULL), SD_XSTART, SD_LINE(5), 0, m_defshade, 0, MNU_GetSaveCustom, NULL, NULL},
    {DefOption(0, NULL), SD_XSTART, SD_LINE(6), 0, m_defshade, 0, MNU_GetSaveCustom, NULL, NULL},
    {DefOption(0, NULL), SD_XSTART, SD_LINE(7), 0, m_defshade, 0, MNU_GetSaveCustom, NULL, NULL},
    {DefOption(0, NULL), SD_XSTART, SD_LINE(8), 0, m_defshade, 0, MNU_GetSaveCustom, NULL, NULL},
    {DefOption(0, NULL), SD_XSTART, SD_LINE(9), 0, m_defshade, 0, MNU_GetSaveCustom, NULL, NULL},
    {DefNone}
    };

// No actual submenus for this, just quit text.
MenuGroup quitgroup = {0, 0, NULL, NULL, 0, 0, m_defshade, MNU_QuitCustom, NULL, 0};
MenuGroup quickloadgroup = {0, 0, NULL, NULL, 0, 0, m_defshade, MNU_QuickLoadCustom, NULL, 0};
MenuGroup ordergroup = {0, 0, NULL, NULL, 0, 0, m_defshade, MNU_OrderCustom, NULL, 0};

// save and load function calls
MenuGroup SaveGameGroup = {100, 5, "^Save Game", save_i, pic_savegame, 0, m_defshade, MNU_LoadSaveDraw, MNU_LoadSaveMove, 0};
MenuGroup LoadGameGroup = {100, 5, "^Load Game", load_i, pic_loadgame, 0, m_defshade, MNU_LoadSaveDraw, MNU_LoadSaveMove, 0};

#define MAIN_YSTART 38 // 44
#define MAIN_YOFF 18   // 17
#define MAIN_XSTART 75 //55
#define MAIN_LINE(line) (MAIN_YSTART + (MAIN_YOFF * line))

#define MAIN_MENU_COOL_STUFF "^Cool Stuff"
#define MAIN_MENU_HOW_TO_ORDER "^How to Order"

MenuItem main_i[] =
    {
    {DefLayer(KEYSC_N, "^New Game", &episodegroup),      MAIN_XSTART, MAIN_LINE(0), pic_newgame, m_defshade, 0, NULL, NULL, NULL},
    {DefLayer(KEYSC_L, "^Load Game", &LoadGameGroup),    MAIN_XSTART, MAIN_LINE(1), pic_load, m_defshade, 0, NULL, MNU_LoadGameCheck, NULL},
    {DefLayer(KEYSC_S, "^Save Game", &SaveGameGroup),    MAIN_XSTART, MAIN_LINE(2), pic_save, m_defshade, 0, NULL, MNU_SaveGameCheck, NULL},
    {DefLayer(KEYSC_O, "^Options", &optiongroup),        MAIN_XSTART, MAIN_LINE(3), pic_options, m_defshade, 0, NULL, NULL, NULL},
    {DefLayer(KEYSC_H, "^Oh Dear", &ordergroup),         MAIN_XSTART, MAIN_LINE(4), pic_orderinfo, m_defshade, 0, NULL, NULL, NULL},
    {DefLayer(KEYSC_M, "^Multiplay", &networkgroup),     MAIN_XSTART, MAIN_LINE(5), pic_optionstitl, m_defshade,0,NULL, MNU_CheckMulti, NULL},
    {DefLayer(KEYSC_Q, "^Quit", &quitgroup),             MAIN_XSTART, MAIN_LINE(6), pic_quit, m_defshade, 0, NULL, NULL, NULL},
    {DefNone}
    };

MenuGroup maingroup = {160, 15, NULL, main_i, pic_shadow_warrior, 0, m_defshade, NULL, NULL, 0};

CTLType ControlPanelType;

#define MaxLayers    10         // Maximum layers deep a menu can go
short       menuarrayptr;       // Index into menuarray
MenuGroup   *menuarray[MaxLayers], *currentmenu;
BOOL UsingMenus = FALSE;

#define MAXDIALOG    4          // Maximum number of dialog strings allowed wxas
char *dialog[MAXDIALOG];

// Global menu setting values ////////////////////////////////////////////////////////////////////
// Mouse slider vars
#define SENSE_MIN               75
#define SENSE_MUL               10
int SENSITIVITY = SENSE_MIN + (SENSE_DEFAULT * SENSE_MUL);

// Sound vars
#define FX_MIN                  0
#define MUSIC_MIN               0
#define VOL_MUL                 21 // 16

// User input data for all devices
UserInput mnu_input, mnu_input_buffered, order_input_buffered;

// Menu function call back pointer for multiplay menus
BOOL(*cust_callback) (UserCall call, MenuItem_p item);
UserCall cust_callback_call;
MenuItem_p cust_callback_item;

// Prototypes ///////////////////////////////////////////////////////////////////////////////////

static void MNU_ClearDialog(void);
static BOOL MNU_Dialog(void);
VOID LoadSaveMsg(char *msg);
static VOID MNU_ItemPreProcess(MenuGroup *group);
static void MNU_SelectItem(MenuGroup * group, short index, BOOL draw);
static void MNU_PushItem(MenuItem * item, BOOL draw);


// F U N C T I O N S ////////////////////////////////////////////////////////////////////////////

// CUSTOM ROUTINES ////////////////////////////////////////////////////////////////////////////////
// CTW REMOVED
/*
BOOL
MNU_Ten(void)
    {
    TEN_Setup();

    return(FALSE);
    }
*/
// CTW REMOVED END
BOOL MNU_DoEpisodeSelect(UserCall call, MenuItem * item)
{
    short w,h,x;
    char *extra_text;

    x = 1;
    if (SW_SHAREWARE)
        x = 16;

    extra_text = EpisodeSubtitles[0];
    MNU_MeasureString(extra_text, &w, &h);
    MNU_DrawString(35, 63, extra_text, 1, 16);

    extra_text = EpisodeSubtitles[1];
    MNU_MeasureString(extra_text, &w, &h);
    MNU_DrawString(35, 96, extra_text, 1, 16);

    extra_text = EpisodeSubtitles[2];
    MNU_MeasureString(extra_text, &w, &h);
    MNU_DrawString(35, 129, extra_text, x, 16);

    return(TRUE);
}

BOOL MNU_DoParentalPassword(UserCall call, MenuItem_p item)
    {
    short w,h;
    signed char MNU_InputString(char *, short);
    static BOOL cur_show;
    char TempString[80];
    char *extra_text;

    extra_text = "This mode should remove most of the offensive content.";
    MNU_MeasureSmallString(extra_text, &w, &h);
    MNU_DrawSmallString(TEXT_XCENTER(w), 70, extra_text, 1, 14);
    extra_text = "We still recommend you review the game prior to play.";
    MNU_MeasureSmallString(extra_text, &w, &h);
    MNU_DrawSmallString(TEXT_XCENTER(w), 80, extra_text, 1, 14);

    // get input
    if(MenuInputMode)
    {
    switch(MNU_InputString(MessageInputString, 80))
        {
        case -1: // Cancel Input (pressed ESC) or Err
            KB_ClearKeysDown();
            KB_FlushKeyboardQueue();
            MenuInputMode = FALSE;
            memset(MessageInputString, '\0', sizeof(MessageInputString));
            break;
        case FALSE: // Input finished (RETURN)
            if (MessageInputString[0] == '\0')
                {
                MenuInputMode = FALSE;
                KB_ClearKeysDown();
                KB_FlushKeyboardQueue();
                memset(MessageInputString, '\0', sizeof(MessageInputString));
                }
            else
                {
                MenuInputMode = FALSE;
                KB_ClearKeysDown();
                KB_FlushKeyboardQueue();

                if (gs.Password[0] != '\0' && passwordvalid == FALSE)
                   {
                   if (!Bstrcasecmp(gs.Password,MessageInputString))
                      {
                      passwordvalid = TRUE;
                      if (currentmenu->cursor == 0 && gs.ParentalLock == TRUE)
                         {
                         buttonsettings[btn_parental] = gs.ParentalLock = FALSE;
                         if (!InMenuLevel)
                         JS_ToggleLockouts();
                         }

                      if (currentmenu->cursor == 1) // Is it on the password line?
                         {
                         MenuInputMode = TRUE;
                         KB_ClearKeysDown();
                         KB_FlushKeyboardQueue();
                         }
                        //memset(gs.Password, '\0', sizeof(gs.Password));
                      }
                   }
                else
                   {
                   if (currentmenu->cursor == 1) // Is it on the password line?
                      {
                      strcpy(gs.Password,MessageInputString);
                      passwordvalid = FALSE;
                      }
                   }
                memset(MessageInputString, '\0', sizeof(MessageInputString));
                }
            break;
        case TRUE: // Got input
            break;
        }

        //CON_Message("Password = '%s'",gs.Password);
        //CON_Message("Passwordvalid = %d",passwordvalid);

        if(gs.Password[0] != '\0' && passwordvalid == FALSE && currentmenu->cursor == 1)
            {
            sprintf(TempString,"Enter Old Password");
            MNU_MeasureString(TempString, &w, &h);
            MNU_DrawString(TEXT_XCENTER(w), MESSAGE_LINE-10, TempString,1,mPal);
            }
        else
        if (passwordvalid == TRUE && currentmenu->cursor == 1)
            {
            sprintf(TempString,"Enter New Password");
            MNU_MeasureString(TempString, &w, &h);
            MNU_DrawString(TEXT_XCENTER(w), MESSAGE_LINE-10, TempString,1,mPal);
            }
        else
            {
            sprintf(TempString,"Enter Password");
            MNU_MeasureString(TempString, &w, &h);
            MNU_DrawString(TEXT_XCENTER(w), MESSAGE_LINE-10, TempString,1,mPal);
            }

        MNU_MeasureString(MessageInputString, &w, &h);

        cur_show ^= 1;
        if (cur_show)
            {
            MNU_DrawString(TEXT_XCENTER(w), MESSAGE_LINE, MessageInputString,1,16);
            rotatesprite((TEXT_XCENTER(w)+w+7)<<16,(MESSAGE_LINE+3)<<16,64<<9,0,COINCURSOR+((totalclock>>3)%7),0,0,MenuDrawFlags,0,0,xdim-1,ydim-1);
            }
        else
            {
            MNU_DrawString(TEXT_XCENTER(w), MESSAGE_LINE, MessageInputString,1,16);
            rotatesprite((TEXT_XCENTER(w)+w+7)<<16,(MESSAGE_LINE+3)<<16,64<<9,0,COINCURSOR+((totalclock>>3)%7),0,0,MenuDrawFlags,0,0,xdim-1,ydim-1);
            }

    }

    return(TRUE);
    }


BOOL MNU_ParentalCustom(void)
    {

    if (MenuInputMode)
        {
        // toggle edit mode
        MenuInputMode = FALSE;
        memset(MessageInputString, '\0', sizeof(MessageInputString));
        }
    else
        {
        // clear keyboard buffer
        while (KB_KeyWaiting())
            {
            if (KB_Getch() == 0)
                KB_Getch();
            }

        // toggle edit mode
        MenuInputMode = TRUE;
        }

    return (TRUE);
    }

BOOL MNU_KeySetupCustom(UserCall call, MenuItem *item)
{
	static int currentkey = 0, currentcol = 0;
	static int currentmode = 0;
	extern byte KeyboardKeys[NUMGAMEFUNCTIONS][2];

	if (call == uc_touchup)
        	return (TRUE);

	if (cust_callback == NULL)
	{
		if (call != uc_setup)
			return (FALSE);
		currentkey = 0;
		currentcol = 0;
		currentmode = 0;
	}

	cust_callback = MNU_KeySetupCustom;
	cust_callback_call = call;
	cust_callback_item = item;

	{
		short w, h = 0;
		char *s = "Keys Setup";
		rotatesprite(10 << 16, (5-3) << 16, MZ, 0, 2427,
		             m_defshade, 0, MenuDrawFlags|ROTATE_SPRITE_CORNER, 0, 0, xdim - 1, ydim - 1);
		MNU_MeasureStringLarge(s, &w, &h);
		MNU_DrawStringLarge(TEXT_XCENTER(w), 5, s);
	}

	if (currentmode)
	{
		// customising a key
		char *strs[] = { "Press the key to assign to", "\"%s\" %s", "or ESCAPE to cancel." };
		char *col[2] = { "(primary)", "(secondary)" };
		short w, h = 8;
		int i, j, y;

		if (KEY_PRESSED(KEYSC_ESC))
		{
			KB_ClearKeyDown(sc_Escape);
			currentmode = 0;
		}
		else if (KB_GetLastScanCode() > 0)
		{
			KB_ClearKeyDown( KB_GetLastScanCode() );

			KeyboardKeys[currentkey][currentcol] = KB_GetLastScanCode();
			if (currentkey != gamefunc_Show_Console)
			{
				CONTROL_MapKey(currentkey,
				KeyboardKeys[currentkey][0],
				KeyboardKeys[currentkey][1]);
			}
			else
			{
				OSD_CaptureKey(KB_GetLastScanCode());
			}
			currentmode = 0;
		}

		MNU_MeasureString(strs[0], &w, &h);

		y = (YDIM - (h+3) * SIZ(strs)) / 2;

		for (i=0; i<(int)SIZ(strs); i++)
		{
			w = 0;
			sprintf(ds,strs[i],gamefunctions[currentkey],col[currentcol]);
			for (j=0; ds[j]; j++) if (ds[j] == '_') ds[j] = ' ';
			MNU_MeasureString(ds, &w, &h);
			MNU_DrawString((XDIM - w)/2, y, ds, 0, 16);
			y += h+3;
		}

	}
	else
	{
		// key list
#define PGSIZ 14
		int topitem = 0, botitem = NUMGAMEFUNCTIONS;
		int i,j;
		char *morestr = "More...";
		const char *p;

		UserInput inpt = {FALSE,FALSE,dir_None};
		CONTROL_GetUserInput(&inpt);

		if (KEY_PRESSED(KEYSC_ESC) || inpt.button1)
		{
			KEY_PRESSED(KEYSC_ESC) = FALSE;
			cust_callback = NULL;
			return TRUE;
		}
		else
		if (KB_KeyPressed(sc_Delete))
		{
			KB_ClearKeyDown(sc_Delete);
			if (currentkey != gamefunc_Show_Console)
			{
				KeyboardKeys[currentkey][currentcol] = 0xff;
				CONTROL_MapKey(currentkey,
					KeyboardKeys[currentkey][0],
					KeyboardKeys[currentkey][1]);
			}
		}
		else
		if (KB_KeyPressed(sc_Home))
		{
			currentkey = 0;
			KB_ClearKeyDown(sc_Home);
		}
		else
		if (KB_KeyPressed(sc_End))
		{
			currentkey = NUMGAMEFUNCTIONS-1;
			KB_ClearKeyDown(sc_End);
		}
		else
		if (KB_KeyPressed(sc_PgDn))
		{
			currentkey += PGSIZ;
			if (currentkey >= NUMGAMEFUNCTIONS) currentkey = NUMGAMEFUNCTIONS-1;
			KB_ClearKeyDown(sc_PgDn);
		}
		else
		if (KB_KeyPressed(sc_PgUp))
		{
			currentkey -= PGSIZ;
			if (currentkey < 0) currentkey = 0;
			KB_ClearKeyDown(sc_PgUp);
		}
		else
		if (inpt.button0)
		{
			currentmode = 1;
			KB_ClearLastScanCode();
			KB_ClearKeysDown();
		}
		else
		if (inpt.dir == dir_North)
		    currentkey = max(0,currentkey-1);
		else
		if (inpt.dir == dir_South)
		    currentkey = min(NUMGAMEFUNCTIONS-1,currentkey+1);
		else
		if (inpt.dir == dir_East)
		    currentcol = 1;
		else
		if (inpt.dir == dir_West)
		    currentcol = 0;

		if (currentkey == gamefunc_Show_Console)
		    currentcol = 0;

		CONTROL_ClearUserInput(&inpt);

		if (NUMGAMEFUNCTIONS > PGSIZ)
		{
			topitem = currentkey - PGSIZ/2;
			botitem = topitem + PGSIZ;

			if (topitem < 0)
			{
				botitem += -topitem;
				topitem = 0;
			}
			else
			if (botitem >= NUMGAMEFUNCTIONS)
			{
				botitem = NUMGAMEFUNCTIONS-1;
				topitem = botitem - PGSIZ;
			}
		}

		for (i = topitem; i <= botitem; i++)
		{
			for (j = 0; gamefunctions[i][j]; j++)
			{
				if (gamefunctions[i][j] == '_')
				    ds[j] = ' ';
				else
				    ds[j] = gamefunctions[i][j];
			}
			ds[j] = 0;

			j = OPT_LINE(0)+(i-topitem)*8;                                    // start at 0
			// draw left message
			MNU_DrawSmallString(OPT_XS+45, j, ds, (i==currentkey)?0:12, 16);  // + 45 in

			p = getkeyname(KeyboardKeys[i][0]);
			if (!p || KeyboardKeys[i][0]==0xff)
			    p = "  -";
			// draw primary settings
			MNU_DrawSmallString(OPT_XSIDE, j, (char*)p, (i==currentkey)?-5:12,
					(i==currentkey && currentcol==0) ? 14:17);       // 14 = red 17 = silver

			if (i == gamefunc_Show_Console)
			    continue;

			p = getkeyname(KeyboardKeys[i][1]);
			if (!p || KeyboardKeys[i][1]==0xff)
			    p = "  -";
			// draw secondary settings
			MNU_DrawSmallString(OPT_XSIDE + 4*14, j, (char*)p, (i==currentkey)?-5:12,
					(i==currentkey && currentcol==1) ? 14:11);       // 11 = green
		}

		{
			short dx,dy;
			dx = 0, dy = 8;
			MNU_MeasureSmallString(morestr,&dx,&dy);
			if (topitem > 0)
				MNU_DrawSmallString(XDIM - OPT_XS - dx, OPT_LINE(1), morestr, 8,16);
			if (botitem < NUMGAMEFUNCTIONS-1)
				MNU_DrawSmallString(XDIM - OPT_XS - dx, OPT_LINE(1)+PGSIZ*8, morestr, 8,16);
		}
#undef PGSIZ
	}
	return (TRUE);
}

BOOL MNU_MouseSetupCustom(UserCall call, MenuItem *item)
{
	static int currentbut = 0, currentmode = 0, currentfunc = 0;
	extern int32 MouseFunctions[MAXMOUSEBUTTONS][2];
	const char *colnames[2] = { "", "Double " };
	const char *butnames[6] = { "Left", "Right", "Middle", "Thumb", "Wheel Down", "Wheel Up" };
#define FIRSTWHEEL 8
#define NUMBUTS (FIRSTWHEEL+2)

	if (call == uc_touchup)
        	return (TRUE);

	if (cust_callback == NULL) {
		if (call != uc_setup)
			return (FALSE);
		currentbut = 0;
		currentmode = 0;
		currentfunc = 0;
	}

	cust_callback = MNU_MouseSetupCustom;
	cust_callback_call = call;
	cust_callback_item = item;

	{
		short w, h = 0;
		char *s = "Mouse Setup";
		rotatesprite(10 << 16, (5-3) << 16, MZ, 0, 2427,
		    m_defshade, 0, MenuDrawFlags|ROTATE_SPRITE_CORNER, 0, 0, xdim - 1, ydim - 1);
		MNU_MeasureStringLarge(s, &w, &h);
		MNU_DrawStringLarge(TEXT_XCENTER(w), 5, s);
	}

	if (currentmode) {
		// customising a button
#define PGSIZ 9
		char *strs[] = { "Select the function to assign to", "%s%s", "or ESCAPE to cancel." };
		int topitem = 0, botitem = NUMGAMEFUNCTIONS-1;
		int i,j,y;
		short w,h=0;
		char *morestr = "More...";

		UserInput inpt = {FALSE,FALSE,dir_None};
		CONTROL_GetUserInput(&inpt);

		if (KEY_PRESSED(KEYSC_ESC) || inpt.button1) {
			KB_ClearKeyDown(sc_Escape);
			currentmode = 0;
		}
		else if (KB_KeyPressed(sc_Home)) {
			currentfunc = 0;
			KB_ClearKeyDown(sc_Home);
		}
		else if (KB_KeyPressed(sc_End)) {
			currentfunc = NUMGAMEFUNCTIONS-2;	// -2 because the last one is the console
			KB_ClearKeyDown(sc_End);
		}
		else if (KB_KeyPressed(sc_PgDn)) {
			currentfunc += PGSIZ;
			if (currentfunc >= NUMGAMEFUNCTIONS-1) currentfunc = NUMGAMEFUNCTIONS-2;
			KB_ClearKeyDown(sc_PgDn);
		}
		else if (KB_KeyPressed(sc_PgUp)) {
			currentfunc -= PGSIZ;
			if (currentfunc < 0) currentfunc = 0;
			KB_ClearKeyDown(sc_PgUp);
		}
		else if (inpt.button0) {
			MouseFunctions[ currentbut<FIRSTWHEEL ? currentbut/2 : currentbut-FIRSTWHEEL/2 ]
				[ currentbut<FIRSTWHEEL ? currentbut%2 : 0 ] = currentfunc;
			CONTROL_MapButton( currentfunc, currentbut<FIRSTWHEEL ? currentbut/2 : currentbut-FIRSTWHEEL/2,
					currentbut<FIRSTWHEEL ? currentbut%2 : 0, controldevice_mouse);
			currentmode = 0;
			KB_ClearLastScanCode();
			KB_ClearKeysDown();
		}
		else if (inpt.dir == dir_North) currentfunc = max(0,currentfunc-1);
		else if (inpt.dir == dir_South) currentfunc = min(NUMGAMEFUNCTIONS-2,currentfunc+1);

		CONTROL_ClearUserInput(&inpt);

		if (NUMGAMEFUNCTIONS-1 > PGSIZ) {
			topitem = currentfunc - PGSIZ/2;
			botitem = topitem + PGSIZ;

			if (topitem < 0) {
				botitem += -topitem;
				topitem = 0;
			} else if (botitem >= NUMGAMEFUNCTIONS-1) {
				botitem = NUMGAMEFUNCTIONS-2;
				topitem = botitem - PGSIZ;
			}
		}

		y = OPT_LINE(0);
		for (i=0; i<(int)SIZ(strs); i++) {
			w = 0;
			sprintf(ds,strs[i],colnames[currentbut<FIRSTWHEEL ? currentbut%2 : 0],
				butnames[currentbut<FIRSTWHEEL ? currentbut/2 : currentbut-FIRSTWHEEL/2]);
			for (j=0; ds[j]; j++) if (ds[j] == '_') ds[j] = ' ';
			MNU_MeasureString(ds, &w, &h);
			MNU_DrawString((XDIM - w)/2, y, ds, 0, 16);
			y += h;
		}

		for (i = topitem; i <= botitem; i++) {
			for (j = 0; gamefunctions[i][j]; j++) {
				if (gamefunctions[i][j] == '_') ds[j] = ' ';
				else ds[j] = gamefunctions[i][j];
			}
			ds[j] = 0;

			j = OPT_LINE(4)+(i-topitem)*8;
			MNU_DrawSmallString(130, j, ds, (i==currentfunc)?0:12, 16);
		}

		{
			short dx,dy;
			dx = 0, dy = 8;
			MNU_MeasureSmallString(morestr,&dx,&dy);
			if (topitem > 0)
				MNU_DrawSmallString(XDIM - OPT_XS - dx, OPT_LINE(4), morestr, 8,16);
			if (botitem < NUMGAMEFUNCTIONS-2)
				MNU_DrawSmallString(XDIM - OPT_XS - dx, OPT_LINE(4)+PGSIZ*8, morestr, 8,16);
		}
#undef PGSIZ
	} else {
		// button list
		int i,j;
		const char *morestr = "More...";
		const char *p;

		UserInput inpt = {FALSE,FALSE,dir_None};
		CONTROL_GetUserInput(&inpt);

		if (KEY_PRESSED(KEYSC_ESC) || inpt.button1) {
			KEY_PRESSED(KEYSC_ESC) = FALSE;
			cust_callback = NULL;
			return TRUE;
		}
		else if (KB_KeyPressed(sc_Delete)) {
			KB_ClearKeyDown(sc_Delete);
			MouseFunctions[ currentbut<FIRSTWHEEL ? currentbut/2 : currentbut-FIRSTWHEEL/2 ]
				[ currentbut<FIRSTWHEEL ? currentbut%2 : 0 ] = -1;
			CONTROL_MapButton( -1, currentbut<FIRSTWHEEL ? currentbut/2 : currentbut-FIRSTWHEEL/2,
					currentbut<FIRSTWHEEL ? currentbut%2 : 0, controldevice_mouse);
		}
		else if (KB_KeyPressed(sc_Home) || KB_KeyPressed(sc_PgUp)) {
			currentbut = 0;
			KB_ClearKeyDown(sc_Home);
			KB_ClearKeyDown(sc_PgUp);
		}
		else if (KB_KeyPressed(sc_End) || KB_KeyPressed(sc_PgDn)) {
			currentbut = NUMBUTS-1;
			KB_ClearKeyDown(sc_End);
			KB_ClearKeyDown(sc_PgDn);
		}
		else if (inpt.button0) {
			currentmode = 1;
			currentfunc = MouseFunctions[ currentbut<FIRSTWHEEL ? currentbut/2 : currentbut-FIRSTWHEEL/2 ]
				[ currentbut<FIRSTWHEEL ? currentbut%2 : 0 ];
			KB_ClearLastScanCode();
			KB_ClearKeysDown();
		}
		else if (inpt.dir == dir_North) currentbut = max(0,currentbut-1);
		else if (inpt.dir == dir_South) currentbut = min(NUMBUTS-1,currentbut+1);

		CONTROL_ClearUserInput(&inpt);

		for (i = 0; i < NUMBUTS; i++) {
			int x = i<FIRSTWHEEL ? i%2 : 0, y = i<FIRSTWHEEL ? i/2 : i-FIRSTWHEEL/2;
			sprintf(ds, "%s%s", colnames[x], butnames[y]);

            // text part
			j = OPT_LINE(1)+i*8;
            MNU_DrawSmallString(OPT_XS+70, j, ds, (i==currentbut)?0:12, 16);   //  + 45 in

			p = MouseFunctions[y][x] < 0 ? "  -" : gamefunctions[MouseFunctions[y][x]];
			for (x=0; p[x]; x++) ds[x] = (p[x] == '_' ? ' ' : p[x]);
			ds[x] = 0;
			// binding part
			MNU_DrawSmallString(OPT_XSIDE+20, j, ds, (i==currentbut)?-5:12,
					(i==currentbut) ? 14:17);                                  // 17
		}
	}

#undef FIRSTWHEEL
#undef NUMBUTS
	return (TRUE);
}

BOOL MNU_OrderCustom(UserCall call, MenuItem * item)
{
    static signed char on_screen = 0,last_screen = 0;
    UserInput order_input;
    static long limitmove=0;
    UserInput tst_input;
    BOOL select_held = FALSE;
    long zero = 0;
    static BOOL DidOrderSound = FALSE;
    short choose_snd;
    static int wanghandle;

    static short RegOrderScreen[] =
    {
        5262,
        5261,
        4979,
        5111,
        5118,
        5113,
        //5111,
        //5118,
        //4979,
        //5261,
        //5262

	5114		// JBF: for my credits
    };
    static short SWOrderScreen[] =
    {
        5262,
        5110,
        5112,
        5113,
        5111,
        5118,
        4979,

	5114		// JBF: for my credits
    };
    short *OrderScreen, OrderScreenSiz;

    if (SW_SHAREWARE)
    {
	OrderScreen = SWOrderScreen;
	OrderScreenSiz = SIZ(SWOrderScreen);
    }
    else
    {
	OrderScreen = RegOrderScreen;
	OrderScreenSiz = SIZ(RegOrderScreen);
    }

    // Ignore the special touchup calls
    if (call == uc_touchup)
        return (TRUE);

    if (cust_callback == NULL)
    {
        if (call != uc_setup)
            return (FALSE);
    }

    if (SW_SHAREWARE && on_screen == 0 && !DidOrderSound)
    {
        DidOrderSound = TRUE;
        choose_snd = STD_RANDOM_RANGE(1000);
        if (choose_snd > 500 && !FX_SoundActive(wanghandle))
            wanghandle = PlaySound(DIGI_WANGORDER1, &zero, &zero, &zero, v3df_dontpan);
        else
        if (!FX_SoundActive(wanghandle))
            wanghandle = PlaySound(DIGI_WANGORDER2, &zero, &zero, &zero, v3df_dontpan);
    }

    order_input.button0 = order_input.button1 = FALSE;
    order_input.dir = dir_None;

    // Zero out the input structure
    tst_input.button0 = tst_input.button1 = FALSE;
    tst_input.dir = dir_None;

    if (!select_held)
    {
        CONTROL_GetUserInput(&tst_input);
        //order_input_buffered.dir = tst_input.dir;
        // Support a few other keys too
        if (KEY_PRESSED(KEYSC_SPACE)||KEY_PRESSED(KEYSC_ENTER))
        {
            KEY_PRESSED(KEYSC_SPACE) = FALSE;
            KEY_PRESSED(KEYSC_ENTER) = FALSE;
            tst_input.dir = dir_South;
        }
    }

    if (order_input_buffered.button0 || order_input_buffered.button1 || order_input_buffered.dir != dir_None)
    {
        if (tst_input.button0 == order_input_buffered.button0 &&
           tst_input.button1 == order_input_buffered.button1 &&
           tst_input.dir == order_input_buffered.dir)
        {
            select_held = TRUE;
        }
        else
        {
            if (labs(totalclock - limitmove) > 7)
            {
                order_input.button0 = order_input_buffered.button0;
                order_input.button1 = order_input_buffered.button1;
                order_input.dir = order_input_buffered.dir;

                order_input_buffered.button0 = tst_input.button0;
                order_input_buffered.button1 = tst_input.button1;
                order_input_buffered.dir = tst_input.dir;

                limitmove = totalclock;
            }
        }
    }
    else
    {
        select_held = FALSE;
        order_input_buffered.button0 = tst_input.button0;
        order_input_buffered.button1 = tst_input.button1;
        order_input_buffered.dir = tst_input.dir;
    }

    if (!KEY_PRESSED(KEYSC_ESC))
    {
        cust_callback = MNU_OrderCustom;
        cust_callback_call = call;
        cust_callback_item = item;
    }
    else
    {
        KEY_PRESSED(KEYSC_ESC) = FALSE;
        cust_callback = NULL;
        DidOrderSound = FALSE;
        on_screen = 0;
        ExitMenus();
    }

    if (order_input.dir == dir_North)
    {
        on_screen--;
    }
    else
    if (order_input.dir == dir_South)
    {
        on_screen++;
    }
    else
    if (order_input.dir == dir_West)
    {
        on_screen--;
    }
    else
    if (order_input.dir == dir_East)
    {
        on_screen++;
    }

// CTW MODIFICATION
// I reversed the logic in here to allow the user to loop around.
// Yeah... I changed default behavior just because I wanted to.
// AND YOU CAN'T STOP ME SUCKER!!!
    if (on_screen < 0)
        on_screen = OrderScreenSiz-1;

    if (on_screen > OrderScreenSiz-1)
        on_screen = 0;
// CTW MODIFICATION END

    rotatesprite(0,0,RS_SCALE,0,OrderScreen[on_screen],0,0,
        (ROTATE_SPRITE_CORNER|ROTATE_SPRITE_SCREEN_CLIP|ROTATE_SPRITE_NON_MASK|ROTATE_SPRITE_IGNORE_START_MOST),
        0, 0, xdim-1, ydim-1);

    if (on_screen == OrderScreenSiz-1)
    {
	// Jonathon's credits page hack :-)

	static char *jtitle = "^Port Credits";
	static char *jtext[] = {
		"*GAME AND ENGINE PORT",
		" Jonathon \"JonoF\" Fowler",
		"-",
		"*\"POLYMOST\" 3D RENDERER",
		"*NETWORKING, OTHER CODE",
		" Ken \"Awesoken\" Silverman",
		"-",
		" Visit http://www.jonof.id.au/jfsw for the",
		" source code, latest news, and updates of this port.",
		"-",
		"-",
		"-",
		"-",
		"-",
		"-",
		" SWP version "SwpVer"",
        " by ProAsm - http://www.proasm.com/",
        " Modified by Hendricks266 - http://hendricks266.duke4.net/",
	};
	short dimx, dimy;
	int ycur = 54;
	unsigned ji;

	dimy = 0; MNU_MeasureString(jtitle, &dimx, &dimy);
	MNU_DrawString(160-(dimx>>1), ycur, jtitle, 0, 0);
	ycur += dimy + 8;

	for (ji = 0; ji < SIZ(jtext); ji++)
	{
		switch (jtext[ji][0])
		{
			case '-':
				ycur += 6;
				break;
			case '*':
				dimx = dimy = 0;
				MNU_MeasureString(&jtext[ji][1], &dimx, &dimy);
				MNU_DrawString(160-(dimx>>1), ycur, &jtext[ji][1], 0, 16);
				ycur += dimy+1;
				break;
			default:
				if (ji>0 && jtext[ji-1][0] == '*') ycur += 3;
				dimx = dimy = 0;
				MNU_MeasureSmallString(&jtext[ji][1], &dimx, &dimy);
				MNU_DrawSmallString(160-(dimx>>1), ycur, &jtext[ji][1], 0, 0);
				ycur += dimy+1;
				break;
		}
	}

	{
		static char *scroller[] =
		{
			"This program is free software; you can redistribute it",
			"and/or modify it under the terms of the GNU General",
			"Public License as published by the Free Software",
			"Foundation; either version 2 of the License, or (at your",
			"option) any later version.",
			"",
			"This program is distributed in the hope that it will be",
			"useful but WITHOUT ANY WARRANTY; without even the implied",
			"warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR",
			"PURPOSE. See the GNU General Public License (GPL.TXT) for",
			"more details.",
			"",
			"",
			"",
			"",
			"Thanks to these people for their input and contributions:",
			"",
			"Richard \"TerminX\" Gobeille,",
			"Par \"Parkar\" Karlsson", // "Pär \"Parkar\" Karlsson",
			"Ben \"ProAsm\" Smit",
			"",
			"and all those who submitted bug reports and ",
			"supported the project financially!",
			"",
			"",
			"--x--",
			"",
			"",
			"",
			""
		};
		const int numlines = SIZ(scroller);
		int m,i;
		for (m=0,i=(totalclock/104)%numlines; m<4; m++,i++) {
			if (i==numlines) i=0;
			dimx = dimy = 0;
			MNU_MeasureSmallString(scroller[i], &dimx, &dimy);
			MNU_DrawSmallString(160-(dimx>>1), 154+(m*7), scroller[i], 0, 8);
		}
	}
    }

    //KB_ClearKeysDown();

    return (TRUE);
    }

short EpisodeMenuSelection;

void ExitMenus(void)
{
    ControlPanelType = ct_mainmenu;
    UsingMenus = FALSE;

    if (LoadGameOutsideMoveLoop)
        return;

    ResumeGame();
    SetRedrawScreen(&Player[myconnectindex]);
}

BOOL MNU_StartGame(void)
    {
    PLAYERp pp = Player + screenpeek;
    int handle = 0;
    long zero = 0;
    extern char LevelSong[];

    // always assumed that a demo is playing

    ready2send = 0;
    Skill = currentmenu->cursor;

    if (!bMapmenu)
    {
       if (EpisodeMenuSelection >= 1)
           Level = 5;
       else
           Level = 1;

       UserMapName[0] = 0;
       LevelSong[0]   = 0;
       CurrMapName[0] = 0;

       if (numplayers <= 1)
       {
           sprintf(ds,"%sgamex.asv", svgame);
           DeleteAutosave(0, ds);
       }
       Autodone = 0;

       ExitMenus();
       DemoPlaying = FALSE;
       ExitLevel = TRUE;
       NewGame = TRUE;
       DemoMode = FALSE;
       CameraTestMode = FALSE;
    }

    if(Skill == 0)
        handle = PlaySound(DIGI_TAUNTAI3,&zero,&zero,&zero,v3df_none);
    else
    if(Skill == 1)
        handle = PlaySound(DIGI_NOFEAR,&zero,&zero,&zero,v3df_none);
    else
    if(Skill == 2)
        handle = PlaySound(DIGI_WHOWANTSWANG,&zero,&zero,&zero,v3df_none);
    else
    if(Skill == 3)
        handle = PlaySound(DIGI_NOPAIN,&zero,&zero,&zero,v3df_none);

    if (handle > FX_Ok)
        while(FX_SoundActive(handle))
			handleevents();

    if (bMapmenu)
    {
       bMapmenu = FALSE;
       MNU_MapStartUp();
    }

    if (UserMapName[0] == 0)
        checkforsaves("");

    return (TRUE);
}

VOID ResetMenuInput(VOID)
    {
    cust_callback = NULL;
    InputMode = FALSE;
    }


BOOL MNU_EpisodeCustom(void)
{
    EpisodeMenuSelection = currentmenu->cursor;

    return (TRUE);
}

BOOL MNU_QuitCustom(UserCall call, MenuItem_p item)
{
    int select;
    int ret;
    extern BOOL DrawScreen;

    // Ignore the special touchup calls
    if (call == uc_touchup)
        return (TRUE);

    if (cust_callback == NULL)
    {
        if (call != uc_setup)
            return (FALSE);

        memset(dialog, 0, sizeof(dialog));

        dialog[0] = S_QUITYN;
        dialog[1] = " ";
        if (bypass == 0)
            dialog[2] = "Press (X) for Startup Menu.";
        else
            dialog[2] = " ";
    }

    ret = MNU_Dialog();

    if (DrawScreen)
        return(TRUE);

    if (!ret)
    {
        if (!mnu_input.button1 && !KB_KeyPressed(sc_N))
        {
            cust_callback = MNU_QuitCustom;
            cust_callback_call = call;
            cust_callback_item = item;
        }
        else
        {
            cust_callback = NULL;
            ExitMenus();
        }
    }
    else
    {
        cust_callback = NULL;
        ExitMenus();
    }

    if (KB_KeyPressed(sc_Y) || KB_KeyPressed(sc_Enter) || KB_KeyPressed(sc_Space))
    {
        if (CommEnabled || CommPlayers >= 2)
        {
            MultiPlayQuitFlag = TRUE;
            ds[0] = PACKET_TYPE_MENU_LEVEL_QUIT;
            netbroadcastpacket(ds, 1);
            gNet.TimeLimitClock = 0;
        }
        QuitFlag = TRUE;
        StopSong();
        StopSound();
        ExitMenus();
    }

    if (bypass == 0 && KB_KeyPressed(sc_X))
    {
        if (CommEnabled || CommPlayers >= 2)
        {
            MultiPlayQuitFlag = TRUE;
            ds[0] = PACKET_TYPE_MENU_LEVEL_QUIT;
            netbroadcastpacket(ds, 1);
        }
        QuitFlag = TRUE;
        ShellFlag = TRUE;
        StopSound();
        ExitMenus();
    }

    KB_ClearKeysDown();

    return (TRUE);
}

BOOL MNU_QuickLoadCustom(UserCall call, MenuItem_p item)
{
    int select;
    extern BOOL ReloadPrompt;
    long bak;
    PLAYERp pp = Player + myconnectindex;
    extern short GlobInfoStringTime;
    extern BOOL DrawScreen;
    extern long Autotick;
    long ret;

    if (cust_callback == NULL)
    {
        Autodone = CheckAutosave();
        if (call != uc_setup)
            return (FALSE);
        memset(dialog, 0, sizeof(dialog));
        if (strlen(SaveGameDescr[QuickLoadNum]) > 0)
        {
           dialog[0] = "Load saved game";
           sprintf(QuickLoadDescrDialog,"\"%s\" (Y/N)?", SaveGameDescr[QuickLoadNum]);
           dialog[1] = QuickLoadDescrDialog;
           dialog[2] = " ";
           dialog[3] = " ";
           if (Autodone == 1 && numplayers <= 1)
               dialog[3] = "Press 'A' to load Autosaved game";
        }
        else
        {
           dialog[0] = " ";
           dialog[1] = " ";
           dialog[2] = " ";
           dialog[3] = " ";
           if (Autodone == 1 && numplayers <= 1)
               dialog[3] = "Load Autosaved Game (A/N)";
        }
    }

    // Ignore the special touchup calls
    if (call == uc_touchup)
        return (TRUE);

    ret = MNU_Dialog();

    if (DrawScreen)
    {
        return(TRUE);
    }

    if (ret == FALSE)
    {
        if (Autodone > 0 && (KB_KeyPressed(sc_A) || BUTTON(gamefunc_Open)))
        {
           cust_callback = NULL;
           KB_ClearKeysDown();
           LoadSaveMsg("Loading...");
           ReloadPrompt = FALSE;
           Autotick = 0;

           if (LoadGame(10) == -1)
           {
              ResumeAction();
              return (FALSE);
           }
           ready2send = 1;
           LastSaveNum = -1;
           strcpy(CurrMapName, UserMapName);

           KB_ClearKeysDown();
           ExitMenus();
        }
        else
        if (KB_KeyPressed(sc_N))
        {
            cust_callback = NULL;
            if (ReloadPrompt)
            {
                ReloadPrompt = FALSE;
                bak = GlobInfoStringTime;
                GlobInfoStringTime = 999;
                PutStringInfo2(pp, "Press SPACE to restart");
                GlobInfoStringTime = bak;
            }

            KB_ClearKeysDown();
            ExitMenus();
        }
        else
        {
            cust_callback = MNU_QuickLoadCustom;
            cust_callback_call = call;
            cust_callback_item = item;
        }
    }
    else
    {
        // N not pressed
        cust_callback = NULL;

        KB_ClearKeysDown();
        LoadSaveMsg("Loading...");

        PauseAction();

        ReloadPrompt = FALSE;
        if (LoadGame(QuickLoadNum) == -1)
        {
            ResumeAction();
            return (FALSE);
        }

        ready2send = 1;
        LastSaveNum = -1;

        // do a load game here
        KB_ClearKeysDown();
        ExitMenus();
    }

    KB_ClearKeysDown();

    return (TRUE);
}

// MENU FUNCTIONS /////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////
// Set some global menu related defaults
////////////////////////////////////////////////
void MNU_InitMenus(void)
{
    short i;
    short filternum[5] = {0,2,1,3,1};
    short tropynum[6]  = {1,2,4,8,16,1};

    pClearTextLine(Player + myconnectindex, TEXT_INFO_LINE(0));

    slidersettings[sldr_mouse]  = gs.MouseSpeed/(MOUSE_SENS_MAX_VALUE/SLDR_MOUSESENSEMAX);
    slidersettings[sldr_mousex] = gs.MouseScaleX/(MOUSE_X_MAX_VALUE/SLDR_MOUSESENSEMAX);
    slidersettings[sldr_mousey] = gs.MouseScaleY/(MOUSE_Y_MAX_VALUE/SLDR_MOUSESENSEMAX);

    slidersettings[sldr_musicvolume] = gs.MusicVolume / (MUSIC_VOL_MAX_VALUE/SLDR_MUSICVOLMAX);
    slidersettings[sldr_sndfxvolume] = gs.SoundVolume / (FX_VOL_MAX_VALUE/SLDR_SNDFXVOLMAX);

    if (gs.BorderNum > SLDR_SCRSIZEMAX)
        gs.BorderNum = SLDR_SCRSIZEMAX;
    slidersettings[sldr_scrsize] = gs.BorderNum;
    slidersettings[sldr_fov] = glfovscreen;
    slidersettings[sldr_bordertile] = gs.BorderTile;
    slidersettings[sldr_brightness] = gs.Brightness;
    slidersettings[sldr_textcl] = gs.MenuTextColor;
    slidersettings[sldr_hud] = gs.MiniHudTyp;
    slidersettings[sldr_filter] = 2;
    slidersettings[sldr_tropy] = 3;

    for (i=0; i<5; i++)
    {
        if (i < 4 && filternum[i] == gltexfiltermode)
        {
            slidersettings[sldr_filter] = i;
        }
        if (tropynum[i] == glanisotropy)
        {
            slidersettings[sldr_tropy] = i;
        }
    }

    {
	    int i,newx=xdim,newy=ydim;

	    buttonsettings[btn_videofs] = fullscreen;
	    buttonsettings[btn_videows] = glwidescreen;
	    buttonsettings[btn_hitile]  = usehightile;
	    buttonsettings[btn_models]  = usemodels;
	    buttonsettings[btn_txcach]  = glusetexcache;
	    buttonsettings[btn_txcomp]  = glusetexcompr;
	    buttonsettings[btn_cacomp]  = glusetexcachecompression;
	    buttonsettings[btn_galpha]  = usegoodalpha;
	    buttonsettings[btn_vsync]   = vsync;

	    UpdateValidModes(bpp,fullscreen);
	    for (i=0; i<numvalidbpps; i++)
		    if (validbpps[i] == bpp)
			    slidersettings[sldr_videobpp] = i;

	    i = checkvideomode(&newx, &newy, bpp, fullscreen, 1);
	    if (i != 0x7fffffff && i >= 0)
		    for (i=0; i<numvalidresolutions; i++)
			    if (validresolutions[i].xdim == newx && validresolutions[i].ydim == newy)
				    slidersettings[sldr_videores] = i;
    }

    buttonsettings[btn_auto_run] = gs.AutoRun;
    buttonsettings[btn_auto_aim] = gs.AutoAim;
    buttonsettings[btn_messages] = gs.Messages;
    buttonsettings[btn_crosshair] = gs.Crosshair;
    //buttonsettings[btn_bobbing] = gs.Bobbing;
    buttonsettings[btn_shadows] = gs.Shadows;

    buttonsettings[btn_mouse_aim] = gs.MouseAimingType;
    buttonsettings[btn_mouse_invert] = gs.MouseInvert;
    buttonsettings[btn_sound] = gs.FxOn;
    buttonsettings[btn_music] = gs.MusicOn;
    buttonsettings[btn_talking] = gs.Talking;

    buttonsettings[btn_voxels] = gs.Voxels;
    buttonsettings[btn_ambience] = gs.Ambient;
    buttonsettings[btn_playcd] = gs.PlayCD;
    buttonsettings[btn_flipstereo] = gs.FlipStereo;
    buttonsettings[btn_stats] = gs.Stats;
    buttonsettings[btn_wep] = gs.WeaponSwitch;
    buttonsettings[btn_str] = ForceSetup;
    buttonsettings[btn_drt] = gs.UseDarts;
    buttonsettings[btn_yin] = gs.SwapYinyang;
    buttonsettings[btn_hid] = gs.WeaponHide;
    buttonsettings[btn_nja] = gs.UseNinjaHack;
    buttonsettings[btn_car] = gs.UseCarHack;
    buttonsettings[btn_asv] = Autosave;
    buttonsettings[btn_rand] = RandomMusic;

    slidersettings[sldr_gametype] = gs.NetGameType;
    slidersettings[sldr_netlevel] = gs.NetLevel;
    slidersettings[sldr_monsters] = gs.NetMonsters;
    slidersettings[sldr_killlimit] = gs.NetKillLimit;
    slidersettings[sldr_timelimit] = gs.NetTimeLimit;
    slidersettings[sldr_playercolor] = gs.NetColor;

    buttonsettings[btn_nuke] = gs.NetNuke;
    buttonsettings[btn_markers] = gs.NetSpawnMarkers;
    buttonsettings[btn_teamplay] = gs.NetTeamPlay;
    buttonsettings[btn_friendlyfire] = gs.NetHurtTeammate;
    buttonsettings[btn_parental] = gs.ParentalLock;

    // Distinguish between Single or Multiplay for new game menu types
    if(numplayers > 1)
        main_i[0].child = &networkgroup;
    else
        main_i[0].child = &episodegroup;
    main_i[4].text = (SW_SHAREWARE) ? MAIN_MENU_HOW_TO_ORDER : MAIN_MENU_COOL_STUFF;
    main_i[4].hotkey = (SW_SHAREWARE) ? KEYSC_H : KEYSC_C;

    for (i=0; i<15; i++)
    {
        if (CommEnabled)
            maps_i[i].child = &networkgroup;
        else
            maps_i[i].child = &skillgroup;
    }
}

////////////////////////////////////////////////
// Measure the pixel width of a graphic string
////////////////////////////////////////////////
static char lg_xlat_num[] = {0,1,2,3,4,5,6,7,8,9};
#define FONT_LARGE_ALPHA 3706
#define FONT_LARGE_DIGIT 3732

void MNU_MeasureStringLarge(char *string, short *w, short *h)
{
    short ndx, width, height;
    char c;
    short pic;

    width = 0;
    height = *h;

    for (ndx = 0; (c = string[ndx]) != 0; ndx++)
    {
        if (isalpha(c))
        {
            c = toupper(c);
            pic = FONT_LARGE_ALPHA + (c - 'A');
        }
        else
        if (isdigit(c))
        {
            pic = FONT_LARGE_DIGIT + lg_xlat_num[(c - '0')];
        }
        else
        if (c == ' ')
        {
            width += 10;                 // Special case for space char
            continue;
        }
        else
        {
            continue;
        }

        width += tilesizx[pic]+1;
        if (height < tilesizy[pic])
            height = tilesizy[pic];
    }

    *w = width;
    *h = height;
}

////////////////////////////////////////////////
// Draw a string using a graphic font
////////////////////////////////////////////////
void MNU_DrawStringLarge(short x, short y, char *string)
{
    int ndx, offset;
    char c;
    short pic;

    offset = x;

    for (ndx = 0; (c = string[ndx]) != 0; ndx++)
    {
        if (isalpha(c))
        {
            c = toupper(c);
            pic = FONT_LARGE_ALPHA + (c - 'A');
        }
        else
        if (isdigit(c))
        {
            pic = FONT_LARGE_DIGIT + lg_xlat_num[(c - '0')];
        }
        else
        if (c == ' ')
        {
            offset += 10;
            continue;
        }
        else
        {
            continue;
        }

        rotatesprite(offset << 16, y << 16, MZ, 0, pic, MenuTextShade, 0, MenuDrawFlags|ROTATE_SPRITE_CORNER, 0, 0, xdim - 1, ydim - 1);
        offset += tilesizx[pic] + 1;
    }
}


////////////////////////////////////////////////
// Measure the pixel width of a graphic string
////////////////////////////////////////////////
void MNU_MeasureString(char *string, short *w, short *h)
{
    short ndx, width, height;
    char c;
    short ac;

    if (string[0] == '^')
    {
        MNU_MeasureStringLarge(&string[1], w, h);
        return;
    }

    width = 0;
    height = *h;

    for (ndx = 0; (c = string[ndx]) != 0; ndx++)
    {
        ac = c - '!' + STARTALPHANUM;
        if( (ac < STARTALPHANUM || ac > ENDALPHANUM)  && c != asc_Space )
            break;

        if (c > asc_Space && c < 127)
        {
            width += tilesizx[ac];
            if (height < tilesizy[ac])
                height = tilesizy[ac];
        }
        else
        if (c == asc_Space)
        {
            width += 4;                 // Special case for space char
        }
    }

    *w = width;
    *h = height;
}

////////////////////////////////////////////////
// Draw a string using a graphic font
//
// MenuTextShade and MenuDrawFlags
////////////////////////////////////////////////
void MNU_DrawString(short x, short y, char *string, short shade, short pal)
{
    int ndx, offset, i;
    char c;
    short ac;

    if (string[0] == '^')
    {
        MNU_DrawStringLarge(x,y, &string[1]);
        return;
    }

    if (pal == mPal)
       {
       if (!gs.SetHighres || !usehightile || gs.MenuTextColor == 0 || NoHrp > 0)
           pal = 16;
       }

    offset = x;

    for (ndx = 0; (c = string[ndx]) != 0; ndx++)
    {
        ac = c - '!' + STARTALPHANUM;
        if ((ac < STARTALPHANUM || ac > ENDALPHANUM)  && c != asc_Space )
            break;

        if (c > asc_Space && c < 127)
        {
            rotatesprite(offset<<16,y<<16,MZ,0,ac, shade, pal, MenuDrawFlags, 0, 0, xdim - 1, ydim - 1);
            offset += tilesizx[ac];
        } else
        if (c == asc_Space)
        {
            offset += 4;                // Special case for space char
        }
    }

}

////////////////////////////////////////////////
// Measure the pixel width of a small font string
////////////////////////////////////////////////
void MNU_MeasureSmallString(char *string, short *w, short *h)
    {
    short ndx, width, height;
    char c;
    short ac;

    width = 0;
    height = *h;

    for (ndx = 0; (c = string[ndx]) != 0; ndx++)
        {
        ac = (c - '!') + 2930;
        if( (ac < 2930 || ac > 3023) && c != asc_Space )
            break;

        if (c > asc_Space && c < 127)
            {
            width += tilesizx[ac];
            if (height < tilesizy[ac])
                height = tilesizy[ac];
            }
        else
        if (c == asc_Space)
            width += 4;                 // Special case for space char
        }

    *w = width;
    *h = height;
    }

////////////////////////////////////////////////
// Draw a string using a small graphic font
////////////////////////////////////////////////
void MNU_DrawSmallString(short x, short y, char *string, short shade, short pal)
{
    int ndx;
    char c;
    short ac,offset;


    offset = x;

    for (ndx = 0; (c = string[ndx]) != 0; ndx++)
    {
        ac = c - '!' + 2930;
        if( (ac < 2930 || ac > 3023)  && c != asc_Space )
            break;

        if (c > asc_Space && c < 127)
        {
           rotatesprite(offset<<16,y<<16,MZ,0,ac, shade, pal, MenuDrawFlags, 0, 0, xdim - 1, ydim - 1);
           offset += tilesizx[ac];

        }
        else
        if (c == asc_Space)
        {
            offset += 4;                // Special case for space char
        }
    }

}

////////////////////////////////////////////////
//  Get an input string from user using small font
////////////////////////////////////////////////

signed char MNU_InputSmallString(char *name, short pix_width)
    {
    char ch;
    short w, h;
    UserInput con_input;

#define ascii_backspace 8
#define ascii_esc 27
#define ascii_return 13

    if (!MoveSkip4 && !MessageInputMode)
        {
        con_input.dir = dir_None;
        CONTROL_GetUserInput(&con_input);

        if (con_input.dir == dir_North)
            {
            CON_CommandHistory(1);
            }
        else
        if (con_input.dir == dir_South)
            {
            CON_CommandHistory(-1);
            }
        }

    while (KB_KeyWaiting())
        {
        ch = KB_Getch();

        // skip any extended key
        if (ch == 0)
            {
            ch = KB_Getch();
            if (ch == 104) // extended enter
                ch = ascii_return;
            else
                continue;
            }

        if (ch == ascii_backspace)
            {
            name[strlen(name) - 1] = '\0';
            continue;
            }
        else
        if (ch == ascii_esc)
            {
            return (-1);
            }
        else
        if (ch == ascii_return)
            {
            return (FALSE);
            }
        else
        if (!isprint(ch))
            continue;

        MNU_MeasureSmallString(name, &w, &h);
        if (w < pix_width)
            {
            if(strlen(name) < 256) // Dont let it go too far!
                sprintf(name, "%s%c", name, ch);
            }
        }

    return (TRUE);

    }

////////////////////////////////////////////////
// Draw dialog text on screen
////////////////////////////////////////////////
static BOOL MNU_Dialog(void)
    {
    short ndx, linecnt, w[MAXDIALOG], h, x, y;

    linecnt = 0;
    h = 8;

    for (ndx = 0; ndx < MAXDIALOG && dialog[ndx]; ndx++)
        {
        MNU_MeasureString(dialog[ndx], &w[ndx], &h);
        ASSERT(w[ndx] < XDIM);
        linecnt++;
        }

    y = ((YDIM - ((h * linecnt) + (linecnt * 2))) / 2);

    for (ndx = 0; ndx < linecnt; ndx++)
        {
        x = ((XDIM - w[ndx]) / 2);
        MNU_DrawString(x, y, dialog[ndx],1,16);
        y += (h + 3);
        }

    mnu_input.button0 = mnu_input.button1 = FALSE;
    CONTROL_ClearUserInput(&mnu_input);
    CONTROL_GetUserInput(&mnu_input);

    if (KB_KeyPressed(sc_Y) || KB_KeyPressed(sc_Space) || KB_KeyPressed(sc_Enter))  // quit/load F9 wxyk
        return (TRUE);
    else
        return (FALSE);
}

////////////////////////////////////////////////
//  Get an input string from user
////////////////////////////////////////////////

signed char MNU_InputString(char *name, short pix_width)
    {
    char ch;
    short w, h;

#define ascii_backspace 8
#define ascii_esc 27
#define ascii_return 13

    while (KB_KeyWaiting())
        {
        ch = KB_Getch();

        ////DSPRINTF(ds, "%c %d", ch, ch);
        //MONO_PRINT(ds);

        // skip most extended keys
        if (ch == 0)
            {
            ch = KB_Getch();

            ////DSPRINTF(ds, "extended key %c %d", ch, ch);
            //MONO_PRINT(ds);

            if (ch == 104) // extended enter
                ch = ascii_return;
            else
                continue;
            }

        if (ch == ascii_backspace)
            {
            name[strlen(name) - 1] = '\0';
            continue;
            }
        else
        if (ch == ascii_esc)
            {
            return (-1);
            }
        else
        if (ch == ascii_return)
            {
            return (FALSE);
            }
        else
        if (!isprint(ch))
            continue;

        MNU_MeasureString(name, &w, &h);
        if (w < pix_width)
            {
            sprintf(name, "%s%c", name, ch);
            }
        }

    return (TRUE);

    }

#define SS_XSTART 146L
#define SS_YSTART SD_YSTART
#define SS_BORDER_SIZE 5L

VOID LoadSaveMsg(char *msg)
    {
    short w,h;

    flushperms();
    DrawMenuLevelScreen();
    strcpy((char*)ds, (char*)msg);
    MNU_MeasureString(ds, &w, &h);
    MNU_DrawString(TEXT_XCENTER(w), 170, ds, 1, 16);
    nextpage();
    }


////////////////////////////////////////////////
//  Load Game menu
//  This function gets called whenever you
//  press enter on one of the load game
//  spots.
//  I'm figuring it need to do the following:
//  . Load the game if there is one by calling: MNU_LoadGameCustom.
////////////////////////////////////////////////
BOOL MNU_GetLoadCustom(void)
{
    short load_num;

    load_num = currentmenu->cursor;

    // no saved game exists - don't do anything
    if (SaveGameDescr[load_num][0] == '\0')
        return (FALSE);

    if (InMenuLevel || DemoMode || DemoPlaying)
    {
        LoadSaveMsg("Loading...");

        if (LoadGame(load_num) == -1)
            return (FALSE);

        if (load_num < 10)
            QuickLoadNum = load_num;

        strcpy(CurrMapName, UserMapName);

        ExitMenus();
        ExitLevel = TRUE;
        LoadGameOutsideMoveLoop = TRUE;
        if (DemoMode || DemoPlaying)
            LoadGameFromDemo = TRUE;

        return(TRUE);
    }

    LoadSaveMsg("Loading...");

    PauseAction();

    if (LoadGame(load_num) == -1)
    {
        ResumeAction();
        return (FALSE);
    }

    if (load_num < 10)
        QuickLoadNum = load_num;

    strcpy(CurrMapName, UserMapName);

    ready2send = 1;
    LastSaveNum = -1;
    ExitMenus();

    if (DemoMode)
    {
        ExitLevel = TRUE;
        DemoPlaying = FALSE;
    }

    return (TRUE);
}

////////////////////////////////////////////////
//  Save Game menu
//  This function gets called whenever you
//  press enter on one of the load game
//  spots.
//  I'm figuring it need to do the following:
//  . Call MNU_GetInput to allow string input of description.
//  . Save the game if there is one by calling: MNU_SaveGameCustom.
////////////////////////////////////////////////
BOOL MNU_GetSaveCustom(void)
{
    short save_num;
    extern BOOL InMenuLevel, LoadGameOutsideMoveLoop;

    save_num = currentmenu->cursor;

    if (InMenuLevel)
        return(FALSE);

    if (MenuInputMode)
    {
        PauseAction();

        LoadSaveMsg("Saving...");

        if (SaveGame(save_num) != -1)
        {
            QuickLoadNum = save_num;
            LoadGameGroup.cursor = save_num;
            LastSaveNum = -1;
        }

        ResumeAction();
        ExitMenus();

        // toggle edit mode
        MenuInputMode = FALSE;
    }
    else
    {
        strcpy(BackupSaveGameDescr, SaveGameDescr[save_num]);

        // clear keyboard buffer
        while (KB_KeyWaiting())
        {
            if (KB_Getch() == 0)
                KB_Getch();
        }

        // toggle edit mode
        MenuInputMode = TRUE;
    }

    return (TRUE);
}

////////////////////////////////////////////////
//  Load/Save Touchup function
//  This function gets called each frame by DrawMenus
////////////////////////////////////////////////

static BOOL MNU_DrawLoadSave(short game_num)
{
    // screenshot border
    rotatesprite(SS_XSTART << 16, SS_YSTART << 16, MZ, 0, pic_loadsavescreen,
        0, 0, MenuDrawFlags | ROTATE_SPRITE_CORNER, 0, 0, xdim - 1, ydim - 1);

    // description box
    rotatesprite((SD_XSTART) << 16, (SD_YSTART) << 16, MZ, 0, pic_savedescr,
        0, 0, MenuDrawFlags | ROTATE_SPRITE_CORNER, 0, 0, xdim - 1, ydim - 1);

    if (menucmp(currentmenu->text, "^Load Game") == 0)
        rotatesprite((SD_XSTART) << 16, (SD_YSTART + 13) << 16, MZ, 0, pic_savedescr,
        0, 0, MenuDrawFlags | ROTATE_SPRITE_CORNER, 0, 0, xdim - 1, ydim - 1);

    // cursor for text boxes
    rotatesprite((SD_XSTART + 3) << 16, (SD_LINE(game_num) + 1) << 16, MZ, 0, pic_loadsavecursor,
        0, 0, MenuDrawFlags | ROTATE_SPRITE_CORNER, 0, 0, xdim - 1, ydim - 1);

    return(TRUE);
}

static char SaveGameInfo1[80];
static char SaveGameInfo2[80];
static char SaveGameInfo3[80];
static char SaveGameInfo4[80];
static char SaveGameInfo5[80];

BOOL MNU_LoadSaveMove(UserCall call, MenuItem_p item)
{
    short i;
    short game_num;
    short tile;
    static short SaveGameEpisode, SaveGameLevel, SaveGameSkill;
    char SaveGameMap[40];
    BOOL GotInput = FALSE;

    if (!UsingMenus)
        return(TRUE);

    game_num = currentmenu->cursor;

    // read all descr first time through - LastSaveNum starts at 99
    if (LastSaveNum == 99)
    {
        memset(SaveGameDescr, '\0', sizeof(SaveGameDescr));
        for (i = 0; i < 11; i++)                                     //wxas
             LoadGameDescr(i, SaveGameDescr[i]);
    }

    // cursor has moved - read header
    if (game_num != LastSaveNum)
    {
        screen_tile = LoadGameFullHeader(game_num, SaveGameDescr[game_num],
                      &SaveGameLevel, &SaveGameSkill, SaveGameMap);

        sprintf(SaveGameInfo1, "Map: %s", SaveGameMap);
        sprintf(SaveGameInfo2, "Lev: %d  /  Skill: %d", SaveGameLevel, SaveGameSkill+1);
        SaveGameInfo3[0] = 0;
    }

    if (QuickSaveMode)
    {
        QuickSaveMode = FALSE;
        MenuInputMode = TRUE;
        strcpy(BackupSaveGameDescr, SaveGameDescr[game_num]);
        KB_ClearKeysDown();
        KB_FlushKeyboardQueue();
    }

    LastSaveNum = game_num;

    // input mode check
    if (MenuInputMode)
    {
        MenuItem *item = &currentmenu->items[currentmenu->cursor];

        if (SavePrompt)
        {
            if (KB_KeyPressed(sc_Y) || KB_KeyPressed(sc_Enter))
            {
		        KB_ClearKeyDown(sc_Y);
		        KB_ClearKeyDown(sc_Enter);
                SavePrompt = FALSE;
                // use input
                item->custom();
            }
            else
            if (KB_KeyPressed(sc_N) || KB_KeyPressed(sc_Escape))
            {
		        KB_ClearKeyDown(sc_N);
                strcpy(SaveGameDescr[game_num], BackupSaveGameDescr);
                SavePrompt = FALSE;
                MenuInputMode = FALSE;
            }
        }
        else
        // get input
        switch (MNU_InputString(SaveGameDescr[game_num], 114))
        {
        case -1:                        // Cancel Input (pressed ESC) or Err
            strcpy(SaveGameDescr[game_num], BackupSaveGameDescr);
            MenuInputMode = FALSE;
            KB_ClearKeysDown();
            break;
        case FALSE:                     // Input finished (RETURN)
            // no input
            if (SaveGameDescr[game_num][0] == '\0')
            {
                strcpy(SaveGameDescr[game_num], BackupSaveGameDescr);
                MenuInputMode = FALSE;
            }
            else
            {
                GotInput = TRUE;
            }
	    KB_ClearKeyDown(sc_Enter);
            break;
        case TRUE:                      // Got input
            break;
        }

        if (GotInput)
        {
            if (BackupSaveGameDescr[0])
                SavePrompt = TRUE;

            if (!SavePrompt)
            {
                // use input
                item->custom();
            }
        }
    }
    return (TRUE);
}

BOOL MNU_LoadSaveDraw(UserCall call, MenuItem_p item)
{
    short i, j;
    short game_num;
    short tile;
    long  second_tics;
    extern long PlayClock;

    game_num = currentmenu->cursor;

    // misc drawing
    MNU_DrawLoadSave(game_num);

    if (CurrMapName[0])
    {
       sprintf(SaveGameInfo3, "Current Map: %s", CurrMapName );
       MNU_DrawString(SS_XSTART - 97-40, SS_YSTART + 100 + 45, SaveGameInfo3, 1, 23);
       second_tics = (PlayClock/120);
       sprintf(SaveGameInfo4, "Time: %d mins - %d secs", (second_tics/60), (second_tics%60));
       MNU_DrawString(SS_XSTART - 38-40, SS_YSTART + 100 + 55, SaveGameInfo4, 1, 23);
       if (strlen(LevelInfo[Level].Description) > 3)
       {
          sprintf(SaveGameInfo5, "Title: %s", LevelInfo[Level].Description );
          MNU_DrawString(SS_XSTART - 40-40, SS_YSTART + 100 + 65, SaveGameInfo5, 1, 23);
       }
    }

    // print game descriptions
    j = 10;
    if (menucmp(currentmenu->text, "^Load Game") == 0)
        j = 11;
    for (i = 0; i < j; i++)
    {
        if (i == game_num && MenuInputMode && !SavePrompt)
        {
            static BOOL cur_show;
            char tmp[sizeof(SaveGameDescr[0])];

            //cur_show ^= 1;
	        cur_show = (totalclock & 32);
            if (cur_show)
            {
                // add a cursor to the end
                sprintf(tmp, "%s_", SaveGameDescr[i]);
            }
            else
                strcpy(tmp, SaveGameDescr[i]);

            MNU_DrawString(SD_XSTART + 4, SD_YSTART + (i * SD_YOFF) + 2, tmp, 1, 16);
        }
        else                             // force update display   wxup
        if (SaveGameDescr[i][0] != '\0')
        {
            MNU_DrawString(SD_XSTART + 4, SD_YSTART + (i * SD_YOFF) + 2, SaveGameDescr[i], 1, 16);
        }
    }

    if (screen_tile != -1)
    {
        // draw 160x100 saved screen
        rotatesprite((SS_XSTART + SS_BORDER_SIZE) << 16, (SS_YSTART + SS_BORDER_SIZE) << 16, (1 << 16), 0 + 512,
                      screen_tile, 0, 0, MenuDrawFlags | ROTATE_SPRITE_CORNER | ROTATE_SPRITE_NON_MASK |
                      ROTATE_SPRITE_YFLIP, 0, 0, xdim - 1, ydim - 1);

        // draw info string
        MNU_DrawString(SS_XSTART + 4, SS_YSTART + 100 + 12, SaveGameInfo1, 1, 13);
        MNU_DrawString(SS_XSTART + 4, SS_YSTART + 100 + 22, SaveGameInfo2, 1, 13);

        if (SavePrompt)
        {
            MNU_DrawString(SS_XSTART + SS_BORDER_SIZE + 5, SS_YSTART + SS_BORDER_SIZE + 47, "Overwrite previous", 1, 16);
            MNU_DrawString(SS_XSTART + SS_BORDER_SIZE + 5, SS_YSTART + SS_BORDER_SIZE + 47 + 12, "  saved game (Y/N)", 1, 16);
        }
    }
    else
    {
        // draw 160x100 black pic
        rotatesprite((SS_XSTART + SS_BORDER_SIZE) << 16, (SS_YSTART + SS_BORDER_SIZE) << 16, (1 << 16), 0, pic_loadsavescreenbak,
            0, 0, MenuDrawFlags | ROTATE_SPRITE_CORNER | ROTATE_SPRITE_NON_MASK, 0, 0, xdim - 1, ydim - 1);

        MNU_DrawString(SS_XSTART + SS_BORDER_SIZE + 60, SS_YSTART + SS_BORDER_SIZE + 47, "Empty", 1, 16);
    }
    return (TRUE);
}

BOOL MNU_ShareWareCheck(MenuItem *item)
{
    if (SW_SHAREWARE)
    {
        SET(item->flags, mf_disabled);
    }

    return (TRUE);
}

BOOL MNU_CheckUserMap(MenuItem *item)
{
    if (UserMapName[0] == '\0')
        RESET(item->flags, mf_disabled);
    else
        SET(item->flags, mf_disabled);
    if (CommEnabled && !bHosting)
        SET(item->flags, mf_disabled);
    return (TRUE);
}

BOOL MNU_ShareWareMessage(MenuItem *item)
{
    char *extra_text;
    short w,h;

    if (SW_SHAREWARE)
    {
        extra_text = "Be sure to call 800-3DREALMS today";
        MNU_MeasureString(extra_text, &w, &h);
        MNU_DrawString(TEXT_XCENTER(w), 110, extra_text, 1, 16);
        extra_text = "and order the game.";
        MNU_MeasureString(extra_text, &w, &h);
        MNU_DrawString(TEXT_XCENTER(w), 120, extra_text, 1, 16);
        extra_text = "You are only playing the first ";
        MNU_MeasureString(extra_text, &w, &h);
        MNU_DrawString(TEXT_XCENTER(w), 130, extra_text, 1, 16);
        extra_text = "four levels, and are missing most";
        MNU_MeasureString(extra_text, &w, &h);
        MNU_DrawString(TEXT_XCENTER(w), 140, extra_text, 1, 16);
        extra_text = "of the game, weapons and monsters.";
        MNU_MeasureString(extra_text, &w, &h);
        MNU_DrawString(TEXT_XCENTER(w), 150, extra_text, 1, 16);
        extra_text = "See the ordering information.";
        MNU_MeasureString(extra_text, &w, &h);
        MNU_DrawString(TEXT_XCENTER(w), 160, extra_text, 1, 16);
        SET(item->flags, mf_disabled);
    }
    return (TRUE);
}

BOOL MNU_SaveGameCheck(MenuItem *item)
{
    extern BOOL InMenuLevel;
    extern BOOL DemoMode;

    if (0)
    {	// JBF: Until we fix the symbol table dilemma, saving is off limits
        SET(item->flags, mf_disabled);
        return(TRUE);
    }

    if (CommEnabled || numplayers > 1 || DemoMode)
    {
        SET(item->flags, mf_disabled);
        return(TRUE);
    }

    if (InMenuLevel)
        SET(item->flags, mf_disabled);
    else
    {
        if (TEST(Player[myconnectindex].Flags, PF_DEAD))
            SET(item->flags, mf_disabled);
        else
            RESET(item->flags, mf_disabled);
    }
    return (TRUE);
}

BOOL MNU_TenCheck(MenuItem *item)
{
    if (CommEnabled || numplayers > 1)
    {
        SET(item->flags, mf_disabled);
        return(TRUE);
    }

    return (TRUE);
}

BOOL MNU_LoadGameCheck(MenuItem *item)
{

    if (0)
    {	// JBF: Until we fix the symbol table dilemma, loading is off limits
        SET(item->flags, mf_disabled);
        return(TRUE);
    }

    if (CommEnabled || numplayers > 1)
    {
        SET(item->flags, mf_disabled);
        return(TRUE);
    }

    return (TRUE);
}

BOOL MNU_StatCheck(MenuItem *item)
{
    if (CommEnabled || numplayers > 1)
    {
        SET(item->flags, mf_disabled);
        return(TRUE);
    }

    return (TRUE);
}

BOOL MNU_HurtTeammateCheck(MenuItem *item)
{
    switch (gs.NetGameType+1)
    {
        // deathmatch and deathmatch no respawn
        case MULTI_GAME_COMMBAT:
        case MULTI_GAME_COMMBAT_NO_RESPAWN:
            if (gs.NetTeamPlay)
                RESET(item->flags, mf_disabled);
            else
                SET(item->flags, mf_disabled);
            break;
        // co-op
        case MULTI_GAME_COOPERATIVE:
            RESET(item->flags, mf_disabled);
            break;
    }
    if (CommEnabled && !bHosting)
        SET(item->flags, mf_disabled);

    return (TRUE);
}

BOOL MNU_TeamPlayCheck(MenuItem *item)
{
    switch (gs.NetGameType+1)
    {
        // co-op
        case MULTI_GAME_COOPERATIVE:
            SET(item->flags, mf_disabled);
            break;
        default:
            RESET(item->flags, mf_disabled);
            break;
    }
    if (CommEnabled && !bHosting)
        SET(item->flags, mf_disabled);

    return (TRUE);
}

BOOL MNU_CoopPlayCheck(MenuItem *item)
{
    switch (gs.NetGameType+1)
    {
        // co-op
        case MULTI_GAME_COOPERATIVE:
            SET(item->flags, mf_disabled);
            break;
        default:
            RESET(item->flags, mf_disabled);
            break;
    }
    if (CommEnabled && !bHosting)
        SET(item->flags, mf_disabled);

    return (TRUE);
}

BOOL MNU_TeamPlayChange(MenuItem *item)
{
    if (CommEnabled && !bHosting)
        SET(item->flags, mf_disabled);
    // if team play changes then do a pre process again
    MNU_ItemPreProcess(currentmenu);
    return (TRUE);
}

BOOL MNU_MouseCheck(MenuItem *item)
{
    if (!CONTROL_MousePresent)
    {
        SET(item->flags, mf_disabled);
    }
    else
    {
        RESET(item->flags, mf_disabled);
    }

    return (TRUE);
}

// This is only called when Enter is pressed
static BOOL MNU_TryMusicInit(void)
{
    if (!cdvalid)
    {
        if (gs.PlayCD)
        {
           enabled = TRUE; // Let it in to try and reset
           cdvalid = TRUE;
           CDAudio_Init();
        }
        if (cdvalid)
        {
            if (currentmenu->cursor == 0)
                MNU_MusicCheck(&currentmenu->items[currentmenu->cursor+1]);
            //CDAudio_Play(RedBookSong[Level], TRUE);
        }
    }
    return (TRUE);
}

BOOL MNU_MusicCheck(MenuItem *item)
{

    if (gs.MusicOn)
    {
       if (MusicDevice < 0 || !MusicInitialized)
       {
          SET(item->flags, mf_disabled);
       }
       else
       {
          RESET(item->flags, mf_disabled);
       }
    }
/*
    else
    {
       // Redbook audio stuff
       if (!cdvalid)
       {
          SET(item->flags, mf_disabled); // Just don't let CD Redbook ever be invalid!
       }
       else
       {
         RESET(item->flags, mf_disabled);
       }
    }
*/
    return (TRUE);
}

BOOL MNU_FxCheck(MenuItem *item)
{
    if (FXDevice < 0 || !FxInitialized)
    {
        SET(item->flags, mf_disabled);
    }
    else
    {
        RESET(item->flags, mf_disabled);
    }

    return (TRUE);
}

BOOL MNU_MusicFxCheck(MenuItem *item)
{
    if (FXDevice < 0 && MusicDevice < 0)
    {
        SET(item->flags, mf_disabled);
    }
    else
    {
        RESET(item->flags, mf_disabled);
    }

    return (TRUE);
}

////////////////////////////////////////////////
// Do a toggle button
////////////////////////////////////////////////
void MNU_DoButton(MenuItem_p item, BOOL draw)
{
    long x, y;
    BOOL state;
    long last_value;
    short shade = MENU_SHADE_DEFAULT;
    extern char LevelSong[];
    char *extra_text = NULL;
    PLAYERp pp = &Player[myconnectindex];
    long button_x,zero=0;
    int handle=0;
    extern BOOL MusicInitialized,FxInitialized;
    short i, w, h, iPos = 0, iCurs, xPos = 10;
    char *sHint, *sRest;
    button_x = OPT_XSIDE;

    x = item->x;
    y = item->y;

    sHint = " ";
    sRest = " ";

    if (menucmp(currentmenu->text, "^Video") == 0 || menucmp(currentmenu->text, "^Sound") == 0)
        iPos = 30;
    else
    if (menucmp(currentmenu->text, "^Screen") == 0)
    {
        iPos = 20;
    }
    else
    if (menucmp(currentmenu->text, "^Mouse") == 0)
        iPos = 20;
    else
    if (menucmp(currentmenu->text, "^Game") == 0)
        iPos = 80;
    else
    if (menucmp(currentmenu->text, "^Game 2") == 0)
    {
       iPos = 75;
       iCurs = currentmenu->cursor;

       if (gs.BorderNum > 1)
           i = 108;

       if (iCurs == 4) //Set Ninja Slice Hack
           sHint = "Disables 8 bit Ninja dying sequence";
       else
       if (iCurs == 5) //Set $bullet Car Hack
       {
           sHint = "Enables Red controls on Yellow car";
           sRest = "Requires a Level Restart";
       }
    }
    else
    if (menucmp(currentmenu->text, "^Kid Mode") == 0)
        iPos = 65;
    else
    if (menucmp(currentmenu->text, "^Peer2Peer") == 0)
        iPos = 20;

    if (menucmp(sHint, " ") != 0 || menucmp(sRest, " ") != 0)
    {
       MNU_MeasureSmallString(sHint, &w, &h);
       MNU_DrawSmallString(TEXT_XCENTER(w), SS_YSTART + 152, sHint,  1, 0);
       MNU_MeasureSmallString(sRest, &w, &h);
       MNU_DrawSmallString(TEXT_XCENTER(w), SS_YSTART + 152 + 8, sRest, 14, 14);
    }

    if (TEST(item->flags, mf_disabled))
    {
        shade = MENU_SHADE_INACTIVE;
    }

    if (!draw)
    {
        switch (item->button)
        {
        case btn_nuke:
            gs.NetNuke = state = buttonsettings[item->button];
            break;
        case btn_voxels:
            gs.Voxels = state = buttonsettings[item->button];
            break;
        case btn_stats:
            gs.Stats = state = buttonsettings[item->button];
            break;
        case btn_wep:
            last_value = state = buttonsettings[item->button];
            gs.WeaponSwitch = last_value;
            break;
        case btn_str:
            ForceSetup = state = buttonsettings[item->button];
            break;
        case btn_nja:
            gs.UseNinjaHack = state = buttonsettings[item->button];
            break;
        case btn_car:
            gs.UseCarHack = state = buttonsettings[item->button];
            break;
        case btn_drt:
            gs.UseDarts = state = buttonsettings[item->button];
            if (gs.UseDarts)
                useDarts = TRUE;
            else
                useDarts = FALSE;
            break;
        case btn_yin:
            gs.SwapYinyang = state = buttonsettings[item->button];
            break;
        case btn_hid:
            gs.WeaponHide = (gs.WeaponHide==1)?0:gs.WeaponHide+1;
            break;
        case btn_asv:
            Autosave = (Autosave == 2)?0:Autosave+1;
            if (Autosave == 0)
            {
               sprintf(ds,"%sgamex.asv", svgame);
               DeleteAutosave(0, ds);
               Autodone = 0;
            }
            break;
        case btn_markers:
            gs.NetSpawnMarkers = state = buttonsettings[item->button];
            break;
        case btn_teamplay:
            gs.NetTeamPlay = state = buttonsettings[item->button];
            break;
        case btn_friendlyfire:
            gs.NetHurtTeammate = state = buttonsettings[item->button];
            break;
        case btn_crosshair:
            gs.Crosshair = (gs.Crosshair==2)?0:gs.Crosshair+1;
            break;
        case btn_auto_aim:
            last_value = gs.AutoAim;
            gs.AutoAim = state = buttonsettings[item->button];
            if (gs.AutoAim != last_value)
                MenuButtonAutoAim = TRUE;
            break;
        case btn_messages:
            gs.Messages = state = buttonsettings[item->button];
            break;
        case btn_auto_run:
            last_value = gs.AutoRun;
            gs.AutoRun = state = buttonsettings[item->button];
            if (gs.AutoRun != last_value)
                MenuButtonAutoRun = TRUE;
            break;
        case btn_mouse_aim:
            last_value = gs.MouseAimingType;
            gs.MouseAimingType = state = buttonsettings[item->button];
            if (gs.MouseAimingType != last_value)
            {
                //RESET(pp->Flags, PF_MOUSE_AIMING_ON);
                //gs.MouseAimingOn = FALSE;
            }
            //extra_text = gs.MouseAimingType ? "Momentary" : "Toggle";
            break;
        case btn_mouse_invert:
            gs.MouseInvert = state = buttonsettings[item->button];
            break;
//        case btn_bobbing:
//            gs.Bobbing = state = buttonsettings[item->button];
//            break;
        case btn_sound:
            if (!FxInitialized)
                 break;
            last_value = gs.FxOn;
            gs.FxOn = state = buttonsettings[item->button];
            if (gs.FxOn != last_value)
            {
                if (!gs.FxOn)
                    StopFX();
            }
            break;
        case btn_playcd:
            last_value = gs.PlayCD;
            gs.PlayCD = state = buttonsettings[item->button];
            if (SW_SHAREWARE)
                break;
            if (gs.PlayCD)
            {
               BOOL bak = DemoMode;
               CDAudio_Stop();
               StopSound();
               CDAudio_Init();
               GetUserMusic();
               if (InMenuLevel)
                   CDAudio_Play(ThemeTrack[0], TRUE);
               else
                   CDAudio_Play(RedBookSong[Level], TRUE);
               if (CDAudio_Playing() && gs.MusicOn)
               {
                  if (MusicInitialized)
                      MUSIC_Pause();
               }
               DemoMode = bak;
            }
            else
            {
               BOOL bak = DemoMode;

               CDAudio_Stop();
               StopSound();
               NumUserMusic = 0;
               GetUserMusic();
               if (gs.MusicOn)
               {
                  if (MusicInitialized)
                      MUSIC_Continue();
                  else
                  {
                     InitMusic();
                     if (InMenuLevel)
                         strcpy(LevelSong, ThemeSongs[0]);
                     else
                         strcpy(LevelSong, LevelInfo[Level].SongName);
                     PlayMusic(LevelSong);
                  }
               }
               else
               {
                  handle = PlaySound(DIGI_NOLIKEMUSIC,&zero,&zero,&zero,v3df_none);
                  if (handle > FX_Ok)
                      while(FX_SoundActive(handle))
                            handleevents();
               }
               DemoMode = bak;
            }
            break;
        case btn_music:
            last_value = gs.MusicOn;
            gs.MusicOn = state = buttonsettings[item->button];
            if (CDAudio_Playing())
                break;
            if (gs.MusicOn)
            {
               BOOL bak = DemoMode;
               if (!MusicInitialized)
               {
                  InitMusic();
                  if (InMenuLevel)
                      strcpy(LevelSong, ThemeSongs[0]);
                  else
                      strcpy(LevelSong, LevelInfo[Level].SongName);
                  PlayMusic(LevelSong);
               }
               else
               {
                  MUSIC_Continue();
               }
               DemoMode = bak;
            }
            else
            {
               BOOL bak = DemoMode;
               if (MusicInitialized)
                   MUSIC_Pause();
               if (!gs.PlayCD)
               {
                  handle = PlaySound(DIGI_NOLIKEMUSIC,&zero,&zero,&zero,v3df_none);
                  if (handle > FX_Ok)
                      while(FX_SoundActive(handle))
                            handleevents();
               }
               DemoMode = bak;
            }
            break;
        case btn_talking:
            gs.Talking = state = buttonsettings[item->button];
            break;
        case btn_ambience:
            last_value = gs.Ambient;
            gs.Ambient = state = buttonsettings[item->button];
            if (gs.Ambient != last_value)
            {
                if (!InMenuLevel)
                {
                    if (gs.Ambient)
                        StartAmbientSound();
                    else
                        StopAmbientSound();
                }
            }
            break;

        case btn_flipstereo:
            last_value = gs.FlipStereo;
            gs.FlipStereo = state = buttonsettings[item->button];
            if (gs.FlipStereo != last_value)
                FlipStereo();
            break;

        case btn_rand:
            RandomMusic = state = buttonsettings[item->button];
            break;

        case btn_shadows:
            gs.Shadows = state = buttonsettings[item->button];
            break;

        case btn_parental:
            if (gs.Password[0] != '\0' && gs.ParentalLock == TRUE)
            {
                if(passwordvalid)
                {
                    state = buttonsettings[btn_parental] = gs.ParentalLock = FALSE;
                    if (!InMenuLevel)
                    JS_ToggleLockouts();
                }
                else
                {
                    state = buttonsettings[btn_parental] = gs.ParentalLock = TRUE;
                    MenuInputMode = TRUE;
                    memset(MessageInputString, '\0', sizeof(MessageInputString));
                    KB_ClearKeysDown();
                    KB_FlushKeyboardQueue();
                }
            }
            else
            {
                gs.ParentalLock = state = buttonsettings[item->button];
                if (!InMenuLevel)
                JS_ToggleLockouts();
            }
            break;

		case btn_videofs:
        {
			int lastx, lasty, lastbpp, newoffset;

			state = buttonsettings[btn_videofs];

			lastx   = validresolutions[ slidersettings[sldr_videores] ].xdim;
			lasty   = validresolutions[ slidersettings[sldr_videores] ].ydim;
			lastbpp = validbpps[ slidersettings[sldr_videobpp] ];
			UpdateValidModes(lastbpp, buttonsettings[btn_videofs]);

			// check if the last bpp is still a valid choice
			for (i=0; i<numvalidbpps; i++)
				if (validbpps[i] == lastbpp) break;
			if (i == numvalidbpps)
			{
				// it wasn't
				slidersettings[sldr_videobpp] = 0;
				lastbpp = validbpps[0];
				UpdateValidModes(lastbpp, buttonsettings[btn_videofs]);
			}
			else
			{
				slidersettings[sldr_videobpp] = i;
			}

			// find the nearest resolution to the one last selected
			newoffset = 0;
			for (i=0; i<numvalidresolutions; i++)
			{
				if (abs(lastx * lasty - validresolutions[i].xdim         * validresolutions[i].ydim) <
				    abs(lastx * lasty - validresolutions[newoffset].xdim * validresolutions[newoffset].ydim))
					newoffset = i;
			}
			slidersettings[sldr_videores] = newoffset;
		}
		    break;

        case btn_videows:
            state = buttonsettings[btn_videows];
            glwidescreen = state = buttonsettings[item->button];
            if (glwidescreen != state)
                glwidescreen = state;
            break;

        case btn_hitile:
            state = buttonsettings[btn_hitile];
            usehightile = state = buttonsettings[item->button];
            if (usehightile != state)
                usehightile = state;
            break;

        case btn_models:
            state = buttonsettings[btn_models];
            usemodels = state = buttonsettings[item->button];
            if (usemodels != state)
                usemodels = state;
            break;

        case btn_txcach:
            state = buttonsettings[btn_txcach];
            glusetexcache = state = buttonsettings[item->button];
            if (glusetexcache != state)
                glusetexcache = state;
            break;

        case btn_txcomp:
            state = buttonsettings[btn_txcomp];
            glusetexcompr = state = buttonsettings[item->button];
            if (glusetexcompr != state)
                glusetexcompr = state;
            break;

        case btn_cacomp:
            state = buttonsettings[btn_cacomp];
            glusetexcachecompression = state = buttonsettings[item->button];
            if (glusetexcachecompression != state)
                glusetexcachecompression = state;
            break;

        case btn_galpha:
            state = buttonsettings[btn_galpha];
            usegoodalpha = state = buttonsettings[item->button];
            if (usegoodalpha != state)
                usegoodalpha = state;
            break;

        case btn_vsync:
            state = buttonsettings[btn_vsync];
            vsync = state = buttonsettings[item->button];
            if (vsync != state)
                vsync = state;
            SetVSync(vsync);
            break;

        default:
            state = buttonsettings[item->button];
            break;
            }
        }

    if (!draw)
        return;

    switch (item->button)
    {
        case btn_mouse_aim:
            extra_text = gs.MouseAimingType ? "Momentary" : "Toggle";
            xPos = 25;
            break;
        case btn_galpha:
            extra_text = usegoodalpha ? "Enhanced" : "Normal";
            xPos = 35;
            break;
        case btn_vsync:
            extra_text = vsync ? "Enabled" : "Disabled";
            xPos = 35;
            break;
        case btn_crosshair:
        {
            char *validxhairs[3] = {"Off", "Max", "Min"};

            extra_text = validxhairs[gs.Crosshair];
            xPos = 72;
            break;
        }
        case btn_asv:
        {
            char *validsaves[3] = {"Off", "Auto", "Manual"};

            extra_text = validsaves[Autosave];
            xPos = 68;
            break;
        }
        case btn_hid:
            extra_text = gs.WeaponHide  ? "Hide" : "Show";
            xPos = 68;
            break;
	default: break;
    }

    state = buttonsettings[item->button];

    // Draw the button
    if (item->text)
    {
        if (xPos != 72 && xPos != 68)
        {
           if (state) // set
               rotatesprite((button_x + iPos) << 16, y << 16, MZ, 0, pic_radiobuttn2, shade, 0, MenuDrawFlags, 0, 0, xdim - 1, ydim - 1);
           else // not set
               rotatesprite((button_x + iPos) << 16, y << 16, MZ, 0, pic_radiobuttn1, shade+7, 0, MenuDrawFlags, 0, 0, xdim - 1, ydim - 1);
        }
        MenuTextShade = shade;
        MNU_DrawString(x, y, item->text, MenuTextShade, mPal);
        if (extra_text)
            MNU_DrawString(OPT_XSIDE + tilesizx[pic_radiobuttn1] + xPos, y+2, extra_text, MenuTextShade, 16);
        MenuTextShade = MENU_SHADE_DEFAULT;
    }
    else
    {
        if (xPos != 72)
        {
           if (state)
               rotatesprite(x << 16, y << 16, MZ, 0, pic_radiobuttn2, 2, 0, MenuDrawFlags, 0, 0, xdim - 1, ydim - 1);
           else
               rotatesprite(x << 16, y << 16, MZ, 0, pic_radiobuttn1, 2, 0, MenuDrawFlags, 0, 0, xdim - 1, ydim - 1);
        }
        x += tilesizx[pic_radiobuttn1] + 4;
        // Draw the menu item text
        rotatesprite(x << 16, y << 16, MZ, 0, item->pic, 2, 0, MenuDrawFlags, 0, 0, xdim - 1, ydim - 1);
    }

}

//char *gametype[] = {"War [Respawn]","Cooperative","War [No Respawn]"};
char *gametype[] = {"WB (spawn)","WB (no spawn)","Cooperative","Other"};
char *playercolors[] = {"Brown","Gray","Purple","Red","Yellow","Olive","Green","Blue"};
char *monsterskills[] = {"No Monsters","Easy","Normal","Hard","Insane!"};

void MNU_DoSlider(short dir, MenuItem_p item, BOOL draw)
{
    short offset, i = 0, barwidth, iPos=0;
    long x, y, knobx;
    short shade = MENU_SHADE_DEFAULT;
    char *extra_text=NULL;
    char tmp_text[256];

    memset(tmp_text,0,256);

    if (TEST(item->flags, mf_disabled))
    {
        shade = MENU_SHADE_INACTIVE;
        dir = 0;
    }

    if (menucmp(currentmenu->text, "^Video") == 0 || menucmp(currentmenu->text, "^Sound") == 0)
        iPos = 30;
    else
    if (menucmp(currentmenu->text, "^Screen") == 0)
        iPos = 20;
    else
    if (menucmp(currentmenu->text, "^Mouse") == 0)
        iPos = 20;

    switch (item->slider)
    {
    case sldr_mouse:
        barwidth = SLDR_MOUSESENSEMAX;
        offset = slidersettings[sldr_mouse] += dir;

        if (TEST(item->flags, mf_disabled))
            break;

        offset = max(offset, 0);
        offset = min(offset, SLDR_MOUSESENSEMAX-1);

        slidersettings[sldr_mouse] = offset;

        gs.MouseSpeed = offset * (MOUSE_SENS_MAX_VALUE/SLDR_MOUSESENSEMAX);
        CONTROL_SetMouseSensitivity(gs.MouseSpeed<<1);

        break;

    case sldr_mousex:
        barwidth = SLDR_MOUSESENSEMAX;
        offset = slidersettings[sldr_mousex] += dir;

        if (TEST(item->flags, mf_disabled))
            break;

        offset = max(offset, 0);
        offset = min(offset, SLDR_MOUSESENSEMAX-1);

        slidersettings[sldr_mousex] = offset;

        gs.MouseScaleX = offset * (MOUSE_X_MAX_VALUE/SLDR_MOUSESENSEMAX);
        CONTROL_SetAnalogAxisScale( 0, gs.MouseScaleX<<1, controldevice_mouse );

        break;

    case sldr_mousey:
        barwidth = SLDR_MOUSESENSEMAX;
        offset = slidersettings[sldr_mousey] += dir;

        if (TEST(item->flags, mf_disabled))
            break;

        offset = max(offset, 0);
        offset = min(offset, SLDR_MOUSESENSEMAX-1);

        slidersettings[sldr_mousey] = offset;

        gs.MouseScaleY = offset * (MOUSE_Y_MAX_VALUE/SLDR_MOUSESENSEMAX);
        CONTROL_SetAnalogAxisScale( 1, gs.MouseScaleY>>1, controldevice_mouse );

        break;

    case sldr_musicvolume:
        barwidth = SLDR_MUSICVOLMAX;
        offset = slidersettings[sldr_musicvolume] += dir;
        if (TEST(item->flags, mf_disabled))
            break;

        offset = max(offset, 0);
        offset = min(offset, SLDR_MUSICVOLMAX-1);

        slidersettings[sldr_musicvolume] = offset;
        gs.MusicVolume = MUSIC_MIN + (offset * VOL_MUL);
        if (!SW_SHAREWARE && gs.PlayCD)
           CDAudio_SetVolume(gs.MusicVolume);
        else
           MUSIC_SetVolume(gs.MusicVolume);
        break;

    case sldr_sndfxvolume:
        barwidth = SLDR_SNDFXVOLMAX;
        offset = slidersettings[sldr_sndfxvolume] += dir;

        if (TEST(item->flags, mf_disabled))
            break;

        offset = max(offset, 0);
        offset = min(offset, SLDR_SNDFXVOLMAX-1);

        slidersettings[sldr_sndfxvolume] = offset;
        gs.SoundVolume = FX_MIN + (offset * VOL_MUL);
        FX_SetVolume(gs.SoundVolume);
        break;

    case sldr_textcl:
    {
        barwidth = 12;
        offset = slidersettings[sldr_textcl] += dir;

        if (TEST(item->flags, mf_disabled))
            break;

        offset = max(offset, 0);
        offset = min(offset, 12);

        if (offset < 0)
            offset = 0;
        else
        if (offset > 12)
            offset = 12;

        gs.MenuTextColor = offset;
        slidersettings[sldr_textcl] = offset;
        hicsetpalettetint(mPal, 255, gs.MenuTextColor*18, 16, 0);  // pal - gold
        break;
    }

//------------------------------ VIDEO OPTIONS 1 --------------------------------

    case sldr_scrsize:
        {
        barwidth = SLDR_SCRSIZEMAX;
        offset = slidersettings[sldr_scrsize] += dir;

        if (TEST(item->flags, mf_disabled))
            break;

        offset = max(offset, 0);
        offset = min(offset, SLDR_SCRSIZEMAX - 1);

        if (offset < 0)
            offset = 0;
        else
        if (offset > SLDR_SCRSIZEMAX)
            offset = SLDR_SCRSIZEMAX;

        slidersettings[sldr_scrsize] = offset;
        gs.BorderNum = offset;
        gs.BorderMem = gs.BorderNum;
        UpdateMiniBar(&Player[myconnectindex]);
        break;
        }

    case sldr_fov:
    {
        barwidth = SLDR_SCRSIZEMAX;
        offset = slidersettings[sldr_fov] += dir;

        if (TEST(item->flags, mf_disabled))
            break;

        offset = max(offset, 0);
        offset = min(offset, SLDR_SCRSIZEMAX - 1);
        slidersettings[sldr_fov] = offset;

        if (glfovscreen != offset)
            glfovscreen = offset;

        break;
    }

    case sldr_bordertile:
        barwidth = SLDR_BORDERTILEMAX;
        offset = slidersettings[sldr_bordertile] += dir;

        if (TEST(item->flags, mf_disabled))
            break;

        offset = max(offset, 0);
        offset = min(offset, SLDR_BORDERTILEMAX - 1);
        slidersettings[sldr_bordertile] = offset;

        if (gs.BorderTile != offset)
            gs.BorderTile = offset;
        break;

    case sldr_brightness:
        barwidth = SLDR_BRIGHTNESSMAX;
        offset = slidersettings[sldr_brightness] += dir;

        if (TEST(item->flags, mf_disabled))
            break;

        offset = max(offset, 0);
        offset = min(offset, SLDR_BRIGHTNESSMAX - 1);
        slidersettings[sldr_brightness] = offset;

        if (gs.Brightness != offset)
        {
            gs.Brightness = offset;
            COVERsetbrightness(gs.Brightness,(char *)palette_data);
        }
        break;

    case sldr_tropy:
	{
        short tropynum[5]  = {1,2,4,8,16};

		offset = slidersettings[sldr_tropy] += dir;
		barwidth = 5;

		if (TEST(item->flags, mf_disabled))
			break;

        offset = max(offset, 0);
        offset = min(offset, 4);
        slidersettings[sldr_tropy] = offset;

        if (offset < 5)
        {
            char *validtropys[5]  = {"None", "2x", "4x", "8x", "16x"};
   		    sprintf(tmp_text, "%s", validtropys[offset]);
		    MNU_DrawString(OPT_XSIDE+tilesizx[pic_slidelend]+tilesizx[pic_sliderend]+9*tilesizx[pic_slidebar]+5, item->y+2, tmp_text, 1, 16);
            if (glanisotropy != tropynum[offset])
            {
                glanisotropy = tropynum[offset];
                gltexapplyprops();
            }
        }
	}
	break;

    case sldr_hud:
	{
		barwidth = 3;
		offset = slidersettings[sldr_hud] += dir;

		if (TEST(item->flags, mf_disabled))
			break;

        offset = max(offset, 0);
        offset = min(offset, 2);
        slidersettings[sldr_hud] = offset;

        if (offset < 3)
        {
            char *validhuds[3] = {"Classic", "Modern", "Alternate"};

  		    sprintf(tmp_text, "%s", validhuds[offset]);
  		    MNU_DrawString(OPT_XSIDE+tilesizx[pic_slidelend]+tilesizx[pic_sliderend]+9*tilesizx[pic_slidebar]+5, item->y+2, tmp_text, 1, 16);
            gs.MiniHudTyp = offset;
        }
	}
	break;

    case sldr_filter:
	{
        short filternum[4] = {0,2,1,3};

		offset = slidersettings[sldr_filter] += dir;
		barwidth = 4;

		if (TEST(item->flags, mf_disabled))
			break;

        offset = max(offset, 0);
        offset = min(offset, 3);
        slidersettings[sldr_filter] = offset;

        if (offset < 4)
        {
           char *validfilters[4] = {"None", "Linear", "Bilinear", "Trilinear"};

  		   sprintf(tmp_text, "%s", validfilters[offset]);
		   MNU_DrawString(OPT_XSIDE+tilesizx[pic_slidelend]+tilesizx[pic_sliderend]+9*tilesizx[pic_slidebar]+5, item->y+2, tmp_text, 1, 16);
           if (gltexfiltermode != filternum[offset])
           {
              gltexfiltermode = filternum[offset];
              gltexapplyprops();
           }
        }
	}
	break;

//--------------------------------- MULTIPLAY -----------------------------------

    case sldr_gametype:
        barwidth = SLDR_GAMETYPEMAX;
        offset = slidersettings[sldr_gametype] += dir;

        if (TEST(item->flags, mf_disabled))
            break;

        offset = max(offset, 0);
        offset = min(offset, SLDR_GAMETYPEMAX - 1);
        slidersettings[sldr_gametype] = offset;

        extra_text = gametype[offset];
        MNU_DrawString(OPT_XSIDE+40, item->y, extra_text, 1, 16);
        gs.NetGameType = offset;
        // friendly fire menu
        MNU_ItemPreProcess(currentmenu);
        break;

    case sldr_netlevel:
        barwidth = SLDR_NETLEVELMAX;
        offset = slidersettings[sldr_netlevel] += dir;

        if (TEST(item->flags, mf_disabled))
            break;

        offset = max(offset, 0);
        offset = min(offset, SLDR_NETLEVELMAX - 1);
        slidersettings[sldr_netlevel] = offset;

        // Show the currently selected level on next line
        //extra_text = MNU_LevelName[offset];
        //MNU_DrawString(OPT_XS, item->y+10, extra_text, 1, 16);
	    sprintf(tmp_text, "L%02d:  %s", offset+1, LevelInfo[offset+1].Description);
        MNU_DrawString(OPT_XS+20, item->y+10, tmp_text, 1, i);
        gs.NetLevel = offset;
        break;

    case sldr_monsters:
        barwidth = SLDR_MONSTERSMAX;
        offset = slidersettings[sldr_monsters] += dir;

        if (TEST(item->flags, mf_disabled))
            break;

        offset = max(offset, 0);
        offset = min(offset, SLDR_MONSTERSMAX - 1);
        slidersettings[sldr_monsters] = offset;

        extra_text = monsterskills[offset];
        MNU_DrawString(OPT_XSIDE+54, item->y, extra_text, 1, 16);
        gs.NetMonsters = offset;
        break;

    case sldr_killlimit:
        barwidth = SLDR_KILLLIMITMAX;
        offset = slidersettings[sldr_killlimit] += dir;

        if (TEST(item->flags, mf_disabled))
            break;

        offset = max(offset, 0);
        offset = min(offset, SLDR_KILLLIMITMAX - 1);
        slidersettings[sldr_killlimit] = offset;

        if (offset == 0)
        {
            strcpy(tmp_text,"Infinite\n");
        }
        else
        {
            sprintf(tmp_text,"%d",offset*10);
            //itoa(offset*10,tmp_text,10);
        }
        MNU_DrawString(OPT_XSIDE+101, item->y, tmp_text, 1, 16);
        gs.NetKillLimit = offset;
        break;

    case sldr_timelimit:
        barwidth = SLDR_TIMELIMITMAX;
        offset = slidersettings[sldr_timelimit] += dir;

        if (TEST(item->flags, mf_disabled))
            break;

        offset = max(offset, 0);
        offset = min(offset, SLDR_TIMELIMITMAX - 1);
        slidersettings[sldr_timelimit] = offset;

        if (offset == 0)
        {
            strcpy(tmp_text,"Infinite\n");
        }
        else
        {
            sprintf(tmp_text,"%d Minutes\n",TimeLimitTable[offset]);
        }

        MNU_DrawString(OPT_XSIDE+86, item->y, tmp_text, 1, 16);
        gs.NetTimeLimit = offset;
        break;

    case sldr_playercolor:
    {
        short red[8] = {110,87,192,240,255,168,0,0};
        short grn[8] = {70,87,0,0,192,168,160,0};
        short blu[8] = {0,87,192,0,64,0,0,240};
        barwidth = SLDR_PLAYERCOLORMAX;
        offset = slidersettings[sldr_playercolor] += dir;

        if (TEST(item->flags, mf_disabled))
            break;

        offset = max(offset, 0);
        offset = min(offset, SLDR_PLAYERCOLORMAX - 1);
        slidersettings[sldr_playercolor] = offset;

        extra_text = playercolors[offset];
        if (!gs.SetHighres || !usehightile || NoHrp > 0)
            MNU_DrawString(OPT_XSIDE+78, item->y, extra_text, 1, PALETTE_PLAYER0+offset);
        else
        {
            hicsetpalettetint(30, red[offset], grn[offset], blu[offset], 0);
            MNU_DrawString(OPT_XSIDE+78, item->y, extra_text, 0, 30);
        }
        gs.NetColor = offset;
        break;
    }

//------------------------------ VIDEO OPTIONS 2 --------------------------------

    case sldr_videobpp:
	{
		offset = max(0,min(slidersettings[sldr_videobpp] + dir, numvalidbpps-1));
        barwidth = numvalidbpps;

		if (TEST(item->flags, mf_disabled))
			break;

        if (ScreenBPP > 8)
        {
		    if (offset == 0)
		        offset = 1;
		}

		if (slidersettings[sldr_videobpp] != offset)
		{
			int lastx, lasty, newoffset, i;

			slidersettings[sldr_videobpp] = offset;

			// find the nearest resolution to the one last selected
			lastx = validresolutions[ slidersettings[sldr_videores] ].xdim;
			lasty = validresolutions[ slidersettings[sldr_videores] ].ydim;
			UpdateValidModes(validbpps[offset], buttonsettings[btn_videofs]);
			newoffset = 0;
			for (i=0; i<numvalidresolutions; i++)
			{
				if (abs(lastx * lasty - validresolutions[i].xdim         * validresolutions[i].ydim) <
				    abs(lastx * lasty - validresolutions[newoffset].xdim * validresolutions[newoffset].ydim))
					newoffset = i;
			}
			slidersettings[sldr_videores] = newoffset;

		}

		sprintf(tmp_text, "%d bpp", validbpps[offset]);
		MNU_DrawString(OPT_XSIDE+20, item->y+OPT_YINC+1, tmp_text, 1, 16);
	}
	break;

    case sldr_videores:
	{
		offset = max(0,min(slidersettings[sldr_videores] + dir, numvalidresolutions-1));
		barwidth = numvalidresolutions;

		if (TEST(item->flags, mf_disabled))
			break;

		slidersettings[sldr_videores] = offset;

		sprintf(tmp_text, "%dx%d", validresolutions[offset].xdim, validresolutions[offset].ydim);
		MNU_DrawString(OPT_XSIDE+20, item->y+OPT_YINC+1, tmp_text, 1, 16);
	}
	break;

    default:
        return;
    }

    if (!draw)
        return;

    // Now draw it
    item++;
    x = item->x + iPos;
    y = item->y;

    // Draw the left end cap of the bar
    rotatesprite(x << 16, y << 16, MZ, 0, pic_slidelend, shade, 0, MenuDrawFlags, 0, 0, xdim - 1, ydim - 1);

    x += tilesizx[pic_slidelend];
    knobx = x;

    // Draw the in between sections
    for (i = 0; i < min(barwidth,MAX_SLDR_WIDTH); i++)
    {
        rotatesprite(x << 16, y << 16, MZ, 0, pic_slidebar, shade, 0, MenuDrawFlags, 0, 0, xdim - 1, ydim - 1);
        x += tilesizx[pic_slidebar];
    }

    // Draw the right end cap
    rotatesprite(x << 16, y << 16, MZ, 0, pic_sliderend, shade, 0, MenuDrawFlags, 0, 0, xdim - 1, ydim - 1);

    // Draw the knob, compressing the X coordinate if the bar is too wide
    if (barwidth > MAX_SLDR_WIDTH)
    {
	knobx += offset * (MAX_SLDR_WIDTH*tilesizx[pic_slidebar]-tilesizx[pic_sliderknob]) / (barwidth-1);
    }
    else
    {
        knobx += tilesizx[pic_slidebar] * offset;
    }
    rotatesprite(knobx << 16, (y + 2) << 16, MZ, 0, pic_sliderknob, shade, 0, MenuDrawFlags, 0, 0, xdim - 1, ydim - 1);
}

////////////////////////////////////////////////
// Start up menu array
////////////////////////////////////////////////
static void MNU_SetupMenu(void)
{
    MenuGroup *rootmenu;

    static MenuGroup *rootmenulist[] =
    {
        &maingroup,
        &SaveGameGroup,
        &LoadGameGroup,
        &soundgroup,
        &optiongroup,
        &quickloadgroup,
        &quitgroup,
        &ordergroup,
        &episodegroup,
        &game2group,             // so as to adjust button icon position wxic
        &networkgroup,
    };

    rootmenu = rootmenulist[ControlPanelType];
    ASSERT(ControlPanelType < ct_max);

    menuarrayptr = 0;
    menuarray[0] = currentmenu = rootmenu;
    if(ControlPanelType == ct_mainmenu)

    mnu_input_buffered.button0 = mnu_input_buffered.button1 = FALSE;
    mnu_input_buffered.dir = dir_None;
    order_input_buffered.button0 = order_input_buffered.button1 = FALSE;
    order_input_buffered.dir = dir_None;
    ResetKeys();

    // custom cust_callback starts out as null
    cust_callback = NULL;

    // for QuitCustom and QuickLoadCustom
    if (currentmenu->items == NULL)
    {
        if (currentmenu->draw_custom)
            currentmenu->draw_custom(uc_setup, NULL);
    }

    if (ControlPanelType == ct_mainmenu)
    {
       if (bGameQuit)
       {
          bGameQuit = FALSE;                                         // quit game
          currentmenu->cursor = 6;
       }
       else
          currentmenu->cursor = 0;
    }

    // disable any items necessary
    MNU_ItemPreProcess(currentmenu);
}

////////////////////////////////////////////////
// Draw an item
////////////////////////////////////////////////
static void MNU_ClearFlags(MenuGroup * node)
{
    MenuItem *i;

    if (!node->items)
        return;

    for (i = node->items; i->type != mt_none; i++)
    {
        i->flags &= ~MenuSelectFlags;
        if (i->child)
            MNU_ClearFlags((MenuGroup *) i->child);
    }
}

////////////////////////////////////////////////
// Pop a group off the menu stack
////////////////////////////////////////////////
static void MNU_PopGroup(void)
{
    if (!menuarrayptr)
        return;

    currentmenu = menuarray[--menuarrayptr];

    SetFragBar(Player + myconnectindex);
    //PanelRefresh(Player + myconnectindex);
}

////////////////////////////////////////////////
// Push a group on to the menu stack
////////////////////////////////////////////////
static void MNU_PushGroup(MenuGroup * node)
{
    if (menuarrayptr == MaxLayers - 1)
        return;

    currentmenu = menuarray[++menuarrayptr] = node;

    SetFragBar(Player + myconnectindex);
}

////////////////////////////////////////////////
// Setup a new menu subgroup
////////////////////////////////////////////////
static void MNU_SetupGroup(void)
{
    MNU_SelectItem(currentmenu, currentmenu->cursor, FALSE);
    MNU_DrawMenu();
}

static VOID MNU_ItemPreProcess(MenuGroup * group)
{
    MenuItem *item;

    if (!group->items)
        return;

    // process all items when going down a level
    // to see if anything is disabled
    for (item = group->items; item->type != mt_none; item++)
    {
        if (item->preprocess)
            item->preprocess(item);
    }
}

VOID MNU_ItemPostProcess(MenuGroup * group)
{
    MenuItem *item;
    long zero = 0;

    if (!group->items)
        return;

    item = &currentmenu->items[currentmenu->cursor];

    if (item->postprocess)
    {
        item->postprocess(item);
    }
}

////////////////////////////////////////////////
// Go to next menu subgroup
////////////////////////////////////////////////
static void MNU_DownLevel(MenuGroup * group)
{

    if (!group)
    {
        TerminateGame();
        printf("MNU_DownLevel() - NULL card\n");
        exit(0);
    }

    MNU_PushGroup(group);

    if (group->items == NULL)
    {
        if (group->draw_custom && group->draw_custom(uc_setup, NULL))
            MNU_PopGroup();
    }

    MNU_ItemPreProcess(currentmenu);

    MNU_SetupGroup();

    SetRedrawScreen(&Player[myconnectindex]);
}

////////////////////////////////////////////////
// Go to previous menu subgroup
////////////////////////////////////////////////
static void MNU_UpLevel(void)
{
    long zero = 0;
    static int handle1=0;
    // if run out of menus then EXIT
    if (!menuarrayptr)
    {
        if(!FX_SoundActive(handle1))
            handle1 = PlaySound(DIGI_STARCLINK,&zero,&zero,&zero,v3df_dontpan);
        ExitMenus();
        return;
    }

    if (currentmenu->items)
        currentmenu->items[currentmenu->cursor].flags &= ~mf_selected;
    MNU_PopGroup();
    MNU_SetupGroup();

    SetRedrawScreen(&Player[myconnectindex]);
}

////////////////////////////////////////////////
// Do a menu item action
////////////////////////////////////////////////
static void MNU_DoItem(void)
{
    MenuItem *item;

    item = &currentmenu->items[currentmenu->cursor];
    if (!item) return;

    if (TEST(item->flags, mf_disabled))
    {
        // Try to process again
        if (item->preprocess)
            item->preprocess(item);

        // Check once more
        if (TEST(item->flags, mf_disabled))
            return;
    }

    switch (item->type)
    {
        case mt_option:
            if (item->custom != NULL)
                item->custom();
            break;
        case mt_button:
            MNU_PushItem(item, FALSE);
            if (item->custom != NULL)
                item->custom();
            break;
        case mt_layer:
            if (item->custom != NULL)
                item->custom();
            MNU_DownLevel(item->child);
            break;
	default: break;
    }
}

////////////////////////////////////////////////
// Draw an item icon or cursor
////////////////////////////////////////////////
static void MNU_DrawItemIcon(MenuItem * item)
{
    //void BorderRefreshClip(PLAYERp pp, short x, short y, short x2, short y2);
    long x = item->x, y = item->y;
    long scale = MZ;
    char *sText = currentmenu->text;
    short w,h;
    int yin = pic_yinyang;
    extern long  Autotick;

    Autotick = 0;          // stop possible autsave

    if  (gs.SwapYinyang)
         yin = pic_shuriken1;

    if (currentmenu->text > 0 && menucmp(sText, "^Episode") != 0 && menucmp(sText, "^Skill") != 0 && menucmp(sText, "^Options") != 0)
        scale = MZ - 32768;
    else
    if (menucmp(sText, "^Episode") == 0 || menucmp(sText, "^Skill") == 0)
    {
        scale = MZ - 16384;
        y = y - 4;
        x = x - 5;
        if (menucmp(sText, "^Skill") == 0)
            x = x - 4;
    }
    else
    if (menucmp(sText, "^Options") == 0)
        scale = MZ - 16384;
    else
        y = y + 3;

    if (item->text)
    {
        scale /= 2;
        x -= mulscale17(tilesizx[yin],scale) + 2;
        y += 4;
    }
    else
    {
        scale -= (1<<13);
        x -= ((tilesizx[yin]) / 2) - 3;
        y += 8;
    }

    x = x - 4;

    if (menucmp(currentmenu->text, "^Options") == 0)
    {
        x = 60;
        y -= 5;
    }

    rotatesprite(x << 16, y << 16, scale, 0, yin, item->shade, 0, MenuDrawFlags, 0, 0, xdim - 1, ydim - 1);

    SetRedrawScreen(&Player[myconnectindex]);
    //BorderRefreshClip(&Player[myconnectindex], x - 24, y - 24, x + 24, y + 24);

    if (menucmp(currentmenu->text, "^Options") == 0)
    {
        char *sVers = "SWP - Version "SwpVer;
        MNU_MeasureString(sVers, &w, &h);
        MNU_DrawString(TEXT_XCENTER(w), 190, sVers, 24, 14);
    }
}

////////////////////////////////////////////////
// Draw an item
////////////////////////////////////////////////
static void MNU_DrawItem(MenuItem * item)
{
    char *ptr;
    short px, py;

    MNU_ItemPostProcess(currentmenu);  // Put this in so things can be drawn on item select

    if (!item->pic)
        return;

    MNU_DrawItemIcon(item);

    // if text string skip this part
    if (item->text)
        return;

    if (TEST(item->flags, mf_selected) && !TEST(item->flags, mf_disabled))
    {
        // Highlighted
        if (item->type != mt_button)
            rotatesprite(item->x << 16, item->y << 16, MZ, 0, item->pic,
                -30 + STD_RANDOM_RANGE(50), PALETTE_MENU_HIGHLIGHT, MenuDrawFlags,
                 0, 0, xdim - 1, ydim - 1);
        else
            rotatesprite((item->x + tilesizx[pic_radiobuttn1] + 4) << 16, item->y << 16,
                MZ, 0, item->pic, item->shade, PALETTE_MENU_HIGHLIGHT, MenuDrawFlags, 0, 0, xdim - 1, ydim - 1);
    }
    else
    {
        // Un highlighted
        if (item->type != mt_button)
            rotatesprite(item->x << 16, item->y << 16, MZ, 0, item->pic,
                item->shade, 0, MenuDrawFlags, 0, 319, 199, 0);
        else
            rotatesprite((item->x + tilesizx[pic_radiobuttn1] + 4) << 16, item->y << 16,
                MZ, 0, item->pic, item->shade, 0, MenuDrawFlags, 0, 0, xdim - 1, ydim - 1);
    }
}

////////////////////////////////////////////////
// Draw the menu contents
////////////////////////////////////////////////
static void MNU_DrawMenuContents(void)
{
    MenuItem *item;
    short i,w,h;

    ASSERT(currentmenu != NULL);

    if (currentmenu->text)
    {
        // Draw the backdrop bar
        rotatesprite(10 << 16, (currentmenu->y-3) << 16, MZ, 0, 2427,
            currentmenu->shade, 0, MenuDrawFlags|ROTATE_SPRITE_CORNER, 0, 0, xdim - 1, ydim - 1);
        MNU_MeasureStringLarge(currentmenu->text, &w, &h);
        MNU_DrawString(TEXT_XCENTER(w), currentmenu->y, currentmenu->text, 1, 16);
    }
    else
    if (currentmenu->titlepic)
    {
        rotatesprite(currentmenu->x << 16, currentmenu->y << 16, MZ, 0, currentmenu->titlepic,
            currentmenu->shade, 0, MenuDrawFlags, 0, 0, xdim - 1, ydim - 1);
    }

    if (!currentmenu->items)
        return;

    for (item = currentmenu->items; item->type != mt_none; item++)
    {
        if (item->pic)
        {
            if (item->type == mt_button)
            {
                // all drawing done here also
                MNU_DoButton(item, TRUE);
            }
            else
            {
                if (item->text)
                {
                    if (TEST(item->flags, mf_disabled))
                        MenuTextShade = MENU_SHADE_INACTIVE;
                    if (menucmp(currentmenu->text, "^Options") == 0 || menucmp(currentmenu->text, "^Episode") == 0 || menucmp(currentmenu->text, "^Skill") == 0)
                        MNU_DrawOptionString(item->x, item->y, item->text, MenuTextShade, 16);
                    else
                        MNU_DrawString(item->x, item->y, item->text, MenuTextShade, mPal);
                    MenuTextShade = MENU_SHADE_DEFAULT;
                }
                else
                {
                    rotatesprite(item->x << 16, item->y << 16, MZ, 0, item->pic,
                        item->shade, 0, MenuDrawFlags, 0, 0, xdim - 1, ydim - 1);
                }
            }
        }

        // Is there a slider attached to this item?  Draw it.
        if (item->type == mt_slider)
            MNU_DoSlider(0, item, TRUE);
    }

    MNU_SelectItem(currentmenu, currentmenu->cursor, TRUE);

    if (currentmenu->draw_custom)
        currentmenu->draw_custom(uc_touchup, NULL);
}

////////////////////////////////////////////////
// Draw the menu
////////////////////////////////////////////////
void MNU_DrawMenu(void)
{
    if (cust_callback != NULL)
    {
        cust_callback(cust_callback_call, cust_callback_item);
        return;
    }

    if (currentmenu->items || currentmenu->titlepic)
    {
        MNU_DrawMenuContents();
    }
}

////////////////////////////////////////////////
// Select a menu item
////////////////////////////////////////////////
void MNU_SelectItem(MenuGroup * group, short index, BOOL draw)
{
    MenuItem *item;

    if (index != group->cursor)
    {
        item = &group->items[group->cursor];
        item->flags &= ~mf_selected;
        if (draw)
            MNU_DrawItem(item);
    }

    group->cursor = index;
    item = &group->items[group->cursor];

    item->flags |= mf_selected;
    if (draw)
        MNU_DrawItem(item);
}

////////////////////////////////////////////////
// Toggle a menu radio button on/off
////////////////////////////////////////////////
static void MNU_PushItem(MenuItem * item, BOOL draw)
{
    if (item->type != mt_button)
        return;

    buttonsettings[item->button] ^= 1;

//    if (draw)
        MNU_DoButton(item, draw);
}

////////////////////////////////////////////////
// Go to next item on menu
////////////////////////////////////////////////
static void MNU_NextItem(void)
{
    MenuTag type;
    MenuFlags flag;

    type = currentmenu->items[currentmenu->cursor + 1].type;
    flag = currentmenu->items[currentmenu->cursor + 1].flags;

    if (type == mt_none)
        MNU_SelectItem(currentmenu, 0, FALSE);
    else
        MNU_SelectItem(currentmenu, currentmenu->cursor + 1, FALSE);

    type = currentmenu->items[currentmenu->cursor].type;
    flag = currentmenu->items[currentmenu->cursor].flags;

    if (type == mt_inert || flag == mf_disabled)
        MNU_NextItem();
}

////////////////////////////////////////////////
// Go to previous item on menu
////////////////////////////////////////////////
static void MNU_PrevItem(void)
{
    MenuTag type;
    MenuFlags flag;

    if (!currentmenu->cursor)
        while (currentmenu->items[++currentmenu->cursor].type != mt_none);

    MNU_SelectItem(currentmenu, currentmenu->cursor - 1, FALSE);

    type = currentmenu->items[currentmenu->cursor].type;
    flag = currentmenu->items[currentmenu->cursor].flags;
    if (type == mt_inert || flag == mf_disabled)
        MNU_PrevItem();
}

////////////////////////////////////////////////
// Find hotkey press on current menu, if any.
////////////////////////////////////////////////
static BOOL MNU_DoHotkey(void)
{
    MenuItem_p item;
    short index;

    if (!currentmenu->items) return FALSE;

    index = 0;
    for (item = currentmenu->items; item->type != mt_none; item++)
    {
        if (KEY_PRESSED(item->hotkey) && item->hotkey != 0)
        {
            MNU_SelectItem(currentmenu, index, FALSE);
            return (TRUE);
        }
        index++;
    }

    return (FALSE);
}

////////////////////////////////////////////////
// Setup Menus
////////////////////////////////////////////////
void SetupMenu(void)
{
    if (!UsingMenus && !ConPanel)       // Doing this check for multiplay
                                        // menus
    {
        MNU_SetupMenu();
        // Clear the previous ESC key press
        KEY_PRESSED(KEYSC_ESC) = FALSE;
        UsingMenus = TRUE;
    }
}

////////////////////////////////////////////////
// Setup the main menu
// This function will not loop if in modem
// or network game, otherwise it stops the
// game play until user finished in menus.
////////////////////////////////////////////////
#define MNU_SENSITIVITY 10              // The menu's mouse sensitivity, should be real low

void MNU_DoMenu( CTLType type, PLAYERp pp )
{
    BOOL resetitem;
    UCHAR key;
    long zero = 0;
    static int handle2 = 0;
    static long limitmove=0;
    static BOOL select_held=FALSE;

    resetitem = TRUE;

    if (cust_callback != NULL)
    {
        cust_callback(cust_callback_call, cust_callback_item);
        return;
    }

    //ControlPanelType = type;
    SetupMenu();

    // Zero out the input structure
    mnu_input.button0 = mnu_input.button1 = FALSE;
    mnu_input.dir = dir_None;

    // should not get input if you are editing a save game slot
    if (totalclock < limitmove)
        limitmove = totalclock;
    if (!MenuInputMode)
    {
        UserInput tst_input;
        BOOL select_held = FALSE;


        // Zero out the input structure
        tst_input.button0 = tst_input.button1 = FALSE;
        tst_input.dir = dir_None;

        if (!select_held)
        {
            CONTROL_GetUserInput(&tst_input);
            mnu_input_buffered.dir = tst_input.dir;
        }

        if (mnu_input_buffered.button0 || mnu_input_buffered.button1)
        {
            if (tst_input.button0 == mnu_input_buffered.button0 &&
                tst_input.button1 == mnu_input_buffered.button1)
            {
                select_held = TRUE;
            }
            else
            if(totalclock - limitmove > 7)
            {
                mnu_input.button0 = mnu_input_buffered.button0;
                mnu_input.button1 = mnu_input_buffered.button1;

                mnu_input_buffered.button0 = tst_input.button0;
                mnu_input_buffered.button1 = tst_input.button1;
            }
        }
        else
        {
            select_held = FALSE;
            mnu_input_buffered.button0 = tst_input.button0;
            mnu_input_buffered.button1 = tst_input.button1;
        }

        if (totalclock - limitmove > 7 && !select_held)
        {
            mnu_input.dir = mnu_input_buffered.dir;

            if (mnu_input.dir != dir_None)
                if(!FX_SoundActive(handle2))
                    handle2 = PlaySound(DIGI_STAR,&zero,&zero,&zero,v3df_dontpan);

            limitmove = totalclock;
            mnu_input_buffered.dir = dir_None;
        }
    }

    if (mnu_input.dir == dir_North)
    {
        MNU_PrevItem();
        resetitem = TRUE;
    }
    else
    if (mnu_input.dir == dir_South)
    {
        MNU_NextItem();
        resetitem = TRUE;
    }
    else
    if (mnu_input.button0)
    {
        static int handle5=0;
        if(!FX_SoundActive(handle5))
            handle5 = PlaySound(DIGI_SWORDSWOOSH,&zero,&zero,&zero,v3df_dontpan);
        KB_ClearKeysDown();
        MNU_DoItem();
        resetitem = TRUE;
    }
    else
    if (mnu_input.dir == dir_West
        && currentmenu->items[currentmenu->cursor].type == mt_slider)
    {
        MNU_DoSlider(-1, &currentmenu->items[currentmenu->cursor], FALSE);
        resetitem = TRUE;
    }
    else
    if (mnu_input.dir == dir_East
        && currentmenu->items[currentmenu->cursor].type == mt_slider)
    {
        MNU_DoSlider(1, &currentmenu->items[currentmenu->cursor], FALSE);
        resetitem = TRUE;
    }
    else
    if (mnu_input.button1)
    {
        static int handle3=0;
        if(!FX_SoundActive(handle3))
            handle3 = PlaySound(DIGI_SWORDSWOOSH,&zero,&zero,&zero,v3df_dontpan);
        MNU_UpLevel();
        resetitem = TRUE;
    }
    else
    if (MNU_DoHotkey())
    {
        static int handle4=0;
        if (!FX_SoundActive(handle4))
            handle4 = PlaySound(DIGI_STAR,&zero,&zero,&zero,v3df_dontpan);
        resetitem = TRUE;
        mnu_input_buffered.button0 = mnu_input_buffered.button1 = FALSE;
    }
    else
    if (KEY_PRESSED(KEYSC_F12))                                      // 100216
    {
        KEY_PRESSED(KEYSC_F12) = 0;
        PauseAction();
        screencapture("swpxxxx.pcx", KEY_PRESSED(KEYSC_LSHIFT) | KEY_PRESSED(KEYSC_LSHIFT));
        ResumeAction();
    }
    else
        resetitem = FALSE;

    // !FRANK! I added this because the old custom was only called for drawing
    // Needed one for drawing and moving.
    if (currentmenu->move_custom)
        currentmenu->move_custom(uc_setup, NULL);

    if (resetitem)
    {
        KB_ClearKeysDown();
        ResetKeys();
    }
}

////////////////////////////////////////////////
//  Checks to see if we should be in menus
////////////////////////////////////////////////
void MNU_CheckForMenus(void)
{
    extern BOOL GamePaused;

    if (UsingMenus)
    {
        //if (MoveSkip2 == 0)
            MNU_DoMenu(ct_mainmenu, Player + myconnectindex);

    }
    else
    {
        if (KEY_PRESSED(KEYSC_ESC) && dimensionmode == 3 && !ConPanel)
        {
            KEY_PRESSED(KEYSC_ESC) = 0;
            KB_ClearKeysDown();
            // setup sliders/buttons
            MNU_InitMenus();
            MNU_DoMenu(ct_mainmenu, Player + myconnectindex);
            pMenuClearTextLine(Player + myconnectindex);
            PauseGame();
        }
    }
}

void MNU_CheckForMenusAnyKey(void)
{
    if (UsingMenus)
    {
        //if (MoveSkip2 == 0)
            MNU_DoMenu(ct_mainmenu, Player + myconnectindex);

    }
    else
    {
        if (KeyPressed())
        {
            ResetKeys();
            KB_ClearKeysDown();
            MNU_InitMenus();
            MNU_DoMenu(ct_mainmenu, Player + myconnectindex);
            pMenuClearTextLine(Player + myconnectindex);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Miscellaneous Routines
///////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct RGB_color_typ
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} RGB_color, *RGB_color_ptr;

#define PALETTE_MASK            0x3c6
#define PALETTE_READ            0x3c7
#define PALETTE_WRITE           0x3c8
#define PALETTE_DATA            0x3c9

unsigned char palette_data[256][3];     // Global palette array

// V E R T I C A L   R E T R A C E   V A R I A B L E S //////////////////////////////////////////

#define VGA_INPUT_STATUS_1      0x3DA   // VGA status register 1, bit 3 is the vsync
                    // 1 = retrace in progress
                    // 0 = no retrace
#define VGA_VSYNC_MASK          0x08    // Masks off unwanted bits of status register.


// These routines are not used and should not be used.  Would interfere with VESA palette
// cards.
#if 0
/////////////////////////////////////////////////
// WaitForVsync
// Waits for a vertical retrace to occur.  If one is in progress, it waits for the next one.
/////////////////////////////////////////////////
void WaitForVsync(void)
{
    while (inp(VGA_INPUT_STATUS_1) & VGA_VSYNC_MASK);
    // Retrace in progress, wait.

    // Wait for vsync, and exit.
    while (!inp(VGA_INPUT_STATUS_1) & VGA_VSYNC_MASK);
}

void Get_Palette(unsigned char *pal)
{
    int i;

    outp(PALETTE_READ, 0);
    for (i = 0; i < 768; i++)
        pal[i] = inp(PALETTE_DATA);
}

void Set_Palette(unsigned char *buff)
{
    int i;

    outp(PALETTE_WRITE, 0);             // Resets color ram pointer to 1st
                                        // color
    for (i = 0; i < 768; i++)
        outp(PALETTE_DATA, buff[i]);
}

#endif


/*
=================================================================================================
=
=       FadeOut - Fades the palette to color at assigned click rate
=
=================================================================================================
*/
// Heres some temp timer junk for this routine.  Replace it with game timer stuff later.
//unsigned int *clock  = (unsigned int *)0x046C;

void Fade_Timer(int clicks)
{
//        unsigned int now;
    long now;

    now = totalclock;

    while (abs(totalclock - now) < clicks) handleevents();
}

void FadeIn(unsigned char startcolor, unsigned int clicks)
{
    int i, palreg, usereg, tmpreg1 = 0, tmpreg2 = 0;
    RGB_color color;
    unsigned char temp_pal[768], *palette;

    if (getrendermode() >= 3)
        return;

    palette = &palette_data[0][0];

    color.red = palette_data[startcolor][0];
    color.green = palette_data[startcolor][1];
    color.blue = palette_data[startcolor][2];

    usereg = 0;
    for (i = 0; i < 768; i++)
    {
        if (usereg == 0)
            temp_pal[i] = color.red;
        else
        if (usereg == 1)
            temp_pal[i] = color.green;
        else
            temp_pal[i] = color.blue;

        if (++usereg > 2)
            usereg = 0;
    }

    for (i = 0; i < 32; i++)
    {
        for (palreg = 0; palreg < 768; palreg++)
        {
            tmpreg1 = (int) (temp_pal[palreg]) + 2;
            tmpreg2 = (int) (temp_pal[palreg]) - 2;
            if (tmpreg1 > 255)
                tmpreg1 = 255;
            if (tmpreg2 < 0)
                tmpreg2 = 0;

            if (temp_pal[palreg] < palette[palreg])
            {
                if ((temp_pal[palreg] = tmpreg1) > palette[palreg])
                    temp_pal[palreg] = palette[palreg];
            }
            else
            if (temp_pal[palreg] > palette[palreg])
                if ((temp_pal[palreg] = tmpreg2) < palette[palreg])
                    temp_pal[palreg] = palette[palreg];

        }

        set_pal(&temp_pal[0]);

        // Delay clicks
        Fade_Timer(clicks);
    }
}

void FadeOut(unsigned char targetcolor, unsigned int clicks)
{
    int i, palreg, usereg = 0, tmpreg1 = 0, tmpreg2 = 0;
    RGB_color color;
    unsigned char temp_pal[768];

    if (getrendermode() >= 3)
        return;

    color.red = palette_data[targetcolor][0];
    color.green = palette_data[targetcolor][1];
    color.blue = palette_data[targetcolor][2];

    memcpy(&temp_pal[0], &palette_data[0][0], 768);

    for (i = 0; i < 32; i++)
    {
        for (palreg = 0; palreg < 768; palreg++)
        {
            tmpreg1 = (int) (temp_pal[palreg]) + 2;
            tmpreg2 = (int) (temp_pal[palreg]) - 2;
            if (tmpreg1 > 255)
                tmpreg1 = 255;
            if (tmpreg2 < 0)
                tmpreg2 = 0;

            if (usereg == 0)
            {
                if (temp_pal[palreg] < color.red)
                {
                    if ((temp_pal[palreg] = tmpreg1) > color.red)
                        temp_pal[palreg] = color.red;
                }
                else
                if (temp_pal[palreg] > color.red)
                    if ((temp_pal[palreg] = tmpreg2) < color.red)
                        temp_pal[palreg] = color.red;
            }
            else
            if (usereg == 1)
            {
                if (temp_pal[palreg] < color.green)
                {
                    if ((temp_pal[palreg] = tmpreg1) > color.green)
                        temp_pal[palreg] = color.green;
                }
                else
                if (temp_pal[palreg] > color.green)
                    if ((temp_pal[palreg] = tmpreg2) < color.green)
                        temp_pal[palreg] = color.green;
            }
            else
            if (usereg == 2)
            {
                if (temp_pal[palreg] < color.blue)
                {
                    if ((temp_pal[palreg] = tmpreg1) > color.blue)
                        temp_pal[palreg] = color.blue;
                }
                else
                if (temp_pal[palreg] > color.blue)
                    if ((temp_pal[palreg] = tmpreg2) < color.blue)
                        temp_pal[palreg] = color.blue;
            }

            if (++usereg > 2)
                usereg = 0;
        }

        set_pal(&temp_pal[0]);

        // Delay clicks
        Fade_Timer(clicks);
    }
}

//////////////////////////////////////////////////////////////////////////////
#define FADE_DAMAGE_FACTOR  3   // 100 health / 32 shade cycles = 3.125

// Fades from 100% to 62.5% somewhat quickly,
//  then from 62.5% to 37.5% slowly,
//  then from 37.5% to 0% quickly.
// This seems to capture the pain caused by enemy shots, plus the extreme
//  fade caused by being blinded or intense pain.
// Perhaps the next step would be to apply a gentle smoothing to the
//  intersections of these lines.
static int faderamp[32] =
{
	64,60,56,52,48,44,	// y=64-4x

	40,39,38,38,37,		// y=44.8-(16/20)x
	36,35,34,34,33,
	32,31,30,30,29,
	28,27,26,26,25,

	24,20,16,12,8, 4	// y=128-4x
};

unsigned char ppalette[MAX_SW_PLAYERS_REG][768];

//////////////////////////////////////////
// Set the amount of redness for damage
// the player just took
//////////////////////////////////////////
void SetFadeAmt(PLAYERp pp, short damage, unsigned char startcolor)
{
    int palreg, usereg = 0, tmpreg1 = 0, tmpreg2 = 0;
    short fadedamage=0;
    RGB_color color;

    //CON_ConMessage("SetAmt: fadeamt = %d, startcolor = %d, pp = %d",pp->FadeAmt,startcolor,pp->StartColor);

    if (abs(pp->FadeAmt) > 0 && startcolor == pp->StartColor)
        return;

    // Don't ever over ride flash bomb
    if (pp->StartColor == 1 && abs(pp->FadeAmt) > 0)
        return;

    // Reset the palette
    if (pp == Player + screenpeek)
    {
	if (getrendermode() < 3)
        COVERsetbrightness(gs.Brightness,(char *)palette_data);
	else
	    setpalettefade(0,0,0,0);
        if (pp->FadeAmt <= 0)
            GetPaletteFromVESA(&ppalette[screenpeek][0]);
    }

    if (damage < -150 && damage > -1000)
        fadedamage = 150;
    else
    if (damage < -1000)  // Underwater
        fadedamage = abs(damage+1000);
    else
        fadedamage = abs(damage);

    if (damage >= -5 && damage < 0)
        fadedamage += 10;

    // Don't let red to TOO red
    if (startcolor == COLOR_PAIN && fadedamage > 100)
        fadedamage = 100;

    pp->FadeAmt = fadedamage / FADE_DAMAGE_FACTOR;

    if (pp->FadeAmt <= 0)
    {
        pp->FadeAmt = 0;
        return;
    }

    // It's a health item, just do a preset flash amount
    if (damage > 0)
        pp->FadeAmt = 3;

    pp->StartColor = startcolor;

    pp->FadeTics = 0;

    // Set player's palette to current game palette
    GetPaletteFromVESA(pp->temp_pal);

    if (damage < -1000 && startcolor == 210) // underwater
    {
        palette_data[pp->StartColor][0] = 0;
        palette_data[pp->StartColor][1] = 32;
        palette_data[pp->StartColor][2] = 255;
    }

    color.red = palette_data[pp->StartColor][0];
    color.green = palette_data[pp->StartColor][1];
    color.blue = palette_data[pp->StartColor][2];

    for (palreg = 0; palreg < 768; palreg++)
    {
        tmpreg1 = (int) (pp->temp_pal[palreg]) + ((2 * pp->FadeAmt) + 4);
        tmpreg2 = (int) (pp->temp_pal[palreg]) - ((2 * pp->FadeAmt) + 4);
        if (tmpreg1 > 255)
            tmpreg1 = 255;
        if (tmpreg2 < 0)
            tmpreg2 = 0;

        if (usereg == 0)
        {
            if (pp->temp_pal[palreg] < color.red)
            {
                if ((pp->temp_pal[palreg] = tmpreg1) > color.red)
                    pp->temp_pal[palreg] = color.red;
            }
            else
            if (pp->temp_pal[palreg] > color.red)
                if ((pp->temp_pal[palreg] = tmpreg2) < color.red)
                    pp->temp_pal[palreg] = color.red;
        }
        else
        if (usereg == 1)
        {
            if (pp->temp_pal[palreg] < color.green)
            {
                if ((pp->temp_pal[palreg] = tmpreg1) > color.green)
                    pp->temp_pal[palreg] = color.green;
            }
            else
            if (pp->temp_pal[palreg] > color.green)
                if ((pp->temp_pal[palreg] = tmpreg2) < color.green)
                    pp->temp_pal[palreg] = color.green;
        }
        else
        if (usereg == 2)
        {
            if (pp->temp_pal[palreg] < color.blue)
            {
                if ((pp->temp_pal[palreg] = tmpreg1) > color.blue)
                    pp->temp_pal[palreg] = color.blue;
            }
            else
            if (pp->temp_pal[palreg] > color.blue)
                if ((pp->temp_pal[palreg] = tmpreg2) < color.blue)
                    pp->temp_pal[palreg] = color.blue;
        }

        if (++usereg > 2)
            usereg = 0;
    }

    // Do initial palette set
    if (pp == Player + screenpeek)
    {
	if (getrendermode() < 3)
	    set_pal(pp->temp_pal);
	else
	    setpalettefade(color.red, color.green, color.blue, faderamp[ min(31,max(0,32-abs(pp->FadeAmt))) ] );
    if (damage < -1000)
        pp->FadeAmt = 1000;  // Don't call DoPaletteFlash for underwater stuff
    }
}

//////////////////////////////////////////
// Do the screen reddness based on damage
//////////////////////////////////////////
#define MAXFADETICS     5
void DoPaletteFlash(PLAYERp pp)
{
    int i, palreg, tmpreg1 = 0, tmpreg2 = 0;
    unsigned char *pal_ptr = &ppalette[screenpeek][0];


    if (pp->FadeAmt <= 1)
    {
        pp->FadeAmt = 0;
        pp->StartColor = 0;
        if (pp == Player + screenpeek)
        {
	    if (getrendermode() < 3)
            COVERsetbrightness(gs.Brightness,(char *)palette_data);
	    else
  		    setpalettefade(0,0,0,0);
        memcpy(pp->temp_pal, palette_data, sizeof(palette_data));
        DoPlayerDivePalette(pp);  // Check Dive again
        DoPlayerNightVisionPalette(pp);  // Check Night Vision again
        }

        return;
    }

    pp->FadeTics += synctics;           // Add this frame's tic amount to
                                        // counter
    if (pp->FadeTics >= MAXFADETICS)
    {
        while (pp->FadeTics >= MAXFADETICS)
        {
            pp->FadeTics -= MAXFADETICS;
            pp->FadeAmt--;              // Decrement FadeAmt till it gets to
                                        // 0 again.
        }
    }
    else
        return;                         // Return if they were not >
                                        // MAXFADETICS
    if (pp->FadeAmt > 32)
        return;

    if (pp->FadeAmt <= 1)
    {
        pp->FadeAmt = 0;
        pp->StartColor = 0;
        if (pp == Player + screenpeek)
        {
            if (getrendermode() < 3)
                COVERsetbrightness(gs.Brightness,(char *)palette_data);
	        else
	            setpalettefade(0,0,0,0);
            memcpy(pp->temp_pal, palette_data, sizeof(palette_data));
            DoPlayerDivePalette(pp);  // Check Dive again
            DoPlayerNightVisionPalette(pp);  // Check Night Vision again
        }
        return;
    }
    else
    {
        //CON_Message("gamavalues = %d, %d, %d",pp->temp_pal[pp->StartColor],pp->temp_pal[pp->StartColor+1],pp->temp_pal[pp->StartColor+2]);
        for (palreg = 0; palreg < 768; palreg++)
        {
            tmpreg1 = (int) (pp->temp_pal[palreg]) + 2;
            tmpreg2 = (int) (pp->temp_pal[palreg]) - 2;
            if (tmpreg1 > 255)
                tmpreg1 = 255;
            if (tmpreg2 < 0)
                tmpreg2 = 0;

            if (pp->temp_pal[palreg] < pal_ptr[palreg])
            {
                if ((pp->temp_pal[palreg] = tmpreg1) > pal_ptr[palreg])
                    pp->temp_pal[palreg] = pal_ptr[palreg];
            }
            else
            if (pp->temp_pal[palreg] > pal_ptr[palreg])
                if ((pp->temp_pal[palreg] = tmpreg2) < pal_ptr[palreg])
                    pp->temp_pal[palreg] = pal_ptr[palreg];

        }

        // Only hard set the palette if this is currently the player's view
        if (pp == Player + screenpeek)
        {
	        if (getrendermode() < 3)
	            set_pal(pp->temp_pal);
	        else
	        {
		        setpalettefade(
		  	    palette_data[pp->StartColor][0],
			    palette_data[pp->StartColor][1],
			    palette_data[pp->StartColor][2],
			    faderamp[ min(31,max(0,32-abs(pp->FadeAmt))) ]);
		    }
        }
    }
}

VOID ResetPalette(PLAYERp pp)
{
    if (getrendermode() < 3)
    COVERsetbrightness(gs.Brightness,(char *)palette_data);
    else
	setpalettefade(0,0,0,0);
    memcpy(pp->temp_pal, palette_data, sizeof(palette_data));
    //DoPlayerDivePalette(pp);  // Check Dive again
    //DoPlayerNightVisionPalette(pp);  // Check Night Vision again
    pp->FadeAmt = 0;
    pp->StartColor = 0;
    pp->FadeTics = 0;
}

// vim:enc=utf-8:

//-----------------------------------------------------------------------------------

void MNU_DrawOptionString(short x, short y, char *string, short shade, short pal)
{
    int siz, ndx, offset, s;
    char c;
    short ac;

    offset = x;
    siz = 52000;

    s = 0;
    if (string[0] == '^')
        s = 1;

    for (ndx = s; (c = string[ndx]) != 0; ndx++)
    {
        c = toupper(c);
        ac = c - 'A' + 3706;
        if ((ac < 3706 || ac > 3741)  && c != asc_Space )
            break;

        if (c > asc_Space && c < 127)
        {
            rotatesprite(offset<<16, y<<16, siz, 0, ac, shade, pal, MenuDrawFlags, 0, 0, xdim - 1, ydim - 1);
            offset += 14;
            if (c == 'M' || c == 'W')
                offset += 5;
            if (c == 'A' || c == 'O' || c == 'U')
                offset += 3;
        }
        else
        if (c == asc_Space)
        {
            offset += 8;                   // Special case for space char
        }
    }
}

BOOL MNU_IpAddress(void)
{
    if (MenuInputMode)
    {
        MenuInputMode = FALSE;
        memset(MessageInputString, '\0', sizeof(MessageInputString));
    }
    else
    {
        while (KB_KeyWaiting())
        {
            if (KB_Getch() == 0)
                KB_Getch();
        }
        MenuInputMode = TRUE;
    }
    return (TRUE);
}

BOOL MNU_DoIpAddress(void)
{
    signed char MNU_InputString(char *, short);
    char tmp[sizeof(MessageInputString)];
    static BOOL cur_show;
    short i, w, xx = 170;
    short ipnum = currentmenu->cursor;

    strcpy(sPlayerBack, IPAddressArg[ipnum]);

    // get input
    if (MenuInputMode)
    {
       switch(MNU_InputString(MessageInputString, 128))
       {
        case -1: // Cancel Input (pressed ESC) or Err
            KB_ClearKeysDown();
            KB_FlushKeyboardQueue();
            MenuInputMode = FALSE;
            memset(MessageInputString, '\0', sizeof(MessageInputString));
            strcpy(IPAddressArg[ipnum], sPlayerBack);
            break;
        case FALSE: // Input finished (RETURN)
            MenuInputMode = FALSE;
            strcpy(IPAddressArg[ipnum], MessageInputString);
            KB_ClearKeysDown();
            KB_FlushKeyboardQueue();
            memset(MessageInputString, '\0', sizeof(MessageInputString));
            break;
        case TRUE: // Got input
            break;
       }

	    cur_show = (totalclock & 32);
        if (cur_show)
            sprintf(tmp, "%s_", MessageInputString); // add a cursor to the end
        else
            strcpy(tmp, MessageInputString);

        MNU_DrawString(xx, OPT_LINE(ipnum+1), tmp, 1,16);
        for (i=0; i<8; i++)
        {
            if (i != ipnum && IPAddressArg[i][0] != '\0')
                MNU_DrawString(xx, OPT_LINE(i+1), IPAddressArg[i], 1, 16);
        }
    }
    else
    {
       for (i=0; i<8; i++)
       {
           if (IPAddressArg[i][0] != '\0')
               MNU_DrawString(xx, OPT_LINE(i+1), IPAddressArg[i], 1, 16);
       }
    }
    return(TRUE);
}

BOOL MNU_PlayerName(void)
{
    if (MenuInputMode)
    {
        MenuInputMode = FALSE;
        memset(MessageInputString, '\0', sizeof(MessageInputString));
    }
    else
    {
        while (KB_KeyWaiting())
        {
            if (KB_Getch() == 0)
                KB_Getch();
        }
        MenuInputMode = TRUE;
    }
    return (TRUE);
}

BOOL MNU_DoPlayerName(void)
{
    signed char MNU_InputString(char *, short);
    char tmp[sizeof(MessageInputString)];
    static BOOL cur_show;

    if (currentmenu->cursor == 0)
        strcpy(sPlayerBack, PlayerNameArg);

    // get input
    if (MenuInputMode)
    {
       switch(MNU_InputString(MessageInputString, 128))
       {
        case -1: // Cancel Input (pressed ESC) or Err
            KB_ClearKeysDown();
            KB_FlushKeyboardQueue();
            MenuInputMode = FALSE;
            memset(MessageInputString, '\0', sizeof(MessageInputString));
            if (currentmenu->cursor == 0)
                strcpy(PlayerNameArg, sPlayerBack);
            break;
        case FALSE: // Input finished (RETURN)
            MenuInputMode = FALSE;
            if (currentmenu->cursor == 0)
                strcpy(PlayerNameArg, MessageInputString);
            KB_ClearKeysDown();
            KB_FlushKeyboardQueue();
            memset(MessageInputString, '\0', sizeof(MessageInputString));
            break;
        case TRUE: // Got input
            break;
       }

	    cur_show = (totalclock & 32);
        if (cur_show)
            sprintf(tmp, "%s_", MessageInputString); // add a cursor to the end
        else
            strcpy(tmp, MessageInputString);

        if (currentmenu->cursor == 0)
            MNU_DrawString(150, OPT_LINE(0), tmp, 1,16);
    }
    else
    if (PlayerNameArg[0] != '\0')
    {
       if (!gs.SetHighres || !usehightile || NoHrp)
           MNU_DrawString(150, OPT_LINE(0), PlayerNameArg, 1, PALETTE_PLAYER0+gs.NetColor); // 16
       else
           MNU_DrawString(150, OPT_LINE(0), PlayerNameArg, 1, 30);
       strcpy(CommPlayerName, PlayerNameArg);
    }
    if (CommEnabled && !bHosting)
        MNU_ClientDisplay();
    return(TRUE);
}

BOOL MNU_CheckMapStatus(MenuItem *item)
{
    if (!CommEnabled)
        SET(item->flags, mf_disabled);
    if (CommEnabled && !bHosting)
        SET(item->flags, mf_disabled);
    if (bGrp)
        SET(item->flags, mf_disabled);
    return (TRUE);
}

BOOL MNU_CheckOption(MenuItem *item)
{
    if (!CommEnabled)
        SET(item->flags, mf_disabled);
      if (CommEnabled && !bHosting)
          strcpy(MultiOption, "Apply...");
      else
          strcpy(MultiOption, "Start Game...");
    return (TRUE);
}

BOOL MNU_ShareMapsCheck(MenuItem *item)
{
    extern BOOL bNoUserMaps;

    MNU_CheckMulti(item);
    MNU_ShareWareCheck(item);
    if (bGrp)
        SET(item->flags, mf_disabled);
    return (TRUE);
}

BOOL MNU_HostingCheck(MenuItem *item)
{
    if (CommEnabled && !bHosting)
        SET(item->flags, mf_disabled);
    return (TRUE);
}

BOOL MNU_CheckMulti(MenuItem *item)
{
    if (!CommEnabled)
        SET(item->flags, mf_disabled);
    //MNU_ShareWareCheck(item);
    return (TRUE);
}

BOOL MNU_HrpCheck(MenuItem *item)
{
    if (NoHrp > 0)
        SET(item->flags, mf_disabled);
    if (menucmp(currentmenu->text, "^Screen") == 0)
        MNU_BitCheck(item);
    return (TRUE);
}

BOOL MNU_BitCheck(MenuItem *item)
{
    if (ScreenBPP < 16)
        SET(item->flags, mf_disabled);
    else
        RESET(item->flags, mf_disabled);
    return (TRUE);
}

BOOL MNU_CheckStart(MenuItem *item)
{
    extern short iStart;

    if (iStart == 0)
        SET(item->flags, mf_disabled);
    else
        RESET(item->flags, mf_disabled);
    return (TRUE);
}

BOOL MNU_MapGameInfo(UserCall call, MenuItem *item)
{
	char *morestr = "Up / Dn / Home / End / PgUp / PgDn";
	short dx, dy;

    if (!gs.PlayCD)
    {
	    dx = 0, dy = 8;
        MNU_MeasureSmallString(morestr,&dx,&dy);
	    MNU_DrawSmallString(TEXT_XCENTER(dx), 175, morestr, 0, 17);
	}

    morestr = "Delete to kill music";
	dx = 0, dy = 8;
    MNU_MeasureSmallString(morestr,&dx,&dy);
	MNU_DrawSmallString(TEXT_XCENTER(dx), 185, morestr, 1, 17);

	return (TRUE);
}

BOOL MNU_StartNetGame(void)
{
    extern BOOL ShortGameMode, DemoInitOnce, FirstTimeIntoGame;  // BOOL ExitLevel,
    extern BOOL FinishedLevel;
    long pnum;

    // always assumed that a demo is playing

    ready2send = 0;
    // Skill can go negative here
    Skill = gs.NetMonsters-1;
    Level = gs.NetLevel + 1;
    if (!AutoNet)
        ExitMenus();
    DemoPlaying = FALSE;
    ExitLevel = TRUE;
    NewGame = TRUE;
    // restart demo for multi-play mode
    DemoInitOnce = FALSE;
    ResetMenuInput();
    if (!AutoNet && FirstTimeIntoGame)
        return TRUE;

    // need to set gNet vars for self
    // everone else gets a packet to set them
    gNet.AutoAim            = gs.AutoAim;
    gNet.SpawnMarkers       = gs.NetSpawnMarkers;
    gNet.HurtTeammate       = gs.NetHurtTeammate;
    gNet.Nuke               = gs.NetNuke;
    gNet.KillLimit          = gs.NetKillLimit*10;
    gNet.TimeLimit          = TimeLimitTable[gs.NetTimeLimit]*60*120;

    if (ShortGameMode)
        {
        gNet.KillLimit /= 10;
        gNet.TimeLimit /= 2;
        }

    gNet.TimeLimitClock     = gNet.TimeLimit;
    gNet.TeamPlay           = gs.NetTeamPlay;
    gNet.MultiGameType      = gs.NetGameType+1;

    if (gNet.MultiGameType == MULTI_GAME_COMMBAT_NO_RESPAWN)
        {
        gNet.MultiGameType = MULTI_GAME_COMMBAT;
        gNet.NoRespawn = TRUE;
        }
    else
        {
        gNet.NoRespawn = FALSE;
        }

    if (CommEnabled)
        {
        PACKET_NEW_GAME p;

        p.PacketType = PACKET_TYPE_NEW_GAME;
        p.Level = Level;
        p.Skill = Skill;
        p.GameType = gs.NetGameType;
        p.AutoAim = gs.AutoAim;
        p.HurtTeammate = gs.NetHurtTeammate;
        p.TeamPlay = gs.NetTeamPlay;
        p.SpawnMarkers = gs.NetSpawnMarkers;
        p.KillLimit = gs.NetKillLimit;
        p.TimeLimit = gs.NetTimeLimit;
        p.Nuke = gs.NetNuke;
        strcpy(p.usermap, UserMapName);
        strcpy(p.useraddon, UserAddon);
        netbroadcastpacket((char *)(&p), sizeof(p));
        strcpy(CurrMapName, UserMapName);
        }

    if (gs.NetGameType == 2)  // co-op
        ExitMenus();

    return (TRUE);
}

BOOL MNU_StartMapGame(void)
{
    PLAYERp pp = Player + screenpeek;
    int handle = 0, x;
    long zero = 0;
    extern char LevelSong[];
    extern BOOL bMusic;

    bMusic = FALSE;
    LevelSong[0]   = 0;
    CurrMapName[0] = 0;
    ready2send = 0;
    Level = 0;
    if (strlen(MyMaps[MapCursor]) < 5)
        return FALSE;
    initprintf("Starting User Map: %s\n", MyMaps[MapCursor]);

    if (!CommEnabled)
    {
        sprintf(ds,"%sgamex.asv", svgame);
        DeleteAutosave(0, ds);
    }

    Autodone = 0;
    if (!AutoNet)
        ExitMenus();

    DemoPlaying = FALSE;
    ExitLevel = TRUE;
    NewGame = TRUE;
    DemoMode = FALSE;
    CameraTestMode = FALSE;

    if (!CommEnabled)
    {
        if (handle > FX_Ok)
            while(FX_SoundActive(handle))
 			      handleevents();
    }

    return (TRUE);
}

BOOL MNU_MapStartUp(void)
{
	cust_callback = NULL;
	if (!MNU_StartMapGame())
	    return FALSE;
	KB_ClearLastScanCode();
	KB_ClearKeysDown();
    strcpy(UserMapName, MyMaps[MapCursor]);
    if (!CommEnabled)
    {
        addsearchpath("Maps");
        checkforsaves(UserMapName);
    }
    checkforhrp(UserMapName, 1);
    LoadDefFiles(1);                       // load custom only
    return TRUE;
}

BOOL MNU_StartMultiGame(void)
{
    short w,h;

    if (!bHosting)                  // apply
    {
        if (UserMapName[0] == 0 && gs.NetLevel == 0)
            return FALSE;

        if (CommEnabled || CommPlayers >= 2)
        {
            PACKET_NAME_CHANGE p;

            p.PacketType = PACKET_TYPE_NAME_CHANGE;
            strncpy(p.PlayerName, PlayerNameArg, strlen(PlayerNameArg));
            p.Color = gs.NetColor;
            netbroadcastpacket((char *)(&p), sizeof(p));
            ExitMenus();
        }
        return (TRUE);
    }
    if (bMapmenu)
        if (!MNU_MapStartUp())
            return FALSE;
    MNU_StartNetGame();

    return (TRUE);
}

BOOL MNU_MapKeyIn(void)
{
    short i, j;

    if (!bMapmenu)
        bMapmenu = TRUE;

    MapCursor = currentmenu->cursor;

	if (KB_KeyPressed(sc_PgDn))
	{
		KB_ClearKeyDown(sc_PgDn);
        MapPosition += 14;
        if (MapPosition > NumUserMaps-15)
           {
           MapPosition = NumUserMaps-15;
           if (MapPosition < 0)
               MapPosition = 0;
           }
        j = 0;
        for (i=MapPosition; i<MapPosition+15; i++)
            {
            strcpy(MyMaps[j], UserMaps[i]);
            j++;
            }
        MNU_SetupGroup();
        SetRedrawScreen(&Player[myconnectindex]);
	}
	else
	if (KB_KeyPressed(sc_PgUp))
	{
		KB_ClearKeyDown(sc_PgDn);

        if (totalclock < Menutick)
            return TRUE;
        Menutick = totalclock + 10;

        MapPosition -= 14;
        if (MapPosition < 0)
            MapPosition = 0;
        j = 0;
        for (i=MapPosition; i<MapPosition+15; i++)
            {
            strcpy(MyMaps[j], UserMaps[i]);
            j++;
            }
        MNU_SetupGroup();
        SetRedrawScreen(&Player[myconnectindex]);
	}
	else
	if (KB_KeyPressed(sc_Home))
	{
		KB_ClearKeyDown(sc_Home);
        MapPosition = 0;
        j = 0;
        for (i=MapPosition; i<MapPosition+15; i++)
            {
            strcpy(MyMaps[j], UserMaps[i]);
            j++;
            }
        MNU_SetupGroup();
        SetRedrawScreen(&Player[myconnectindex]);
	}
	else
	if (KB_KeyPressed(sc_End))
	{
		KB_ClearKeyDown(sc_End);
        MapPosition = NumUserMaps-15;
        if (MapPosition < 0)
            MapPosition = 0;
        j = 0;
        for (i=MapPosition; i<MapPosition+15; i++)
            {
            strcpy(MyMaps[j], UserMaps[i]);
            j++;
            }
        MNU_SetupGroup();
        SetRedrawScreen(&Player[myconnectindex]);
	}

    cust_callback = NULL;
	return TRUE;
}

BOOL MNU_CDMusicKeyIn(void)
{
    if (!bMusicmenu)
        bMusicmenu = TRUE;

    MusicCursor = currentmenu->cursor;

	if (KB_KeyPressed(sc_Delete))
	{
		KB_ClearKeyDown(sc_Delete);
        CDAudio_Stop();
        StopSound();
	}

    cust_callback = NULL;
	return TRUE;
}

BOOL MNU_MusicKeyIn(void)
{
    short i, j;
    long zero = 0;

    if (gs.PlayCD)
    {
        MNU_CDMusicKeyIn();
        return TRUE;
    }

    if (!bMusicmenu)
        bMusicmenu = TRUE;

    MusicCursor = currentmenu->cursor;

	if (KB_KeyPressed(sc_PgDn))
	{
		KB_ClearKeyDown(sc_PgDn);
        MusicPosition += 14;
        if (MusicPosition > NumUserMusic-15)
        {
           MusicPosition = NumUserMusic-15;
           if (MusicPosition < 0)
               MusicPosition = 0;
        }
        j = 0;
        for (i=MusicPosition; i<MusicPosition+15; i++)
        {
            GetName(UserMusic[i], MyMusic[j]);
            j++;
        }
        MNU_SetupGroup();
        SetRedrawScreen(&Player[myconnectindex]);
        PlaySound(DIGI_STAR,&zero,&zero,&zero,v3df_dontpan);
	}
	else
	if (KB_KeyPressed(sc_PgUp))
	{
		KB_ClearKeyDown(sc_PgDn);

        if (totalclock < Menutick)
            return TRUE;
        Menutick = totalclock + 10;

        MusicPosition -= 14;
        if (MusicPosition < 0)
            MusicPosition = 0;
        j = 0;
        for (i=MusicPosition; i<MusicPosition+15; i++)
        {
            GetName(UserMusic[i], MyMusic[j]);
            j++;
        }
        MNU_SetupGroup();
        SetRedrawScreen(&Player[myconnectindex]);
        PlaySound(DIGI_STAR,&zero,&zero,&zero,v3df_dontpan);
	}
	else
	if (KB_KeyPressed(sc_Home))
	{
		KB_ClearKeyDown(sc_Home);
        MusicPosition = 0;
        j = 0;
        for (i=MusicPosition; i<MusicPosition+15; i++)
        {
            GetName(UserMusic[i], MyMusic[j]);
            j++;
        }
        MNU_SetupGroup();
        SetRedrawScreen(&Player[myconnectindex]);
        PlaySound(DIGI_STAR,&zero,&zero,&zero,v3df_dontpan);
	}
	else
	if (KB_KeyPressed(sc_End))
	{
		KB_ClearKeyDown(sc_End);
        MusicPosition = NumUserMusic-15;
        if (MusicPosition < 0)
            MusicPosition = 0;
        j = 0;
        for (i=MusicPosition; i<MusicPosition+15; i++)
        {
            GetName(UserMusic[i], MyMusic[j]);
            j++;
        }
        MNU_SetupGroup();
        SetRedrawScreen(&Player[myconnectindex]);
        PlaySound(DIGI_STAR,&zero,&zero,&zero,v3df_dontpan);
	}
	else
	if (KB_KeyPressed(sc_Delete))
	{
		KB_ClearKeyDown(sc_Delete);
        StopSound();
	}

    cust_callback = NULL;
	return TRUE;
}

void MNU_ClientDisplay(void)
{
    char *extra_text;

    extra_text = gametype[gs.NetGameType]; //[gNet.MultiGameType-1];
    MNU_DrawString(OPT_XSIDE+40, OPT_LINE(1), extra_text, 1, mPal);

    if (gs.NetLevel > 0)
    {
     sprintf(ds, "L%02d:  %s", gs.NetLevel, LevelInfo[gs.NetLevel].Description);
     MNU_DrawString(OPT_XS+20, OPT_LINE(3), ds, 1, mPal);
    }

    sprintf(ds,"%d",gNet.KillLimit);
    MNU_DrawString(OPT_XSIDE+101, OPT_LINE(4), ds, 1, mPal);

    sprintf(ds,"%d Mins",TimeLimitTable[gNet.MaxTime]); //gNet.TimeLimit/120/60
    MNU_DrawString(OPT_XSIDE+86, OPT_LINE(5), ds, 1, mPal);

    // OPT_LINE(6) = Color

    extra_text = monsterskills[gNet.Skill+1];
    MNU_DrawString(OPT_XSIDE+54, OPT_LINE(7), extra_text, 1, mPal);
}

BOOL MNU_GetMapInput(void)
{
	if (CommEnabled)
	{
        strcpy(UserMapName, MyMaps[MapCursor]);
        CurrMapName[0] = 0;
        Level = 0;
    }
    return (TRUE);
}

BOOL MNU_SetMusic(void)
{
    extern char LevelSong[80];
    extern short CurrentTrack;

    if (bMusicmenu)
    {
       bMusicmenu = FALSE;
  	   cust_callback = NULL;
	   KB_ClearKeysDown();
	   if (gs.PlayCD)
	   {
	       CDAudio_Stop();
	       StopSong();
	       if (MusicCursor == 0)  // current track
	           CDAudio_Play(CurrentTrack, TRUE);
	       else
	           CDAudio_Play(MusicCursor+1, TRUE);
	   }
	   else
	   {
           strcpy(LevelSong, MyMusic[MusicCursor]);
           PlayMusic(LevelSong);
       }
       ExitMenus();
    }
    return (TRUE);
}

