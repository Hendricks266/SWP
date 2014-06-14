
// Added bypass dialog for forcesetup = 0
// Added -z for build play
// Added extra gamers, maps, saved to menu
// Added video/sound tabs start menu
// Added hack for 1st time startup
// Added LastIpUsed
// Added .wav files
// Added fix for saved games
// Added pathsearchmode for getfiles
// Added video 16 bit + if win7
// Added NoHrp
// Added gmFiles/mpFiles/muFiles
// Added 0's to variables
// Removed Mapnum to gamep.h
// Added Multi monster default = normal
// Changed iBright int etc to short
// Added start auto saved game
// Added -by detection to bypass menu / press(x)
// Added MenuTrack
//

#ifndef RENDERTYPEWIN
#error Only for Windows
#endif

#include "types.h"
#include "build.h"
#include "winlayer.h"
#include "compat.h"
#include "cache1d.h"

#include "startdlg.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <winbase.h>
#define _WIN32_IE 0x0400
#include <commctrl.h>
#include <stdio.h>

#include "startwin.game.h"

#include "swp.h"

#define TAB_MEDA 0
#define TAB_GAME 1
#define TAB_MULT 2
#define TAB_HELP 3
#define TAB_MESG 4

int iPage = 0;
BOOL bForce = FALSE;

static struct
{
	int fullscreen;
	int widescreen;
	int brightness;
	int stereo;
	int kidmode;
	int shadows;
	int autoaiming;
	int invertmouse;
	int levelstats;
	int messages;
	int xdim, ydim, bpp;
	int forcesetup;
	int usemouse, usejoy;
	char selectedgrp[BMAX_PATH+1];
    int ambience;
    int wangtalk;
    int nohrp;
} settings;

static HWND startupdlg = NULL;
static HWND pages[6] = { NULL, NULL, NULL, NULL, NULL, NULL };
static int done = -1, mode = TAB_MEDA;

extern int32 ScreenMode, ScreenWidth, ScreenHeight, ScreenBPP, ForceSetup, UseMouse, UseJoystick;

//-------------------------- startup menu settings -------------------------

static CACHE1D_FIND_REC *finddirs, *findfiles;

char  sFolder1[80];
char  sFolder2[80];

char  RootFolder[128];
char  MapsFolder[80];
char  MusicFolder[80];
char  GameFolder[80];

char  LastMapsFolder[80];
char  LastMusicFolder[80];
char  LastGameFolder[80];

short MenuTrack = 0;

char  MenuMap[80];
char  MenuMusic[80];
char  MenuGame[80];
char  GameMap[40];
char  LastMap[40];
char  LastGame[40];

short UserSavenum = -1;
short iLevelNum = -1;
short iSkill = 3;

short mapvis = 0;
short iType = 0;
short iMons = 2;
short iFrag = 0;
short iTime = 0;
short iColr = 7;

char  IPAddress[32];
short SelectMulti = -1;
short HostMulti = -1;

char  SaveLabel[80];
char  UserSaves[11][80];
int   GameSaved[11];
short NumSaves;
short LCD;

extern char SaveGameDescr[11][80];

extern int32 MusicDevice;
extern int32 FXDevice;
extern int cddevice;

extern short iBright;
extern short iStereo;
extern short iAmbient;
extern short iTalk;

extern BOOL cdplay;
extern BOOL menudone;
extern BOOL musicon;
extern BOOL fxon;

extern int32 NumVoices;   // 16
extern int32 NumBits;     // 8
extern int32 MixRate;     // 22025

extern char PlayerNameArg[40];
extern char IPAddressArg[8][40];
extern int LastIpUsed;
extern int LcdMon;
extern int32 NoHrp;
extern int is_vista;

extern char tempbuf[];
extern char svgame[40];

long TexCache=0, Tiles=1, Model=1, TexComp=0;

int ivoices[8] = {1,2,4,8,12,16,24,32};
int isample[7] = {8000, 11025, 16000, 22050, 32000, 44100, 48000};

char *Voices[8] = {"1","2","4","8","12","16","24","32"};
char *Sample[7] = {"8000", "11025", "16000", "22050", "32000", "44100", "48000"};

char *defgames[28] = {"$Bullet","$Dozer","$Shrine","$Woods","$Whirl","$Tank","$Boat","$Garden",
                      "$Outpost","$Hidtemp","$Plax1","$Bath","$Airport","$Refiner","$Newmine",
                      "$Subbase","$Rock","$Yamato","$Seabase","$Volcano","$Shore","$Auto","Tank",
                      "$Dmwoods","$Dmshrin","$Rush","Shotgun","$Dmdrop"};

char *MultiColor[8] = { "Brown", "Gray", "Purple", "Red", "Yellow", "Olive", "Green", "Blue" };
char *MultiGType[3] = { "Wangbang Spawn", "Wangbang Nospawn", "CoOperative" };
char *MultiMonst[5] = { "None", "Easy", "Norm", "Hard", "Insane" };
char *MultiTimeL[9] = { "Infinite", "3 minutes", "5 minutes", "10 minutes", "15 minutes", "20 minutes", "30 minutes", "45 minutes", "60 minutes" };
char *MultiFrags[9] = { "Infinite", "10 kills", "20 kills", "30 kills", "40 kills", "50 kills", "60 kills", "70 kills", "80 kills" };

short iVoc, iRat, iBit;

//-----------------------------------------------------------------------------

int getdirs(char *path)
{
    pathsearchmode = 1;
	klistfree(finddirs);
	klistfree(findfiles);
	finddirs = NULL;
	finddirs = klistpath(path,"*",CACHE1D_FIND_DIR);
	pathsearchmode = 0;
	return(0);
}

int getfiles(char *path, char kind[], short pmode)
{
    pathsearchmode = pmode;
	klistfree(finddirs);
	klistfree(findfiles);
	findfiles = NULL;
    findfiles = klistpath(path,kind,CACHE1D_FIND_FILE);
    pathsearchmode = 0;
	return(0);
}

void ResetVideo(void)
{
    HWND hwnd;
    int i, j, mod;
    char vbuf[128+BMAX_PATH];

	hwnd = GetDlgItem(pages[TAB_MEDA], IDVIDMODE);
	mod  = checkvideomode(&settings.xdim, &settings.ydim, settings.bpp, settings.fullscreen, 1);

	if (mod < 0)
	{
		int cd[] = { 32, 24, 16, 15, 8, 0 };

		for (i=0; cd[i]; )
		{
		    if (cd[i] >= settings.bpp)
		        i++;
		    else
		        break;
		}
		for ( ; cd[i]; i++)
		{
			mod = checkvideomode(&settings.xdim, &settings.ydim, cd[i], settings.fullscreen, 1);
			if (mod < 0)
			    continue;
			settings.bpp = cd[i];
			break;
		}
	}

	ComboBox_ResetContent(hwnd);
	for (i=0; i<validmodecnt; i++)
	{
		if (validmode[i].fs != settings.fullscreen) continue;

        //if (is_vista > 1 && validmode[i].bpp < 16) continue;

		// all modes get added to the 3D mode list
		Bsprintf(vbuf, "%ld x %ld %dbpp", validmode[i].xdim, validmode[i].ydim, validmode[i].bpp);
		j = ComboBox_AddString(hwnd, vbuf);

		ComboBox_SetItemData(hwnd, j, i);
		if (i == mod)
		    ComboBox_SetCurSel(hwnd, j);
	}
}

void PopulateMusic(void)
{
    HWND hwnd;
	UINT nummididevs;
	MIDIOUTCAPS mididev;
	UINT i,j;

	nummididevs = midiOutGetNumDevs();
    hwnd = GetDlgItem(pages[TAB_MEDA], IDMIDIDEV);
	ComboBox_ResetContent(hwnd);

	for (i=0; i<nummididevs; i++)
	{
		midiOutGetDevCaps(i, &mididev, sizeof(MIDIOUTCAPS));
		ComboBox_AddString(hwnd, mididev.szPname);
	}

	if (MusicDevice < 0)
	{
		Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDMUSICENABLE), FALSE);
		EnableWindow(GetDlgItem(pages[TAB_MEDA], IDMIDIDEV), FALSE);
		ComboBox_SetCurSel(GetDlgItem(pages[TAB_MEDA], IDMIDIDEV),0);
		musicon = FALSE;
	}
	else
	{
		if (MusicDevice >= nummididevs)
		    MusicDevice = nummididevs-1;
		Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDMUSICENABLE), TRUE);
		EnableWindow(GetDlgItem(pages[TAB_MEDA], IDMIDIDEV), TRUE);
		ComboBox_SetCurSel(GetDlgItem(pages[TAB_MEDA], IDMIDIDEV), MusicDevice);
		musicon = TRUE;

        hwnd = GetDlgItem(pages[TAB_MEDA], IDMIDIDEV);
        ComboBox_SetCurSel(hwnd, MusicDevice);
	}

	if (FXDevice < 0)
	{
		Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDSOUNDENABLE), FALSE);
		fxon = FALSE;
	}
	else
	{
		Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDSOUNDENABLE), TRUE);
		fxon = TRUE;
	}

    {
        char kbuf[40];
        char x = 65;

	    hwnd = GetDlgItem(pages[TAB_MEDA], IDCCDADEV);
	    ComboBox_ResetContent(hwnd);
		for (i=0; i<10; i++)
		{
		    Bsprintf(kbuf, "%c:\\", x + i);
		    j = GetDriveType(kbuf);
		    if (j > 3)
		        ComboBox_AddString(hwnd, kbuf);
		}
		if (cddevice < 0)
            cddevice = 0;
		ComboBox_SetCurSel(hwnd, cddevice);
		Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDPLAYCD), (cdplay ? BST_CHECKED : BST_UNCHECKED));
		}

        {
        hwnd = GetDlgItem(pages[TAB_MEDA], IDVOICES);
        for (i=0; i<8; i++)
        {
            ComboBox_AddString(hwnd, Voices[i]);
        }
        j = 5;
        for (i=0; i<8; i++)
        {
            if (ivoices[i] == NumVoices)
            {
               j = i;
               break;
            }
        }
        ComboBox_SetCurSel(hwnd, j);
        iVoc = j;

        hwnd = GetDlgItem(pages[TAB_MEDA], IDSAMPLE);
        for (i=0; i<7; i++)
        {
            ComboBox_AddItemData(hwnd, Sample[i]);
        }
        j = 3;
        for (i=0; i<7; i++)
        {
            if (isample[i] == MixRate)
            {
               j = i;
               break;
            }
        }
        ComboBox_SetCurSel(hwnd, j);
        iRat = j;
    }

	if (NumBits == 8)
	{
        Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDSOUND8BIT), TRUE);
        iBit = 8;
	}
	else
	{
	    Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDSOUND16BIT), TRUE);
	    iBit = 16;
	}
}

