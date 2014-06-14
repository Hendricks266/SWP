//-------------------------------------------------------------------------
/*                              gamep.h
                  Additional code for SWP by ProAsm
                 Created May 2009 for MinGW compiling etc
*/
//-------------------------------------------------------------------------
/*
Watcom 1.8
Remark out boolean in rpcndr.h line 40
Remove #define WIN32_LEAN_AND_MEAN in compat.c line 16
*/
//-------------------------------------------------------------------------
// Interesting functions
// hicsetpalettetint(18, 255, 155, 16, 0); // pal, r, g, b, fl set palette colours
//
//
// tint { pal 11 red  16 green 240 blue  32 flags 0 }  // green
// tint { pal 13 red  64 green 128 blue 255 flags 0 }  // blue
// tint { pal 14 red 240 green  32 blue  16 flags 0 }  // red
// tint { pal 15 red 255 green 155 blue  16 flags 0 }  // gold
//
// Host = SWP.EXE -map Maps\Citadel.map -name Joesoap -net -n1 192.168.0.2
// Join = SWP.EXE -map Maps\Citadel.map -name ProAsm  -net 192.168.0.4 -n1
//-------------------------------------------------------------------------
/*
DM - swp -map testmap.map -autonet 0,0,0,1,0,0,3,2,1,0 -f4 -name whatever -net ipaddress -n1
CO - swp -map testmap.map -autonet 2,0,3,1,0,0,3,2,1,1 -f4 -name whatever -net ipaddress -n1

Rules     ->    0=WangBang 1=WangBang (No Respawn) 2=CoOperative
Level     ->    0 to 24(?) - always 0
Enemy     ->    0=None 1=Easy 2=Norm 3=Hard 4=Insane
Markers   ->    0=Off 1=On
Team      ->    0=Off 1=On
HurtTeam  ->    0=Off 1=On
KillLimit ->    0=Infinite 1=10 2=20 3=30 4=40 5=50 6=60 7=70 8=80 9=90 10=100
TimeLimit ->    0=Infinite 1=3 2=5 3=10 4=15 5=20 6=30 7=45 8=60
Color     ->    0=Brown 1=Gray 2=Purple 3=Red 4=Yellow 5=Olive 6=Green 7=Blue
Nuke      ->    0=Off 1=On
*/
//-------------------------------------------------------------------------

void KillString(PLAYERp pp, short y);
VOID PutStringInfo2(PLAYERp pp, char *string);

#define TITLE_PICW 2323

extern int is_vista;

extern char PlayerNameArg[40] = "";
extern char IPAddressArg[8][40];
extern short TimeLimitTable[9];

short iLoad = 0;
short bypass = 0;
short iTne = 0;
short mPal = 10;
BOOL bStatScreen = FALSE;
BOOL bDefFile = FALSE;
BOOL NetCmds = FALSE;
short xSkill = 0;      // -s from yang
static int firstnet = 0;	// JBF

static CACHE1D_FIND_REC *finddirs=NULL, *findfiles=NULL, *finddirshigh=NULL, *findfileshigh=NULL;

int32 NoHrp = 0;
long  Autotick = 0;

short Autosave = 1;         // off - auto - manual
short Autodone = 0;
short iLast=0;
short LoadSavedGame = -1;
short NoAnim = 0;
static int numdirs=0, numfiles=0;
static int currentlist=0;
BOOL Autodisp = FALSE;
BOOL RandomMusic = FALSE;
BOOL bGrp = FALSE;
BOOL bMusic = FALSE;
BOOL bHosting = FALSE;

char custdeffile[80] = "";
char custtxtfile[80] = "";
char RTSCustom[128] = "";
char fname[128];
char svgame[40];
char MapsPlayed[2000][40];
char LastMusic[80];

extern char  UserMaps[1000][32];
extern char  MyMaps[15][32];
extern char  UserMusic[300][32];
extern char  MyMusic[15][32];
extern short NumUserMusic;
extern short NumUserMaps;
extern BOOL  bMapmenu;

//-------------------------- startup menu settings -------------------------

const char **argvline;
extern int cddevice;

short iStart = 0;
short iBright;
short iStereo;
short iAmbient;
short iTalk;
short Mapnum = 0;

BOOL menudone;
BOOL cdplay;
BOOL musicon;
BOOL fxon;

extern long ScreenMode, ScreenWidth, ScreenHeight, ScreenBPP;

extern char  RootFolder[128];
extern char  MusicFolder[80];
extern char  MapsFolder[80];
extern char  GameFolder[80];

extern short MenuTrack;

extern char  MenuMap[80];
extern char  MenuMusic[80];
extern char  MenuGame[80];
extern char  GameMap[40];

extern char  UserSaves[11][80];
extern short UserSavenum;
extern short iLevelNum;
extern short iSkill;

extern short iType;
extern short iMons;
extern short iFrag;
extern short iTime;
extern short iColr;

extern char  IPAddress[32];
extern short SelectMulti;
extern short HostMulti;

