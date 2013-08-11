/*
Copyright (C) 1994-1995 Apogee Software, Ltd.

This program is free software; you can redistribute it and/or
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

Modifications for JonoF's port by Jonathon Fowler (jonof@edgenetwk.com)
*/
/**********************************************************************
   module: MPU401.C

   author: James R. Dose
   date:   January 1, 1994

   Low level routines to support sending of MIDI data to MPU401
   compatible MIDI interfaces.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#include "mpu401.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

static HMIDISTRM hmido = (HMIDISTRM)-1;
static MIDIOUTCAPS midicaps;
static DWORD mididevice = -1;

typedef struct {
	long time;
	long stream;
	long event;
} MIDIEVENTHEAD;
#define PAD(x) ((((x)+3)&(~3)))

#define BUFFERLEN (32*4*4)
#define NUMBUFFERS 6
static char eventbuf[NUMBUFFERS][BUFFERLEN];
static int  eventcnt[NUMBUFFERS];
static MIDIHDR bufferheaders[NUMBUFFERS];
       int  _MPU_CurrentBuffer = 0;
       int  _MPU_BuffersWaiting = 0;

extern unsigned long _MIDI_GlobalPositionInTicks;
unsigned long _MPU_LastEvent=0;

#define MIDI_NOTE_OFF         0x80
#define MIDI_NOTE_ON          0x90
#define MIDI_POLY_AFTER_TCH   0xA0
#define MIDI_CONTROL_CHANGE   0xB0
#define MIDI_PROGRAM_CHANGE   0xC0
#define MIDI_AFTER_TOUCH      0xD0
#define MIDI_PITCH_BEND       0xE0
#define MIDI_META_EVENT       0xFF
#define MIDI_END_OF_TRACK     0x2F
#define MIDI_TEMPO_CHANGE     0x51
#define MIDI_MONO_MODE_ON     0x7E
#define MIDI_ALL_NOTES_OFF    0x7B


/**********************************************************************

   Memory locked functions:

**********************************************************************/


void MPU_FinishBuffer( int buffer )
{
	if (!eventcnt[buffer]) return;
	ZeroMemory(&bufferheaders[buffer], sizeof(MIDIHDR));
	bufferheaders[buffer].lpData = eventbuf[buffer];
	bufferheaders[buffer].dwBufferLength =
	bufferheaders[buffer].dwBytesRecorded = eventcnt[buffer];
	midiOutPrepareHeader((HMIDIOUT)hmido, &bufferheaders[buffer], sizeof(MIDIHDR));
	midiStreamOut(hmido, &bufferheaders[buffer], sizeof(MIDIHDR));
//	printf("Sending %d bytes (buffer %d)\n",eventcnt[buffer],buffer);
	_MPU_BuffersWaiting++;
}

void MPU_BeginPlayback( void )
{
	_MPU_LastEvent = _MIDI_GlobalPositionInTicks;
	if (hmido != (HMIDISTRM)-1) midiStreamRestart(hmido);
}

void MPU_Pause(void)
{
	if (hmido != (HMIDISTRM)-1) midiStreamPause(hmido);
}

void MPU_Unpause(void)
{
	if (hmido != (HMIDISTRM)-1) midiStreamRestart(hmido);
}


void CALLBACK MPU_MIDICallback(HMIDIOUT handle, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	int i;
	switch (uMsg) {
		case MOM_DONE:
			midiOutUnprepareHeader((HMIDIOUT)handle, (MIDIHDR*)dwParam1, sizeof(MIDIHDR));
			for (i=0;i<NUMBUFFERS;i++) {
				if (dwParam1 == (long)&bufferheaders[i]) {
					eventcnt[i] = 0;	// marks the buffer as free
//					printf("Finished buffer %d\n",i);
					_MPU_BuffersWaiting--;
					break;
				}
			}
			return;
			
		default: return;
	}
}


/*---------------------------------------------------------------------
   Function: MPU_SendMidi

   Queues a MIDI message to the music device.
---------------------------------------------------------------------*/

int MPU_GetNextBuffer(void)
{
	int i;
	for (i=0; i<NUMBUFFERS; i++) {
		if (eventcnt[i] == 0) return i;
	}
	return -1;
}