//-----------------------------------------------------------------------------

static void PopulateForm(int pgs)
{
	char xbuf[128+BMAX_PATH];
	HWND hwnd;
	int i, j, k;

    GetCurrentDirectory(128, RootFolder);       // _getcwd

	if (pgs & (1<<TAB_MEDA))
	{
        hwnd = GetDlgItem(pages[TAB_MEDA], IDBRIGHT);
        SendMessage(hwnd, TBM_SETRANGE, FALSE, MAKELONG(0, 7));
        SendMessage(hwnd, TBM_SETPOS, TRUE, settings.brightness);
        SendMessage(hwnd, TBM_SETTHUMBLENGTH, 110, 0);

		Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDFULLSC), (settings.fullscreen ? BST_CHECKED : BST_UNCHECKED));
		Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDWIDESC), (settings.widescreen ? BST_CHECKED : BST_UNCHECKED));

        ResetVideo();

	    if (LCD)
            Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDMONLCD), TRUE);
	    else
	        Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDMONCRT), TRUE);

        Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDTCACHE), (TexCache ? BST_CHECKED : BST_UNCHECKED));
        Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDHTILES), (Tiles ?    BST_CHECKED : BST_UNCHECKED));
        Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDHMODEL), (Model ?    BST_CHECKED : BST_UNCHECKED));
        Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDCOMPRN), (TexComp ? BST_CHECKED : BST_UNCHECKED));

        PopulateMusic();

        Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDSTEREO),(settings.stereo ?   BST_CHECKED : BST_UNCHECKED));
        Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDAMBENC),(settings.ambience ? BST_CHECKED : BST_UNCHECKED));
        Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDWANGTK),(settings.wangtalk ? BST_CHECKED : BST_UNCHECKED));
    }

	if (pgs & (1<<TAB_GAME))
	{
	    hwnd = GetDlgItem(pages[TAB_GAME], IDDIRGM);

        getdirs(RootFolder);
        i = 0;
        k = -1;
        while (finddirs)
        {
          Bsprintf(xbuf,"%s",finddirs->name);
          ComboBox_AddString(hwnd, xbuf);
          j = Bstrncasecmp(xbuf, "games", 5);
          if (j == 0 && k == -1)
          {
              k = i;
              strcpy(GameFolder, xbuf);
          }
          i++;
          finddirs = finddirs->next;
        }
        ComboBox_SetCurSel(hwnd, k);
        ComboBox_GetLBText(hwnd, k, GameFolder);

        if (k == 0)
            GameFolder[0] = 0;
        GetGameFiles(GameFolder);
        MenuGame[0] = 0;
        SetDefGames();
        GameMap[0] = 0;

	    hwnd = GetDlgItem(pages[TAB_GAME], IDDIRSP);
        getdirs(RootFolder);
        i = 0;
        k = -1;
        while (finddirs)
        {
          Bsprintf(xbuf,"%s",finddirs->name);
          ComboBox_AddString(hwnd, xbuf);
          j = Bstrncasecmp(xbuf, "maps", 4);
          if (j == 0 && k == -1)
          {
              k = i;
              strcpy(sFolder1, xbuf);
          }
          i++;
          finddirs = finddirs->next;
        }
        ComboBox_SetCurSel(hwnd, k);
        ComboBox_GetLBText(hwnd, k, sFolder1);
        GetMapFiles(sFolder1, 0);
        MenuMap[0] = 0;

        hwnd = GetDlgItem(pages[TAB_GAME], IDSKILL);
        ComboBox_AddString(hwnd, "   Tiny Grasshopper");
        ComboBox_AddString(hwnd, "   I have no Fear");
        ComboBox_AddString(hwnd, "   Who Wants Wang");
        ComboBox_AddString(hwnd, "   No Pain no Gain");
        iSkill = 3;
        ComboBox_SetCurSel(hwnd, 2);

	    hwnd = GetDlgItem(pages[TAB_GAME], IDMFOLD);
        getdirs(RootFolder);
        i = 0;
        k = -1;
        while (finddirs)
        {
          Bsprintf(xbuf,"%s",finddirs->name);
          ComboBox_AddString(hwnd, xbuf);
          j = Bstrncasecmp(xbuf, "music", 5);
          if (j == 0 && k == -1)
          {
              k = i;
              strcpy(MusicFolder, xbuf);
          }
          i++;
          finddirs = finddirs->next;
        }
        ComboBox_SetCurSel(hwnd, k);
        ComboBox_GetLBText(hwnd, k, MusicFolder);

        if (k == 0)
            MusicFolder[0] = 0;

        LoadMusicFiles(MusicFolder);
        MenuMusic[0] = 0;
        MenuTrack = 0;

		Bsprintf(SaveLabel, "Default Game");
		ListSavedGames();

        Button_SetCheck(GetDlgItem(pages[TAB_GAME], IDMSHOW), (settings.forcesetup ? BST_CHECKED : BST_UNCHECKED));
        Button_SetCheck(GetDlgItem(pages[TAB_GAME], IDHIRES), (settings.nohrp ? BST_CHECKED : BST_UNCHECKED));
	}

  	if (pgs & (1<<TAB_MULT))
	{
		// player name
		hwnd = GetDlgItem(pages[TAB_MULT], IDUNAME);
		Edit_SetText(hwnd, PlayerNameArg);

        // player color
		hwnd = GetDlgItem(pages[TAB_MULT], IDUCOLR);
        for (i=0; i<8; i++)
        {
            ComboBox_AddString(hwnd, MultiColor[i]);
        }
        ComboBox_SetCurSel(hwnd, iColr);

        // opponent ip
		hwnd = GetDlgItem(pages[TAB_MULT], IDUIPAD);
		Edit_SetText(hwnd, IPAddressArg[LastIpUsed]);

        // ip listing
		hwnd = GetDlgItem(pages[TAB_MULT], IDUIPPS);
        for (i=0; i<8; i++)
        {
            if (strlen(IPAddressArg[i]) < 8)
                break;
            ComboBox_AddString(hwnd, IPAddressArg[i]);
        }
        ComboBox_SetCurSel(hwnd, LastIpUsed);
        if (IPAddressArg[0] == 0)
            IPAddress[0] = 0;
        else
            strcpy(IPAddress, IPAddressArg[LastIpUsed]);

        // game type
		hwnd = GetDlgItem(pages[TAB_MULT], IDUTYPE);
        for (i=0; i<3; i++)
        {
            ComboBox_AddString(hwnd, MultiGType[i]);
        }
        ComboBox_SetCurSel(hwnd, iType);

        // monsters
        iMons = 2;
		hwnd = GetDlgItem(pages[TAB_MULT], IDUMONS);
        for (i=0; i<5; i++)
        {
            ComboBox_AddString(hwnd, MultiMonst[i]);
        }
        ComboBox_SetCurSel(hwnd, iMons);

        // timelimit
		hwnd = GetDlgItem(pages[TAB_MULT], IDUTIME);
        for (i=0; i<9; i++)
        {
            ComboBox_AddString(hwnd, MultiTimeL[i]);
        }
        ComboBox_SetCurSel(hwnd, iTime);

		// fraglimit
		hwnd = GetDlgItem(pages[TAB_MULT], IDUFRAG);
        for (i=0; i<9; i++)
        {
            ComboBox_AddString(hwnd, MultiFrags[i]);
        }
        ComboBox_SetCurSel(hwnd, iFrag);

        // host / join
	    HostMulti = -1;

        // select
	    Button_SetCheck(GetDlgItem(pages[TAB_MULT], IDWBSEL), FALSE);
	    SelectMulti = -1;

	    hwnd = GetDlgItem(pages[TAB_MULT], IDDIRWB);
        getdirs(RootFolder);
        i = 0;
        k = -1;
        while (finddirs)
        {
          Bsprintf(xbuf,"%s",finddirs->name);
          ComboBox_AddString(hwnd, xbuf);
          j = Bstrncasecmp(xbuf, "mapswb", 6);
          if (j == 0 && k == -1)
          {
              k = i;
              strcpy(sFolder2, xbuf);
          }
          i++;
          finddirs = finddirs->next;
        }
        ComboBox_SetCurSel(hwnd, k);
        ComboBox_GetLBText(hwnd, k, sFolder2);
        GetMapFiles(sFolder2, 1);
        MenuMap[0] = 0;
	}

  	if (pgs & (1<<TAB_HELP))
	{
	    Showhelp(0);
    }

    CheckShareware();
}

static void SetSaves(int pgs)
{
	HWND hwnd;

	if (pgs == 0)
	{
        Bsprintf(SaveLabel, "%s", MenuGame);
        SaveLabel[0] = Btoupper(SaveLabel[0]);
        checkforsaves(MenuGame);
        GetSavedGames();
    }
	if (pgs == 1)
	{
        if (strstr(MenuMap, " **"))
            MenuMap[strlen(MenuMap)-3] = 0;
	    Bsprintf(SaveLabel, "%s", MenuMap);
        SaveLabel[0] = Btoupper(SaveLabel[0]);
        checkforsaves(MenuMap);
        GetSavedGames();
    }
    ListSavedGames();
}