BOOL ShellFlag = FALSE;

VOID checkforhrp(char *pth, short iMap);
long SetupUserMap();
long SetupUserGame(short svg);
long SetupUserLevel(short lnum);
long SetupUserSavedGame(short snum);
long SetupMultiPlayer();

//--------------------------------------------------------------------------

int CheckSwHrp(short iTyp)
{
    long k;

    if (NoHrp == 0 && iTyp == 0)
    {
       k = initgroupfile("Sw_Hrp.zip");
       if (k >= 0)
       {
          initprintf("Adding SW High Resolution Pack\n");
          bDefFile = TRUE;
       }
       else
       {
          k = initgroupfile("Sw_Lrp.zip");
          if (k >= 0)
          {
             initprintf("Adding SW Low Resolution Pack\n");
             bDefFile = TRUE;
          }
       }
    }
    return (0);
}

VOID SetCurrentMap(char *sMap)
{
    if (strchr(sMap, '/') == 0)
    {
       if (strchr(sMap, '\\') == 0)
       {
          strcpy(CurrMapName, sMap);
       }
       else
       {
          strcpy(CurrMapName, strrchr(sMap, '\\')+1);
       }
    }
    else
    {
       strcpy(CurrMapName, strrchr(sMap, '/')+1);
    }
    if (CDAudio_Playing())
        initprintf("  - Detected CD Music playing.\n");
}

BOOL PlayMusic(char *Musicname)
{
    int x, fp;
    char ubuf[128];

    if (MusicDevice < 0)
        return FALSE;

    strcpy(ubuf, Musicname);
    dolower(&ubuf);

    if (strstr(ubuf, ".voc"))
        return FALSE;

    if (iTne == 1 && Level == 1 && strstr(ubuf, "e1l01.mid"))
        return FALSE;

    if (gs.PlayCD && CDAudio_Playing())
        return FALSE;

    if (Bstrcmp(ubuf, LastMusic) == 0)
    {
        //initprintf("  - Same as Last Music : %s\n", ubuf);
        return TRUE;
    }
    else
        strcpy(LastMusic, ubuf);

    StopFX();
    StopSong();
    CDAudio_Stop();

    if (strstr(ubuf, ".mid") || strstr(ubuf, ".wav"))
    {
        fp = kopen4load(ubuf, 0);

        if (fp < 0)
        {
            initprintf("  - Play Music Failed : %s\n", ubuf);
            return FALSE;
        }

        close(fp);

        InitMusic();
        PlaySong(ubuf, -1, TRUE, TRUE);
        initprintf("  - Playing Music : %s\n", ubuf);
        return TRUE;
    }

    initprintf("  - Play Music Failed : %s\n", ubuf);
    return FALSE;
}

void checkfordeffile(void)
{
    FILE *fin;

    fin = fopen("sw.def","r");       // check if sw.def exists
    if (fin == NULL)
        return;
    else
       fclose(fin);
    rename("sw.def", "sw_def.bak" ); // rename it
}

void checkforbakfile(void)
{
    FILE *fin;

    fin = fopen("sw_def.bak","r");   // check if sw_def.bak exists
    if (fin == NULL)
        return;
    else
       fclose(fin);
    rename("sw_def.bak", "sw.def" ); // rename it
}
// 1=map / 2=grp / 3=zip
VOID checkforhrp(char *pth, short iMap)
{
    char px[128];
    long fil, k;
    int i, x;

    if (strchr(pth,'.') != 0)
    {
       x = strlen(pth);

       if (bGrp && iMap == 1)               // incase a .grp has a -map
           goto mapskip;

       for (i=0; i<128; i++)
            px[i] = 0;
       strcpy(px, pth);
       if (iMap == 2)
           strcpy(UserAddon, px);           // store for multiplay
       px[x-4] = 0;
       strcat(px, "_hrp.zip");
       //strcat(px, ".hrp");
       k = initgroupfile(px);               // check for TCName/MapName.hrp
       if (k >= 0)
           initprintf("Adding Custom Hrp: %s\n", px);

       for (i=0; i<128; i++)
            px[i] = 0;
       strncpy(px, pth, x-4);
       px[x-4] = 0;
       strcat(px, ".def");
       strcpy(custdeffile, px);             // check for TCName/MapName.def

       for (i=0; i<128; i++)
            px[i] = 0;
       strncpy(px, pth, x-4);
       px[x-4] = 0;
       strcat(px, ".cfg");
       strcpy(custtxtfile, px);             // check for TCName/MapName.cfg

       i = strlen(pth)-4;
       if (bGrp == FALSE && Bstrncasecmp(pth+i, ".grp", 4) != 0)
       {
          for (i=0; i<128; i++)
               px[i] = 0;
          strncpy(px, pth, x-4);
          px[x-4] = 0;
          strcat(px, ".grp");
		  initgroupfile(px);                // check for MapName.grp
       }

       for (i=0; i<128; i++)
            px[i] = 0;
       strncpy(px, pth, x-4);
       px[x-4] = 0;
       strcat(px, ".rts");
       k = kopen4load(px, 0);               // check for tcname or mapname .rts
       if (k >= 0)
       {
           strcpy(RTSCustom, px);
           initprintf("Adding Custom RTS: %s\n", px);
           kclose(k);
       }

mapskip:

       if (!bMusic && iMap ==1)
       {
          for (i=0; i<128; i++)
          {
               px[i] = 0;
          }
          strncpy(px, pth, x-4);
          px[x-4] = 0;
          strcat(px, ".mid");
          if ((fil = kopen4load(px,0)) >= 0)
          {
             strcpy(LevelSong, px);
             kclose(fil);
          }
          else
          {
              px[x-4] = 0;
              strcat(px, ".wav");
              if ((fil = kopen4load(px,0)) >= 0)
              {
                 strcpy(LevelSong, px);
                 kclose(fil);
              }
          }
       }
    }
}

