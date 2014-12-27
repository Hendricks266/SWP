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

// Added adjust inv pos
// Added WeaponIcons  (gamep.h)
// Adjust armor box
// Added new minibar hud
// Added alternate minihud
// Resized minihud text size
// Added minihudtyp - classic/modern/alternate
// Added clear color fades
// Added underwater tint
//

#undef MAIN
#include "build.h"
#include "compat.h"

#include "keys.h"
#include "names2.h"
#include "panel.h"
#include "game.h"
#include "pal.h"
#include "text.h"
#include "colormap.h"
#include "player.h"

#include "swp.h"

VOID UpdateClassicHud(PLAYERp pp);
VOID UpdateAltMiniHud(PLAYERp pp);
VOID UpdateMiniBarWeapons(PLAYERp pp, short Weapnum);

//#define SAVE_EXTERN
//#include "_save.h"
//#undef SAVE_EXTERN

extern short NormalVisibility;

// indexed by gs.BorderNum
short InventoryBarXpos[] = {95, 95, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80};
short InventoryBarYpos[] = {155, 155, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125};

VOID PlayerUpdateInventory(PLAYERp pp, short InventoryNum);
void SpawnInventoryBar(PLAYERp pp);
VOID InventoryBarUpdatePosition(PLAYERp pp);
VOID InvBorderRefresh(PLAYERp pp);
VOID InventoryUse(PLAYERp pp);
VOID InventoryStop(PLAYERp pp, short InventoryNum);
VOID KillInventoryBar(PLAYERp pp);
VOID PlayerUpdateInventoryPercent(PLAYERp pp);
VOID PlayerUpdateInventoryPic(PLAYERp pp);
VOID PlayerUpdateInventoryState(PLAYERp pp);

//#define INVENTORY_ICON_WIDTH  32
#define INVENTORY_ICON_WIDTH  28

VOID UseInventoryRepairKit(PLAYERp pp);
VOID UseInventoryMedkit(PLAYERp pp);
VOID UseInventoryRepairKit(PLAYERp pp);
VOID UseInventoryCloak(PLAYERp pp);
VOID UseInventoryEnvironSuit(PLAYERp pp);
VOID UseInventoryNightVision(PLAYERp pp);
VOID UseInventoryChemBomb(PLAYERp pp);
VOID UseInventoryFlashBomb(PLAYERp pp);
VOID UseInventoryCaltrops(PLAYERp pp);

VOID StopInventoryRepairKit(PLAYERp pp, short);
VOID StopInventoryMedkit(PLAYERp pp, short);
VOID StopInventoryRepairKit(PLAYERp pp, short);
VOID StopInventoryCloak(PLAYERp pp, short);
VOID StopInventoryEnvironSuit(PLAYERp pp, short);
VOID StopInventoryNightVision(PLAYERp pp, short);

extern PANEL_STATE ps_PanelEnvironSuit[];
extern PANEL_STATE ps_PanelCloak[];
extern PANEL_STATE ps_PanelMedkit[];
extern PANEL_STATE ps_PanelRepairKit[];
extern PANEL_STATE ps_PanelSelectionBox[];
extern PANEL_STATE ps_PanelNightVision[];
extern PANEL_STATE ps_PanelChemBomb[];
extern PANEL_STATE ps_PanelFlashBomb[];
extern PANEL_STATE ps_PanelCaltrops[];

INVENTORY_DATA InventoryData[MAX_INVENTORY+1] =
    {
    {"PORTABLE MEDKIT",  UseInventoryMedkit,      NULL,                 ps_PanelMedkit,        0,   1, (1<<16),     0},
    {"REPAIR KIT",       NULL,                    NULL,                 ps_PanelRepairKit,     100, 1, (1<<16),     INVF_AUTO_USE},
    {"SMOKE BOMB",       UseInventoryCloak,       StopInventoryCloak,   ps_PanelCloak,         4,   1, (1<<16),     INVF_TIMED},
    {"NIGHT VISION",     UseInventoryNightVision, StopInventoryNightVision, ps_PanelNightVision,3,  1, (1<<16),     INVF_TIMED},
    {"GAS BOMB",         UseInventoryChemBomb,    NULL,                 ps_PanelChemBomb,      0,   1, (1<<16),     INVF_COUNT},
    {"FLASH BOMB",       UseInventoryFlashBomb,   NULL,                 ps_PanelFlashBomb,     0,   2, (1<<16),     INVF_COUNT},
    {"CALTROPS",         UseInventoryCaltrops,    NULL,                 ps_PanelCaltrops,      0,   3, (1<<16),     INVF_COUNT},
    {NULL, NULL, NULL, NULL, 0, 0, 0, 0}
    };

VOID PanelInvTestSuicide(PANEL_SPRITEp psp);