void MPU_SendMidi( char *data, int count )
{
	char *p;
	int padded, nextbuffer;
	static int masks[3] = { 0x000000ffl, 0x0000ffffl, 0x00ffffffl };

	if (count <= 0) return;
	if (count <= 3) {
		if (eventcnt[_MPU_CurrentBuffer] + 12 > BUFFERLEN) {
			// buffer over-full
			nextbuffer = MPU_GetNextBuffer();
			if (nextbuffer < 0) {
//				printf("All buffers full!\n");
				return;
			}
			MPU_FinishBuffer(_MPU_CurrentBuffer);
			_MPU_CurrentBuffer = nextbuffer;
		}

		p = eventbuf[_MPU_CurrentBuffer] + eventcnt[_MPU_CurrentBuffer];
		((long*)p)[0] = _MIDI_GlobalPositionInTicks - _MPU_LastEvent;
		((long*)p)[1] = 0;
		((long*)p)[2] = (MEVT_SHORTMSG << 24) | ((*((long*)data)) & masks[count-1]);
		eventcnt[_MPU_CurrentBuffer] += 12;
	} else {
		padded = PAD(count);
		if (eventcnt[_MPU_CurrentBuffer] + 12 + padded > BUFFERLEN) {
			// buffer over-full
			nextbuffer = MPU_GetNextBuffer();
			if (nextbuffer < 0) {
//				printf("All buffers full!\n");
				return;
			}
			MPU_FinishBuffer(_MPU_CurrentBuffer);
			_MPU_CurrentBuffer = nextbuffer;
		}

		p = eventbuf[_MPU_CurrentBuffer] + eventcnt[_MPU_CurrentBuffer];
		((long*)p)[0] = _MIDI_GlobalPositionInTicks - _MPU_LastEvent;
		((long*)p)[1] = 0;
		((long*)p)[2] = (MEVT_LONGMSG<<24) | (count & 0xffffffl);
		p+=12; eventcnt[_MPU_CurrentBuffer] += 12;
		for (; count>0; count--, padded--, eventcnt[_MPU_CurrentBuffer]++)
			*(p++) = *(data++);
		for (; padded>0; padded--, eventcnt[_MPU_CurrentBuffer]++)
			*(p++) = 0;
	}
	_MPU_LastEvent = _MIDI_GlobalPositionInTicks;
}


/*---------------------------------------------------------------------
   Function: MPU_SendMidiImmediate

   Sends a MIDI message immediately to the the music device.
---------------------------------------------------------------------*/
void MPU_SendMidiImmediate( char *data, int count )
{
	MIDIHDR mhdr;
	static int masks[3] = { 0x00ffffffl, 0x0000ffffl, 0x000000ffl };

	if (!count) return;
	if (count<=3) midiOutShortMsg((HMIDIOUT)hmido, (*((long*)data)) & masks[count-1]);
	else {
		ZeroMemory(&mhdr, sizeof(mhdr));
		mhdr.lpData = data;
		mhdr.dwBufferLength = count;
		midiOutPrepareHeader((HMIDIOUT)hmido, &mhdr, sizeof(MIDIHDR));
		midiOutLongMsg((HMIDIOUT)hmido, &mhdr, sizeof(MIDIHDR));
		while (!(mhdr.dwFlags & MHDR_DONE)) ;
		midiOutUnprepareHeader((HMIDIOUT)hmido, &mhdr, sizeof(MIDIHDR));
	}
}


/*---------------------------------------------------------------------
   Function: MPU_Reset

   Resets the MPU401 card.
---------------------------------------------------------------------*/

int MPU_Reset
   (
   void
   )

   {
	midiStreamStop(hmido);
	midiStreamClose(hmido);

	return( MPU_Ok );
   }


/*---------------------------------------------------------------------
   Function: MPU_Init

   Detects and initializes the MPU401 card.
---------------------------------------------------------------------*/

int MPU_Init
   (
   int addr
   )

   {
	int i;

	for (i=0;i<NUMBUFFERS;i++) eventcnt[i]=0;
	
	mididevice = addr;

	if (midiOutGetDevCaps(mididevice, &midicaps, sizeof(MIDIOUTCAPS)) != MMSYSERR_NOERROR) return MPU_Error;

	if (midiStreamOpen(&hmido,&mididevice,1,(DWORD)MPU_MIDICallback,0L,CALLBACK_FUNCTION) != MMSYSERR_NOERROR) return(MPU_Error);

	return( MPU_Ok );
   }


/*---------------------------------------------------------------------
   Function: MPU_NoteOff

   Sends a full MIDI note off event out to the music device.
---------------------------------------------------------------------*/

void MPU_NoteOff
   (
   int channel,
   int key,
   int velocity
   )

   {
   char msg[3];
   msg[0] = ( MIDI_NOTE_OFF | channel );
   msg[1] = ( key );
   msg[2] = ( velocity );
   MPU_SendMidi( msg, 3 );
   }


/*---------------------------------------------------------------------
   Function: MPU_NoteOn

   Sends a full MIDI note on event out to the music device.
---------------------------------------------------------------------*/