int checksearchpath(char *pth)
{
    unsigned char ph[128];
    unsigned char px[128];
    int i, j, k, x, z;

    z = -1;
    px[0] = 0;
    fname[0] = 0;

    if (strchr(pth,'\\') != 0 || strchr(pth,'/') != 0)
    {
        for (i=0; i<128; i++)
        {
            px[i] = 0;
            ph[i] = 0;
            fname[i] = 0;
        }

        strcpy(ph, pth);
        x = strlen(pth);

        for (i=x; i>0; i--)
        {
            if (ph[i] == '\\' || ph[i] == '/')
                break;
        }

        for (j=0; j<i; j++)
        {
            px[j] = ph[j];          // extract the path
        }

        addsearchpath(px);
        initprintf("Adding Searchpath: %s\n", px);
        z = 0;

        i++;
        k = 0;
        for (j=i; j<x; j++)
        {
            fname[k] = ph[j];       // extract the name
            k++;
        }
    }
    else
    {
        strcpy(fname, pth);
    }

    return z;
}

//--------------------------------- Auto Saved Game --------------------------------

void AutoSaveGame(short disp, int iTime)
{
    if (Autosave > 0 && numplayers <= 1)
    {
       Autotick = totalclock + iTime;
       Autodone = 1;
       if (disp == 1)
           Autodisp = 1;
    }
}

void DelayedAutoSave(void)
{
    extern long PlayClock;
    long second_tics = (PlayClock/120);

    if (Autotick > 0)
    {
       PLAYERp pp = Player + myconnectindex;
       USERp u = User[pp->PlayerSprite];

       if (totalclock > Autotick - 20 && totalclock < Autotick - 3)
       {
          KillString(pp, TEXT_INFO_LINE(0)); // causes save errors
          KillString(pp, TEXT_INFO_LINE(1));
          KillString(pp, TEXT_INFO_LINE(2));
       }

       if (totalclock >= Autotick)
       {
          Autotick = 0;
          if (u->Health > 5)
          {
              PauseAction();
              SaveGame(10);
              ResumeAction();
          }
          if (Autodisp)
          {
             PutStringInfo2(pp, "Autosaved Game");
             Autodisp = 0;
          }
          Autotick = 0;
       }
    }
}

void clearfilenames(void)
{
	klistfree(finddirs);
	klistfree(findfiles);
	finddirs = findfiles = NULL;
	numfiles = numdirs = 0;
}

int getfilenames(char *path, char kind[])
{
	CACHE1D_FIND_REC *r;

	clearfilenames();
	pathsearchmode = 1;

    findfiles = klistpath(path,kind,CACHE1D_FIND_FILE);
	for (r = findfiles; r; r=r->next)
	     numfiles++;
	findfileshigh = findfiles;
	currentlist = 0;
	if (findfileshigh)
	    currentlist = 1;

	pathsearchmode = 0;
	return(0);
}

//--------------------------------- Random Music Selection --------------------------------

int GetRandom(short num)
{
    short i, j=0;
    time_t t;

    for (i=0; i<100; i++)
    {
        srand((unsigned) time(&t));
        j = rand()&num;
        if (j >= num)
            j = num - 1;
        if (j > 0 && j != iLast)
            break;
    }
    iLast = j;
    return  j;
}

int GetRandomMusic(void)
{
    char music_dir[51][32];
    short i, j, x=0;

    if (RandomMusic && LevelSong[0] == 0)
    {
       strcpy(tempbuf,"music/");
       getfilenames(tempbuf,"*.mid");
       while (findfiles)
       {
             Bsprintf(tempbuf,"music/%s",findfiles->name);
             strcpy(music_dir[x+1], tempbuf);
             x++;
             if (x > 50)
                 break;
             findfiles = findfiles->next;
       }

       if (x > 1)
       {
          j = GetRandom(x);
          strcpy(LevelSong, music_dir[j]);
          initprintf("Selected Random Music: %s\n", LevelSong);
          x = j;
       }
    }
    return x;
}

