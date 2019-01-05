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

// Added SetHighres
// Added WeaponSwitch
// Added BorderNum store (BorderMem)
// Added Default Config changes
// Added GLFovscreen
// Added GLWidescreen
// Added ninja hack option
// Added swap yinyang
// Added MouseScale X/Y
// Added WeaponIcons
// Added HighTile and Model
// Added Menu Text Color
// Added usegoodalpha
// Added glusetexcompr
// Added Autosave
// Added RandomMusic
// Added Playername and IP defaults
// Added LcdMon
// Added glusetexcache
// Added 1st 8 key defaults manually
// Fixed mouse defaults (used dukes)
// Added alternate minihud
// Added LastIpUsed
// Added minihudtyp - classic/modern/alternate
// Added newer mouse speed settings
// Reset default sounds 44100,16,32
// Added Get/Set LastPlayed
// Added vsync
// Added LoopFactor
//

#include <strings.h>

#include "compat.h"

#include "settings.h"
#include "mytypes.h"
#include "develop.h"
#include "scriplib.h"
#include "file_lib.h"
#include "gamedefs.h"
#include "keyboard.h"
#include "util_lib.h"
#include "function.h"
#include "control.h"
#include "fx_man.h"
#include "sounds.h"
#include "config.h"
#include "common.h"

// we load this in to get default button and key assignments
// as well as setting up function mappings

#include "_functio.h"
#include "_config.h"

#include "build.h"
#include "baselayer.h"
#include "osd.h"

extern void ReadGameSetup(int32 scripthandle);
extern void WriteGameSetup(int32 scripthandle);

//
// Comm variables
//

char CommPlayerName[40];
int32 NumberPlayers,CommPort,PortSpeed,IrqNumber,UartAddress;

char PlayerNameArg[40] = "";
char IPAddressArg[8][40];
int  LastIpUsed;

extern short Autosave;
extern BOOL  RandomMusic;
extern float loopfactor;

//
// Sound variables
//
int32 FXDevice    = 0;
int32 MusicDevice = 0;
int32 FXVolume    = 168;
int32 MusicVolume = 168;
int32 NumVoices   = 32;
int32 NumChannels = 2;
int32 NumBits     = 16;
int32 MixRate     = 44100;
int LcdMon;
int vsync;

int32 SoundToggle;
int32 VoiceToggle;
int32 MusicToggle;

int32 UseMouse = 1, UseJoystick = 0;

byte KeyboardKeys[NUMGAMEFUNCTIONS][2];
int32 MouseFunctions[MAXMOUSEBUTTONS][2];
int32 MouseDigitalFunctions[MAXMOUSEAXES][2];
int32 MouseAnalogueAxes[MAXMOUSEAXES];
int32 MouseAnalogueScale[MAXMOUSEAXES];
int32 JoystickFunctions[MAXJOYBUTTONS][2];
int32 JoystickDigitalFunctions[MAXJOYAXES][2];
int32 JoystickAnalogueAxes[MAXJOYAXES];
int32 JoystickAnalogueScale[MAXJOYAXES];
int32 JoystickAnalogueDead[MAXJOYAXES];
int32 JoystickAnalogueSaturate[MAXJOYAXES];

//
// Screen variables
//

int32 ScreenMode = 1;
int32 ScreenWidth = 1024;
int32 ScreenHeight = 768;
int32 ScreenBPP = 8;
int32 ForceSetup;

extern char WangBangMacro[10][64];
char  RTSName[MAXRTSNAMELENGTH];
//static char setupfilename[64]={SETUPFILENAME};
char setupfilename[64]={SETUPFILENAME};
static int32 scripthandle = -1;

char  MouseAnalogAxes[MAXMOUSEAXES][MAXFUNCTIONLENGTH];
char  JoystickAnalogAxes[MAXJOYAXES][MAXFUNCTIONLENGTH];

char  MouseDigitalAxes[MAXMOUSEAXES][2][MAXFUNCTIONLENGTH];
char  GamePadDigitalAxes[MAXGAMEPADAXES][2][MAXFUNCTIONLENGTH];
char  JoystickDigitalAxes[MAXJOYAXES][2][MAXFUNCTIONLENGTH];


/*
===================
=
= CONFIG_FunctionNameToNum
=
===================
*/

int32 CONFIG_FunctionNameToNum( char * func )
   {
   int32 i;

   if (!func) return -1;
   for (i=0;i<NUMGAMEFUNCTIONS;i++)
      {
      if (!Bstrcasecmp(func,gamefunctions[i]))
         {
         return i;
         }
      }
   return -1;
   }

/*
===================
=
= CONFIG_FunctionNumToName
=
===================
*/

char * CONFIG_FunctionNumToName( int32 func )
{
    if ((unsigned)func >= (unsigned)NUMGAMEFUNCTIONS)
    {
       return NULL;
    }
    else
    {
       return gamefunctions[func];
    }
}