VOID UpdateMiniBar(PLAYERp pp)
{
    USERp u = User[pp->PlayerSprite];
    short i, pal;
    long x, y;
    INVENTORY_DATAp id;
    extern BOOL UsingMenus;
                      //nuk2 3377, 16000, 53, 180
    int ammotype[20] = {4050, 1793, 1823, 1799, 1800, 1831, 1842, 1812, 2055, 1825, 1819, 1809, 0, 492, 1821,1798,2519, 2056, 2057, 0};
    int ammosize[20] = {4000, 32000, 46200, 38000, 32000, 30000, 30000, 30000, 6500, 24200, 32000, 28000, 0, 15000, 44000,46000,16000, 6500, 6500, 0};
    int ammox[20] = {53, 54, 53, 59, 54, 52, 52, 52, 55, 54, 54, 52, 0, 53, 53,55,58, 55, 55, 0};
    int ammoy[20] = {184, 180, 182, 180, 179, 180, 180, 180, 178, 181, 181, 181, 0, 184, 182,180,178, 178, 178, 0};


    #define MINI_HUD_ARMOR_BOX_PIC   3030
    #define MINI_HUD_HEALTH_BOX_PIC  1803

    #define MINI_HUD_INVENTORY_BOX_X 42
    #define MINI_HUD_INVENTORY_BOX_Y 180

    x = 0;
    y = 200 - 49;

    if (UsingMenus && gs.BorderNum > BORDER_MINI_BAR)
    {
        rotatesprite(x << 16, (y+1) << 16, (1 << 16), 0, 2434, 0, 21, 10+16, 0, 0, xdim - 1, ydim - 1); // main hud
        return;
    }
    else
    if (gs.BorderNum != BORDER_MINI_BAR)
        return;

    if (gs.MiniHudTyp == 0)
    {
        UpdateClassicHud(pp);
        return;
    }
    else
    if (gs.MiniHudTyp == 2)
    {
        UpdateAltMiniHud(pp);
        return;
    }

    x = 4;
    y = 182;
    if (gs.SetHighres == 1 && usehightile)
        y = 181;

    //display health icon
    rotatesprite(x << 16, y << 16, 44000, 0, MINI_HUD_HEALTH_BOX_PIC, 22, 0,
                 ROTATE_SPRITE_SCREEN_CLIP | ROTATE_SPRITE_CORNER, 0, 0, xdim - 1, ydim - 1);

    x = 28;
    y = 183;
    //display armor icon
    rotatesprite(x << 16, y << 16, 30000, 0, MINI_HUD_ARMOR_BOX_PIC, 16, 0,
                 ROTATE_SPRITE_SCREEN_CLIP | ROTATE_SPRITE_CORNER, 0, 0, xdim - 1, ydim - 1);

    i = 4;  // rpg ammo
    if (!UsingMenus)
    {
        i = u->WeaponNum;  // 0 - 9 | 13 = sword

        if (i < 0 || i > 13)
            i = 0;

        if (i == 1 && useDarts)
            i = 16;
        else
        if (i == 2 && pp->WpnShotgunType == 1)
            i = 14;
        else
        if (i == 3 && (pp->WpnUziType == 0 || pp->WpnUziType == 3))
            i = 15;
        else
        if (i == 4)            // rockets
        {
            if (pp->WpnRocketType == 1 && pp->WpnRocketHeat)   // heat
                i = 10;
            else
            if (pp->WpnRocketType == 2)   // nuke
                i = 11;
        }
        else
        if (i == 8)            // guru
        {
            if (pp->WpnFlameType > 0)
                i = 16 + pp->WpnFlameType;
        }
    }
    // display ammo icons

    rotatesprite(ammox[i] << 16, (ammoy[i]+3) << 16, ammosize[i], 0, ammotype[i], 6, 0,
                 ROTATE_SPRITE_SCREEN_CLIP | ROTATE_SPRITE_CORNER, 0, 0, xdim - 1, ydim - 1);

    if (UsingMenus)
    {
        UpdateMiniBarWeapons(pp, i);
        rotatesprite(80 << 16, 184 << 16, 20000, 0, 1808, 0, 0,
                     ROTATE_SPRITE_SCREEN_CLIP | ROTATE_SPRITE_CORNER, 0, 0, xdim - 1, ydim - 1);
        return;
    }

    UpdateMiniBarWeapons(pp, u->WeaponNum);

    //display health amount
    DisplayMiniBarNumber(pp, 14, 186, u->Health);

    // display armor amount  (actual)
    DisplayMiniBarNumber(pp, 38, 186, pp->Armor);

    // display ammo amount
    if (u->WeaponNum != WPN_SWORD && u->WeaponNum != WPN_FIST)
        DisplayMiniBarNumber(pp, 62, 186, pp->WpnAmmo[u->WeaponNum]);

    if (pp->InventoryAmount[pp->InventoryNum])
    {
        // Inventory pic in box
        x = 77;
        y = 184;
        id = &InventoryData[pp->InventoryNum];
        rotatesprite(x << 16, y << 16, 44000, 0, id->State->picndx, 0, 0,
                     ROTATE_SPRITE_SCREEN_CLIP | ROTATE_SPRITE_CORNER, 0, 0, xdim - 1, ydim - 1);
        // will update the AUTO and % inventory values
        PlayerUpdateInventory(pp, pp->InventoryNum);
    }
}