void LoadDefFiles(short iTyp)
{
    if (iTyp == 0)
    {
       if (!getdeffile("highres\\Sw.def"))
       {
          initprintf("Adding Definitions file: %s\n", "Sw.def");
          bDefFile = TRUE;
       }
       else
       if (!getdeffile("lowres\\Sw.def"))
       {
          initprintf("Adding Definitions file: %s\n", "Sw.def");
          bDefFile = TRUE;
       }
       else
       if (!getdeffile("Sw.def"))
       {
          initprintf("Adding Definitions file: %s\n", "Sw.def");
          bDefFile = TRUE;
       }
       if (!getdeffile("Swp.def"))
           initprintf("Adding Definitions file: %s\n", "Swp.def");
    }
    else
    if (!SW_SHAREWARE)
    {
       if (!getdeffile(custdeffile))
       {
          initprintf("Adding User Definitions file: %s\n", custdeffile);
       }
	   if (LoadCustomInfoFromScript(custtxtfile) == 0)
	   {
		  initprintf("Adding User SwCustom file: %s\n", custtxtfile);
	   }
       else
 	   if (LoadCustomInfoFromScript("SwCustom.txt") == 0)
 	   {
 	      initprintf("Adding SwCustom file: SwCustom.txt\n");
 	   }
    }
}

long SetupUserMap()
{
    long i = 0;
    char mp[80];
    char mt[10];

    SetLastPlayed(MenuMap, 1);

    mp[0] = 0;
    mt[0] = 0;
    if (MenuMusic[0] != 0)
        sprintf(mp,"-music %s%s", MusicFolder, MenuMusic);
    if (iMons == 0)  // no monsters
        strcpy(mt, " -monst");
    sprintf(ds, "Swp.exe -map %s%s -s%d %s%s", MapsFolder, MenuMap, iSkill, mp, mt);
    argvline = strdup(ds);
    i = SetCommandLine(argvline);
    return i;
}

long SetupUserGame(short gnum)
{
    long i;
    char mp[80];
    char mu[80];
    char sBuff[80];
    char gFol[80];
    char gn[40];

    i = strlen(MenuGame);   // .grp or .zip
    if (i > 4)
    {
       SetLastPlayed(MenuGame, 0);

       gn[0] = 0;
       if (gnum >= 0)
           sprintf(gn, " -game%d", gnum);

       gFol[0] = 0;
       if (GameFolder[0] != 0)
           sprintf(gFol," -j%s ", GameFolder);

       strcpy(sBuff, MenuGame);
       sBuff[i-4] = 0;
       mp[0] = 0;
       mu[0] = 0;

       if (strlen(GameMap) > 4)
       {
           if (MenuMusic[0] != 0)
               sprintf(mu," -music %s%s", MusicFolder, MenuMusic);
           sprintf(mp," -map %s -s%d%s", GameMap, iSkill, mu);
       }
       sprintf(ds, "Swp.exe %s-g%s -h%s.def -c%s.cfg%s%s", gFol, MenuGame, sBuff, sBuff, mp, gn);
       argvline = strdup(ds);
       i = SetCommandLine(argvline);
       return i;
    }
    return 0;
}

long SetupUserSavedGame(short snum)
{
    long i;

   if (MenuGame[0] != 0)
       return SetupUserGame(snum);

    sprintf(ds, "Swp.exe -game%d", snum);
    argvline = strdup(ds);
    i = SetCommandLine(argvline);
    return i;
}

long SetupUserLevel(short lnum)
{
   long i;

   sprintf(ds, "Swp.exe -level%d -s%d", lnum, iSkill);
   argvline = strdup(ds);
   i = SetCommandLine(argvline);
   return i;
}

long SetupMultiPlayer()
{
    long i;
    short iNuke = 0;
    char mbuf[80];
    char mmap[80];
    char mnet[80];

    mbuf[0] = 0;

    if (strstr(IPAddress, "127.0"))
    {
		Bsprintf(tempbuf,"%s","Opponent IP Address not correct!");
		wm_msgbox(apptitle, tempbuf);
        exit(0);
    }
    if (strstr(PlayerNameArg, "Player"))
    {
		Bsprintf(tempbuf,"%s","Name should be other than Player!");
		wm_msgbox(apptitle, tempbuf);
        exit(0);
    }
    if (HostMulti == -1)
    {
		Bsprintf(tempbuf,"%s","You must select to Host or to Join!");
		wm_msgbox(apptitle, tempbuf);
        exit(0);
    }

    if (iLevelNum > -1 && MenuGame[0] == 0)
    {
        sprintf(ds, "-level%d", iLevelNum+1);
        strcpy(mbuf, ds);
    }
    else
    if (MenuGame[0] != 0)   // .grp or .zip
    {
       char gFol[80];

       gFol[0] = 0;
       if (GameFolder[0] != 0)
           sprintf(gFol," -j%s ", GameFolder);

       strcpy(mnet, MenuGame);
       i = strlen(MenuGame);
       mnet[i-4] = 0;
       mmap[0] = 0;
       if (strlen(GameMap) > 4)
           sprintf(mmap," -map %s ", GameMap);
       sprintf(ds, "%s-g%s -h%s.def -c%s.cfg%s", gFol, MenuGame, mnet, mnet, mmap);
       strcpy(mbuf, ds);
    }
    else
    if (MenuMap[0] != 0)
    {
        sprintf(ds, "-map %s%s", MapsFolder, MenuMap);
        strcpy(mbuf, ds);
    }

    if (iType > 1)
        iNuke = 1;
    sprintf(ds, " -autonet %d,0,%d,1,0,0,%d,%d,%d,%d -col %d", iType, iMons, iFrag, iTime, iColr, iNuke, iColr);
    strcpy(mnet, ds);

    if (HostMulti == 1)
        sprintf(ds, "Swp.exe %s%s /name %s /net /n1 %s", mbuf, mnet, PlayerNameArg, IPAddress);
    else
        sprintf(ds, "Swp.exe %s /name %s -col %d /net %s /n1", mbuf, PlayerNameArg, iColr, IPAddress);

    argvline = strdup(ds);
    i = SetCommandLine(argvline);
    return i;
}

