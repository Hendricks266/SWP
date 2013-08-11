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


void SectorLightShade(SPRITEp sp, short intensity);
VOID DiffuseLighting(SPRITEp sp);
VOID DoLightingMatch(short match, short state);
VOID InitLighting(VOID);
VOID DoLighting(VOID);

// Descriptive Light variables mapped from other variables

#define LIGHT_Match(sp)         (SP_TAG2((sp)))
#define LIGHT_Type(sp)          (SP_TAG3((sp)))
#define LIGHT_MaxTics(sp)       (SP_TAG4((sp)))
#define LIGHT_MaxBright(sp)     (SP_TAG5((sp)))
#define LIGHT_MaxDark(sp)       (SP_TAG6((sp)))
#define LIGHT_ShadeInc(sp)      (SP_TAG7((sp)))

#define LIGHT_Dir(sp)           (!!(TEST((sp)->extra, SPRX_BOOL10)))
#define LIGHT_DirChange(sp)     (FLIP((sp)->extra, SPRX_BOOL10))

#define LIGHT_Shade(sp)         ((sp)->shade)
#define LIGHT_FloorShade(sp)    ((sp)->xoffset)
#define LIGHT_CeilingShade(sp)  ((sp)->yoffset)
#define LIGHT_Tics(sp)          ((sp)->z)

#define LIGHT_DiffuseNum(sp) (SP_TAG3((sp)))
#define LIGHT_DiffuseMult(sp) (SP_TAG4((sp)))

enum LightTypes {LIGHT_CONSTANT, LIGHT_FLICKER, LIGHT_FADE, LIGHT_FLICKER_ON, LIGHT_FADE_TO_ON_OFF};