VOID UpdateClassicHud(PLAYERp pp)
{
    USERp u = User[pp->PlayerSprite];
    short i;
    long x,y;
    BOOL found;
    INVENTORY_DATAp id;
    extern BOOL PanelUpdateMode;
    extern BOOL UsingMenus;

    #define MINI_BAR_Y 174

    #define MINI_BAR_HEALTH_BOX_PIC    2437
    #define MINI_BAR_AMMO_BOX_PIC      2437
    #define MINI_BAR_INVENTORY_BOX_PIC 2438

    #define MINI_BAR_HEALTH_BOX_X 4
    #define MINI_BAR_AMMO_BOX_X 32
    #define MINI_BAR_INVENTORY_BOX_X 64          // MINI_HUD_INVENTORY_BOX_X
    #define MINI_BAR_INVENTORY_BOX_Y MINI_BAR_Y  // MINI_HUD_INVENTORY_BOX_Y

    if (!PanelUpdateMode)
        return;

    if (gs.BorderNum != BORDER_MINI_BAR)
        return;

    x = MINI_BAR_HEALTH_BOX_X;
    y = 200 - 26;

    rotatesprite(x << 16, y << 16, (1 << 16), 0, MINI_BAR_HEALTH_BOX_PIC, 0, 0,
                 ROTATE_SPRITE_SCREEN_CLIP | ROTATE_SPRITE_CORNER, 0, 0, xdim - 1, ydim - 1);

    if (UsingMenus)
    {
        x = MINI_BAR_AMMO_BOX_X;
        rotatesprite(x << 16, y << 16, (1 << 16), 0, MINI_BAR_AMMO_BOX_PIC, 0, 0,
                     ROTATE_SPRITE_SCREEN_CLIP | ROTATE_SPRITE_CORNER, 0, 0, xdim - 1, ydim - 1);
        x = MINI_BAR_INVENTORY_BOX_X;
        rotatesprite(x << 16, y << 16, (1 << 16), 0, MINI_BAR_INVENTORY_BOX_PIC, 0, 0,
                     ROTATE_SPRITE_SCREEN_CLIP | ROTATE_SPRITE_CORNER, 0, 0, xdim - 1, ydim - 1);
        return;
    }

    x = MINI_BAR_HEALTH_BOX_X+3;
    DisplayPanelNumber(pp, x, y+5, u->Health);

    if (u->WeaponNum != WPN_SWORD && u->WeaponNum != WPN_FIST)
        {
        x = MINI_BAR_AMMO_BOX_X;

        rotatesprite(x << 16, y << 16, (1 << 16), 0, MINI_BAR_AMMO_BOX_PIC, 0, 0,
                     ROTATE_SPRITE_SCREEN_CLIP | ROTATE_SPRITE_CORNER, 0, 0, xdim - 1, ydim - 1);

        x = MINI_BAR_AMMO_BOX_X+3;
        DisplayPanelNumber(pp, x, y+5, pp->WpnAmmo[u->WeaponNum]);
        }

    if (!pp->InventoryAmount[pp->InventoryNum])
        return;

    // Inventory Box
    x = MINI_BAR_INVENTORY_BOX_X;

    rotatesprite(x << 16, y << 16, (1 << 16), 0, MINI_BAR_INVENTORY_BOX_PIC, 0, 0,
                 ROTATE_SPRITE_SCREEN_CLIP | ROTATE_SPRITE_CORNER, 0, 0, xdim - 1, ydim - 1);

    id = &InventoryData[pp->InventoryNum];

    // Inventory pic
    x = MINI_BAR_INVENTORY_BOX_X + 2;
    y += 2;

    rotatesprite(x << 16, y << 16, (1 << 16), 0, id->State->picndx, 0, 0,
                 ROTATE_SPRITE_SCREEN_CLIP | ROTATE_SPRITE_CORNER, 0, 0, xdim - 1, ydim - 1);

    // will update the AUTO and % inventory values
    PlayerUpdateInventory(pp, pp->InventoryNum);
}

VOID PanelInvTestSuicide(PANEL_SPRITEp psp)
{
    if (TEST(psp->flags, PANF_SUICIDE))
    {
        pKillSprite(psp);
    }
}

PANEL_SPRITEp SpawnInventoryIcon(PLAYERp pp, short InventoryNum)
{
    PANEL_SPRITEp psp;
    short x,y;

    // check invalid value
    ASSERT(InventoryNum < MAX_INVENTORY);

    // check to see if its already spawned
    if (pp->InventorySprite[InventoryNum])
        return(NULL);

    // check for Icon panel state
    if (!InventoryData[InventoryNum].State)
        return(NULL);

    x = InventoryBarXpos[gs.BorderNum] + (InventoryNum*INVENTORY_ICON_WIDTH);
    y = InventoryBarYpos[gs.BorderNum];
    psp = pSpawnSprite(pp, InventoryData[InventoryNum].State, PRI_FRONT, x, y);
    pp->InventorySprite[InventoryNum] = psp;

    psp->x1 = 0;
    psp->y1 = 0;
    psp->x2 = xdim - 1;
    psp->y2 = ydim - 1;
    psp->scale = InventoryData[InventoryNum].Scale;
    SET(psp->flags, PANF_STATUS_AREA | PANF_SCREEN_CLIP);

    return(psp);
}

VOID KillPanelInv(PLAYERp pp, short InventoryNum)
{
    ASSERT(pp->InventorySprite[InventoryNum]);
    ASSERT(InventoryNum < MAX_INVENTORY);

    pp->InventoryTics[InventoryNum] = 0;
    SET(pp->InventorySprite[InventoryNum]->flags, PANF_SUICIDE);
    pp->InventorySprite[InventoryNum] = NULL;
}

VOID KillPlayerIcon(PLAYERp pp, PANEL_SPRITEp *pspp)
{
    SET((*pspp)->flags, PANF_SUICIDE);
    (*pspp) = NULL;
}

VOID KillAllPanelInv(PLAYERp pp)
{
    short i;

    for (i = 0; i < MAX_INVENTORY; i++)
    {
        if (!pp->InventorySprite[i])
            continue;

        pp->InventoryTics[i] = 0;
        SET(pp->InventorySprite[i]->flags, PANF_SUICIDE);
        pp->InventorySprite[i]->numpages = 0;
        pp->InventorySprite[i] = NULL;
    }
}

PANEL_SPRITEp SpawnIcon(PLAYERp pp, PANEL_STATEp state)
{
    PANEL_SPRITEp psp;
    short i;

    psp = pSpawnSprite(pp, state, PRI_FRONT, 0, 0);

    psp->x1 = 0;
    psp->y1 = 0;
    psp->x2 = xdim - 1;
    psp->y2 = ydim - 1;
    SET(psp->flags, PANF_STATUS_AREA | PANF_SCREEN_CLIP);
    return(psp);
}

//////////////////////////////////////////////////////////////////////
//
// MEDKIT
//
//////////////////////////////////////////////////////////////////////

VOID AutoPickInventory(PLAYERp pp)
{
    int i;

    // auto pick only if run out of currently selected one

    if (pp->InventoryAmount[pp->InventoryNum] <= 0)
    {
        for (i = 0; i < MAX_INVENTORY; i++)
        {
            if (i == INVENTORY_REPAIR_KIT)
                continue;

            if (pp->InventoryAmount[i])
            {
                pp->InventoryNum = i;
                return;
            }
        }

        // only take this if there is nothing else
        if (pp->InventoryAmount[INVENTORY_REPAIR_KIT])
            pp->InventoryNum = INVENTORY_REPAIR_KIT;
    }
}