void MNU_DrawSmallNumbers(short x, short y, char *string, short shade, short pal)
{
    int ndx;
    char c;
    short ac, offset;

    offset = x;
    for (ndx = 0; (c = string[ndx]) != 0; ndx++)
    {
        if (pal == 14)
        {
            ac = c - '0' + 2945;
            if (ac < 2945 || ac > 2954)
                return;
        }
        else
        {
        ac = c - '0' + 3613;                // Grn=3601-3610 / Yel=3613-3622
        if (ac < 3613 || ac > 3622)
            return;
        }
        rotatesprite(offset<<16,y<<16, 32768,0,ac, shade, pal, MenuDrawFlags, 0, 0, xdim - 1, ydim - 1);
        offset += tilesizx[ac]-1;
    }
}

VOID DisplayMiniBarInv(PLAYERp pp, short xs, short ys, short pal, CHARp buffer)
{
    short size=3,x;
    CHARp ptr;
    PANEL_SPRITEp nsp;
    short pic;

    for (ptr = buffer, x = xs+10; *ptr; ptr++, x += size)
    {
        if (*ptr == ' ')
            continue;

        ASSERT(*ptr >= '!' && *ptr <= '}');

        pic = 2930 + (*ptr - '!');

        rotatesprite((long)x << 16, (long)ys << 16, 44000, 0, pic, 0, pal,
                     ROTATE_SPRITE_SCREEN_CLIP | ROTATE_SPRITE_CORNER, 0, 0, xdim - 1, ydim - 1);
    }
}

VOID UpdateMiniBarWeapons(PLAYERp pp, short Weapnum)
{
    short i, x, y, yy;
    int   weaps[10] = {1793, 1794, 1797, 1818, 1817, 2223, 1811, 1815, 1825, 0};
    int   wsize[10] = {22000,20000,14000,18000,20000,12000,18000,16000,19000,0};
    int   wp[10], wc[10];
    short xp[10] = {102,122,144,168,190,212,234,256,279,0};

    if (gs.MiniHudTyp != 1)
        return;

    if (Weapnum < 1 || Weapnum > 9)
        Weapnum = 0;

    for (i=0; i<10; i++)
        {
        wp[i] = 30;
        wc[i] = 11;
        if (TEST(pp->WpnFlags, BIT(i)))
           {
           if (pp->WpnAmmo[i] > 0)
              {
              wp[i] = 16;
              wc[i] = 13;
              }
           }
        }
    if  (Weapnum > 0 && Weapnum < 10)
        {
        wp[Weapnum] = 0;
        wc[Weapnum] = 17;
        }

    yy = 189; //184;
    // rotatesprite(long sx, long sy, long z, short a, short picnum, signed char dashade, char dapalnum, char dastat, long cx1, long cy1, long cx2, long cy2)
    // Star
    x  = 98;
    y = 184;
    if (useDarts)
        rotatesprite((x+3) << 16, y << 16, 8000, 0, 2519, wp[1], wc[1], 10+16, 0, 0, xdim - 1, ydim - 1);
    else
        rotatesprite(x << 16, y << 16, wsize[0], 0, weaps[0], wp[1], wc[1], 10+16, 0, 0, xdim - 1, ydim - 1);
    // Shotgun
    x = 115;
    y = 186;
        rotatesprite(x << 16, y << 16, wsize[1], 0, weaps[1], wp[2], wc[2], 10+16, 0, 0, xdim - 1, ydim - 1);
    // Uzi
    x = 139;
    y = 186;
        rotatesprite(x << 16, y << 16, wsize[2], 0,  weaps[2], wp[3], wc[3], 10+16, 0, 0, xdim - 1, ydim - 1);
    // RPG
    x = 162;
    y = 185;
        rotatesprite(x << 16, y << 16, wsize[3], 0, weaps[3], wp[4], wc[4], 10+16, 0, 0, xdim - 1, ydim - 1);
    // Grenade
    x = 184;
    y = 187;
        rotatesprite(x << 16, y << 16, wsize[4], 0, weaps[4], wp[5], wc[5], 10+16, 0, 0, xdim - 1, ydim - 1);
    // Sticky bomb
    x = 209;
    y = 186;
        rotatesprite(x << 16, y << 16, wsize[5], 0, weaps[5], wp[6], wc[6], 10+16, 0, 0, xdim - 1, ydim - 1);
    // Railgun
    x = 223;
    y = 186;
        rotatesprite(x << 16, y << 16, wsize[6], 0, weaps[6], wp[7], wc[7], 10+16, 0, 0, xdim - 1, ydim - 1);
    // Gorohead
    x = 253;
    y = 184;
        rotatesprite(x << 16, y << 16, wsize[7], 0, weaps[7], wp[8], wc[8], 10+16, 0, 0, xdim - 1, ydim - 1);
    // Heart
    x = 273;
    y = 185;
        rotatesprite(x << 16, y << 16, wsize[8], 0, weaps[8], wp[9], wc[9], 10+16, 0, 0, xdim - 1, ydim - 1);

    for (i=0; i<9; i++)
    {
         if (i == 3 && UsingMenus)
             sprintf(ds, "10");
         else
             sprintf(ds, "%d", pp->WpnAmmo[i+1]);
         x = 18;
         if (Weapnum == i+1)
             x = 0;
         MNU_DrawSmallNumbers(xp[i], 195, ds,  x, 0);
    }                                                                                   //10+1 = translucent
}