/*
===================
=
= CONFIG_AnalogNameToNum
=
===================
*/
int32 CONFIG_AnalogNameToNum( char * func )
   {
   if (!Bstrcasecmp(func,"analog_turning"))
      {
      return analog_turning;
      }
   if (!Bstrcasecmp(func,"analog_strafing"))
      {
      return analog_strafing;
      }
   if (!Bstrcasecmp(func,"analog_moving"))
      {
      return analog_moving;
      }
   if (!Bstrcasecmp(func,"analog_lookingupanddown"))
      {
      return analog_lookingupanddown;
      }
   return -1;
   }

char * CONFIG_AnalogNumToName( int32 func )
   {
   switch (func) {
	case analog_turning:
		return "analog_turning";
	case analog_strafing:
		return "analog_strafing";
	case analog_moving:
   		return "analog_moving";
	case analog_lookingupanddown:
		return "analog_lookingupanddown";
	default: break;
   }

   return NULL;
   }

/*
===================
=
= CONFIG_SetDefaults
=
===================
*/

void CONFIG_SetDefaults( void )
{
    // JBF 20031211
    int32 i,f;
    byte k1,k2;

    ScreenMode = 1;
    ScreenWidth = 1024;
    ScreenHeight = 768;
    ScreenBPP = 8;
    FXDevice = 0;
    MusicDevice = 0;
    FXVolume = 168;
    MusicVolume = 168;

    SoundToggle = 1;
    VoiceToggle = 1;
    MusicToggle = 1;

    NumVoices = 32;
    NumChannels = 2;
    NumBits = 16;
    MixRate = 44100;
    usehightile = 1;
    usemodels = 1;
    usegoodalpha = 0;
    glusetexcompr = 0;
    glusetexcache = 1;
    glanisotropy = 4;

    gs.FlipStereo = 0;
    gs.SetHighres = 0;
    if (ScreenBPP > 8)
        gs.SetHighres = 1;
    gs.UseDarts = 0;
    gs.SwapYinyang = 0;
    gs.ShowTEN = 0;
    gs.MiniHudTyp = 1;
    gs.UseNinjaHack = 0;
    gs.UseCarHack = 0;

    usevoxels = 1;
    gs.PlayCD = 1;

    gs.MouseSpeed = 65536;
    gs.MouseScaleX = 65536;
    gs.MouseScaleY = 65536;
    gs.SoundVolume = 168;
    gs.MusicVolume = 168;
    MouseAnalogueScale[0] = 65536;
    MouseAnalogueScale[1] = 65536;
    Autosave = 1;
    LcdMon = 1;
    ForceSetup = 1;

    gs.Brightness = 3;
    gs.AutoRun = 1;
    gs.AutoAim = 0;
    gs.FxOn = 1;
    gs.MouseAimingOn = 1;

    Bstrcpy(RTSName, DEFAULTRTSFILE);

    Bstrcpy(WangBangMacro[0], "Burn baby burn...");
    Bstrcpy(WangBangMacro[1], "You make another stupid move.");
    Bstrcpy(WangBangMacro[2], "Blocking with your head again?");
    Bstrcpy(WangBangMacro[3], "You not fight well with hands!");
    Bstrcpy(WangBangMacro[4], "You so stupid!");
    Bstrcpy(WangBangMacro[5], "Quit jerking off. Come fight me!");
    Bstrcpy(WangBangMacro[6], "What the matter you scaredy cat?");
    Bstrcpy(WangBangMacro[7], "Did I break your concentration?");
    Bstrcpy(WangBangMacro[8], "Hope you were paying attention.");
    Bstrcpy(WangBangMacro[9], "ITTAIIIUUU!!!");

    memset(KeyboardKeys, 0xff, sizeof(KeyboardKeys));    // does not read 0-7 ?
    for (i=0; i < (int32)(sizeof(keydefaults)/sizeof(keydefaults[0]))/3; i++)
    {
       f = CONFIG_FunctionNameToNum( keydefaults[3*i+0] );
       if (f == -1) continue;
       k1 = KB_StringToScanCode( keydefaults[3*i+1] );
       k2 = KB_StringToScanCode( keydefaults[3*i+2] );

       KeyboardKeys[f][0] = k1;
       KeyboardKeys[f][1] = k2;
    }

	memset(MouseFunctions, -1, sizeof(MouseFunctions));
	for (i=0; i<MAXMOUSEBUTTONS; i++)
	{
		MouseFunctions[i][0] = CONFIG_FunctionNameToNum( mousedefaults[i] );
		CONTROL_MapButton( MouseFunctions[i][0], i, 0, controldevice_mouse );
		if (i>=4) continue;

		MouseFunctions[i][1] = CONFIG_FunctionNameToNum( mouseclickeddefaults[i] );
		CONTROL_MapButton( MouseFunctions[i][1], i, 1, controldevice_mouse );
	}

    memset(MouseDigitalFunctions, -1, sizeof(MouseDigitalFunctions));
    for (i=0; i<MAXMOUSEAXES; i++)
    {
        MouseAnalogueScale[i] = 60000;
        MouseDigitalFunctions[i][0] = CONFIG_FunctionNameToNum( mousedigitaldefaults[i*2] );
        MouseDigitalFunctions[i][1] = CONFIG_FunctionNameToNum( mousedigitaldefaults[i*2+1] );
        MouseAnalogueAxes[i] = CONFIG_AnalogNameToNum( mouseanalogdefaults[i] );
    }
    CONTROL_SetMouseSensitivity(30000<<2);

    memset(JoystickFunctions, -1, sizeof(JoystickFunctions));
    for (i=0; i < (int32)(sizeof(joystickdefaults)/sizeof(char*)); i++)
    {
        JoystickFunctions[i][0] = CONFIG_FunctionNameToNum( joystickdefaults[i] );
        JoystickFunctions[i][1] = CONFIG_FunctionNameToNum( joystickclickeddefaults[i] );
    }

    memset(JoystickDigitalFunctions, -1, sizeof(JoystickDigitalFunctions));
    for (i=0; i < MAXJOYAXES; i++)
    {
	    JoystickAnalogueScale[i] = 65536;
	    JoystickAnalogueDead[i] = 1000;
	    JoystickAnalogueSaturate[i] = 9500;

        JoystickDigitalFunctions[i][0] = CONFIG_FunctionNameToNum( joystickdigitaldefaults[i*2] );
	    JoystickDigitalFunctions[i][1] = CONFIG_FunctionNameToNum( joystickdigitaldefaults[i*2+1] );

	    JoystickAnalogueAxes[i] = CONFIG_AnalogNameToNum( joystickanalogdefaults[i] );
    }

    Bstrcpy(PlayerNameArg, "Player");
    Bstrcpy(IPAddressArg[0], "127.0.0.0");

    // forced in as routing does not read 0-7 ?

    KeyboardKeys[0][0] = 200;    // "Move_Forward",  "Up",
    KeyboardKeys[0][1] = 17;     //                  "W",
    KeyboardKeys[1][0] = 208;    // "Move_Backward", "Down",
    KeyboardKeys[1][1] = 31;     //                  "S",
    KeyboardKeys[2][0] = 203;    // "Turn_Left",     "Left",
    KeyboardKeys[2][1] = 75;     //                  "Kpad4",
    KeyboardKeys[3][0] = 205;    // "Turn_Right",    "Right",
    KeyboardKeys[3][1] = 77;     //                  "KPad6",
    KeyboardKeys[4][0] = 56;     // "Strafe",        "LAlt",
    KeyboardKeys[4][1] = 184;    //                  "RAlt",
    KeyboardKeys[5][0] = 0;      // "Fire",          "",
    KeyboardKeys[5][1] = 157;    //                  "RCtrl",
    KeyboardKeys[6][0] = 18;     // "Open",          "E",
    KeyboardKeys[6][1] = 24;     //                  "O",
    KeyboardKeys[7][0] = 42;     // "Run",           "LShift",
    KeyboardKeys[7][1] = 54;     //                  "RShift",
}