VOID UseInventoryMedkit(PLAYERp pp)
{
    USERp u = User[pp->PlayerSprite];
    short diff;
    short inv = INVENTORY_MEDKIT;
    short amt;


    if (!pp->InventoryAmount[inv])
        return;

    diff = 100 - u->Health;
    if (diff <= 0)
        return;

    if(diff > pp->InventoryPercent[inv]) // If not enough to get to 100, use what's left
        amt = pp->InventoryPercent[inv];
    else
        amt = diff;

    PlayerUpdateHealth(pp, amt);

    pp->InventoryPercent[inv] -= diff;
    if (pp->InventoryPercent[inv] < 0)
    {
        pp->InventoryPercent[inv] = 0;
        pp->InventoryAmount[inv]--;
    }

    AutoPickInventory(pp);

    //percent
    PlayerUpdateInventory(pp, pp->InventoryNum);

    if (pp == Player+myconnectindex)
    {
        if (amt >= 30)
            PlayerSound(DIGI_GETMEDKIT,&pp->posx,&pp->posy,&pp->posz,v3df_follow|v3df_dontpan,pp);
        else
            PlayerSound(DIGI_AHH,&pp->posx,&pp->posy,&pp->posz,v3df_follow|v3df_dontpan,pp);
    }
}

//////////////////////////////////////////////////////////////////////
//
// CHEMICAL WARFARE CANISTERS
//
//////////////////////////////////////////////////////////////////////
VOID UseInventoryChemBomb(PLAYERp pp)
{
    USERp u = User[pp->PlayerSprite];
    short diff;
    short inv = INVENTORY_CHEMBOMB;

    if (!pp->InventoryAmount[inv])
        return;

    PlayerInitChemBomb(pp); // Throw a chemical bomb out there

    pp->InventoryPercent[inv] = 0;
    if (--pp->InventoryAmount[inv] < 0)
        pp->InventoryAmount[inv] = 0;

    AutoPickInventory(pp);

    PlayerUpdateInventory(pp, pp->InventoryNum);
}

//////////////////////////////////////////////////////////////////////
//
// FLASH BOMBS
//
//////////////////////////////////////////////////////////////////////
VOID UseInventoryFlashBomb(PLAYERp pp)
{
    USERp u = User[pp->PlayerSprite];
    short diff;
    short inv = INVENTORY_FLASHBOMB;

    if (!pp->InventoryAmount[inv])
        return;

    PlayerInitFlashBomb(pp);

    pp->InventoryPercent[inv] = 0;
    if (--pp->InventoryAmount[inv] < 0)
        pp->InventoryAmount[inv] = 0;

    AutoPickInventory(pp);

    PlayerUpdateInventory(pp, pp->InventoryNum);
}

//////////////////////////////////////////////////////////////////////
//
// CALTROPS
//
//////////////////////////////////////////////////////////////////////
VOID UseInventoryCaltrops(PLAYERp pp)
{
    USERp u = User[pp->PlayerSprite];
    short diff;
    short inv = INVENTORY_CALTROPS;

    if (!pp->InventoryAmount[inv])
        return;

    PlayerInitCaltrops(pp);

    pp->InventoryPercent[inv] = 0;
    if (--pp->InventoryAmount[inv] < 0)
        pp->InventoryAmount[inv] = 0;

    AutoPickInventory(pp);

    PlayerUpdateInventory(pp, pp->InventoryNum);
}

//////////////////////////////////////////////////////////////////////
//
// REPAIR KIT
//
//////////////////////////////////////////////////////////////////////

VOID UseInventoryRepairKit(PLAYERp pp)
{
    SPRITEp sp = pp->SpriteP;
    short inv = INVENTORY_REPAIR_KIT;

    //PlaySound(DIGI_TOOLBOX, &pp->posx, &pp->posy, &pp->posz, v3df_none);
    if (pp == Player + myconnectindex)
    {
        if(STD_RANDOM_RANGE(1000) > 500)
            PlayerSound(DIGI_NOREPAIRMAN,&pp->posx,&pp->posy,&pp->posz,v3df_follow|v3df_dontpan,pp);
        else
            PlayerSound(DIGI_NOREPAIRMAN2,&pp->posx,&pp->posy,&pp->posz,v3df_follow|v3df_dontpan,pp);
    }

    pp->InventoryPercent[inv] = 0;
    pp->InventoryAmount[inv] = 0;

    AutoPickInventory(pp);

    //percent
    PlayerUpdateInventory(pp, pp->InventoryNum);
}

//////////////////////////////////////////////////////////////////////
//
// CLOAK
//
//////////////////////////////////////////////////////////////////////

VOID UseInventoryCloak(PLAYERp pp)
{
    SPRITEp sp = pp->SpriteP;

    if (pp->InventoryActive[pp->InventoryNum])
    {
//        StopInventoryCloak(pp, pp->InventoryNum);
        return;
    }

    pp->InventoryActive[pp->InventoryNum] = TRUE;

    AutoPickInventory(pp);

    // on/off
    PlayerUpdateInventory(pp, pp->InventoryNum);

    SET(sp->cstat, CSTAT_SPRITE_TRANSLUCENT);
    sp->shade = 100;

    PlaySound(DIGI_GASPOP, &pp->posx, &pp->posy, &pp->posz, v3df_none);
    //if(RANDOM_RANGE(1000) > 950)
    if(pp == Player+myconnectindex)
        PlayerSound(DIGI_IAMSHADOW,&pp->posx,&pp->posy,&pp->posz,v3df_follow|v3df_dontpan,pp);
}