void MPU_NoteOn
   (
   int channel,
   int key,
   int velocity
   )

   {
   char msg[3];
   msg[0] = ( MIDI_NOTE_ON | channel );
   msg[1] = ( key );
   msg[2] = ( velocity );
   MPU_SendMidi( msg, 3 );
   }


/*---------------------------------------------------------------------
   Function: MPU_PolyAftertouch

   Sends a full MIDI polyphonic aftertouch event out to the music device.
---------------------------------------------------------------------*/

void MPU_PolyAftertouch
   (
   int channel,
   int key,
   int pressure
   )

   {
   char msg[3];
   msg[0] = ( MIDI_POLY_AFTER_TCH | channel );
   msg[1] = ( key );
   msg[2] = ( pressure );
   MPU_SendMidi( msg, 3 );
   }


/*---------------------------------------------------------------------
   Function: MPU_ControlChange

   Sends a full MIDI control change event out to the music device.
---------------------------------------------------------------------*/

void MPU_ControlChange
   (
   int channel,
   int number,
   int value
   )

   {
   char msg[3];
   msg[0] = ( MIDI_CONTROL_CHANGE | channel );
   msg[1] = ( number );
   msg[2] = ( value );
   MPU_SendMidi( msg, 3 );
   }


/*---------------------------------------------------------------------
   Function: MPU_ProgramChange

   Sends a full MIDI program change event out to the music device.
---------------------------------------------------------------------*/

void MPU_ProgramChange
   (
   int channel,
   int program
   )

   {
   char msg[2];
   msg[0] = ( MIDI_PROGRAM_CHANGE | channel );
   msg[1] = ( program );
   MPU_SendMidi( msg, 2 );
   }


/*---------------------------------------------------------------------
   Function: MPU_ChannelAftertouch

   Sends a full MIDI channel aftertouch event out to the music device.
---------------------------------------------------------------------*/

void MPU_ChannelAftertouch
   (
   int channel,
   int pressure
   )

   {
   char msg[2];
   msg[0] = ( MIDI_AFTER_TOUCH | channel );
   msg[1] = ( pressure );
   MPU_SendMidi( msg, 2 );
   }


/*---------------------------------------------------------------------
   Function: MPU_PitchBend

   Sends a full MIDI pitch bend event out to the music device.
---------------------------------------------------------------------*/

void MPU_PitchBend
   (
   int channel,
   int lsb,
   int msb
   )

   {
   char msg[3];
   msg[0] = ( MIDI_PITCH_BEND | channel );
   msg[1] = ( lsb );
   msg[2] = ( msb );
   MPU_SendMidi( msg, 3 );
   }



void MPU_SetTempo( int tempo )
{
	MIDIPROPTEMPO prop;
	prop.cbStruct = sizeof(MIDIPROPTEMPO);
        prop.dwTempo = 60000000l/tempo;
        midiStreamProperty(hmido, (LPBYTE)&prop, MIDIPROP_SET|MIDIPROP_TEMPO);
}

void MPU_SetDivision( int division )
{
	MIDIPROPTIMEDIV prop;
	prop.cbStruct = sizeof(MIDIPROPTIMEDIV);
        prop.dwTimeDiv = division;
        midiStreamProperty(hmido, (LPBYTE)&prop, MIDIPROP_SET|MIDIPROP_TIMEDIV);
}

void MPU_SetVolume(int volume)
{
	/*
	HMIXER hmixer;
	long mixerid;
	MIXERCONTROLDETAILS mxcd;
	MIXERCONTROLDETAILS_UNSIGNED mxcdu;
	MMRESULT mme;
	
	if (mididevice < 0) return;

	mme = mixerOpen(&hmixer, mididevice, 0,0, MIXER_OBJECTF_MIDIOUT);
	if (mme) {
		puts("Failed opening mixer");
		return;
	}

	mixerGetID(hmixer, &mixerid, MIXER_OBJECTF_HMIXER);
	printf("mixerid=%d\n",mixerid);

	ZeroMemory(&mxcd,sizeof(mxcd));
	mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mxcd.dwControlID = MIXERCONTROL_CONTROLTYPE_VOLUME;
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mxcd.paDetails = (LPVOID)&mxcdu;
	mxcdu.dwValue = (volume << 8) & 0xffff;

	printf("set %d\n",mixerSetControlDetails((HMIXEROBJ)mididevice, &mxcd,
		MIXER_OBJECTF_MIDIOUT|MIXER_SETCONTROLDETAILSF_VALUE));

	mixerClose(hmixer);
	*/
}

int MPU_GetVolume(void)
{
	if (mididevice < 0) return 0;

	return 0;
}