/*
===================
=
= CONFIG_ReadKeys
=
===================
*/

void CONFIG_ReadKeys( int32 scripthandle )
{
    int32 i;
    int32 numkeyentries;
    int32 function;
    char keyname1[80];
    char keyname2[80];
    kb_scancode key1,key2;

    if (scripthandle < 0)
        return;

    numkeyentries = SCRIPT_NumberEntries( scripthandle,"KeyDefinitions" );

    for (i=0;i<numkeyentries;i++)
    {
        function = CONFIG_FunctionNameToNum(SCRIPT_Entry(scripthandle,"KeyDefinitions", i ));
        if (function != -1)
        {
            memset(keyname1,0,sizeof(keyname1));
            memset(keyname2,0,sizeof(keyname2));
            SCRIPT_GetDoubleString(scripthandle,"KeyDefinitions", SCRIPT_Entry(scripthandle, "KeyDefinitions", i), keyname1, keyname2);
            key1 = 0xff;
            key2 = 0xff;
            if (keyname1[0])
            {
                key1 = (byte) KB_StringToScanCode( keyname1 );
            }
            if (keyname2[0])
            {
                key2 = (byte) KB_StringToScanCode( keyname2 );
            }
            KeyboardKeys[function][0] = key1;
            KeyboardKeys[function][1] = key2;
        }
    }

    for (i=0; i<NUMGAMEFUNCTIONS; i++)
    {
        if (i == gamefunc_Show_Console)
            OSD_CaptureKey(KeyboardKeys[i][0]);
        else
            CONTROL_MapKey( i, KeyboardKeys[i][0], KeyboardKeys[i][1] );
    }
}

/*
===================
=
= CONFIG_SetupMouse
=
===================
*/