VOID StopInventoryCloak(PLAYERp pp, short InventoryNum)
{
    SPRITEp sp = pp->SpriteP;

    pp->InventoryActive[InventoryNum] = FALSE;

    if (pp->InventoryPercent[InventoryNum] <= 0)
    {
        pp->InventoryPercent[InventoryNum] = 0;
        if (--pp->InventoryAmount[InventoryNum] < 0)
            pp->InventoryAmount[InventoryNum] = 0;
    }

    // on/off
    PlayerUpdateInventory(pp, InventoryNum);

    RESET(sp->cstat, CSTAT_SPRITE_TRANSLUCENT);
    sp->shade = 0;

    PlaySound(DIGI_GASPOP, &pp->posx, &pp->posy, &pp->posz, v3df_none);
}

//////////////////////////////////////////////////////////////////////
//
// ENVIRONMENT SUIT
//
//////////////////////////////////////////////////////////////////////
#if 0
VOID UseInventoryEnvironSuit(PLAYERp pp)
{
    SPRITEp sp = pp->SpriteP;
    short inv = INVENTORY_ENVIRON_SUIT;

    // only activate if you have one
    if (pp->InventoryAmount[inv])
    {
        pp->InventoryActive[inv] = TRUE;
        // on/off
        PlayerUpdateInventory(pp, inv);
    }
}

VOID StopInventoryEnvironSuit(PLAYERp pp, short InventoryNum)
{
    SPRITEp sp = pp->SpriteP;
    short inv = INVENTORY_ENVIRON_SUIT;

    pp->InventoryActive[InventoryNum] = FALSE;

    if (pp->InventoryPercent[InventoryNum] <= 0)
    {
        pp->InventoryPercent[InventoryNum] = 0;
        if (--pp->InventoryAmount[InventoryNum] < 0)
            pp->InventoryAmount[InventoryNum] = 0;
    }

    // on/off
    PlayerUpdateInventory(pp, InventoryNum);

    PlaySound(DIGI_SWCLOAKUNCLOAK, &pp->posx, &pp->posy, &pp->posz, v3df_none);
}
#endif

//////////////////////////////////////////////////////////////////////
//
// NIGHT VISION
//
//////////////////////////////////////////////////////////////////////

static char sectorfloorpals[MAXSECTORS], sectorceilingpals[MAXSECTORS], wallpals[MAXWALLS];

#if 0
VOID DoPlayerNightVisionPalette(PLAYERp pp)
{
    extern char DefaultPalette[];
    short i;

    if (pp->InventoryActive[INVENTORY_NIGHT_VISION] && (pp - Player == screenpeek))
    {
        if (NightVision)
            return;                     // Already using night vision, don't
                                        // bother.
        visibility = 0;
        for (i = 0; i < numsectors; i++)
        {
            sectorfloorpals[i] = sector[i].floorpal;
            sectorceilingpals[i] = sector[i].ceilingpal;
            sector[i].floorpal = PALETTE_GREEN_LIGHTING;
            sector[i].ceilingpal = PALETTE_GREEN_LIGHTING;
        }
        for (i = 0; i < numwalls; i++)
        {
            wallpals[i] = wall[i].pal;
            wall[i].pal = PALETTE_GREEN_LIGHTING;
        }

        NightVision = TRUE;
        COVERsetbrightness(4, (char *) &palette_data[0][0]);
    }
    else
    {
        visibility = NormalVisibility;
        for (i = 0; i < numsectors; i++)
        {
            sector[i].floorpal = sectorfloorpals[i];
            sector[i].ceilingpal = sectorceilingpals[i];
        }
        for (i = 0; i < numwalls; i++)
        {
            wall[i].pal = wallpals[i];
        }
        DoPlayerDivePalette(pp);  // Check again to see if its a water palette
        NightVision = FALSE;
        COVERsetbrightness(gs.Brightness, (char *) &palette_data[0][0]);
    }
}
#endif

VOID DoPlayerNightVisionPalette(PLAYERp pp)
{
    extern char DefaultPalette[];
    extern int32 ScreenBPP;

    if (pp != Player + screenpeek) return;

    if (pp->InventoryActive[INVENTORY_NIGHT_VISION])
    {
//        if (pp->NightVision && pp->StartColor == 148)
//            return;
        SetFadeAmt(pp,-1005,148); // Night vision green tint
        pp->NightVision = TRUE;
    }
    else
    {
        // Put it all back to normal
        if (pp->StartColor == 148)
        {
            memcpy(pp->temp_pal, palette_data, sizeof(palette_data));
            memcpy(palookup[PALETTE_DEFAULT], DefaultPalette, 256 * 32);
            pp->FadeAmt = 0;
            if (getrendermode() < 3)
                COVERsetbrightness(gs.Brightness, (char *) &palette_data[0][0]);
	        else
		        setpalettefade(0,0,0,0);
        }
        // clear Night vision green tint
        if (ScreenBPP > 8 && !pp->Underwater && (pp->StartColor == 0 || pp->NightVision || pp->StartColor == 112))
            SetFadeAmt(pp,-1005,0);
        pp->NightVision = FALSE;
    }
}

VOID UseInventoryNightVision(PLAYERp pp)
{
    SPRITEp sp = pp->SpriteP;
    #define NIGHT_INVENTORY_TIME 30

    if (pp->InventoryActive[pp->InventoryNum])
    {
        StopInventoryNightVision(pp, pp->InventoryNum);
        return;
    }

    pp->InventoryActive[pp->InventoryNum] = TRUE;

    // on/off
    PlayerUpdateInventory(pp, pp->InventoryNum);

    DoPlayerNightVisionPalette(pp);
    PlaySound(DIGI_NIGHTON, &pp->posx, &pp->posy, &pp->posz, v3df_dontpan|v3df_follow);
}