VOID UpdateAltMiniHud(PLAYERp pp)
{
    USERp u = User[pp->PlayerSprite];
    INVENTORY_DATAp id;
    long x = 0, y = 152;
    short i;
    extern int32 ScreenBPP;
    static short StatusKeyPics[] = { 2392, 2394, 2393, 2395, 2448, 2449, 2458, 2459 };


    x = 0;
    y = 200 - tilesizy[2472];

    if (gs.SetHighres == 0 || ScreenBPP == 8)
        pSpawnFullScreenSprite(pp, 2472, PRI_FRONT, x, y);
    else
        rotatesprite(x << 16, y << 16, (1 << 16), 0, 2472, 0, 21, 10+16, 0, 0, xdim - 1, ydim - 1);

    if (UsingMenus)
        return;

    y = 172;

    //display health amount
    x = 22;
    DisplayPanelNumber(pp, x, y, u->Health);

    // display ammo amount
    x = 198;
    if (u->WeaponNum != WPN_SWORD && u->WeaponNum != WPN_FIST)
        DisplayPanelNumber(pp, x, y, pp->WpnAmmo[u->WeaponNum]);
}

#include "gamepics.h"

void kloadhud(long *pic, long *bpl, long *xsiz, long *ysiz)
{
	char *buf;
	long leng = 964;

	(*pic) = 0;
/*
    buf = (char *)malloc(leng);
    if (!buf)
        return;
*/

    buf = Pic2472;

	kpgetdim(buf,leng,xsiz,ysiz);
	(*bpl) = ((*xsiz)<<2);
	(*pic) = (long)malloc((*ysiz)*(*bpl));
	if (!(*pic))
	{
	    // free(buf);
	    return;
	}
	if (kprender(buf,leng,*pic,*bpl,*xsiz,*ysiz,0,0) < 0)
	{
	    // free(buf);
	    free((void *)*pic);
	    (*pic) = 0;
	    return;
	}
	// free(buf);
}

void kloaduzi(long *pic, long *bpl, long *xsiz, long *ysiz)
{
	char *buf;
	long leng = 1292;

	(*pic) = 0;
/*
    buf = (char *)malloc(leng);
    if (!buf)
        return;
*/

    buf = Pic1798;

	kpgetdim(buf,leng,xsiz,ysiz);
	(*bpl) = ((*xsiz)<<2);
	(*pic) = (long)malloc((*ysiz)*(*bpl));
	if (!(*pic))
	{
	   // free(buf);
	    return;
	}
	if (kprender(buf,leng,*pic,*bpl,*xsiz,*ysiz,0,0) < 0)
	{
	    // free(buf);
	    free((void *)*pic);
	    (*pic) = 0;
	    return;
	}
	// free(buf);
}

void kloadriot(long *pic, long *bpl, long *xsiz, long *ysiz)
{
	char *buf;
	long leng = 1297;

	(*pic) = 0;
/*
    buf = (char *)malloc(leng);
    if (!buf)
        return;
*/

    buf = Pic1821;

	kpgetdim(buf,leng,xsiz,ysiz);
	(*bpl) = ((*xsiz)<<2);
	(*pic) = (long)malloc((*ysiz)*(*bpl));
	if (!(*pic))
	{
	   // free(buf);
	    return;
	}
	if (kprender(buf,leng,*pic,*bpl,*xsiz,*ysiz,0,0) < 0)
	{
	   // free(buf);
	    free((void *)*pic);
	    (*pic) = 0;
	    return;
	}
   // free(buf);
}