static void ResetForm(int pgs)
{
	HWND hwnd;

    if (pgs == 97)
    {
	    hwnd = GetDlgItem(pages[TAB_GAME], IDMUSIC);
	    ListBox_SetCurSel(hwnd, -1);
        MenuMusic[0] = 0;
        MenuTrack = 0;
        EnableWindow(GetDlgItem(pages[TAB_GAME], IDMUSIC), 0);
    }

    if (pgs == 98 || pgs == 99)
    {
        hwnd = GetDlgItem(pages[TAB_GAME], IDGMAPS);
        if (pgs == 99)
	        ListBox_ResetContent(hwnd);
	    ListBox_SetCurSel(hwnd, -1);

	    MenuMap[0] = 0;
	    GameMap[0] = 0;
	    iLevelNum = -1;
	    return;
    }

	if (pgs & (1<<TAB_MEDA))
	{
	    short i, j;

        TexCache = glusetexcache;
        Tiles = usehightile;
        Model = usemodels;
        TexComp = glusetexcompr;

        Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDTCACHE), (TexCache ? BST_CHECKED : BST_UNCHECKED));
        Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDHTILES), (Tiles ?    BST_CHECKED : BST_UNCHECKED));
        Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDHMODEL), (Model ?    BST_CHECKED : BST_UNCHECKED));
        Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDCOMPRN), (TexComp ?  BST_CHECKED : BST_UNCHECKED));

        LCD = LcdMon;
	    if (LCD)
	    {
            Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDMONLCD), TRUE);
            Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDMONCRT), FALSE);
        }
	    else
	    {
	        Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDMONCRT), TRUE);
	        Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDMONLCD), FALSE);
	    }

	    settings.brightness = iBright;
        hwnd = GetDlgItem(pages[TAB_MEDA], IDBRIGHT);
        SendMessage(hwnd, TBM_SETRANGE, FALSE, MAKELONG(0, 7));
        SendMessage(hwnd, TBM_SETPOS, TRUE, settings.brightness);
        SendMessage(hwnd, TBM_SETTHUMBLENGTH, 110, 0);

	    settings.fullscreen = ScreenMode;
	    settings.widescreen = glwidescreen;

		Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDFULLSC), (settings.fullscreen ? BST_CHECKED : BST_UNCHECKED));
		Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDWIDESC), (settings.widescreen ? BST_CHECKED : BST_UNCHECKED));

        ResetVideo();

	    settings.stereo = iStereo;
        settings.ambience = iAmbient;
        settings.wangtalk = iTalk;

        Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDSTEREO),(settings.stereo ? BST_CHECKED : BST_UNCHECKED));
        Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDAMBENC),(settings.ambience ? BST_CHECKED : BST_UNCHECKED));
        Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDWANGTK),(settings.wangtalk ? BST_CHECKED : BST_UNCHECKED));
        Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDPLAYCD), FALSE);

        hwnd = GetDlgItem(pages[TAB_MEDA], IDVOICES);
        ComboBox_SetCurSel(hwnd, iVoc);

        hwnd = GetDlgItem(pages[TAB_MEDA], IDSAMPLE);
        ComboBox_SetCurSel(hwnd, iRat);

		if (iBit == 8)
		{
            Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDSOUND8BIT), TRUE);
            Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDSOUND16BIT), FALSE);
		}
		else
		{
		    Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDSOUND8BIT), FALSE);
		    Button_SetCheck(GetDlgItem(pages[TAB_MEDA], IDSOUND16BIT), TRUE);
		}
    }

	if (pgs & (1<<TAB_GAME))
	{
        resetgame();
		resetmaps();
		resetsave(1);

		settings.forcesetup = ForceSetup;
		settings.nohrp = NoHrp;

        Button_SetCheck(GetDlgItem(pages[TAB_GAME], IDMSHOW), (settings.forcesetup ? BST_CHECKED : BST_UNCHECKED));
        Button_SetCheck(GetDlgItem(pages[TAB_GAME], IDHIRES), (settings.nohrp ? BST_CHECKED : BST_UNCHECKED));

        hwnd = GetDlgItem(pages[TAB_GAME], IDMUSIC);
        ListBox_SetCurSel(hwnd, -1);
	}

	if (pgs & (1<<TAB_MULT))
	{
        Button_SetCheck(GetDlgItem(pages[TAB_MULT], IDWBSEL), FALSE);
        Button_SetCheck(GetDlgItem(pages[TAB_MULT], IDUHOST), FALSE);
        Button_SetCheck(GetDlgItem(pages[TAB_MULT], IDUJOIN), FALSE);
	    SelectMulti = -1;
	    HostMulti = -1;
	    resetwbmaps();

        iMons = 2;
		hwnd = GetDlgItem(pages[TAB_MULT], IDUMONS);
        ComboBox_SetCurSel(hwnd, iMons);
	}
}

static INT_PTR CALLBACK MediaPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    short i;
    HWND Hh;

	switch (uMsg)
	{
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDFULLSC:
				    settings.fullscreen = IsDlgButtonChecked(hwndDlg, IDFULLSC) == BST_CHECKED;
				    bForce = TRUE;
				    ResetVideo();
					return TRUE;
				case IDVIDMODE:
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						i = ComboBox_GetCurSel((HWND)lParam);
						if (i != CB_ERR)
						    i = ComboBox_GetItemData((HWND)lParam, i);
						if (i != CB_ERR)
						{
							settings.xdim = validmode[i].xdim;
							settings.ydim = validmode[i].ydim;
							settings.bpp  = validmode[i].bpp;
						}
					}
					return TRUE;
				case IDBRIGHT:
                     return TRUE;
				case IDWIDESC:
 				     settings.widescreen = IsDlgButtonChecked(hwndDlg, IDWIDESC) == BST_CHECKED;
					 return TRUE;
				case IDTCACHE:
					 TexCache = IsDlgButtonChecked(hwndDlg, IDTCACHE) == BST_CHECKED;
					 return TRUE;
				case IDHTILES:
					 Tiles = IsDlgButtonChecked(hwndDlg, IDHTILES) == BST_CHECKED;
					 return TRUE;
				case IDHMODEL:
					 Model = IsDlgButtonChecked(hwndDlg, IDHMODEL) == BST_CHECKED;
					 return TRUE;
				case IDCOMPRN:
					 TexComp = IsDlgButtonChecked(hwndDlg, IDCOMPRN) == BST_CHECKED;
					 return TRUE;
				case IDMONLCD:
					 LCD = 1;
					 bForce = TRUE;
					 return TRUE;
				case IDMONCRT:
					 LCD = 0;
					 bForce = TRUE;
					 return TRUE;
				case IDSOUNDENABLE:
					 fxon = IsDlgButtonChecked(hwndDlg, IDSOUNDENABLE) == BST_CHECKED;
					 return TRUE;
				case IDMUSICENABLE:
					 musicon = IsDlgButtonChecked(hwndDlg, IDMUSICENABLE) == BST_CHECKED;
					 if (musicon && MusicDevice == -1)
					 {
					     MusicDevice = 0;
					     EnableWindow(GetDlgItem(pages[TAB_MEDA], IDMIDIDEV), TRUE);
					 }
					 return TRUE;
				case IDMIDIDEV:
					 i = ComboBox_GetCurSel((HWND)lParam);
					 if (i != CB_ERR)
 					     MusicDevice = i;
					 return TRUE;
				case IDCCDADEV:
					 i = ComboBox_GetCurSel((HWND)lParam);
					 if (i != CB_ERR)
					     cddevice = i;
					 return TRUE;
				case IDPLAYCD:
					 cdplay = IsDlgButtonChecked(hwndDlg, IDPLAYCD) == BST_CHECKED;
                     LoadMusicFiles(MusicFolder);
					 return TRUE;
				case IDSTEREO:
					 settings.stereo = IsDlgButtonChecked(hwndDlg, IDSTEREO) == BST_CHECKED;
					 return TRUE;
				case IDVOICES:
					 i = ComboBox_GetCurSel((HWND)lParam);
					 if (i != CB_ERR)
 					     NumVoices = ivoices[i];
					 return TRUE;
				case IDSAMPLE:
					 i = ComboBox_GetCurSel((HWND)lParam);
					 if (i != CB_ERR)
 					     MixRate = isample[i];
					 return TRUE;
				case IDSOUND8BIT:
					 NumBits = 8;
					 return TRUE;
				case IDSOUND16BIT:
					 NumBits = 16;
					 return TRUE;
				case IDAMBENC:
					 settings.ambience = IsDlgButtonChecked(hwndDlg, IDAMBENC) == BST_CHECKED;
					 return TRUE;
				case IDWANGTK:
					 settings.wangtalk = IsDlgButtonChecked(hwndDlg, IDWANGTK) == BST_CHECKED;
					 return TRUE;
				default: break;
			}
			break;
		default: break;
	}
	return FALSE;
}