VOID StopInventoryNightVision(PLAYERp pp, short InventoryNum)
{
    SPRITEp sp = pp->SpriteP;

    pp->InventoryActive[InventoryNum] = FALSE;

    if (pp->InventoryPercent[InventoryNum] <= 0)
    {
        pp->InventoryPercent[InventoryNum] = 0;
        if (--pp->InventoryAmount[InventoryNum] < 0)
            pp->InventoryAmount[InventoryNum] = 0;
    }

    AutoPickInventory(pp);

    // on/off
    PlayerUpdateInventory(pp, pp->InventoryNum);

    DoPlayerNightVisionPalette(pp);
    DoPlayerDivePalette(pp);
    PlaySound(DIGI_NIGHTOFF, &pp->posx, &pp->posy, &pp->posz, v3df_dontpan|v3df_follow);
}

//////////////////////////////////////////////////////////////////////
//
// INVENTORY KEYS
//
//////////////////////////////////////////////////////////////////////

VOID InventoryKeys(PLAYERp pp)
{
    short inv_hotkey;

    // scroll SPELLs left
    if (TEST_SYNC_KEY(pp, SK_INV_LEFT))
    {
        if (FLAG_KEY_PRESSED(pp, SK_INV_LEFT))
        {
            FLAG_KEY_RELEASE(pp, SK_INV_LEFT);
            SpawnInventoryBar(pp);
            PlayerUpdateInventory(pp, pp->InventoryNum - 1);
            PutStringInfo(pp, InventoryData[pp->InventoryNum].Name);
            InventoryBarUpdatePosition(pp);
            InvBorderRefresh(pp);
        }
    }
    else
    {
        FLAG_KEY_RESET(pp, SK_INV_LEFT);
    }

    // scroll SPELLs right
    if (TEST_SYNC_KEY(pp, SK_INV_RIGHT))
    {
        if (FLAG_KEY_PRESSED(pp, SK_INV_RIGHT))
        {
            FLAG_KEY_RELEASE(pp, SK_INV_RIGHT);
            SpawnInventoryBar(pp);
            PlayerUpdateInventory(pp, pp->InventoryNum + 1);
            PutStringInfo(pp, InventoryData[pp->InventoryNum].Name);
            InventoryBarUpdatePosition(pp);
            InvBorderRefresh(pp);
        }
    }
    else
    {
        FLAG_KEY_RESET(pp, SK_INV_RIGHT);
    }

    if (TEST_SYNC_KEY(pp, SK_INV_USE))
    {
        if (FLAG_KEY_PRESSED(pp, SK_INV_USE))
        {
            FLAG_KEY_RELEASE(pp, SK_INV_USE);

            if (InventoryData[pp->InventoryNum].Init)
            {
                if (pp->InventoryAmount[pp->InventoryNum])
                {
                    InventoryUse(pp);
                }
                else
                {
                    sprintf(ds,"No %s",InventoryData[pp->InventoryNum].Name);
                    PutStringInfo(pp,ds); // DONT have message
                }
            }
        }
    }
    else
    {
        FLAG_KEY_RESET(pp, SK_INV_USE);
    }

    // get hotkey number out of input bits
    inv_hotkey = TEST(pp->input.bits, SK_INV_HOTKEY_MASK) >> SK_INV_HOTKEY_BIT0;

    if (inv_hotkey)
    {
        if (FLAG_KEY_PRESSED(pp, SK_INV_HOTKEY_BIT0))
        {
            FLAG_KEY_RELEASE(pp, SK_INV_HOTKEY_BIT0);

            inv_hotkey -= 1;

            ////DSPRINTF(ds,"inv_hotkey %d",inv_hotkey);
            //MONO_PRINT(ds);

            // switches you to this inventory item
            //PlayerUpdateInventory(pp, inv_hotkey);
            pp->InventoryNum = inv_hotkey;

            if (InventoryData[pp->InventoryNum].Init && !TEST(pp->Flags, PF_CLIMBING))
            {
                if (pp->InventoryAmount[pp->InventoryNum])
                {
                    InventoryUse(pp);
                }
                #if 0
                else
                {
                    sprintf(ds,"No %s",InventoryData[pp->InventoryNum].Name);
                    PutStringInfo(pp,ds); // DONT have message
                }
                #endif
            }

            //PlayerUpdateInventory(pp, pp->InventoryNum);
        }
    }
    else
    {
        FLAG_KEY_RESET(pp, SK_INV_HOTKEY_BIT0);
    }
}

VOID InvBorderRefresh(PLAYERp pp)
{
    long x,y;

    if (pp != Player + myconnectindex)
        return;

    x = InventoryBarXpos[gs.BorderNum];
    y = InventoryBarYpos[gs.BorderNum];

    SetRedrawScreen(pp);
    //BorderRefreshClip(pp, x-5, y-5, x + (MAX_INVENTORY * INVENTORY_ICON_WIDTH), y + 24);
}