void CreatePics()
{
    int i, j, xsiz, ysiz, tile, *picptr;
    palette_t *col;

    tile = 2472; // alt minihud
    kloadhud((intptr_t *)&picptr, &j, &xsiz, &ysiz);
    faketiledata[tile] = Bmalloc(xsiz*ysiz);

    for (i=xsiz-1;i>=0;i--)
    {
        for (j=ysiz-1;j>=0;j--)
        {
            col = (palette_t *)&picptr[j*xsiz+i];
            if (col->f < 0)
            {
                faketiledata[tile][i*ysiz+j] = 255;  // transparent
                continue;
            }
            faketiledata[tile][i*ysiz+j] = getclosestcol(col->b>>2,col->g>>2,col->r>>2);
        }
     }

    tilesizx[tile] = xsiz;
    tilesizy[tile] = ysiz;
    faketile[tile] = 2;
    Bfree(picptr);

    tile = 1798; // alt uzi ammo
    kloaduzi((intptr_t *)&picptr, &j, &xsiz, &ysiz);
    faketiledata[tile] = Bmalloc(xsiz*ysiz);

    for (i=xsiz-1;i>=0;i--)
    {
        for (j=ysiz-1;j>=0;j--)
        {
            col = (palette_t *)&picptr[j*xsiz+i];
            if (col->f < 0)
            {
                faketiledata[tile][i*ysiz+j] = 255;  // transparent
                continue;
            }
            faketiledata[tile][i*ysiz+j] = getclosestcol(col->b>>2,col->g>>2,col->r>>2);
        }
     }

    tilesizx[tile] = xsiz;
    tilesizy[tile] = ysiz;
    faketile[tile] = 2;
    Bfree(picptr);

    tile = 1821; // alt riot ammo
    kloadriot((intptr_t *)&picptr, &j, &xsiz, &ysiz);
    faketiledata[tile] = Bmalloc(xsiz*ysiz);

    for (i=xsiz-1;i>=0;i--)
    {
        for (j=ysiz-1;j>=0;j--)
        {
            col = (palette_t *)&picptr[j*xsiz+i];
            if (col->f < 0)
            {
                faketiledata[tile][i*ysiz+j] = 255;  // transparent
                continue;
            }
            faketiledata[tile][i*ysiz+j] = getclosestcol(col->b>>2,col->g>>2,col->r>>2);
        }
     }

    tilesizx[tile] = xsiz;
    tilesizy[tile] = ysiz;
    faketile[tile] = 2;
    Bfree(picptr);
}

void GetUserMaps(void)
{
    short x = 0;

    NumUserMaps = 0;
    if (firstnet || AutoNet)
    {
        strcpy(tempbuf,"mapswb/");
        getfilenames(tempbuf,"*.map");
        if (numfiles > 1)
            goto domaps;
    }
    strcpy(tempbuf,MapsFolder);
    getfilenames(tempbuf,"*.map");

domaps:

    while (findfiles)
          {
          Bsprintf(tempbuf,"%s",findfiles->name);
          if (strlen(tempbuf) < 32)
             {
             if (x < 15)
                 strcpy(MyMaps[x], tempbuf);
             strcpy(UserMaps[x], tempbuf);
             x++;
             }
          if (x >= 1000)
              break;
          findfiles = findfiles->next;
          }
    NumUserMaps = x;
}

void ShowMultiMenu(void) // not used
{
    short w,h;

    //DrawStatScreen();
    DrawMenuLevelScreen();
    sprintf(ds,"Lo Wang is waiting for other players.");
    MNU_MeasureString(ds, &w, &h);
    MNU_DrawString(TEXT_TEST_COL(w), 160, ds, 1, 16);
    sprintf(ds,"Press Escape to Quit.");
    MNU_MeasureString(ds, &w, &h);
    MNU_DrawString(TEXT_TEST_COL(w), 170, ds, 1, 16);

    StopSound();
    nextpage();
}

void GetName(char *sPath, char *sTemp)
{
    if (strchr(sPath, '/') == 0)
    {
       if (strchr(sPath, '\\') == 0)
           strcpy(sTemp, sPath);
       else
           strcpy(sTemp, strrchr(sPath, '\\')+1);
    }
    else
    {
       strcpy(sTemp, strrchr(sPath, '/')+1);
    }

}

