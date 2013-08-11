/*
 * mouse.c
 * MACT library -to- JonoF's Build Port Mouse Glue
 * 
 * by Jonathon Fowler
 *
 * Since we don't have the source to the MACT library I've had to
 * concoct some magic to glue its idea of controllers into that of
 * my Build port.
 *
 */
//-------------------------------------------------------------------------
/*
Duke Nukem Copyright (C) 1996, 2003 3D Realms Entertainment

This file is part of Duke Nukem 3D version 1.5 - Atomic Edition

Duke Nukem 3D is free software; you can redistribute it and/or
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
*/
//-------------------------------------------------------------------------

#include "types.h"
#include "mouse.h"
#include "baselayer.h"


boolean MOUSE_Init( void )
{
	initmouse();
	return ((inputdevices & 2) == 2);
}


void MOUSE_Shutdown( void )
{
	uninitmouse();
}


void MOUSE_ShowCursor( void )
{
}


void MOUSE_HideCursor( void )
{
}


int32 MOUSE_GetButtons( void )
{
	int32 buttons;
	readmousebstatus(&buttons);
	return buttons;
}


int32 MOUSE_ClearButton( int32 b )
{
	return (mouseb &= ~b);
}


void MOUSE_GetPosition( int32*x, int32*y  )
{
}


void MOUSE_GetDelta( int32*x, int32*y  )
{
	readmousexy(x,y);
}



