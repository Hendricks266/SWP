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

//****************************************************************************
//
// sounds.h
//
//****************************************************************************

#include "mytypes.h"


#ifndef _sounds_public_
#define _sounds_public_

void SoundStartup( void );
void SoundShutdown( void );
void MusicStartup( void );
void MusicShutdown( void );

// 3D sound engine declarations //////////////////////////////////////////////
// Flag settings used to turn on and off 3d sound options
typedef enum
{
    v3df_none       = 0,    // Default, take no action, use all defaults
    v3df_follow     = 1,    // 1 = Do coordinate updates on sound
                            // Use this only if the sprite won't be deleted soon
    v3df_kill       = 2,    // 1 = Sound is to be deleted
    v3df_doppler    = 4,    // 1 = Don't use doppler pitch variance
    v3df_dontpan        = 8,    // 1 = Don't do panning of sound
    v3df_ambient    = 16,   // 1 = Sound is ambient, use ambient struct info.
    v3df_intermit   = 32,   // 1 = Intermittant sound
    v3df_init       = 64,   // 1 = First pass of sound, don't play it.
                            // This is mainly used for intermittent sounds
    v3df_nolookup   = 128   // don't use ambient table lookup
} Voc3D_Flags;

struct VOCstruct;
typedef struct VOCstruct VOC_INFO, *VOC_INFOp;
struct VOC3Dstruct;
typedef struct VOC3Dstruct VOC3D_INFO, *VOC3D_INFOp;
struct ambientstruct;
typedef struct ambientstruct AMB_INFO, *AMB_INFOp;


extern VOC3D_INFOp voc3dstart;
extern VOC3D_INFOp voc3dend;

void DoUpdateSounds3D( void );
void Terminate3DSounds( void );

void Set3DSoundOwner( short spritenum );
void PlaySpriteSound( short spritenum, int attrib_ndx, Voc3D_Flags flags );
void DeleteNoSoundOwner( short spritenum );
void DeleteNoFollowSoundOwner(short spritenum);

BOOL CacheSound(int num, int type);
void COVER_SetReverb(long amt);
VOID UnInitSound(VOID);
void InitFX(void );
VOID InitMusic(VOID);
VOID StopFX(VOID);
void FlipStereo(void );
int StopSong(void );
void StopSound(void );
void StartAmbientSound(void );
void StopAmbientSound(void );
int PlaySong(char *song_file_name, int cdaudio_track, BOOL loop, BOOL restart);
VOID PlaySoundRTS(long rts_num);

//
// Standard VOC format information - generally don't need this
//

typedef struct
    {
    BYTE filler[0x1a];
    BYTE type;
    WORD length;
    BYTE filler2;
    BYTE freq;
    BYTE pack;
    BYTE data[1];
    } *VOC_HDRp;

// Ambient Sound Structure
struct ambientstruct
    {
    SHORT name;
    SHORT diginame;
    Voc3D_Flags ambient_flags;
    long maxtics;                       // When tics reaches this number next
    // sound happens
    };

// VOC File flag settings for digi_entries.
typedef enum
    {
    vf_normal = 0,
    vf_loop = 1
    } Voc_Flags;

//
// Table that describes the voc file and how it will be played
// Can be easily extended, but you may need to change digi.h
//

//struct STATEstruct;
//typedef struct VOCstruct VOC_INFO, *VOC_INFOp;

struct VOCstruct
    {
    char name[14];                      // name of voc file on disk
    char *data;                         // pointer to voc data
    SHORT pitch_lo;                     // lo pitch value
    SHORT pitch_hi;                     // hi pitch value
    BYTE priority;                      // priority at which vocs are played
    SHORT voc_num;                      // Backward reference to parent sound
    int voc_distance;                   // Sound's distance effectiveness
    Voc_Flags voc_flags;                // Various allowable flag settings for voc
    BYTE lock;                          // locking byte for caching
    BYTE playing;                       // number of this type of sound currently playing
    };


// JIMSOUND3D(tm) variables section //////////////////////////////////////////

struct VOC3Dstruct
    {
    VOC_INFOp vp;                       // Pointer to the sound
    long *x;                            // Pointer to x coordinate
    long *y;                            // Pointer to y coordinate
    long *z;                            // Pointer to z coordinate
    long fx, fy, fz;                    // Non-Follow literal values
    Voc3D_Flags flags;                  // 3d voc sound flags
    int handle;                         // Current handle to the voc
    short doplr_delta;                  // Change in distance since last call
    VOC3D_INFOp prev, next;             // Linked voc list
    short owner;                        // Hold index into user array to
                                        // delete looping sounds
    int num;                            // Digital Entry number used for
                                        // callback of looping sounds
                                        // If sound is active but user == 0, stop the sound
    short dist;                         // Current distance of sound from player
    BYTE priority;                      // Used to force a higher priority based on distance
    long tics;                          // Tics used to count to next sound occurance
    long maxtics;                       // Tics until next sound occurance
                                        // for intermittent sounds
    BOOL deleted;                       // Has sound been marked for deletion?
    BOOL FX_Ok;                         // Did this sound play ok?
    };

extern VOC_INFO voc[];

#endif