int GetUserMusic(void)
{
    short i, j, x = NumUserMusic;

    for (i=0; i<15; i++)
         strcpy(MyMusic[i], "");

    if (gs.PlayCD)
    {
        strcpy(MyMusic[0], "Current Track");
        for (i=2; i<16; i++)
        {
             Bsprintf(tempbuf, "Music Track: %d", i);
             strcpy(MyMusic[i-1], tempbuf);
        }
        NumUserMusic = 15;
        return 15;
    }

    if (x == 0)
    {
        strcpy(tempbuf, MusicFolder);
        getfilenames(tempbuf,"*.mid");
        while (findfiles)
        {
              Bsprintf(tempbuf,"%s%s",MusicFolder, findfiles->name);
              strcpy(UserMusic[x], tempbuf);
              x++;
              if (x > 300)
                  break;
              findfiles = findfiles->next;
        }

        if (x < 295)
        {
            strcpy(tempbuf, MusicFolder);
            getfilenames(tempbuf,"*.wav");
            while (findfiles)
               {
                  Bsprintf(tempbuf,"%s%s",MusicFolder, findfiles->name);
                  strcpy(UserMusic[x], tempbuf);
                  x++;
                  if (x > 300)
                      break;
                  findfiles = findfiles->next;
               }
        }

    }

    if (x > 1)
    {
        NumUserMusic = x;
        for (j=0; j<15; j++)
        {
             GetName(UserMusic[j], MyMusic[j]);
        }
    }

    return x;
}

int CheckAutosave()
{
    short i;

    if (Autosave > 0 && numplayers <= 1)
    {
       char fn[40];

       Bsprintf(fn, "%sgamex.asv", svgame);
       i = kopen4load(fn, 0);
	   if (i > 0)
       {
	      kclose(i);
	      return 1;
	   }
    }
    return 0;
}

int getdeffile(char *fn)
{
    if (NoHrp)
        return -1;
    else
        return loaddefinitionsfile(fn);
}

void checkforsaves(char *pth)                                        // 100303
{
    char tmp[40];
    char mp[6];
    int i;

    strcpy(tempbuf, pth);
    extractname(tempbuf);
    strcpy(tmp, tempbuf);
    if (tmp[0] == 0)
        strcpy(tmp, tempbuf);
    dolower(&tmp);
    if (!bGrp)
        svgame[0] = 0;
    if (strstr(tmp, ".grp") || strstr(tmp, ".zip") || (strstr(tmp, ".map") && !bGrp))
    {
        mp[0] = 0;
        if (strstr(tmp, ".map"))
            strcpy(mp, "map-");
        i = strlen(tmp);
        tmp[i-4] = 0;
        sprintf(svgame, "Saves/%s%s-", mp, tmp);
        CreateFolder("Saves");
    }
    LastSaveNum = 99;        // need for ingame maps
}

void extractname(char *pth)
{
    char tmp[128];
    int k;

    tmp[0] = 0;

    if (strchr(pth, '/') == 0)
    {
       if (strchr(pth, '\\') == 0)
           strcpy(tmp, pth);
       else
           strcpy(tmp, strrchr(pth, '\\')+1);
    }
    else
    {
       strcpy(tmp, strrchr(pth, '/')+1);
    }

    k = strlen(tmp);
    strcpy(pth, tmp);
    pth[k] = 0;
}

void dolower(char *cpath)                                            // 100303
{
    int i;
    char sTemp[128];
    char ch;

    for (i=0; i<128; i++)
         sTemp[i] = 0;

    strcpy(sTemp, cpath);
    for (i=0; i<strlen(sTemp)+1; i++)
    {
         ch = sTemp[i];
         if (ch > 0x40 && ch < 0x5b)   // A - Z
         {
             ch = ch ^ 0x20 ;
             sTemp[i] = ch;
         }
    }
    strcpy(cpath, sTemp);
}

BOOL CheckMapsPlayed(char *smap)                                     // 100128
{
    int i;

    for (i=0; i<2000; i++)
    {
         if (strstr(MapsPlayed[i], smap))
             return TRUE;
         if (i > 10 && strlen(MapsPlayed[i]) < 4)
             break;
    }
    return FALSE;
}

void SetMapPlayed(char *smap)
{
    char buf[80];
    int i, scH;

    if (MapsPlayed[0] == 0 || MapsPlayed[1] == 0)
        GetMapsPlayed();

    if (CheckMapsPlayed(smap))
        return;

    strcpy(MapsPlayed[Mapnum], smap);

    scH = SCRIPT_Init("SWP.mps");
	if (scH < 0)
	    return;

    for (i=0; i<Mapnum+1; i++)
    {
	    Bsprintf(buf,"Map_%d", i);
		SCRIPT_PutString(scH, "Maps Played", buf, MapsPlayed[i]);
    }

	SCRIPT_Save (scH, "SWP.mps");
	SCRIPT_Free (scH);
}

void GetMapsPlayed(void)
{
    char buf[80];
    char tmp[80];
    int i, scH = -1;

	if (SafeFileExists("SWP.mps"))
 	    scH = SCRIPT_Load("SWP.mps");

	if (scH < 0)
	    return;

    for (i=0; i<2000; i++)
    {
        Bsprintf(buf,"Map_%d",i);
        tmp[0] = 0;
        SCRIPT_GetString(scH, "Maps Played", buf, &tmp);
        if (strlen(tmp) < 4)
        {
            Mapnum = i;
            break;
        }
        dolower(&tmp);
        strcpy(MapsPlayed[i], tmp);
    }
    SCRIPT_Free (scH);
}