void CONFIG_SetupMouse( void )
   {
   int32 i;
   char str[80],*p;
   char temp[80];
   int32 function, scale;

   if (scripthandle < 0) return;

   for (i=0;i<MAXMOUSEBUTTONS;i++)
      {
      Bsprintf(str,"MouseButton%ld",i); temp[0] = 0;
      if (!SCRIPT_GetString( scripthandle,"Controls", str,temp))
         MouseFunctions[i][0] = CONFIG_FunctionNameToNum(temp);

      Bsprintf(str,"MouseButtonClicked%ld",i); temp[0] = 0;
      if (!SCRIPT_GetString( scripthandle,"Controls", str,temp))
         MouseFunctions[i][1] = CONFIG_FunctionNameToNum(temp);
      }

   // map over the axes
   for (i=0;i<MAXMOUSEAXES;i++)
      {
      Bsprintf(str,"MouseAnalogAxes%ld",i); temp[0] = 0;
      if (!SCRIPT_GetString(scripthandle, "Controls", str,temp))
         MouseAnalogueAxes[i] = CONFIG_AnalogNameToNum(temp);

      Bsprintf(str,"MouseDigitalAxes%ld_0",i); temp[0] = 0;
      if (!SCRIPT_GetString(scripthandle, "Controls", str,temp))
         MouseDigitalFunctions[i][0] = CONFIG_FunctionNameToNum(temp);

      Bsprintf(str,"MouseDigitalAxes%ld_1",i); temp[0] = 0;
      if (!SCRIPT_GetString(scripthandle, "Controls", str,temp))
         MouseDigitalFunctions[i][1] = CONFIG_FunctionNameToNum(temp);

      Bsprintf(str,"MouseAnalogScale%ld",i);
      scale = MouseAnalogueScale[i];
      SCRIPT_GetNumber(scripthandle, "Controls", str,&scale);
      MouseAnalogueScale[i] = scale;
      if (i == 0)
          gs.MouseScaleX = scale;
      else
          gs.MouseScaleY = scale;
      }

   function = 30000;
   SCRIPT_GetNumber( scripthandle, "Controls","MouseSensitivity",&function);
   gs.MouseSpeed = function;

   for (i=0; i<MAXMOUSEBUTTONS; i++)
      {
      CONTROL_MapButton( MouseFunctions[i][0], i, FALSE, controldevice_mouse );
      CONTROL_MapButton( MouseFunctions[i][1], i, TRUE,  controldevice_mouse );
      }
   for (i=0; i<MAXMOUSEAXES; i++)
      {
      CONTROL_MapAnalogAxis( i, MouseAnalogueAxes[i], controldevice_mouse);
      CONTROL_MapDigitalAxis( i, MouseDigitalFunctions[i][0], 0,controldevice_mouse );
      CONTROL_MapDigitalAxis( i, MouseDigitalFunctions[i][1], 1,controldevice_mouse );
      CONTROL_SetAnalogAxisScale( i, MouseAnalogueScale[i], controldevice_mouse );
      }

   CONTROL_SetMouseSensitivity(gs.MouseSpeed<<1);
   CONTROL_SetAnalogAxisScale( 0, gs.MouseScaleX<<1, controldevice_mouse );
   CONTROL_SetAnalogAxisScale( 1, gs.MouseScaleY>>1, controldevice_mouse );

   }

/*
===================
=
= CONFIG_SetupJoystick
=
===================
*/

void CONFIG_SetupJoystick( void )
   {
   int32 i;
   char str[80],*p;
   char temp[80];
   int32 function, scale;

   if (scripthandle < 0) return;

   for (i=0;i<MAXJOYBUTTONS;i++)
      {
      Bsprintf(str,"JoystickButton%ld",i); temp[0] = 0;
      if (!SCRIPT_GetString( scripthandle,"Controls", str,temp))
         JoystickFunctions[i][0] = CONFIG_FunctionNameToNum(temp);

      Bsprintf(str,"JoystickButtonClicked%ld",i); temp[0] = 0;
      if (!SCRIPT_GetString( scripthandle,"Controls", str,temp))
         JoystickFunctions[i][1] = CONFIG_FunctionNameToNum(temp);
      }

   // map over the axes
   for (i=0;i<MAXJOYAXES;i++)
      {
      Bsprintf(str,"JoystickAnalogAxes%ld",i); temp[0] = 0;
      if (!SCRIPT_GetString(scripthandle, "Controls", str,temp))
         JoystickAnalogueAxes[i] = CONFIG_AnalogNameToNum(temp);

      Bsprintf(str,"JoystickDigitalAxes%ld_0",i); temp[0] = 0;
      if (!SCRIPT_GetString(scripthandle, "Controls", str,temp))
         JoystickDigitalFunctions[i][0] = CONFIG_FunctionNameToNum(temp);

      Bsprintf(str,"JoystickDigitalAxes%ld_1",i); temp[0] = 0;
      if (!SCRIPT_GetString(scripthandle, "Controls", str,temp))
	  JoystickDigitalFunctions[i][1] = CONFIG_FunctionNameToNum(temp);

      Bsprintf(str,"JoystickAnalogScale%ld",i);
      scale = JoystickAnalogueScale[i];
      SCRIPT_GetNumber(scripthandle, "Controls", str,&scale);
      JoystickAnalogueScale[i] = scale;

      Bsprintf(str,"JoystickAnalogDead%ld",i);
      scale = JoystickAnalogueDead[i];
      SCRIPT_GetNumber(scripthandle, "Controls", str,&scale);
      JoystickAnalogueDead[i] = scale;

      Bsprintf(str,"JoystickAnalogSaturate%ld",i);
      scale = JoystickAnalogueSaturate[i];
      SCRIPT_GetNumber(scripthandle, "Controls", str,&scale);
      JoystickAnalogueSaturate[i] = scale;
      }

   for (i=0;i<MAXJOYBUTTONS;i++)
      {
         CONTROL_MapButton( JoystickFunctions[i][0], i, FALSE, controldevice_joystick );
         CONTROL_MapButton( JoystickFunctions[i][1], i, TRUE,  controldevice_joystick );
      }
   for (i=0;i<MAXJOYAXES;i++)
      {
         CONTROL_MapAnalogAxis(i, JoystickAnalogueAxes[i], controldevice_joystick);
         CONTROL_MapDigitalAxis( i, JoystickDigitalFunctions[i][0], 0, controldevice_joystick );
         CONTROL_MapDigitalAxis( i, JoystickDigitalFunctions[i][1], 1, controldevice_joystick );
         CONTROL_SetAnalogAxisScale( i, JoystickAnalogueScale[i], controldevice_joystick );
      }
   }

