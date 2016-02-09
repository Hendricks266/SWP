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
// Shifted Screen and Sound Setup stuff
//

#include "build.h"
#include "compat.h"

#include "keys.h"
#include "names2.h"
#include "panel.h"
#include "game.h"

//#include "settings.h"
#include "mytypes.h"
#include "develop.h"
#include "scriplib.h"
#include "fx_man.h"
#include "gamedefs.h"
#include "common.h"
#include "config.h"
#include "function.h"

#include "rts.h"

short GamePlays = 0;

/*
===================
=
= SetGameDefaults
=
===================
*/

void SetGameDefaults( void )
{
}

extern BOOL DrawScreen;

void EncodePassword(char *pw)
{
    long bak_DrawScreen = DrawScreen;
    long bak_randomseed = randomseed;
    long i;
    long len;

    DrawScreen = FALSE;
    randomseed = 1234L;

    Bstrupr(pw);

    len = strlen(pw);
    for (i = 0; i < len; i++)
        pw[i] += RANDOM_RANGE(26);

    randomseed = bak_randomseed;
    DrawScreen = bak_DrawScreen;
}

void DecodePassword(char *pw)
{
    long bak_DrawScreen = DrawScreen;
    long bak_randomseed = randomseed;
    long i;
    long len;

    DrawScreen = FALSE;
    randomseed = 1234L;

    Bstrupr(pw);

    len = strlen(pw);
    for (i = 0; i < len; i++)
        pw[i] -= RANDOM_RANGE(26);

    randomseed = bak_randomseed;
    DrawScreen = bak_DrawScreen;
}

/*
===================
=
= ReadGameSetup
=
===================
*/

void ReadGameSetup( int32 scripthandle )
{
    long dummy;
    long ret;
    extern char WangBangMacro[10][64];
    extern int cddevice;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Sound Setup", "FxOn",&dummy);
    if (dummy != -1) gs.FxOn = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Sound Setup", "MusicOn",&dummy);
    if (dummy != -1) gs.MusicOn = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Sound Setup", "PlayCD",&dummy);
    if (dummy != -1) gs.PlayCD = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Sound Setup", "CDDevice",&dummy);
    if (dummy < 0) dummy = -1;
    cddevice = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Screen Setup", "Brightness",&dummy);
    if (dummy != -1) gs.Brightness = dummy;

    for(dummy=0; dummy<10; dummy++)
        {
        sprintf(ds,"CommbatMacro#%ld",dummy);
        SCRIPT_GetString( scripthandle, "Network",ds,WangBangMacro[dummy]);
        }

    SCRIPT_GetNumber( scripthandle, "Network", "NetGameType",&dummy);
    if (dummy != -1) gs.NetGameType = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Network", "NetLevel",&dummy);
    if (dummy != -1) gs.NetLevel = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Network", "NetMonsters",&dummy);
    if (dummy != -1) gs.NetMonsters = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Network", "NetHurtTeammate",&dummy);
    if (dummy != -1) gs.NetHurtTeammate = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Network", "NetSpawnMarkers",&dummy);
    if (dummy != -1) gs.NetSpawnMarkers = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Network", "NetTeamPlay",&dummy);
    if (dummy != -1) gs.NetTeamPlay = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Network", "NetKillLimit",&dummy);
    if (dummy != -1) gs.NetKillLimit = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Network", "NetTimeLimit",&dummy);
    if (dummy != -1) gs.NetTimeLimit = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Network", "NetColor",&dummy);
    if (dummy != -1) gs.NetColor = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Network", "NetNuke",&dummy);
    if (dummy != -1) gs.NetNuke = dummy;

    SCRIPT_GetString( scripthandle, "Options","Rooster",gs.Password);
    DecodePassword(gs.Password);

    // option stuff
    dummy = -1;
    ret = SCRIPT_GetNumber( scripthandle, "Options", "BorderNum",&dummy);
    if (dummy != -1) gs.BorderNum = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Options", "BorderTile",&dummy);
    if (dummy != -1) gs.BorderTile = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Options", "Bobbing",&dummy);
    if (dummy != -1) gs.Bobbing = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Options", "Tilting",&dummy);
    if (dummy != -1) gs.Tilting = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Options", "Shadows",&dummy);
    if (dummy != -1) gs.Shadows = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Options", "AutoRun",&dummy);
    if (dummy != -1) gs.AutoRun = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Options", "Crosshair",&dummy);
    if (dummy != -1) gs.Crosshair = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Options", "AutoAim",&dummy);
    if (dummy != -1) gs.AutoAim = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Options", "Messages",&dummy);
    if (dummy != -1) gs.Messages = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Options", "Talking",&dummy);
    if (dummy != -1) gs.Talking = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Options", "Ambient",&dummy);
    if (dummy != -1) gs.Ambient = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Controls", "MouseAiming",&dummy);
    if (dummy != -1) gs.MouseAimingType = dummy;

    dummy = -1;
    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Options", "Voxels",&dummy);
    if (dummy != -1) usevoxels = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Options", "Stats",&dummy);
    if (dummy != -1) gs.Stats = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Options", "MouseAimingOn",&dummy);
    if (dummy != -1) gs.MouseAimingOn = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Options", "MouseInvert",&dummy);
    if (dummy != -1) gs.MouseInvert = dummy;

    dummy = -1;
    SCRIPT_GetNumber( scripthandle, "Options", "Kiwi",&dummy);
    if (dummy != -1) gs.ParentalLock = dummy;

    if (SW_SHAREWARE)
    {
       dummy = -1;
       SCRIPT_GetNumber( scripthandle, "Options", "Chickens",&dummy);
       if (dummy != -1) GamePlays = dummy;
   	   initprintf
            (
            "\n"
            "You have played Shadow Warrior %d times.  Please call and order the full\n"
            "version at 1(800)-3DREALMS or see the ORDER.FRM file.\n\n"
            ,GamePlays);

       GamePlays++;
    }
}