static INT_PTR CALLBACK GamePageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int i;

	switch (uMsg)
	{
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDDIRGM:
				{
					i = ComboBox_GetCurSel((HWND)lParam);
					if (i != CB_ERR)
					{
                       ComboBox_GetLBText((HWND)lParam, i, GameFolder);
                       if (i == 0)
                           GameFolder[0] = 0;
                       GetGameFiles(GameFolder);
                       resetgame();
                       resetsave(0); // 0 here is a guess
                       mapvis = 0;
                    }
					return TRUE;
				}
				case IDGDATA:
				{
  					ResetForm(99);
  					resetmaps();
  					resetwbmaps();
					i = ListBox_GetCurSel((HWND)lParam);
					if (i != CB_ERR)
					{
                       ListBox_GetText((HWND)lParam, i, MenuGame);
                       dolower(MenuGame);                           // 100303
                       SetSaves(0);
                       GetMapNames(MenuGame);
                       mapvis = 0;
                       EnableWindow(GetDlgItem(pages[TAB_GAME], IDMUSIC), 1);
                    }
					return TRUE;
				}
				case IDGMAPS:
				{
					resetmaps();
					resetwbmaps();
					i = ListBox_GetCurSel((HWND)lParam);
					if (i != CB_ERR)
					   {
					   ListBox_GetText((HWND)lParam, i, GameMap);
					   dolower(GameMap);                            // 100303
					   iLevelNum = i;  // incase default maps
					   resetsave(0);
					   EnableWindow(GetDlgItem(pages[TAB_GAME], IDMUSIC), 1);
					   }
					return TRUE;
				}
				case IDDIRSP:
				{
					i = ComboBox_GetCurSel((HWND)lParam);
					if (i != CB_ERR)
					{
                       ComboBox_GetLBText((HWND)lParam, i, sFolder1);
                       if (i == 0)
                           sFolder1[0] = 0;
                       GetMapFiles(sFolder1, 0);
                       resetwbmaps();
                       mapvis = 1;
                    }
					return TRUE;
				}
				case IDMDATA:
				{
				    resetwbmaps();
				    resetgame();
					i = ListBox_GetCurSel((HWND)lParam);
					if (i != CB_ERR)
					{
					    ListBox_GetText((HWND)lParam, i, MenuMap);
					    strcpy(MapsFolder, sFolder1);
					    SetSaves(1);
					    mapvis = 1;
					    EnableWindow(GetDlgItem(pages[TAB_GAME], IDMUSIC), 1);
					}
					return TRUE;
				}
				case IDSKILL:
				{
					i = ComboBox_GetCurSel((HWND)lParam);
					if (i != CB_ERR)
					    iSkill = i+1;
					return TRUE;
				}
				case IDMFOLD:
				{
					i = ComboBox_GetCurSel((HWND)lParam);
					if (i != CB_ERR)
					{
                       ComboBox_GetLBText((HWND)lParam, i, MusicFolder);
                       dolower(MusicFolder);
                       if (i == 0)
                           MusicFolder[0] = 0;
                       LoadMusicFiles(MusicFolder);
                    }
					return TRUE;
				}
				case IDMUSIC:
				{
					i = ListBox_GetCurSel((HWND)lParam);
					if (i != CB_ERR)
					{
					    if (cdplay)
					    {
					        MenuTrack = i+3;
					        MenuMusic[0] = 0;
					    }
					    else
					    {
					        ListBox_GetText((HWND)lParam, i, MenuMusic);
					        if (i == 0)
					            MenuMusic[0] = 0;
					        MenuTrack = 0;
					    }
					}
					return TRUE;
				}
				case IDSDATA:
				{
				    ResetForm(97);
					ResetForm(98);
					i = ListBox_GetCurSel((HWND)lParam);
					if (i != CB_ERR)
 			            UserSavenum = GameSaved[i];
					return TRUE;
				}
				case IDMSHOW:
				    settings.forcesetup = IsDlgButtonChecked(hwndDlg, IDMSHOW) == BST_CHECKED;
					bForce = TRUE;
					return TRUE;
				case IDHIRES:
					settings.nohrp = IsDlgButtonChecked(hwndDlg, IDHIRES) == BST_CHECKED;
					return TRUE;
				default: break;
			}
			break;
		default: break;
	}
	return FALSE;
}

static INT_PTR CALLBACK MultiPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    short i;
    HWND Hh;

	switch (uMsg)
	{
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDUNAME:
					Edit_GetText((HWND)lParam, PlayerNameArg, sizeof(PlayerNameArg));
                    //Edit_GetLine(hwnd, 0, PlayerNameArg, sizeof(PlayerNameArg));  // alternative
					return TRUE;
				case IDUIPAD:
					Edit_GetText((HWND)lParam, IPAddress, sizeof(IPAddress));
					return TRUE;
				case IDUIPPS:
				{
					int i;
					i = ComboBox_GetCurSel((HWND)lParam);
					if (i != CB_ERR)
					{
                        ComboBox_GetLBText((HWND)lParam, i, IPAddress);
 		                Hh = GetDlgItem(pages[TAB_MULT], IDUIPAD);
		                Edit_SetText(Hh, IPAddress);
		                LastIpUsed = i;
                    }
					return TRUE;
				}
				case IDUCOLR:
					 i = ComboBox_GetCurSel((HWND)lParam);
					 if (i != CB_ERR)
 					     iColr = i;
					 return TRUE;
				case IDUTYPE:
					 i = ComboBox_GetCurSel((HWND)lParam);
					 if (i != CB_ERR)
 					     iType = i;
					 return TRUE;
				case IDUMONS:
					 i = ComboBox_GetCurSel((HWND)lParam);
					 if (i != CB_ERR)
 					     iMons = i;
					 return TRUE;
				case IDUTIME:
					 i = ComboBox_GetCurSel((HWND)lParam);
					 if (i != CB_ERR)
 					     iTime = i;
					 return TRUE;
				case IDUFRAG:
					 i = ComboBox_GetCurSel((HWND)lParam);
					 if (i != CB_ERR)
 					     iFrag = i;
					 return TRUE;
				case IDUHOST:
					 HostMulti = 1;
					 return TRUE;
				case IDUJOIN:
					 HostMulti = 0;
					 return TRUE;
				case IDWBSEL:
					 SelectMulti = IsDlgButtonChecked(hwndDlg, IDWBSEL) == BST_CHECKED;
					 if (SelectMulti == 0)
					     SelectMulti = -1;
					 return TRUE;
				case IDDIRWB:
				{
					 i = ComboBox_GetCurSel((HWND)lParam);
					 if (i != CB_ERR)
					 {
                        ComboBox_GetLBText((HWND)lParam, i, sFolder2);
                        if (i == 0)
                            sFolder2[0] = 0;
                        GetMapFiles(sFolder2, 1);
                     }
					 return TRUE;
				}
				case IDMAPWB:
				{
				     resetmaps();
				     resetgame();
				     resetsave(0);
					 i = ListBox_GetCurSel((HWND)lParam);
					 if (i != CB_ERR)
					 {
					     ListBox_GetText((HWND)lParam, i, MenuMap);
					     strcpy(MapsFolder, sFolder2);
					 }
					 return TRUE;
				}
				default: break;
			}
			break;
		default: break;
	}
	return FALSE;
}

static INT_PTR CALLBACK AboutPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    short i;
    HWND Hh, Hw, Hz;

	switch (uMsg)
	{
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDAHELP:
					return TRUE;
				default: break;
			}
			break;
		default: break;
	}
	return FALSE;
}

static void SetPage(int n)
{
	HWND tab;
	int cur;

	tab = GetDlgItem(startupdlg, WIN_STARTWIN_TABCTL);
	cur = (int)SendMessage(tab, TCM_GETCURSEL,0,0);
	ShowWindow(pages[cur],SW_HIDE);
	SendMessage(tab, TCM_SETCURSEL, n, 0);
	ShowWindow(pages[n],SW_SHOW);
	mode = n;

	SetFocus(GetDlgItem(startupdlg, WIN_STARTWIN_TABCTL));
}

static void EnableConfig(int n)
{
	EnableWindow(GetDlgItem(startupdlg, WIN_STARTWIN_START), n);
	EnableWindow(GetDlgItem(pages[TAB_MEDA], IDFULLSC), n);
	EnableWindow(GetDlgItem(pages[TAB_MEDA], IDWIDESC), n);
	EnableWindow(GetDlgItem(pages[TAB_MEDA], IDVIDMODE), n);
	EnableWindow(GetDlgItem(pages[TAB_MEDA], IDVOICES), n);
	EnableWindow(GetDlgItem(pages[TAB_MEDA], IDSAMPLE), n);
	EnableWindow(GetDlgItem(pages[TAB_GAME], IDDIRGM), n);
	EnableWindow(GetDlgItem(pages[TAB_GAME], IDGDATA), n);
	EnableWindow(GetDlgItem(pages[TAB_GAME], IDDIRSP), n);
	EnableWindow(GetDlgItem(pages[TAB_GAME], IDMDATA), n);
	EnableWindow(GetDlgItem(pages[TAB_GAME], IDGMAPS), n);
	EnableWindow(GetDlgItem(pages[TAB_GAME], IDSKILL), n);
	EnableWindow(GetDlgItem(pages[TAB_GAME], IDSDATA), n);
	EnableWindow(GetDlgItem(pages[TAB_GAME], IDMFOLD), n);
	EnableWindow(GetDlgItem(pages[TAB_GAME], IDMUSIC), n);
	EnableWindow(GetDlgItem(pages[TAB_HELP], IDAHELP), n);
	EnableWindow(GetDlgItem(pages[TAB_MULT], IDMAPWB), n);
	EnableWindow(GetDlgItem(pages[TAB_MULT], IDDIRWB), n);
}