/*
===================
=
= CONFIG_ReadSetup
=
===================
*/

int32 CONFIG_ReadSetup( void )
   {
   int32 dummy;
   char ret;
   short i;
   char buf[80];
   extern char ds[];

   CONTROL_ClearAssignments();
   CONFIG_SetDefaults();

   if (SafeFileExists(setupfilename))
       scripthandle = SCRIPT_Load(setupfilename);

   if (scripthandle < 0)
	   return -1;

      SCRIPT_GetNumber( scripthandle, "Screen Setup", "ScreenMode",&ScreenMode);
      SCRIPT_GetNumber( scripthandle, "Screen Setup", "ScreenWidth",&ScreenWidth);
      SCRIPT_GetNumber( scripthandle, "Screen Setup", "ScreenHeight",&ScreenHeight);
	  if (!SCRIPT_GetNumber( scripthandle, "Screen Setup", "ScreenBPP", &ScreenBPP))
	  {
		// If ScreenBPP was set/changed...
		if (ScreenBPP < 8)
		  ScreenBPP = 8;
	    gs.SetHighres = 0;
        if (ScreenBPP > 8)
		  gs.SetHighres = 1;
	  }

#ifdef RENDERTYPEWIN
      SCRIPT_GetNumber( scripthandle, "Screen Setup", "MaxRefreshFreq", (int32*)&maxrefreshfreq);
#endif

      SCRIPT_GetNumber( scripthandle, "Screen Setup", "GLTextureMode", &gltexfiltermode);
      SCRIPT_GetNumber( scripthandle, "Screen Setup", "GLAnisotropy", &glanisotropy);
      SCRIPT_GetNumber( scripthandle, "Screen Setup", "GLUseTextureCompr", &glusetexcompr);
      SCRIPT_GetNumber( scripthandle, "Screen Setup", "GLUseTextureCache", &glusetexcache);

      SCRIPT_GetNumber( scripthandle, "Screen Setup", "GLFovscreen", &glfovscreen);
      SCRIPT_GetNumber( scripthandle, "Screen Setup", "GLWidescreen", &glwidescreen);
	  if (!SCRIPT_GetNumber( scripthandle, "Screen Setup", "GLVSync", &dummy))
	  {
		// If vsync was set/changed...
		vsync = dummy;
	  }

      SCRIPT_GetNumber( scripthandle, "Screen Setup", "HighTiles", &usehightile);
      SCRIPT_GetNumber( scripthandle, "Screen Setup", "HighModels", &usemodels);

      SCRIPT_GetNumber( scripthandle, "Screen Setup", "Usegoodalpha", &usegoodalpha);
      if (!SCRIPT_GetNumber( scripthandle, "Screen Setup", "LcdMonitor",&dummy))
	  {
		// If LcdMon was set/changed...
		LcdMon = dummy;
	  }
      

      SCRIPT_GetNumber( scripthandle, "Sound Setup", "FXDevice",&FXDevice);
      SCRIPT_GetNumber( scripthandle, "Sound Setup", "MusicDevice",&MusicDevice);
      if (!SCRIPT_GetNumber( scripthandle, "Sound Setup", "FXVolume",&FXVolume))
	  {
		// If FXVolume was set/changed...
		gs.SoundVolume = FXVolume;
	  }
      if (!SCRIPT_GetNumber( scripthandle, "Sound Setup", "MusicVolume",&MusicVolume))
	  {
		// If MusicVolume was set/changed...
		gs.MusicVolume = FXVolume;
	  }

      SCRIPT_GetNumber( scripthandle, "Sound Setup", "NumVoices",&NumVoices);
      SCRIPT_GetNumber( scripthandle, "Sound Setup", "NumChannels",&NumChannels);
      SCRIPT_GetNumber( scripthandle, "Sound Setup", "NumBits",&NumBits);
      SCRIPT_GetNumber( scripthandle, "Sound Setup", "MixRate",&MixRate);
      if (!SCRIPT_GetNumber( scripthandle, "Sound Setup", "ReverseStereo",&dummy))
	  {
		// If ReverseStereo was set/changed...
        gs.FlipStereo = dummy;
        if (gs.FlipStereo) gs.FlipStereo = 1;
	  }

      SCRIPT_GetNumber( scripthandle, "Setup", "ForceSetup",&ForceSetup);
      SCRIPT_GetNumber( scripthandle, "Controls","UseMouse",&UseMouse);
      SCRIPT_GetNumber( scripthandle, "Controls","UseJoystick",&UseJoystick);

	  if (!SCRIPT_GetNumber( scripthandle, "SwpOptions","WeaponSwitch",&dummy))
	  {
		// If WeaponSwitch was set/changed...
        gs.WeaponSwitch = dummy;
	  }

	  if (!SCRIPT_GetNumber( scripthandle, "SwpOptions","SwpUseDarts",&dummy))
	  {
		// If SwpUseDarts was set/changed...
        gs.UseDarts = dummy;
	  }

	  if (!SCRIPT_GetNumber( scripthandle, "SwpOptions","SwapYinyang",&dummy))
	  {
		// If SwapYinyang was set/changed...
        gs.SwapYinyang = dummy;
	  }

	  if (!SCRIPT_GetNumber( scripthandle, "SwpOptions","ShowTEN",&dummy))
	  {
		// If ShowTEN was set/changed...
        gs.ShowTEN = dummy;
	  }

	  if (!SCRIPT_GetNumber( scripthandle, "SwpOptions","AutoSaveGame",&dummy))
	  {
		// If AutoSaveGame was set/changed...
        Autosave = dummy;
	  }

	  if (!SCRIPT_GetNumber( scripthandle, "SwpOptions","RandomMusic",&dummy))
	  {
		// If RandomMusic was set/changed...
        RandomMusic = dummy;
	  }

	  if (!SCRIPT_GetNumber( scripthandle, "SwpOptions","SwpNinjaHack",&dummy))
	  {
		// If SwpNinjaHack was set/changed...
        gs.UseNinjaHack = dummy;
	  }

	  if (!SCRIPT_GetNumber( scripthandle, "SwpOptions","SwpCarHack",&dummy))
	  {
		// If SwpCarHack was set/changed...
        gs.UseCarHack = dummy;
	  }

	  if (!SCRIPT_GetNumber( scripthandle, "SwpOptions","MenuTextColor",&dummy))
	  {
		// If MenuTextColor was set/changed...
        if (dummy < 0 || dummy > 14)
            dummy = 5;
        gs.MenuTextColor = dummy;
	  }

      dummy = 0;
      SCRIPT_GetNumber( scripthandle, "SwpOptions", "MiniHudType",&dummy);
      gs.MiniHudTyp = dummy;

      dummy = 0;
      SCRIPT_GetNumber( scripthandle, "SwpOptions", "WeaponHide",&dummy);
      gs.WeaponHide = dummy;

      dummy = 0;
      SCRIPT_GetNumber( scripthandle, "SwpOptions", "LoopFactor",&dummy);
      loopfactor = dummy;
      loopfactor = loopfactor / 10;
      if (loopfactor < 1)
          loopfactor = 19.0;

      SCRIPT_GetString( scripthandle, "Network", "RTSName",RTSName);
      SCRIPT_GetString( scripthandle, "Network", "PlayerName", PlayerNameArg);

      for (i=0; i<8; i++)
          {
          Bsprintf(buf,"IPAddress_%d",i+1);
          SCRIPT_GetString( scripthandle, "Network", buf, IPAddressArg[i]);
          }

	  if (!SCRIPT_GetNumber( scripthandle, "Network", "LastIpUsed", &dummy))
	  {
		// If LastIpUsed was set/changed...
        if (IPAddressArg[dummy] == NULL)
            dummy = 0;
        LastIpUsed = dummy;
	  }

   ReadGameSetup(scripthandle);
   gs.BorderMem = gs.BorderNum;

   CONFIG_ReadKeys(scripthandle);

   //CONFIG_SetupMouse();
   //CONFIG_SetupJoystick(scripthandle);

   if (PlayerNameArg[0] != '\0')
       strcpy(CommPlayerName, PlayerNameArg);

   return 0;
   }

