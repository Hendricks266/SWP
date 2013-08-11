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

#ifndef _config_public
#define _config_public
#ifdef __cplusplus
extern "C" {
#endif

#define SETUPNAMEPARM "SETUPFILE"

// screen externs
extern int32 ScreenMode; // Screen mode
extern int32 ScreenWidth;
extern int32 ScreenHeight;
extern int32 ScreenBPP;
extern int32 ScreenBufferMode;
extern int32 VesaBufferMode;
extern int32 ForceSetup;

// sound externs
extern int32 FXDevice; // Sound FX Card number
extern int32 MusicDevice; // Music Card number
extern int32 FXVolume; // FX Volume
extern int32 MusicVolume; // Music Volume
extern int32 NumVoices; // Number of voices
extern int32 NumChannels; // Number of channels
extern int32 NumBits; // Number of bits
extern int32 MixRate; // Mixing rate
extern int32 MidiPort; // Midi Port
extern fx_blaster_config BlasterConfig; // Blaster settings
extern int32 ReverseStereo; // Reverse Stereo Channels

// comm externs
extern int32 ComPort;
extern int32 IrqNumber;
extern int32 UartAddress;
extern int32 PortSpeed;

extern int32 ToneDial;
extern char  ModemName[MAXMODEMSTRING];
extern char  InitString[MAXMODEMSTRING];
extern char  HangupString[MAXMODEMSTRING];
extern char  DialoutString[MAXMODEMSTRING];
extern int32 SocketNumber;
extern char  CommbatMacro[MAXMACROS][MAXMACROLENGTH];
extern char  PhoneNames[MAXPHONEENTRIES][PHONENAMELENGTH];
extern char  PhoneNumbers[MAXPHONEENTRIES][PHONENUMBERLENGTH];
extern char  PhoneNumber[PHONENUMBERLENGTH];
extern int32 NumberPlayers;
extern int32 ConnectType;
extern char  PlayerName[MAXPLAYERNAMELENGTH];
extern char  RTSName[MAXRTSNAMELENGTH];
extern char  UserLevel[MAXUSERLEVELNAMELENGTH];
extern char  RTSPath[MAXRTSPATHLENGTH];
extern char  UserPath[MAXUSERLEVELPATHLENGTH];

// controller externs
extern int32 UseMouse, UseJoystick;
extern int32 JoystickPort;
extern int32 MouseSensitivity;
extern int32 MouseAiming;
extern int32 MouseAimingFlipped;

extern char  MouseButtons[MAXMOUSEBUTTONS][MAXFUNCTIONLENGTH];
extern char  MouseButtonsClicked[MAXMOUSEBUTTONS][MAXFUNCTIONLENGTH];

extern char  JoystickButtons[MAXJOYBUTTONS][MAXFUNCTIONLENGTH];
extern char  JoystickButtonsClicked[MAXJOYBUTTONS][MAXFUNCTIONLENGTH];

extern char  MouseAnalogAxes[MAXMOUSEAXES][MAXFUNCTIONLENGTH];
extern char  JoystickAnalogAxes[MAXJOYAXES][MAXFUNCTIONLENGTH];
extern int32 MouseAnalogScales[MAXMOUSEAXES];
extern int32 JoystickAnalogScales[MAXJOYAXES];
extern int32 JoystickAnalogueDead[MAXJOYAXES];
extern int32 JoystickAnalogueSaturate[MAXJOYAXES];

extern int32 EnableRudder;

extern char  MouseDigitalAxes[MAXMOUSEAXES][2][MAXFUNCTIONLENGTH];
extern char  GamePadDigitalAxes[MAXGAMEPADAXES][2][MAXFUNCTIONLENGTH];
extern char  JoystickDigitalAxes[MAXJOYAXES][2][MAXFUNCTIONLENGTH];

extern char setupfilename[64];
extern char ExternalControlFilename[64];

void SetMouseDefaults( void );
void SetJoystickDefaults( void );
void SetDefaultKeyDefinitions( void );

int32 CONFIG_ReadSetup( void );
void CONFIG_SetupMouse( void );
void CONFIG_SetupJoystick( void );
void CONFIG_WriteSetup( void );
void WriteCommitFile( int32 gametype );
void CONFIG_GetSetupFilename( void );

#ifdef __cplusplus
};
#endif
#endif
