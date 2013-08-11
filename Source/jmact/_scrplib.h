//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment

This file is NOT part of Duke Nukem 3D version 1.5 - Atomic Edition
However, it is either an older version of a file that is, or is
some test code written during the development of Duke Nukem 3D.
This file is provided purely for educational interest.

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

Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms
Modifications for JonoF's port by Jonathon Fowler (jonof@edgenetwk.com)
*/
//-------------------------------------------------------------------------

// scrplib.h

#ifndef _scriplib_private
#define _scriplib_private
#ifdef __cplusplus
extern "C" {
#endif

#define SCRIPTSECTIONSTART ('[')
#define SCRIPTSECTIONEND   (']')
#define SCRIPTENTRYSEPARATOR ('=')
#define SCRIPTCOMMENT      (';')
#define SCRIPTEOL          ('\n')
#define SCRIPTNULL         ('\0')
#define SCRIPTSTRINGSEPARATOR ('"')
#define SCRIPTHEXFIRST ('0')
#define SCRIPTHEXSECOND ('x')
#define SCRIPTSPACE     (' ')
#define SCRIPTDEFAULTVALUE ('~')
#define MAXSCRIPTFILES 20
#define SCRIPT(scripthandle,item) (scriptfiles[(scripthandle)]->item)

typedef enum
   {
   linetype_comment,
   linetype_section,
   linetype_entry
   } linetype_t;

typedef struct scriptline
   {
   int32  type;
   void * ptr;
   struct scriptline *nextline;
   struct scriptline *prevline;
   } ScriptLineType;

typedef struct scriptentry
   {
   char * name;
   char * value;
   struct scriptentry *nextentry;
   struct scriptentry *preventry;
   } ScriptEntryType;

typedef struct scriptsection
   {
   char * name;
   ScriptEntryType      *entries;
   ScriptLineType       *lastline;
   struct scriptsection *nextsection;
   struct scriptsection *prevsection;
   } ScriptSectionType;

typedef struct
   {
   ScriptSectionType * script;
   ScriptSectionType * lastsection;
   ScriptLineType * scriptlines;
   char scriptfilename[128];
   } script_t;

/*
==============
=
= SCRIPT_New
=
==============
*/

int32 SCRIPT_New( void );

/*
==============
=
= SCRIPT_Delete
=
==============
*/
void SCRIPT_Delete( int32 scripthandle );

/*
==============
=
= SCRIPT_FreeSection
=
==============
*/
void SCRIPT_FreeSection( ScriptSectionType * section );

/*
==============
=
= SafeWriteString
=
==============
*/
void SafeWriteString (int32 handle, char * string);

/*
==============
=
= SCRIPT_AddLine
=
==============
*/


ScriptLineType * SCRIPT_AddLine
   (
   ScriptLineType * root,
   int32 type,
   void * ptr
   );

/*
==============
=
= SCRIPT_SectionExists
=
==============
*/
ScriptSectionType * SCRIPT_SectionExists
   (
   int32 scripthandle,
   char * sectionname
   );

/*
==============
=
= SCRIPT_AddSection
=
==============
*/
ScriptSectionType * SCRIPT_AddSection( int32 scripthandle, char * sectionname );

/*
==============
=
= SCRIPT_EntryExists
=
==============
*/
ScriptEntryType * SCRIPT_EntryExists
   (
   ScriptSectionType * section,
   char * entryname
   );

/*
==============
=
= SCRIPT_AddEntry
=
==============
*/
void SCRIPT_AddEntry
   (
   int32 scripthandle,
   char * sectionname,
   char * entryname,
   char * entryvalue
   );

/*
==============
=
= SCRIPT_DecodeToken
=
==============
*/

void SCRIPT_DecodeToken ( int32 scripthandle, char * str );


#ifdef __cplusplus
};
#endif
#endif