/*
===================
=
= CONFIG_WriteSetup
=
===================
*/

void CONFIG_WriteSetup( void )
{
   int32 dummy;
   char buf[80];
   short i;

   if (scripthandle < 0)
	   scripthandle = SCRIPT_Init(setupfilename);

   SCRIPT_PutNumber( scripthandle, "Screen Setup", "ScreenMode",ScreenMode,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "ScreenWidth", ScreenWidth,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "ScreenHeight",ScreenHeight,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "ScreenBPP",ScreenBPP,FALSE,FALSE);
#ifdef RENDERTYPEWIN
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "MaxRefreshFreq",maxrefreshfreq,FALSE,FALSE);
#endif
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "GLTextureMode",gltexfiltermode,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "GLAnisotropy",glanisotropy,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "GLUseTextureCompr",glusetexcompr,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "GLUseTextureCache",glusetexcache,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "GLFovscreen",glfovscreen,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "GLWidescreen",glwidescreen,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "GLVSync",vsync,FALSE,FALSE);

   SCRIPT_PutNumber( scripthandle, "Screen Setup", "HighTiles", usehightile,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "HighModels", usemodels,FALSE,FALSE);

   SCRIPT_PutNumber( scripthandle, "Screen Setup", "Usegoodalpha", usegoodalpha,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "LcdMonitor", LcdMon,FALSE,FALSE);

   SCRIPT_PutNumber( scripthandle, "Sound Setup", "FXVolume",gs.SoundVolume,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "MusicVolume",gs.MusicVolume,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "SoundToggle",SoundToggle,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "VoiceToggle",VoiceToggle,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "MusicToggle",MusicToggle,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "FXDevice",FXDevice,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "MusicDevice",MusicDevice,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "NumVoices",NumVoices,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "NumChannels",NumChannels,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "NumBits",NumBits,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "MixRate",MixRate,FALSE,FALSE);
   dummy = gs.FlipStereo;
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "ReverseStereo",dummy,FALSE,FALSE);

   SCRIPT_PutNumber( scripthandle, "Setup", "ForceSetup",ForceSetup,FALSE,FALSE);

   dummy = gs.WeaponSwitch;
   SCRIPT_PutNumber( scripthandle, "SwpOptions","WeaponSwitch",dummy,FALSE,FALSE);
   dummy = gs.UseDarts;
   SCRIPT_PutNumber( scripthandle, "SwpOptions","SwpUseDarts",dummy,FALSE,FALSE);
   dummy = gs.SwapYinyang;
   SCRIPT_PutNumber( scripthandle, "SwpOptions","SwapYinyang",dummy,FALSE,FALSE);
   dummy = gs.ShowTEN;
   SCRIPT_PutNumber( scripthandle, "SwpOptions","ShowTEN",dummy,FALSE,FALSE);
   dummy = Autosave;
   SCRIPT_PutNumber( scripthandle, "SwpOptions","AutoSaveGame",dummy,FALSE,FALSE);
   dummy = RandomMusic;
   SCRIPT_PutNumber( scripthandle, "SwpOptions","RandomMusic",dummy,FALSE,FALSE);
   dummy = gs.UseNinjaHack;
   SCRIPT_PutNumber( scripthandle, "SwpOptions","SwpNinjaHack",dummy,FALSE,FALSE);
   dummy = gs.UseCarHack;
   SCRIPT_PutNumber( scripthandle, "SwpOptions","SwpCarHack",dummy,FALSE,FALSE);
   dummy = gs.MenuTextColor;
   SCRIPT_PutNumber( scripthandle, "SwpOptions","MenuTextColor",dummy,FALSE,FALSE);
   dummy = gs.MiniHudTyp;
   SCRIPT_PutNumber( scripthandle, "SwpOptions", "MiniHudType",dummy,FALSE,FALSE);
   dummy = gs.WeaponHide;
   SCRIPT_PutNumber( scripthandle, "SwpOptions", "WeaponHide",dummy,FALSE,FALSE);
   dummy = loopfactor * 10;
   SCRIPT_PutNumber( scripthandle, "SwpOptions", "LoopFactor",dummy,FALSE,FALSE);

   WriteGameSetup(scripthandle);

   for (dummy=0; dummy<NUMGAMEFUNCTIONS; dummy++)
   {
       SCRIPT_PutDoubleString(scripthandle, "KeyDefinitions", CONFIG_FunctionNumToName(dummy),
                              KB_ScanCodeToString(KeyboardKeys[dummy][0]), KB_ScanCodeToString(KeyboardKeys[dummy][1]));
   }

   SCRIPT_PutNumber( scripthandle, "Controls","UseMouse",UseMouse,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Controls","UseJoystick",UseJoystick,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Controls","MouseSensitivity",gs.MouseSpeed,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Controls","MouseAnalogScale0",gs.MouseScaleX,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Controls","MouseAnalogScale1",gs.MouseScaleY,FALSE,FALSE);


   for (dummy=0; dummy<MAXMOUSEBUTTONS; dummy++)
   {
       Bsprintf(buf,"MouseButton%ld",dummy);
       SCRIPT_PutString( scripthandle,"Controls", buf, CONFIG_FunctionNumToName( MouseFunctions[dummy][0] ));

       if (dummy >= (MAXMOUSEBUTTONS-2)) continue;	// scroll wheel

       Bsprintf(buf,"MouseButtonClicked%ld",dummy);
       SCRIPT_PutString( scripthandle,"Controls", buf, CONFIG_FunctionNumToName( MouseFunctions[dummy][1] ));
   }

   SCRIPT_GetString(scripthandle, "Controls", "MouseButton0",buf);   // always fails on initial startup wxyz
   if (buf[0] == '\0')
       SCRIPT_PutString(scripthandle, "Controls", "MouseButton0", "Fire");

   for (dummy=0;dummy<MAXMOUSEAXES;dummy++)
   {
       Bsprintf(buf,"MouseAnalogAxes%ld",dummy);
       SCRIPT_PutString(scripthandle, "Controls", buf, CONFIG_AnalogNumToName( MouseAnalogueAxes[dummy] ));

       Bsprintf(buf,"MouseDigitalAxes%ld_0",dummy);
       SCRIPT_PutString(scripthandle, "Controls", buf, CONFIG_FunctionNumToName(MouseDigitalFunctions[dummy][0]));

       Bsprintf(buf,"MouseDigitalAxes%ld_1",dummy);
       SCRIPT_PutString(scripthandle, "Controls", buf, CONFIG_FunctionNumToName(MouseDigitalFunctions[dummy][1]));

//       Bsprintf(buf,"MouseAnalogScale%ld",dummy);
//       SCRIPT_PutNumber(scripthandle, "Controls", buf, MouseAnalogueScale[dummy], FALSE, FALSE);
   }

   for (dummy=0;dummy<MAXJOYBUTTONS;dummy++)
   {
       Bsprintf(buf,"JoystickButton%ld",dummy);
       SCRIPT_PutString( scripthandle,"Controls", buf, CONFIG_FunctionNumToName( JoystickFunctions[dummy][0] ));

       Bsprintf(buf,"JoystickButtonClicked%ld",dummy);
       SCRIPT_PutString( scripthandle,"Controls", buf, CONFIG_FunctionNumToName( JoystickFunctions[dummy][1] ));
   }

   for (dummy=0;dummy<MAXJOYAXES;dummy++)
   {
       Bsprintf(buf,"JoystickAnalogAxes%ld",dummy);
       SCRIPT_PutString(scripthandle, "Controls", buf, CONFIG_AnalogNumToName( JoystickAnalogueAxes[dummy] ));

       Bsprintf(buf,"JoystickDigitalAxes%ld_0",dummy);
       SCRIPT_PutString(scripthandle, "Controls", buf, CONFIG_FunctionNumToName(JoystickDigitalFunctions[dummy][0]));

       Bsprintf(buf,"JoystickDigitalAxes%ld_1",dummy);
       SCRIPT_PutString(scripthandle, "Controls", buf, CONFIG_FunctionNumToName(JoystickDigitalFunctions[dummy][1]));

       Bsprintf(buf,"JoystickAnalogScale%ld",dummy);
       SCRIPT_PutNumber(scripthandle, "Controls", buf, JoystickAnalogueScale[dummy], FALSE, FALSE);

       Bsprintf(buf,"JoystickAnalogDead%ld",dummy);
       SCRIPT_PutNumber(scripthandle, "Controls", buf, JoystickAnalogueDead[dummy], FALSE, FALSE);

       Bsprintf(buf,"JoystickAnalogSaturate%ld",dummy);
       SCRIPT_PutNumber(scripthandle, "Controls", buf, JoystickAnalogueSaturate[dummy], FALSE, FALSE);
   }

   SCRIPT_PutString(scripthandle, "Network", "RTSName", RTSName);
   SCRIPT_PutString(scripthandle, "Network", "PlayerName", PlayerNameArg);

   for (i=0; i<8; i++)
       {
       Bsprintf(buf,"IPAddress_%d",i+1);
       SCRIPT_PutString( scripthandle, "Network", buf, IPAddressArg[i]);
       }
   SCRIPT_PutNumber(scripthandle, "Network", "LastIpUsed", LastIpUsed, FALSE, FALSE);

   SCRIPT_Save (scripthandle, setupfilename);
   SCRIPT_Free (scripthandle);
}

void SetLastPlayed(char *smap, short x)                              // 20100304
{
	if (scripthandle < 0)
	    scripthandle = SCRIPT_Init(setupfilename);

	if (scripthandle < 0)
	    return;

    if (x == 1)
	    SCRIPT_PutString(scripthandle, "SwpOptions", "LastMap", smap);
	else
	    SCRIPT_PutString(scripthandle, "SwpOptions", "LastGame", smap);
}

void GetLastPlayed(char *smap, int x)                                // 20100304
{
    char xbuf[40];

	if (scripthandle < 0)
	    scripthandle = SCRIPT_Init(setupfilename);

	if (scripthandle < 0)
	    return;

    if (x == 1)
	    SCRIPT_GetString(scripthandle, "SwpOptions", "LastMap", xbuf);
	else
	    SCRIPT_GetString(scripthandle, "SwpOptions", "LastGame", xbuf);

	if (strchr(xbuf,'.') != 0)
	    strcpy(smap, xbuf);
}