static INT_PTR CALLBACK startup_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HBITMAP hbmp = NULL;
	HDC hdc;

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			HWND hwnd;
			RECT r, rdlg, chrome, rtab, rcancel, rstart, rdes, rhlp;
			int xoffset = 0, yoffset = 0;

			// Fetch the positions (in screen coordinates) of all the windows we need to tweak
			ZeroMemory(&chrome, sizeof(chrome));
			AdjustWindowRect(&chrome, GetWindowLong(hwndDlg, GWL_STYLE), FALSE);
			GetWindowRect(hwndDlg, &rdlg);
			GetWindowRect(GetDlgItem(hwndDlg, WIN_STARTWIN_TABCTL), &rtab);
			GetWindowRect(GetDlgItem(hwndDlg, WIN_STARTWIN_CANCEL), &rcancel);
			GetWindowRect(GetDlgItem(hwndDlg, WIN_STARTWIN_START), &rstart);
			GetWindowRect(GetDlgItem(hwndDlg, WIN_STARTWIN_TBHELP), &rhlp);
			GetWindowRect(GetDlgItem(hwndDlg, WIN_STARTWIN_ABORT), &rdes);

			// Knock off the non-client area of the main dialogue to give just the client area
			rdlg.left -= chrome.left; rdlg.top -= chrome.top;
			rdlg.right -= chrome.right; rdlg.bottom -= chrome.bottom;

			// Translate them to client-relative coordinates wrt the main dialogue window
			rtab.right -= rtab.left - 1; rtab.bottom -= rtab.top - 1;
			rtab.left  -= rdlg.left; rtab.top -= rdlg.top;

			rcancel.right -= rcancel.left - 1; rcancel.bottom -= rcancel.top - 1;
			rcancel.left -= rdlg.left; rcancel.top -= rdlg.top;

			rstart.right -= rstart.left - 1; rstart.bottom -= rstart.top - 1;
			rstart.left -= rdlg.left; rstart.top -= rdlg.top;

			rdes.right -= rdes.left - 1; rdes.bottom -= rdes.top - 1;
			rdes.left -= rdlg.left; rdes.top -= rdlg.top;

			rhlp.right -= rhlp.left - 1; rhlp.bottom -= rhlp.top - 1;
			rhlp.left -= rdlg.left; rhlp.top -= rdlg.top;

			// And then convert the main dialogue coordinates to just width/length
			rdlg.right -= rdlg.left - 1; rdlg.bottom -= rdlg.top - 1;
			rdlg.left = 0; rdlg.top = 0;

			// Load the bitmap into the bitmap control and fetch its dimensions
			hbmp = LoadBitmap((HINSTANCE)win_gethinstance(), MAKEINTRESOURCE(RSRC_BMP));
			hwnd = GetDlgItem(hwndDlg,WIN_STARTWIN_BITMAP);
			SendMessage(hwnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmp);
			GetClientRect(hwnd, &r);
			xoffset = r.right;
			yoffset = r.bottom - rdlg.bottom;

			// Shift and resize the controls that require it
			rtab.left += xoffset; rtab.bottom += yoffset;
			rcancel.left += xoffset; rcancel.top += yoffset;
			rstart.left += xoffset; rstart.top += yoffset;
			rdes.left += xoffset; rdes.top += yoffset;
			rhlp.left += xoffset; rhlp.top += yoffset;
			rdlg.right += xoffset;
			rdlg.bottom += yoffset;

			// Move the controls to their new positions
			MoveWindow(GetDlgItem(hwndDlg, WIN_STARTWIN_TABCTL), rtab.left, rtab.top, rtab.right, rtab.bottom, FALSE);
			MoveWindow(GetDlgItem(hwndDlg, WIN_STARTWIN_CANCEL), rcancel.left, rcancel.top, rcancel.right, rcancel.bottom, FALSE);
			MoveWindow(GetDlgItem(hwndDlg, WIN_STARTWIN_START), rstart.left, rstart.top, rstart.right, rstart.bottom, FALSE);
			MoveWindow(GetDlgItem(hwndDlg, WIN_STARTWIN_ABORT), rdes.left, rdes.top, rdes.right, rdes.bottom, FALSE);
			MoveWindow(GetDlgItem(hwndDlg, WIN_STARTWIN_TBHELP), rhlp.left, rhlp.top, rhlp.right, rhlp.bottom, FALSE);

			// Move the main dialogue to the centre of the screen
			hdc = GetDC(NULL);
			rdlg.left = (GetDeviceCaps(hdc, HORZRES) - rdlg.right) / 2;
			rdlg.top = (GetDeviceCaps(hdc, VERTRES) - rdlg.bottom) / 2;
			ReleaseDC(NULL, hdc);
			MoveWindow(hwndDlg, rdlg.left + chrome.left, rdlg.top + chrome.left,
				       rdlg.right + (-chrome.left+chrome.right), rdlg.bottom + (-chrome.top+chrome.bottom), TRUE);

			// Add tabs to the tab control
			{
				TCITEM tab;

				hwnd = GetDlgItem(hwndDlg, WIN_STARTWIN_TABCTL);

				ZeroMemory(&tab, sizeof(tab));
				tab.mask = TCIF_TEXT;
				tab.pszText = TEXT("  Multimedia  ");
				SendMessage(hwnd, TCM_INSERTITEM, (WPARAM)TAB_MEDA, (LPARAM)&tab);
				tab.mask = TCIF_TEXT;
				tab.pszText = TEXT("  Gaming  ");
				SendMessage(hwnd, TCM_INSERTITEM, (WPARAM)TAB_GAME, (LPARAM)&tab);
				tab.mask = TCIF_TEXT;
				tab.pszText = TEXT("  Multiplay  ");
				SendMessage(hwnd, TCM_INSERTITEM, (WPARAM)TAB_MULT, (LPARAM)&tab);
				tab.mask = TCIF_TEXT;
				tab.pszText = TEXT("  About  ");
				SendMessage(hwnd, TCM_INSERTITEM, (WPARAM)TAB_HELP, (LPARAM)&tab);
				tab.mask = TCIF_TEXT;
				tab.pszText = TEXT("   Log   ");
				SendMessage(hwnd, TCM_INSERTITEM, (WPARAM)TAB_MESG, (LPARAM)&tab);

				// Work out the position and size of the area inside the tab control for the pages
				ZeroMemory(&r, sizeof(r));
				GetClientRect(hwnd, &r);
				SendMessage(hwnd, TCM_ADJUSTRECT, FALSE, (LPARAM)&r);
				r.right -= r.left-1;
				r.bottom -= r.top-1;
				r.top += rtab.top;
				r.left += rtab.left;

				// Create the pages and position them in the tab control, but hide them
				pages[TAB_MEDA] = CreateDialog((HINSTANCE)win_gethinstance(),
					              MAKEINTRESOURCE(WIN_STARTWINPAGE_MEDIA), hwndDlg, MediaPageProc);
				pages[TAB_GAME] = CreateDialog((HINSTANCE)win_gethinstance(),
					              MAKEINTRESOURCE(WIN_STARTWINPAGE_GAME), hwndDlg, GamePageProc);
				pages[TAB_MULT] = CreateDialog((HINSTANCE)win_gethinstance(),
					              MAKEINTRESOURCE(WIN_STARTWINPAGE_MULT), hwndDlg, MultiPageProc);
				pages[TAB_HELP] = CreateDialog((HINSTANCE)win_gethinstance(),
					              MAKEINTRESOURCE(WIN_STARTWINPAGE_ABOUT), hwndDlg, AboutPageProc);
				pages[TAB_MESG] = GetDlgItem(hwndDlg, WIN_STARTWIN_MESSAGES);
				SetWindowPos(pages[TAB_MEDA], hwnd,r.left,r.top,r.right,r.bottom,SWP_HIDEWINDOW);
				SetWindowPos(pages[TAB_GAME], hwnd,r.left,r.top,r.right,r.bottom,SWP_HIDEWINDOW);
				SetWindowPos(pages[TAB_MULT], hwnd,r.left,r.top,r.right,r.bottom,SWP_HIDEWINDOW);
				SetWindowPos(pages[TAB_HELP], hwnd,r.left,r.top,r.right,r.bottom,SWP_HIDEWINDOW);
				SetWindowPos(pages[TAB_MESG], hwnd,r.left,r.top,r.right,r.bottom,SWP_HIDEWINDOW);

				// Tell the editfield acting as the console to exclude the width of the scrollbar
				GetClientRect(pages[TAB_MESG],&r);
				r.right -= GetSystemMetrics(SM_CXVSCROLL)+4;
				r.left = r.top = 0;
				SendMessage(pages[TAB_MESG], EM_SETRECTNP,0,(LPARAM)&r);

				// Set a tab stop in the game data listbox
				{
				DWORD tabs[1] = { 150 };
				ListBox_SetTabStops(GetDlgItem(pages[TAB_GAME], IDGDATA), 1, tabs);
				ListBox_SetTabStops(GetDlgItem(pages[TAB_GAME], IDMDATA), 1, tabs);
				ListBox_SetTabStops(GetDlgItem(pages[TAB_GAME], IDSDATA), 1, tabs);
				ListBox_SetTabStops(GetDlgItem(pages[TAB_GAME], IDMUSIC), 1, tabs);
				ListBox_SetTabStops(GetDlgItem(pages[TAB_GAME], IDSKILL), 1, tabs);
				}

				SetFocus(GetDlgItem(hwndDlg, WIN_STARTWIN_START));
				SetWindowText(hwndDlg, apptitle);
			}
			return FALSE;
		}

		case WM_NOTIFY:
		{
			LPNMHDR nmhdr = (LPNMHDR)lParam;
			int cur;
			if (nmhdr->idFrom != WIN_STARTWIN_TABCTL) break;
			cur = (int)SendMessage(nmhdr->hwndFrom, TCM_GETCURSEL,0,0);
			switch (nmhdr->code)
			{
				case TCN_SELCHANGING:
				{
					if (cur < 0 || !pages[cur]) break;
					ShowWindow(pages[cur],SW_HIDE);
					return TRUE;
				}
				case TCN_SELCHANGE:
				{
					if (cur < 0 || !pages[cur]) break;
					ShowWindow(pages[cur],SW_SHOW);
					iPage = cur;
					if (iPage == TAB_MESG || iPage == TAB_HELP)
                        EnableWindow(GetDlgItem(startupdlg, WIN_STARTWIN_ABORT), FALSE);
                    else
                        EnableWindow(GetDlgItem(startupdlg, WIN_STARTWIN_ABORT), TRUE);
					return TRUE;
				}
			}
			break;
		}

		case WM_CLOSE:
			if (mode == TAB_MEDA)
			    done = 0;
			else
			    quitevent++;
			return TRUE;

		case WM_DESTROY:
			if (hbmp)
			{
				DeleteObject(hbmp);
				hbmp = NULL;
			}

			if (pages[TAB_MEDA])
			{
				DestroyWindow(pages[TAB_MEDA]);
				pages[TAB_MEDA] = NULL;
			}

			if (pages[TAB_GAME])
			{
				DestroyWindow(pages[TAB_GAME]);
				pages[TAB_GAME] = NULL;
			}

			if (pages[TAB_MULT])
			{
				DestroyWindow(pages[TAB_MULT]);
				pages[TAB_MULT] = NULL;
			}

			startupdlg = NULL;
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case WIN_STARTWIN_TBHELP:
				     MessageBeep(48);
				     tabhelp();
					 return TRUE;
			    case WIN_STARTWIN_ABORT:
			         if (iPage == TAB_MEDA)
			             ResetForm(1<<TAB_MEDA);
			         else
			         if (iPage == TAB_GAME)
			         {
			             ResetForm(1<<TAB_GAME);
			             EnableWindow(GetDlgItem(pages[TAB_GAME], IDMUSIC), 1);
			         }
			         else
			         if (iPage == TAB_MULT)
			             ResetForm(1<<TAB_MULT);
			         return TRUE;
				case WIN_STARTWIN_CANCEL:
					 if (mode == TAB_MEDA)
					     done = 0;
					 else
					     quitevent++;
					 return TRUE;
				case WIN_STARTWIN_START:
				     done = 1;
				     return TRUE;
			}
			return FALSE;

		case WM_CTLCOLORSTATIC:
			if ((HWND)lParam == pages[TAB_MESG])
				return (BOOL)GetSysColorBrush(COLOR_WINDOW);
			break;

		default: break;
	}

	return FALSE;
}