VOID InventoryTimer(PLAYERp pp)
{
    // called every time through loop
    short inv = 0;
    INVENTORY_DATAp id;

    // if bar is up
    if (pp->InventoryBarTics)
    {
        InventoryBarUpdatePosition(pp);

        pp->InventoryBarTics -= synctics;
        // if bar time has elapsed
        if (pp->InventoryBarTics <= 0)
        {
            // get rid of the bar
            KillInventoryBar(pp);
            // don't update bar anymore
            pp->InventoryBarTics = 0;

            InvBorderRefresh(pp);
            //BorderRefresh(pp);
        }
    }

    for (id = InventoryData; id->Name; id++, inv++)
    {
        // if timed and active
        if (TEST(id->Flags, INVF_TIMED) && pp->InventoryActive[inv])
        {
            // dec tics
            pp->InventoryTics[inv] -= synctics;
            if (pp->InventoryTics[inv] <= 0)
            {
                // take off a percentage
                pp->InventoryPercent[inv] -= id->DecPerSec;
                if (pp->InventoryPercent[inv] <= 0)
                {
                    // ALL USED UP
                    pp->InventoryPercent[inv] = 0;
                    InventoryStop(pp, inv);
                    pp->InventoryActive[inv] = FALSE;
                }
                else
                {
                    // reset 1 sec tic clock
                    pp->InventoryTics[inv] = SEC(1);
                }

                //PlayerUpdateInventoryPercent(pp);
                PlayerUpdateInventory(pp, pp->InventoryNum);
            }
        }
        else
        // the idea behind this is that the USE function will get called
        // every time the player is in an AUTO_USE situation.
        // This code will decrement the timer and set the Item to InActive
        // EVERY SINGLE TIME.  Relies on the USE function getting called!
        if (TEST(id->Flags, INVF_AUTO_USE) && pp->InventoryActive[inv])
        {
            pp->InventoryTics[inv] -= synctics;
            if (pp->InventoryTics[inv] <= 0)
            {
                // take off a percentage
                pp->InventoryPercent[inv] -= id->DecPerSec;
                if (pp->InventoryPercent[inv] <= 0)
                {
                    // ALL USED UP
                    pp->InventoryPercent[inv] = 0;
                    // should get rid if Amount - stop it for good
                    InventoryStop(pp, inv);
                }
                else
                {
                    // reset 1 sec tic clock
                    pp->InventoryTics[inv] = SEC(1);
                    // set to InActive EVERY TIME THROUGH THE LOOP!
                    pp->InventoryActive[inv] = FALSE;
                }

                //PlayerUpdateInventoryPercent(pp);
                PlayerUpdateInventory(pp, pp->InventoryNum);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////
//
// INVENTORY BAR
//
//////////////////////////////////////////////////////////////////////

VOID SpawnInventoryBar(PLAYERp pp)
{
    short inv = 0;
    INVENTORY_DATAp id;
    PANEL_SPRITEp psp;

    // its already up
    if (pp->InventoryBarTics)
    {
        pp->InventoryBarTics = SEC(2);
        return;
    }

    pp->InventorySelectionBox = SpawnIcon(pp, ps_PanelSelectionBox);

    for (id = InventoryData; id->Name; id++, inv++)
    {
        psp = SpawnInventoryIcon(pp, inv);

        if (!pp->InventoryAmount[inv])
        {
            //SET(psp->flags, PANF_TRANSLUCENT);
            //SET(psp->flags, PANF_TRANS_FLIP);
            psp->shade = 100; //Darken it
        }
    }

    pp->InventoryBarTics = SEC(2);
}

VOID KillInventoryBar(PLAYERp pp)
{
    KillAllPanelInv(pp);
    KillPlayerIcon(pp, &pp->InventorySelectionBox);
}

// In case the BorderNum changes - move the postions
VOID InventoryBarUpdatePosition(PLAYERp pp)
{
    short ybuf[] = {1,2,0,3,0,0,3,0};
    short inv = 0;
    short x,y;
    INVENTORY_DATAp id;

    x = InventoryBarXpos[gs.BorderNum] + (pp->InventoryNum * INVENTORY_ICON_WIDTH);
    y = InventoryBarYpos[gs.BorderNum];

    if (gs.BorderNum == BORDER_MINI_BAR)
    {
        if (gs.MiniHudTyp == 0)
            y -= 7;
        else
        if (gs.MiniHudTyp == 2)
            y -= 15;
    }

    pp->InventorySelectionBox->x = x - 5;
    pp->InventorySelectionBox->y = y - 2; //5;

    for (id = InventoryData; id->Name && inv < MAX_INVENTORY; id++, inv++)
    {
        x = InventoryBarXpos[gs.BorderNum] + (inv * INVENTORY_ICON_WIDTH);
        y = InventoryBarYpos[gs.BorderNum];

        if (inv < 7)
            y = y + ybuf[inv];

        if (gs.BorderNum == BORDER_MINI_BAR)
        {
            if (gs.MiniHudTyp == 0)
                y -= 7;
            else
            if (gs.MiniHudTyp == 2)
                y -= 15;
        }

        if (pp->InventorySprite[inv])
        {
            pp->InventorySprite[inv]->x = x;
            pp->InventorySprite[inv]->y = y;
        }
    }
}

VOID InventoryUse(PLAYERp pp)
{
    INVENTORY_DATAp id = &InventoryData[pp->InventoryNum];

    if (id->Init)
        (*id->Init)(pp);
}

VOID InventoryStop(PLAYERp pp, short InventoryNum)
{
    INVENTORY_DATAp id = &InventoryData[InventoryNum];

    if (id->Stop)
        (*id->Stop)(pp, InventoryNum);
}

/////////////////////////////////////////////////////////////////
//
// Inventory Console Area
//
/////////////////////////////////////////////////////////////////

#define INVENTORY_BOX_X 231
#define INVENTORY_BOX_Y (176-8)
#define INVENTORY_BOX_ERASE 2403

short InventoryBoxX;
short InventoryBoxY;
short InventoryXoff;
short InventoryYoff;
VOID (*InventoryDisplayString)(PLAYERp, short, short, short, CHARp);

#define INVENTORY_PIC_XOFF 1
#define INVENTORY_PIC_YOFF 1

#define INVENTORY_PERCENT_XOFF 19
#define INVENTORY_PERCENT_YOFF 13

#define INVENTORY_STATE_XOFF 19
#define INVENTORY_STATE_YOFF 1

VOID PlayerUpdateInventory(PLAYERp pp, short InventoryNum)
{
    USERp u = User[pp->PlayerSprite];
    short w,h;
    VOID DisplayMiniBarInv(PLAYERp pp, short xs, short ys, short pal, CHARp buffer);
    VOID DisplaySmString(PLAYERp pp, short xs, short ys, short pal, CHARp buffer);

    // Check for items that need to go translucent from use
    if (pp->InventoryBarTics)
    {
        short inv = 0;
        INVENTORY_DATAp id;
        PANEL_SPRITEp psp;

        // Go translucent if used
        for (id = InventoryData; id->Name && inv < MAX_INVENTORY; id++, inv++)
        {
            psp = pp->InventorySprite[inv];

            if (!psp)
                continue;

            if (!pp->InventoryAmount[inv])
            {
                //SET(psp->flags, PANF_TRANSLUCENT);
                //SET(psp->flags, PANF_TRANS_FLIP);
                psp->shade = 100; // Darken it
            }
            else
            {
                //RESET(psp->flags, PANF_TRANSLUCENT);
                //RESET(psp->flags, PANF_TRANS_FLIP);
                psp->shade = 0;
            }
        }
    }

    pp->InventoryNum = InventoryNum;

    if (pp->InventoryNum < 0)
        pp->InventoryNum = MAX_INVENTORY-1;

    if (pp->InventoryNum >= MAX_INVENTORY)
        pp->InventoryNum = 0;

    if (pp - Player != screenpeek)
        return;

    if (gs.BorderNum == BORDER_MINI_BAR && gs.MiniHudTyp != 2)
    {
        InventoryXoff = 1;
        InventoryYoff = 1;

        if (gs.MiniHudTyp == 0)
        {
            InventoryBoxX = MINI_BAR_INVENTORY_BOX_X;
            InventoryBoxY = MINI_BAR_INVENTORY_BOX_Y;
            InventoryDisplayString = DisplayMiniBarSmString;
        }
        else
        if (gs.MiniHudTyp == 1)
        {
            InventoryBoxX = MINI_HUD_INVENTORY_BOX_X;
            InventoryBoxY = MINI_HUD_INVENTORY_BOX_Y;
            InventoryDisplayString = DisplayMiniBarInv;
        }
    }
    else
    {
        if (gs.BorderNum < BORDER_MINI_BAR)
            return;

        InventoryBoxX = INVENTORY_BOX_X;
        InventoryBoxY = INVENTORY_BOX_Y;

        InventoryXoff = 0;
        InventoryYoff = 0;

        InventoryDisplayString = DisplaySmString;

        // erase old info
        if (gs.BorderNum >= BORDER_BAR)
            pSpawnFullScreenSprite(pp, INVENTORY_BOX_ERASE, PRI_MID, INVENTORY_BOX_X, INVENTORY_BOX_Y);

        // put pic
        if (pp->InventoryAmount[pp->InventoryNum])
            PlayerUpdateInventoryPic(pp);
    }

    if (pp->InventoryAmount[pp->InventoryNum])
    {
        // Auto/On/Off
        PlayerUpdateInventoryState(pp);
        // Percent count/Item count
        PlayerUpdateInventoryPercent(pp);
    }
}

VOID PlayerUpdateInventoryPercent(PLAYERp pp)
{
    USERp u = User[pp->PlayerSprite];
    short x,y;
    INVENTORY_DATAp id = &InventoryData[pp->InventoryNum];

    x = InventoryBoxX + INVENTORY_PERCENT_XOFF + InventoryXoff;
    y = InventoryBoxY + INVENTORY_PERCENT_YOFF + InventoryYoff;

    if (gs.BorderNum == BORDER_MINI_BAR)
    {
        if (gs.MiniHudTyp == 1)
        {
            x = x + 6;
            y = y - 1;
        }
    }

    if (TEST(id->Flags, INVF_COUNT))
    {
        sprintf(ds,"%d", pp->InventoryAmount[pp->InventoryNum]);
        InventoryDisplayString(pp, x, y, 0, ds);
    }
    else
    {
        sprintf(ds,"%d%c", pp->InventoryPercent[pp->InventoryNum],'%');
        InventoryDisplayString(pp, x, y, 0, ds);
    }
}

VOID PlayerUpdateInventoryPic(PLAYERp pp)
{
    USERp u = User[pp->PlayerSprite];
    PANEL_SPRITEp psp;
    short pic;
    short x,y;
    INVENTORY_DATAp id = &InventoryData[pp->InventoryNum];

    x = InventoryBoxX + INVENTORY_PIC_XOFF + InventoryXoff;
    y = InventoryBoxY + INVENTORY_PIC_YOFF + InventoryYoff;

    pic = id->State->picndx;

    psp = pSpawnFullScreenSprite(pp, pic, PRI_FRONT_MAX, x, y);

    psp->scale = id->Scale;
}

VOID PlayerUpdateInventoryState(PLAYERp pp)
{
    USERp u = User[pp->PlayerSprite];
    short x,y;
    INVENTORY_DATAp id = &InventoryData[pp->InventoryNum];

    x = InventoryBoxX + INVENTORY_STATE_XOFF + InventoryXoff;
    y = InventoryBoxY + INVENTORY_STATE_YOFF + InventoryYoff;

    if (gs.BorderNum == BORDER_MINI_BAR)
    {
        if (gs.MiniHudTyp == 1)
        {
            x = x + 6;
            y = y - 2;
            if (gs.SetHighres == 1 && usehightile)
                y = y + 2;
        }
    }

    if (TEST(id->Flags, INVF_AUTO_USE))
    {
        sprintf(ds,"%s", "AUTO");
        InventoryDisplayString(pp, x, y, 0, ds);
    }
    else
    if (TEST(id->Flags, INVF_TIMED))
    {
        sprintf(ds,"%s", pp->InventoryActive[pp->InventoryNum] ? "ON" : "OFF");
        InventoryDisplayString(pp, x, y, 0, ds);
    }
}