/*
===================
=
= WriteGameSetup
=
===================
*/

void WriteGameSetup( int32 scripthandle)
{
   long dummy;
   extern int cddevice;

   dummy = gs.FxOn;
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "FxOn",dummy,FALSE,FALSE);
   dummy = gs.MusicOn;
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "MusicOn",dummy,FALSE,FALSE);
   dummy = gs.PlayCD;
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "PlayCD",dummy,FALSE,FALSE);
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "CDDevice",cddevice,FALSE,FALSE);

   dummy = gs.Brightness;
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "Brightness",dummy,FALSE,FALSE);

   dummy = gs.NetGameType;
   SCRIPT_PutNumber( scripthandle, "Network", "NetGameType",dummy,FALSE,FALSE);
   dummy = gs.NetLevel;
   SCRIPT_PutNumber( scripthandle, "Network", "NetLevel",dummy,FALSE,FALSE);
   dummy = gs.NetMonsters;
   SCRIPT_PutNumber( scripthandle, "Network", "NetMonsters",dummy,FALSE,FALSE);
   dummy = gs.NetHurtTeammate;
   SCRIPT_PutNumber( scripthandle, "Network", "NetHurtTeammate",dummy,FALSE,FALSE);
   dummy = gs.NetSpawnMarkers;
   SCRIPT_PutNumber( scripthandle, "Network", "NetSpawnMarkers",dummy,FALSE,FALSE);
   dummy = gs.NetTeamPlay;
   SCRIPT_PutNumber( scripthandle, "Network", "NetTeamPlay",dummy,FALSE,FALSE);
   dummy = gs.NetKillLimit;
   SCRIPT_PutNumber( scripthandle, "Network", "NetKillLimit",dummy,FALSE,FALSE);
   dummy = gs.NetTimeLimit;
   SCRIPT_PutNumber( scripthandle, "Network", "NetTimeLimit",dummy,FALSE,FALSE);
   dummy = gs.NetColor;
   SCRIPT_PutNumber( scripthandle, "Network", "NetColor",dummy,FALSE,FALSE);
   dummy = gs.NetNuke;
   SCRIPT_PutNumber( scripthandle, "Network", "NetNuke",dummy,FALSE,FALSE);

   dummy = gs.BorderNum;
   SCRIPT_PutNumber( scripthandle, "Options", "BorderNum",dummy,FALSE,FALSE);
   dummy = gs.BorderTile;
   SCRIPT_PutNumber( scripthandle, "Options", "BorderTile",dummy,FALSE,FALSE);
   dummy = gs.Bobbing;
   SCRIPT_PutNumber( scripthandle, "Options", "Bobbing",dummy,FALSE,FALSE);
   dummy = gs.Tilting;
   SCRIPT_PutNumber( scripthandle, "Options", "Tilting",dummy,FALSE,FALSE);
   dummy = gs.Shadows;
   SCRIPT_PutNumber( scripthandle, "Options", "Shadows",dummy,FALSE,FALSE);
   dummy = gs.AutoRun;
   SCRIPT_PutNumber( scripthandle, "Options", "AutoRun",dummy,FALSE,FALSE);
   dummy = gs.Crosshair;
   SCRIPT_PutNumber( scripthandle, "Options", "Crosshair",dummy,FALSE,FALSE);
   dummy = gs.AutoAim;
   SCRIPT_PutNumber( scripthandle, "Options", "AutoAim",dummy,FALSE,FALSE);
   dummy = gs.Messages;
   SCRIPT_PutNumber( scripthandle, "Options", "Messages",dummy,FALSE,FALSE);
   dummy = gs.Talking;
   SCRIPT_PutNumber( scripthandle, "Options", "Talking",dummy,FALSE,FALSE);
   dummy = gs.Ambient;
   SCRIPT_PutNumber( scripthandle, "Options", "Ambient",dummy,FALSE,FALSE);
   dummy = gs.MouseAimingType;
   SCRIPT_PutNumber( scripthandle, "Controls", "MouseAiming",dummy,FALSE,FALSE);


   dummy = usevoxels;
   SCRIPT_PutNumber( scripthandle, "Options", "Voxels",dummy,FALSE,FALSE);
   dummy = gs.MouseAimingOn;
   SCRIPT_PutNumber( scripthandle, "Options", "MouseAimingOn",dummy,FALSE,FALSE);
   dummy = gs.MouseInvert;
   SCRIPT_PutNumber( scripthandle, "Options", "MouseInvert",dummy,FALSE,FALSE);
   dummy = gs.Stats;
   SCRIPT_PutNumber( scripthandle, "Options", "Stats",dummy,FALSE,FALSE);

   EncodePassword(gs.Password);
   SCRIPT_PutString( scripthandle, "Options","Rooster",gs.Password);
   DecodePassword(gs.Password);

   dummy = gs.ParentalLock;
   SCRIPT_PutNumber( scripthandle, "Options", "Kiwi",dummy,FALSE,FALSE);

   if (SW_SHAREWARE)
   {
   dummy = GamePlays;
   SCRIPT_PutNumber( scripthandle, "Options", "Chickens",dummy,FALSE,FALSE);
   }
}

void TermSetup(void)
{
extern BOOL BotMode;
CONFIG_WriteSetup();
RTS_Shutdown();
}