int startwin_open(void)
{
    int32 CONFIG_ReadSetup(void);
	INITCOMMONCONTROLSEX icc;
    char *cmdbuf = NULL;
    long fl;
    int i, j;

    CONFIG_ReadSetup();
    cmdbuf = strdup(GetCommandLine());

    if (Bstrncasecmp(cmdbuf, "Swp.exe -z", 10) == 0)
        return 0;

    if (Bstrncasecmp(cmdbuf, "Swp.exe -by", 11) == 0)
        return 0;

	if (startupdlg)
	    return 1;

    fl = kopen4load("SWP.cfg",0);
    if (fl == -1)
    {
        CONFIG_WriteSetup();
        CONFIG_ReadSetup();
    }
    kclose(fl);

	if (PlayerNameArg[0] == 0)                                       // wont work at configs ?
	    Bstrcpy(PlayerNameArg, "Player");
	if (IPAddressArg[0] == 0)
	{
	    Bstrcpy(IPAddressArg[0], "127.0.0.0");
	    LastIpUsed = 0;
    }

	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_TAB_CLASSES;
	InitCommonControlsEx(&icc);

    i = 0;
  	if (strstr(cmdbuf, " -") == 0 && strstr(cmdbuf, " /") == 0)      // check for parameters
  	    i = 1;

    j = 0;
    if (strstr(cmdbuf, "setup"))
    {
       i = 1;
       j = 1;
    }

    free(cmdbuf);

	if ((ForceSetup == 1 && i == 1) || (i == 1 && j == 1))
    {
   	   startupdlg = CreateDialog((HINSTANCE)win_gethinstance(), MAKEINTRESOURCE(WIN_STARTWIN), NULL, startup_dlgproc);
	   if (startupdlg)
       {
		  SetPage(TAB_MESG);
		  EnableConfig(0);
		  return 0;
	   }
	}

	return 0;
}

int startwin_close(void)
{
	if (!startupdlg)
	    return 1;
	DestroyWindow(startupdlg);
	startupdlg = NULL;
	return 0;
}

int startwin_puts(const char *buf)
{
	const char *p = NULL, *q = NULL;
	char workbuf[1024];
	static int newline = 0;
	int curlen, linesbefore, linesafter;
	HWND edctl;
	int vis;

	if (!startupdlg)
	    return 1;

	edctl = pages[TAB_MESG];
	if (!edctl)
	    return -1;

	vis = ((int)SendMessage(GetDlgItem(startupdlg, WIN_STARTWIN_TABCTL), TCM_GETCURSEL,0,0) == TAB_MESG);

	if (vis)
	    SendMessage(edctl, WM_SETREDRAW, FALSE,0);
	curlen = SendMessage(edctl, WM_GETTEXTLENGTH, 0,0);
	SendMessage(edctl, EM_SETSEL, (WPARAM)curlen, (LPARAM)curlen);
	linesbefore = SendMessage(edctl, EM_GETLINECOUNT, 0,0);
	p = buf;
	while (*p)
	{
		if (newline)
		{
			SendMessage(edctl, EM_REPLACESEL, 0, (LPARAM)"\r\n");
			newline = 0;
		}
		q = p;
		while (*q && *q != '\n') q++;
		memcpy(workbuf, p, q-p);
		if (*q == '\n')
		{
			if (!q[1])
			{
				newline = 1;
				workbuf[q-p] = 0;
			}
			else
			{
				workbuf[q-p] = '\r';
				workbuf[q-p+1] = '\n';
				workbuf[q-p+2] = 0;
			}
			p = q+1;
		}
		else
		{
			workbuf[q-p] = 0;
			p = q;
		}
		SendMessage(edctl, EM_REPLACESEL, 0, (LPARAM)workbuf);
	}
	linesafter = SendMessage(edctl, EM_GETLINECOUNT, 0,0);
	SendMessage(edctl, EM_LINESCROLL, 0, linesafter-linesbefore);
	if (vis)
	    SendMessage(edctl, WM_SETREDRAW, TRUE,0);
	return 0;
}

int startwin_settitle(const char *str)
{
	if (!startupdlg)
	    return 1;
	SetWindowText(startupdlg, str);
	return 0;
}

int startwin_idle(void *v)
{
	if (!startupdlg || !IsWindow(startupdlg))
	    return 0;
	if (IsDialogMessage(startupdlg, (MSG*)v))
	    return 1;
	return 0;
}

int startwin_run(void)
{
    DWORD dwPos;
    HWND hwnd;
	MSG msg;
	long fl;

	if (!startupdlg)
	    return 1;

	done = -1;

    GetSavedGames();

	SetPage(TAB_MEDA);
	EnableConfig(1);

	settings.fullscreen = ScreenMode;
	settings.xdim = ScreenWidth;
	settings.ydim = ScreenHeight;
	settings.bpp = ScreenBPP;
	settings.forcesetup = ForceSetup;
	settings.widescreen = glwidescreen;
    settings.nohrp = NoHrp;

	settings.brightness = iBright;
	settings.stereo = iStereo;
	settings.wangtalk = iTalk;
	settings.ambience = iAmbient;

    TexCache = glusetexcache;
    Tiles = usehightile;
    Model = usemodels;
    TexComp = glusetexcompr;
    LCD = LcdMon;

    LastMap[0] = 0;
    LastGame[0] = 0;
    GetMapsPlayed();                                                 // 100128

    GetLastPlayed(LastGame, 0);                                     // 100201
    if (LastGame[0] != 0)
    {
        dolower(LastGame);
        initprintf("Last Played Game = %s\n", LastGame);
    }
    GetLastPlayed(LastMap, 1);                                      // 100201
    if (LastMap[0] != 0)
    {
        dolower(LastMap);
        initprintf("Last Played Map  = %s\n", LastMap);
    }
	GetSavedGames();                                                 // 100125

    MenuMap[0] = 0;
    MenuGame[0] = 0;
    UserSavenum = -1;

	PopulateForm(-1);
    SetPage(TAB_MEDA);

	while (done < 0)
	{
		switch (GetMessage(&msg, NULL, 0,0))
		{
			case 0:
			     done = 1;
			     break;
			case -1:
			     return -1;
			default:
				 if (IsWindow(startupdlg) && IsDialogMessage(startupdlg, &msg))
				     break;
				 TranslateMessage(&msg);
				 DispatchMessage(&msg);
				 break;
		}
	}

    hwnd = GetDlgItem(pages[TAB_MEDA], IDBRIGHT);
    dwPos = SendMessage(hwnd, TBM_GETPOS, 0, 0);
    if (dwPos > 63)
        dwPos = 63;
    settings.brightness = dwPos;

	SetPage(TAB_MESG);
    EnableConfig(0);

	if (done)
	{
		ScreenMode = settings.fullscreen;
		ScreenWidth = settings.xdim;
		ScreenHeight = settings.ydim;
		ScreenBPP = settings.bpp;
		ForceSetup = settings.forcesetup;
        NoHrp = settings.nohrp;

		glwidescreen = settings.widescreen;
        iBright = settings.brightness;
	    iStereo = settings.stereo;
	    iTalk = settings.wangtalk;
	    iAmbient = settings.ambience;

        glusetexcache = TexCache;
        usehightile = Tiles;
        usemodels = Model;
        glusetexcompr = TexComp;
        LcdMon = LCD;

   	    CheckIPAddress();               // add to list if not there  wxsm
		menudone = TRUE;

        if (MapsFolder[0] != 0)
            strcat(MapsFolder, "/");
        if (MusicFolder[0] != 0)
            strcat(MusicFolder, "/");
	}
	else
	{
	    if (bForce)
	    {
	        ScreenMode = settings.fullscreen;
	        ForceSetup = settings.forcesetup;
	        LcdMon = LCD;
	        CONFIG_WriteSetup();
	    }
	}

	FreeGroups();

	return done;
}

//-----------------------------------------------------------------------------

void SetDefGames(void)
{
    HWND hwnd;
    short i;

    hwnd = GetDlgItem(pages[TAB_GAME], IDGMAPS);
	ListBox_ResetContent(hwnd);

    for (i=0; i<28; i++)
         ListBox_AddString(hwnd, defgames[i]);

    ListBox_SetCurSel(hwnd, -1);
    iLevelNum = -1;

    Edit_SetText(GetDlgItem(pages[TAB_GAME], IDLBINT), "Default Game");
}

void GetMapNames(char *fname)
{
    HWND hwnd;
    FILE *fn;
    char grpname[80];
    char gbuf[16];
    int i, j;

    hwnd = GetDlgItem(pages[TAB_GAME], IDGMAPS);
	ListBox_ResetContent(hwnd);

	if (Bstrcmp(fname, ".grp") == 0)
	    return;

    if (Bstrcmp(fname, "Sw.grp") == 0)
    {
       SetDefGames();
       return;
    }
    else
       Bsprintf(grpname, "%s\\%s", GameFolder, fname);

    fn = fopen(grpname,"rb");

    if (fn != NULL)
    {
	   for (i=0; i<1000; i++)  // 1000 possible entries max
	   {
           fread((gbuf),(16),(1),(fn));
           for (j=12; j<16; j++)
                gbuf[j] = 0;
           if (gbuf[0] < 0x21 || gbuf[0] > 0x7a)
               break;
           if (i == 0)
           {
              if (!Bstrcmp(grpname, "KenSilverman"))
                  break;
           }
           else
           {
              dolower(gbuf);
              j = strlen(gbuf)-4;
		      if (Bstrncasecmp(gbuf+j, ".map", 4) == 0)
                  ListBox_AddString(hwnd, gbuf);
           }
       }
       fclose(fn);
       ListBox_SetCurSel(hwnd, -1);
       Bsprintf(grpname, "%s", MenuGame);
       grpname[0] = Btoupper(grpname[0]);
       Edit_SetText(GetDlgItem(pages[TAB_GAME], IDLBINT), grpname);
    }
}

void GetGameFiles(char *gpath)
{
    HWND hwnd;
    char gbuf[128];
    int x, j=0;

    x = Bstrcasecmp(GameFolder, LastGameFolder);
    if (x == 0)
        return;
    strcpy(LastGameFolder, gpath);

    hwnd = GetDlgItem(pages[TAB_GAME], IDGDATA);
	ListBox_ResetContent(hwnd);

    x = 0;
    Bsprintf(gbuf, "%s/", gpath);
    getfiles(gbuf,"*.grp", 1);
    while (findfiles)
    {
        Bsprintf(gbuf,"%s",findfiles->name);
        dolower(gbuf);
        if (strstr(gbuf, LastGame) && strlen(LastGame) > 4)
            j = x;
        if (strlen(gbuf) > 4)
        {
            if (j == x && j > 0)
                ListBox_InsertString(hwnd, 0, gbuf);
            else
                ListBox_AddString(hwnd, gbuf);
            x++;
        }
        findfiles = findfiles->next;
    }

    Bsprintf(gbuf, "%s/", gpath);
    getfiles(gbuf,"*.zip", 1);
    while (findfiles)
    {
        Bsprintf(gbuf,"%s",findfiles->name);
        dolower(gbuf);
        if (Bstrcasecmp(gbuf, LastGame) == 0)
            j = x;
        if (!strstr(gbuf, "_hrp"))
        {
            if (j == x && j > 0)
                ListBox_InsertString(hwnd, 0, gbuf);
            else
                ListBox_AddString(hwnd, gbuf);
            x++;
        }
        findfiles = findfiles->next;
    }
    ResetForm(99);
    ListBox_SetCurSel(hwnd, -1);
    //ListBox_SetTopIndex(hwnd, j);
}

void GetMapFiles(char *mpath, short wb)
{
    HWND hwnd;
    char mbuf[80];
    char xbuf[80];
    short i, j=0;

    if (wb)
        hwnd = GetDlgItem(pages[TAB_MULT], IDMAPWB);
    else
        hwnd = GetDlgItem(pages[TAB_GAME], IDMDATA);
	ListBox_ResetContent(hwnd);

    Bsprintf(mbuf, "%s/", mpath);
    getfiles(mbuf,"*.map", 1);
    i = 0;
    while (findfiles)
    {
          Bsprintf(mbuf,"%s",findfiles->name);
          dolower(mbuf);
          if (Bstrcasecmp(mbuf, LastMap) == 0)                       // 100201
              j = i;
          if (CheckMapsPlayed(mbuf))                                 // 100128
              Bsprintf(xbuf,"%s **",mbuf);
          else
              Bsprintf(xbuf,"%s",mbuf);
          if (j == i && j > 0)
              ListBox_InsertString(hwnd, 0, xbuf);
          else
              ListBox_AddString(hwnd, xbuf);
          findfiles = findfiles->next;
          i++;
    }
    //ListBox_SetTopIndex(hwnd, j);
    ListBox_SetCurSel(hwnd, -1);
}

void LoadMusicFiles(char *mpath)
{
    HWND hwnd;
    char mbuf[128];

    if (cdplay)
    {
        LoadTracks();
        return;
    }

    hwnd = GetDlgItem(pages[TAB_GAME], IDMUSIC);
	ListBox_ResetContent(hwnd);

    Bsprintf(mbuf, "%s/%s/", RootFolder, mpath);
    getfiles(mbuf,"*.mid", 1);
    while (findfiles)
    {
          Bsprintf(mbuf,"%s",findfiles->name);
          dolower(mbuf);
          ListBox_AddString(hwnd, mbuf);
          findfiles = findfiles->next;
    }
    Bsprintf(mbuf, "%s/%s/", RootFolder, mpath);
    getfiles(mbuf,"*.wav", 1);
    while (findfiles)
    {
          Bsprintf(mbuf,"%s",findfiles->name);
          dolower(mbuf);
          ListBox_AddString(hwnd, mbuf);
          findfiles = findfiles->next;
    }
    ListBox_InsertString(hwnd, 0, "No Music");
    ListBox_SetCurSel(hwnd, -1);
}

void LoadTracks()
{
    HWND hwnd;
    char mbuf[128];
    int i;

    hwnd = GetDlgItem(pages[TAB_GAME], IDMUSIC);
	ListBox_ResetContent(hwnd);

    for (i=0; i<11; i++)
    {
         Bsprintf(mbuf,"Track %d", i+4);
         ListBox_AddString(hwnd, mbuf);
    }
    ListBox_InsertString(hwnd, 0, "No Music");
    ListBox_SetCurSel(hwnd, -1);
}

void ListSavedGames(void)                                            // 100130
{
    HWND hwnd;
    short i;

    if (strstr(MenuMap, " **"))
        MenuMap[strlen(MenuMap)-3] = 0;

    GetSavedGames();
	hwnd = GetDlgItem(pages[TAB_GAME], IDSDATA);
    ListBox_ResetContent(hwnd);

    for (i=0; i<NumSaves; i++)
         ListBox_AddString(hwnd, UserSaves[i]);

    ListBox_SetCurSel(hwnd, -1);
    UserSavenum = -1;

	Edit_SetText(GetDlgItem(pages[TAB_GAME], IDLBSAV), SaveLabel);
}

void LoadGameDes(short sv_num, char *desc)                           // 100130
{
    int i;
    FILE *fl;
    char sgame[80];
    char xbuf[40];

    sprintf(sgame,"%sgame%d.sav",svgame,sv_num);
    fl = fopen(sgame,"rb");
    if (fl != NULL)
    {
        fseek(fl, -23, 2);
        fread((xbuf),(3),(1),(fl));
        if (strstr(xbuf, "swp"))
        {
            fread((xbuf),(20),(1),(fl));
            xbuf[20] = 0;
        }
        else
            sprintf(xbuf, "game%d.sav", sv_num);
        strcpy(desc, xbuf);
    }
    fclose(fl);
}

void GetSavedGames(void)                                             // 100130
{
    char fn[80];
    long fil;
    short i, j, x, ln = strlen(svgame);
    char sbuf[128];
    char ch;

    x = 0;
    i = 1;
    sbuf[0] = 0;

    Bsprintf(fn, "%sgame0.sav", svgame);

    for (i=0; i<10; i++)
    {
        fn[4+ln] = i+'0';
        fil = kopen4load(fn, 0);
        if (fil != -1)
        {
            kclose(fil);
            strcpy(sbuf, fn);
            extractname(sbuf);
            strcpy(tempbuf, sbuf);
            j = strlen(tempbuf);
            ch = tempbuf[j-5];
            j = ch-'0';
            sbuf[0] = 0;
            LoadGameDes(j, sbuf);
            if (strlen(sbuf) < 2)
                strcpy(sbuf, tempbuf);
            dolower(sbuf);
            strcpy(UserSaves[x], sbuf);
            GameSaved[x] = j;
            x++;
        }
    }

    Bsprintf(fn,"%sgamex.asv", svgame);
    fil = kopen4load(fn, 0);
    if (fil != -1)
    {
        kclose(fil);
        strcpy(UserSaves[x], "auto saved game");
        GameSaved[x] = 10;
        x++;
    }

    NumSaves = x;
}

void CheckIPAddress(void)
{
    short i;

    for (i=0; i<8; i++)
    {
        if (strlen(IPAddressArg[i]) < 8)
            break;
        if (Bstrcasecmp(IPAddressArg[i], IPAddress) == 0)
        {
            i = -1;
            break;
        }
    }

    if (i != -1)
    {
       if (i > 6)
           i = 7;
       strcpy(IPAddressArg[i], IPAddress);
       initprintf("Added IPAddress: %s to list\n", IPAddress);
    }
}

void CheckShareware(void)
{
    if (!DetectShareware())
        return;

    EnableWindow(GetDlgItem(pages[TAB_GAME],  IDDIRGM), FALSE);
    EnableWindow(GetDlgItem(pages[TAB_GAME],  IDGDATA), FALSE);
    EnableWindow(GetDlgItem(pages[TAB_GAME],  IDDIRSP), FALSE);
    EnableWindow(GetDlgItem(pages[TAB_GAME],  IDMDATA), FALSE);
    EnableWindow(GetDlgItem(pages[TAB_GAME],  IDGMAPS), FALSE);
    EnableWindow(GetDlgItem(pages[TAB_GAME],  IDSDATA), FALSE);
    EnableWindow(GetDlgItem(pages[TAB_GAME],  IDMFOLD), FALSE);
    EnableWindow(GetDlgItem(pages[TAB_GAME],  IDMUSIC), FALSE);
    EnableWindow(GetDlgItem(pages[TAB_MULT],  IDDIRWB), FALSE);
    EnableWindow(GetDlgItem(pages[TAB_MULT],  IDMAPWB), FALSE);
    Button_Enable(GetDlgItem(pages[TAB_GAME], IDHIRES), FALSE);
    Button_Enable(GetDlgItem(pages[TAB_MULT], IDUJOIN), FALSE);
    Button_Enable(GetDlgItem(pages[TAB_MULT], IDUHOST), FALSE);
    Button_Enable(GetDlgItem(pages[TAB_MULT], IDWBSEL), FALSE);
}

void Showhelp(short x)
{
    char tbuf[128];
    HWND hwnd;
    int i;

	char lbuf[82][80] =
	{
	    " About SWP",
	    "  ",
        " This port is a modified version of Jonathon Fowler's Shadow Warrior port.",
        " Although it is important to try and keep the game as original as",
        " possible it is often necessary to make changes to improve things",
        " which adds to the gameplay.",
        " ",
	    " [Installation]",
        " Suggested directories for SWP.",
        " SWP",
        " \tGames\t - All .grp addons and TC's",
        " \tMaps \t - All .map files",
        " \tMusic\t - All music files",
        " ",
        " [Basic files required]",
        " 1. SWP.exe",
        " 2. SWP.grp",
        " 3. SWP.cfg\t - Will get created 1st time.",
        " 4. SW.grp \t - Shareware or retail.",
        " 5. Sw_Hrp.zip\t - Optional.",
        " ",
        " \t\t-----------------------------------",
        " ",
        " Additional Cheat Codes",
        " ",
        " SWGOD\t\t - Toggle god mode",
        " SWMEDIC\t - Adds 25% Health",
        " SWKEYS\t\t - Gives ALL 8 keys",
        " SWREDCARD\t - Gives Red Cardkey",
        " SWBLUECARD\t - Gives Blue Cardkey",
        " SWGREENCARD\t - Gives Green Cardkey",
        " SWYELLOWCARD\t - Gives Yellow Cardkey",
        " SWGOLDKEY\t - Gives Gold Skeleton key",
        " SWSILVERKEY\t - Gives Silver Skeleton key",
        " SWBRONZEKEY\t - Gives Bronze Skeleton key",
        " SWREDKEY\t - Gives Red Skeleton key",
        " SWGUN#\t - Gives weapon 2 to 0",
        " ",
        " \t\t-----------------------------------",
        " ",                                    // 39
	    " Commandline Parameters",
        " ",
	    " [Usage: Swp [options]\noptions:  ('-' or '/' may be used)]",
	    " -?\t\tDump these paramaters to the log file",
        " -music [filename]\tLoad a musicfile",
        " -map [mapname]\tLoad a map",
        " -nocd<audio>\tNo CD Red Book Audio",
        " -nohrp\t\tDo not load the HRP (must be 1st or 2nd command)",
        " -name [player]\tPlayer Name",
        " -s#\t\tSkill (1-4)",
        " -f#\t\tPacket Duplication - 2, 4, 8",
        " -nopred<ict>\tDisable Net Prediction Method",
        " -level#\t\tStart at level# (1-28)",
        " -dr[filename.dmo]\tDemo record. NOTE: Use -level# here."  ,
        " -dp[filename.dmo]\tDemo playback. NOTE: Use -level# here.",
        " -monst\t\tNo Monsters",
        " -nodemo\t\tNo demos on game startup",
        " -nometers\tDon't show air or boss meter bars in game",
        " -movescale\tAdjust movement scale: 256 = 1 unit",
        " -turnscale\tAdjust turning scale: 256 = 1 unit",
        " -extcompat\tController compatibility mode (with Duke 3D)",
        " -g[filename.grp]\tLoad an extra GRP or ZIP file",
        " -h[filename.def]\tUse additional custom filename.def",
        " -j[directory]\tAdd a directory to the file path stack",
        " -c[filename.cfg]\tUse filename.cfg instead of SwCustom.txt",
        " -game#\t\tLoad saved game (0-9)",
        " -setup\t\tForces Startup Menu Dialog to display",
        " -by\t\tBypass startmenu at start and exit (must be 1st)",
        " ",
        "~",
	};

    if (x == 1)
    {
        for (i=39; i<70; i++)
        {
	        if (strstr(lbuf[i], "~"))
	            break;
            strcpy(tbuf, lbuf[i]);
            initprintf("%s\n", tbuf);
        }
        return;
    }

    hwnd = GetDlgItem(pages[TAB_HELP], IDAHELP);
	ListBox_ResetContent(hwnd);
	for (i=0; i<82; i++)
	{
	    if (strstr(lbuf[i], "~"))
	        break;
	    ListBox_AddString(hwnd, lbuf[i]);
	}
	ListBox_SetCurSel(hwnd, -1);
}

void tabhelp(void)
{
    char *s = "ERRRRRRRRRROR!";
    char *t = "Ha ha.";

    if (iPage == TAB_MEDA)
    {
        t = "Multimedia Information";
        s = "[Sound Information]\n"
        "OpenGL rendering support.\n"
        "True-colour textures support.\n"
        "Optional extra Hud with weapon icons.\n\n"

        "[Sound Information]\n"
        "Midi devices.\n"
        "Wave devices.\n"
        "Internal user music selection.\n"
        "Optional user Mapname.mid music.\n"
        "CD music selection."
        ;
    }
    else
    if (iPage == TAB_GAME)
    {
        if (mapvis == 0) // addons
        {
            t = "Addons and Total Conversions";
	        s = "[Playing an Addon]\n"
			"Often it is the case where Addons supply their own textures within\n"
			"their .grp files and it is advisable to select:\n"
            "'Disable Default High Resolution Pack', but should be experimental\n"
            "as some Addons do use the default hrp.\n"
			"[Last Played Addon]\n"
			"The last played Addon will be set at the top of the Addon list.\n"
			"[Addon High Resolution]\n"
			"For any Addon to use its own Hrp, '_hrp.zip' must be added.\n"
			"Example: LoNukem.grp would have LoNukem_hrp.zip\n"
	        "[Internal Maps]\n"
	        "These are the maps contained within an Addon's .grp which can\n"
	        "played separately along with custom music.\n"
            "[Addon Saved Games]\n"
            "Any games saved for an Addon, can be selected\n"
            "in the Game Tab, by either clicking the '>' button to the right\n"
            "or by just clicking on the Saved Games Listbox in the Game Tab.\n"
            "When selecting to play a Addons Saved Game, the appropriate .grp\n"
            "file will also be loaded giving the correct textures etc.\n"
            "[Addon Update]\n"
            "Should there be an update for a particular Addon, just use the\n"
            "same name but with _Addon.zip or .grp, then it will get loaded\n"
            "and merged with the original Addon.\n"
            "[Addon Def and Cfg Files]\n"
            "For an Addon to be launched correctly, its .def and .cfg\n"
            "filenames should follows it's .grp name.\n"
            "Example: LoNukem.grp should have LoNukem.cfg and LoNukem.def\n\n"

            "[Skill Level]\n"
            "This is the difficulty level you choose when playing any level.\n\n"

            "[Saved Games]\n"
            "All Addons, Total Conversions and User Maps each have their own\n"
            "10 seperate Saved Games which are stored in the Saves folder.\n"
            "Selecting to play a Saved Game will also load it's appropriate .grp\n"
            "file etc if it belongs to an Addon or TC."
            ;
	    }
        else
        {
            t = "Maps and Music Information";
	        s = "[Played Maps]\n"
			"A '**' at the end of a mapname shows the map has been played.\n"
			"When a user map is completed in the bonus screen it's name\n"
			"gets saved in a file called SWP.mps\n"
			"[Last Played Map]\n"
			"The last played map will be set at the top of the map list.\n"
			"[Map Music]\n"
			"If a map has its own midi, wav or ogg music then it must be the\n"
			"same name as the map.\n"
			"Example: Lapd.map has Lapd.mid or Lapd.ogg etc.\n"
			"NB. If a map has a .mid and a .ogg, the .ogg will be used.\n"
			"[Map High Resolutions]\n"
			"Any map can have it's own Hrp provided you use mapname_hrp.zip\n"
			"Example: Lapd.map would have Lapd_hrp.zip\n"
            "[Map Dedicated Con]\n"
			"A map can have it's own Game.con provided you use mapname.con\n"
			"Example: Lapd.map would have Lapd.con\n\n"

			"[User Music]\n"
			"Selecting a music file to be played with any map will override\n"
			"any dedicated map music like mapname.mid etc.\n"
			"Music can also be changed in the game - see 'Select Music'\n\n"

            "[Skill Level]\n"
            "This is the difficulty level you choose when playing any level.\n\n"

            "[Saved Games]\n"
            "All Addons, Total Conversions and User Maps each have their own\n"
            "10 seperate Saved Games which are stored in the Saves folder.\n"
            "Selecting to play a Saved Game will also load it's appropriate .grp\n"
            "file etc if it belongs to an Addon or TC."
			;
        }
    }
    else
    if (iPage == TAB_MULT)
    {
        t = "Multiplayer Information";
        s = "[Multiplayer Options]\n"
        "Swp offers a full multiplay section which can be accessed initially from\n"
        "the start menu.\n"
        "Select the mode of play, click on Select Multiplay Game and click on Start.\n"
        "If you chose a map in the Maps section, the game will launch directly in the\n"
        "map unless there are more than 2 players.\n"
        "If no map was selected, you will be transferred to the Multiplay menu in the\n"
        "game to select your options from there.\n"
        "The multiplay section has been greatly improved to work well with Yang."
        ;
    }
    else
    if (iPage == TAB_HELP)
    {
        t = "About Information";
        s = "See here for:\n"
        "1. Installation details\n"
        "2. Additional cheat codes\n"
        "3. Commandline paramaters\n"
        "4. Notable changes\n"
        ;
    }
    else
    if (iPage == TAB_MESG)
    {
        t = "Logs Information";
        s = "All important occurrences are logged here.\n"
        "This logfile is saved as SWP.log and can be viewed in any text editor.\n"
        ;
    }
    wm_msgbox(t,s);
}

void resetgame(void)
{
    HWND hwnd;

    ListBox_SetCurSel(GetDlgItem(pages[TAB_GAME], IDGDATA), -1);
    ListBox_SetCurSel(GetDlgItem(pages[TAB_GAME], IDGMAPS), -1);
    MenuGame[0] = 0;
    GameMap[0] = 0;
    SetDefGames();
}

void resetmaps(void)
{
    HWND hwnd;

    ListBox_SetCurSel(GetDlgItem(pages[TAB_GAME], IDMDATA), -1);
    MenuMap[0] = 0;
}

void resetwbmaps(void)
{
    HWND hwnd;

    ListBox_SetCurSel(GetDlgItem(pages[TAB_MULT], IDMAPWB), -1);
    MenuMap[0] = 0;
}

void resetsave(short def)
{
    UserSavenum = -1;
    svgame[0] = 0;
    ListBox_SetCurSel(GetDlgItem(pages[TAB_GAME], IDSDATA), -1);
    if (def)
    {
        Bsprintf(SaveLabel, "Default Game");
        ListSavedGames();
    }
}

void resetmusic(void)
{
    HWND hwnd;

    hwnd = GetDlgItem(pages[TAB_GAME], IDMUSIC);
    ListBox_SetCurSel(hwnd, -1);
    MenuMusic[0] = 0;
    MenuTrack = 0;
}
