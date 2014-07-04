// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.
//
// This file has been modified from Ken Silverman's original release
// by Jonathon Fowler (jonof@edgenetwk.com)

// Added Version + Revision                      wxvr
// Added default screen 1024x768                 wxsc
// Added Play map                                wxpy
// Added help and notepad                        wxhp
// Added 3 tile display sizes                    wxts
// Added color cBlak                             wxcl
// Added background colors                       wxbg
// Added show monster tiles in view textures     wxvc
// Added newboard map with room at startup       wxnm winlayer.c
// Added CheckSaveNewMap                         wxnm
// Added Display3DStats and 2D                   wxst
// Added caption display with map                wxcp
// --------------------------------------------------
// Version 1.1
// Added Alt Q - direct quit 3D                  wxaq
// Added long filesname                          wxlf
// Added space in filename = '_'                 wxlf
// ---------------------- 2009 ----------------------
// Version 1.2
// Fixed map saving                              wxas
// Version 1.21
// Added extra + fix debug in cache1d            wxer
// Version 1.3
// Removed all debugs in cache1d, else = 1.2
//

#include <process.h>                                                 // wxpy
#include <windows.h>

#include "build.h"
#include "compat.h"
#include "pragmas.h"
#include "osd.h"
#include "cache1d.h"
#include "editor.h"

#define VERSION "SWPBUILD - Version 1.3 beta 1"
#define REVISION "0"

#include "baselayer.h"
#ifdef RENDERTYPEWIN
#include "winlayer.h"
#endif

void CheckSelections(short iKey);
void PlayTestMap(void);
void SaveTestMap(int iforce);
void SWPBUILDHelp(void);
void BackupMap(void);
void SaveConfigs(void);
void Display2DStats(void);
void Display3DStats(void);
void DoMainCaption(void);
void ExtractFilename(char *filename);
int CheckAnimations(long iNum);
extern int GetMenuOption(void);

#define TIMERINTSPERSECOND 120

#define updatecrc16(crc,dat) (crc = (((crc<<8)&65535)^crctable[((((unsigned short)crc)>>8)&65535)^dat]))
static long crctable[256];
static char kensig[24];

int xsA = 128;   //64;                                               // wxts
int xsB = 127;   //63;
int xsC = 7;     //6

long vel, svel, angvel;

char buildkeys[NUMBUILDKEYS] =
{
	0xc8,0xd0,0xcb,0xcd,0x2a,0x9d,0x1d,0x39,
	0x1e,0x2c,0xd1,0xc9,0x33,0x34,
	0x9c,0x1c,0xd,0xc,0xf,0x45
};

long posx, posy, posz, horiz = 100;
long mousexsurp = 0, mouseysurp = 0;
short ang, cursectnum;
long hvel;

long grponlymode = 0;
extern long editorgridextent;	// in engine.c
extern double msens;

static long synctics = 0, lockclock = 0;
static long mytick = 0;

extern char vgacompatible;

extern char picsiz[MAXTILES];
extern long startposx, startposy, startposz;
extern short startang, startsectnum;
extern long frameplace, ydim16, halfxdim16, midydim16;
long xdim2d = 1024, ydim2d = 768, xdimgame = 1024, ydimgame = 768, bppgame = 8; // wxsc
long forcesetup = 1;

extern long cachesize, artsize;

static short oldmousebstatus = 0;
short brightness = 5;
long zlock = 0x7fffffff, zmode = 0, whitecol, kensplayerheight = 32;
short defaultspritecstat = 0;

static short localartfreq[MAXTILES];
static short localartlookup[MAXTILES], localartlookupnum;

char tempbuf[4096];

char names[MAXTILES][25];

short asksave = 0;
extern short editstatus, searchit;
extern long searchx, searchy;                          //search input
extern short searchsector, searchwall, searchstat;     //search output

extern short pointhighlight, linehighlight, highlightcnt;
short grid = 3, gridlock = 1, showtags = 1;
long zoom = 768, gettilezoom = 1;

long numsprites;
extern long mapversion;

short highlight[MAXWALLS];
short highlightsector[MAXSECTORS], highlightsectorcnt = -1;
extern char textfont[128][8];

static char pskysearch[MAXSECTORS];

short temppicnum, tempcstat, templotag, temphitag, tempextra;
char tempshade, temppal, tempvis, tempxrepeat, tempyrepeat;
char somethingintab = 255;

static char boardfilename[BMAX_PATH], selectedboardfilename[BMAX_PATH];
static CACHE1D_FIND_REC *finddirs=NULL, *findfiles=NULL, *finddirshigh=NULL, *findfileshigh=NULL;
static int numdirs=0, numfiles=0;
static int currentlist=0;

static long repeatcountx, repeatcounty;

static long fillist[640];

static char scantoasc[128] =
{
	0,0,'1','2','3','4','5','6','7','8','9','0','-','=',0,0,
	'q','w','e','r','t','y','u','i','o','p','[',']',0,0,'a','s',
	'd','f','g','h','j','k','l',';',39,'`',0,92,'z','x','c','v',
	'b','n','m',',','.','/',0,'*',0,32,0,0,0,0,0,0,
	0,0,0,0,0,0,0,'7','8','9','-','4','5','6','+','1',
	'2','3','0','.',0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};
static char scantoascwithshift[128] =
{
	0,0,'!','@','#','$','%','^','&','*','(',')','_','+',0,0,
	'Q','W','E','R','T','Y','U','I','O','P','{','}',0,0,'A','S',
	'D','F','G','H','J','K','L',':',34,'~',0,'|','Z','X','C','V',
	'B','N','M','<','>','?',0,'*',0,32,0,0,0,0,0,0,
	0,0,0,0,0,0,0,'7','8','9','-','4','5','6','+','1',
	'2','3','0','.',0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};



char changechar(char dachar, long dadir, char smooshyalign, char boundcheck);
long adjustmark(long *xplc, long *yplc, short danumwalls);
long checkautoinsert(long dax, long day, short danumwalls);
void keytimerstuff(void);
long clockdir(short wallstart);
void flipwalls(short numwalls, short newnumwalls);
void insertpoint(short linehighlight, long dax, long day);
void deletepoint(short point);
long deletesector(short sucksect);
long checksectorpointer(short i, short sectnum);
void fixrepeats(short i);
short loopinside(long x, long y, short startwall);
long fillsector(short sectnum, char fillcolor);
short whitelinescan(short dalinehighlight);
void printcoords16(long posxe, long posye, short ange);
void copysector(short soursector, short destsector, short deststartwall, char copystat);
void showsectordata(short sectnum);
void showwalldata(short wallnum);
void showspritedata(short spritenum);
long drawtilescreen(long pictopleft, long picbox);
void overheadeditor(void);
long getlinehighlight(long xplc, long yplc);
void fixspritesectors(void);
long movewalls(long start, long offs);
long loadnames(void);
void updatenumsprites(void);
void getclosestpointonwall(long x, long y, long dawall, long *nx, long *ny);
void initcrc(void);
void AutoAlignWalls(long nWall0, long ply);
long gettile(long tilenum);

long menuselect(void);
long getfilenames(char *path, char *kind);
void clearfilenames(void);

void clearkeys(void) { memset(keystatus,0,sizeof(keystatus)); }

char myMapname[32];                                                  // wxcp
char  quitflag;                                                      // wxyz
short cBlak = 0;                                                     // wxcl
short cBlue = 1;                                                     // wxcl
short cYell = 14;                                                    // wxcl
short cWhit = 15;                                                    // wxcl
short yellcol = 128;                                                 // wxcl
short sTileMax = 70;                                                 // wxvc
long iTile[70]  = {800,817,820,1094,1210,1300,1400,1441,1469,1580,3192,3780,4096,4162,4320,4550,5162,5426,
                  4590,4594,4600,4604,5023,5032,
                  1765,1766,1767,1769,1770,1771,1773,1774,1775,1777,1778,1779,
                  1846,1847,1850,1851,1852,1853,
                  1793,1794,1797,1799,1800,1807,1811,1812,1814,1817,1818,1823,1824,1831,1842,
                  1802,1803,1804,1805,1808,1809,1810,1813,1819,1829,3030,3031,9999};   // wxvc

char *sTile[70] = {"Hornet ", "Betty ", "Skull ", "Player ", "Sumo Boss ", "Serpent Boss ", // wxvc
                   "Coolie ", "Skeleton ", "Guardian ", "Ripper 1 ", "Fireball ", "Fish ",
                   "Ninja ", "Ninja Crawl ", "Ripper 2 ","Bunny ", "Girl Ninja ", "Zilla ",
                   "Mechanicgirl ", "Cargirl ", "Sailorgirl ", "Prunegirl ", "Toiletgirl ", "Washgirl ",
                   "Gold Skelkey ", "Blue Key ", "Blue Card ", "Silver Skelkey ", "Red Key ",
                   "Red Card ", "Bronze Skelkey ", "Green Key ", "Green Card ", "Red Skelkey ",
                   "Yellow Key ", "Yellow Card ",
                   "Skel Locked ", "Skel Unlocked ", "Ramcard Locked ", "Ramcard Unlocked ",
                   "Card Locked ", "Card Unlocked ",
                   "Shurikens ", "Riotgun ", "Uzi ", "Bullets ", "Rockets ", "Uzi Floor ",
                   "Rail Gun ", "Rail Ammo ", "Guard Head ", "Grenade Launcher ", "Micro Gun ",
                   "Shotshells ", "Heart ", "Grenades ", "Mines ",
                   "Health ", "Medkit ", "Smoke Bomb ", "Flashbomb ", "Gasbomb ", "Nuke ",
                   "Cookie ", "Repair Kit ", "Heat Card ", "Caltrops ", "Armor ", "Vision ", ""};

static int osdcmd_restartvid(const osdfuncparm_t *parm)
{
	extern long qsetmode;

	if (qsetmode != 200) return OSDCMD_OK;

	resetvideomode();
	if (setgamemode(fullscreen,xdim,ydim,bpp))
		OSD_Printf("restartvid: Reset failed...\n");

	return OSDCMD_OK;
}

static int osdcmd_vidmode(const osdfuncparm_t *parm)
{
	long newx = xdim, newy = ydim, newbpp = bpp, newfullscreen = fullscreen;
	extern long qsetmode;

	if (qsetmode != 200) return OSDCMD_OK;

	switch (parm->numparms) {
		case 1:	// bpp switch
			newbpp = Batol(parm->parms[0]);
			break;
		case 4:	// fs, res, bpp switch
			newfullscreen = (Batol(parm->parms[3]) != 0);
		case 3:	// res & bpp switch
			newbpp = Batol(parm->parms[2]);
		case 2: // res switch
			newx = Batol(parm->parms[0]);
			newy = Batol(parm->parms[1]);
			break;
		default: return OSDCMD_SHOWHELP;
	}

	if (setgamemode(newfullscreen,newx,newy,newbpp))
		OSD_Printf("vidmode: Mode change failed!\n");
	xdimgame = newx; ydimgame = newy; bppgame = newbpp; fullscreen = newfullscreen;
	return OSDCMD_OK;
}

extern int startwin_run(void);

static void CheckSaveNewMap()
{
	BFILE *fp;
	long   fl;

	char cBuff[195] =
	{
	7,0,0,0,247,250,255,255,10,2,0,0,0,230,255,255,15,0,0,0,1,0,
	0,0,4,0,0,168,255,255,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,2,0,4,0,0,249,255,255,0,254,255,255,1,
	0,255,255,255,255,0,0,0,0,0,0,0,0,18,8,0,0,0,0,0,0,0,0,0,2,0,
	0,0,254,255,255,2,0,255,255,255,255,0,0,0,0,0,0,0,0,16,8,0,0,
	0,0,0,0,0,0,0,2,0,0,0,6,0,0,3,0,255,255,255,255,0,0,0,0,0,0,
	0,0,18,8,0,0,0,0,0,0,0,0,0,249,255,255,0,6,0,0,0,0,255,255,255,
	255,0,0,0,0,0,0,0,0,16,8,0,0,0,0,0,0,0,0,0,0
	};

   	fp = Bfopen("newboard.map","rb");
	if (!fp)
       {
       fp = Bfopen("newboard.map","wb");
       Bfwrite(&cBuff, 194, 1, fp);
       }
    Bfclose(fp);
}

extern char *defsfilename;	// set in bstub.c
int app_main(int argc, const char **argv)
{
	//char ch, quitflag, cmdsetup = 0;
	char ch, cmdsetup = 0;
	int grpstoadd = 0, ky = 0;
	const char **grps = NULL;
	long i, j, k;

	OSD_SetLogFile("SWPBUILD.log");

	initprintf(VERSION "\n"); // wxvr
    initprintf("By ProAsm - http://www.proasm.com/\n");
    initprintf("Modified by Hendricks266 - http://hendricks266.duke4.net/\n\n");

	pathsearchmode = 1;		// unrestrict findfrompath so that full access to the filesystem can be had

#ifdef USE_OPENGL
	OSD_RegisterFunction("restartvid","restartvid: reinitialise the video mode",osdcmd_restartvid);
	OSD_RegisterFunction("vidmode","vidmode [xdim ydim] [bpp] [fullscreen]: immediately change the video mode",osdcmd_vidmode);
#endif

#ifdef RENDERTYPEWIN
	backgroundidle = 1;
#endif

	editstatus = 1;
	boardfilename[0] = 0;
	for (i=1; i<argc; i++)
	    {
		if (argv[i][0] == '-')
		   {
		   if (!strcmp(argv[i], "-setup"))
		       cmdsetup = 1;
		   else
		   if (!strcmp(argv[i], "-g") || !strcmp(argv[i], "-grp"))
		      {
			  i++;
			  grps = (const char**)realloc(grps, sizeof(const char*)*(grpstoadd+1));
			  grps[grpstoadd++] = argv[i];
			  }
		   else
		   if (!strcmp(argv[i], "-help") || !strcmp(argv[i], "--help") || !strcmp(argv[i], "-?"))
		      {
			  char *s =
			  "BUILD by Ken Silverman\n"
			  "Syntax: build [options] mapname\n"
			  "Options:\n"
			  "\t-grp\tUse an extra GRP or ZIP file.\n"
			  "\t-g\tSame as above.\n"
#if defined RENDERTYPEWIN || (defined RENDERTYPESDL && (defined __APPLE__ || defined HAVE_GTK2))
		      "\t-setup\tDisplays the configuration dialogue box before entering the editor.\n"
#endif
			  ;
#if defined RENDERTYPEWIN || (defined RENDERTYPESDL && (defined __APPLE__ || defined HAVE_GTK2))
		      wm_msgbox("BUILD by Ken Silverman",s);
#else
			  puts(s);
#endif
			  return 0;
			  }
		   continue;
		   }
		if (!boardfilename[0])
		   {
		   strncpy(boardfilename, argv[i], BMAX_PATH-4-1);
		   boardfilename[i-BMAX_PATH] = 0;
		   }
	    }


    CheckSaveNewMap();                                         // wxnm

    if (boardfilename[0] == 0) {
		Bstrcpy(boardfilename,"newboard.map");
	}
	else
	if (Bstrchr(boardfilename,'.') == 0)
	   {
		Bstrcat(boardfilename, ".map");
	   }

    wm_setapptitle(VERSION);

	//Bcanonicalisefilename(boardfilename,0);
	if ((i = ExtInit()) < 0)
	   return -1;
#if defined RENDERTYPEWIN || (defined RENDERTYPESDL && (defined __APPLE__ || defined HAVE_GTK2))
	if (i || forcesetup || cmdsetup)
	   {
		if (quitevent || !startwin_run()) return -1;
//       if (quitevent) return -1;                                    // wxyz
	   }
#endif

	DoMainCaption();

	if (grps && grpstoadd > 0)
	   {
	   for (i=0;i<grpstoadd;i++) {
			initprintf("Adding %s\n",grps[i]);
	   initgroupfile((char*)grps[i]);
	   }
    i = initgroupfile("Sw_Hrp.zip");                                 // wxhz
    if (i >= 0)
        initprintf("Adding Sw High Resolution Pack\n");
    free(grps);
	}

	inittimer(TIMERINTSPERSECOND);
	installusertimercallback(keytimerstuff);

	loadpics("tiles000.art",1048576);
	loadnames();

	Bstrcpy(kensig,"BUILD by Ken Silverman");     // main display
	initcrc();

	if (!loaddefinitionsfile(defsfilename))
	    initprintf("Definitions file loaded.\n");

	if (setgamemode(fullscreen,xdimgame,ydimgame,bppgame) < 0)
	   {
	   ExtUnInit();
	   uninitengine();
	   Bprintf("%ld * %ld not supported in this graphics mode\n",xdim,ydim);
	   exit(0);
	   }
	setbrightness(brightness,palette,0);

	k = 0;
	for(i=0; i<256; i++)
	   {
	   j = ((long)palette[i*3])+((long)palette[i*3+1])+((long)palette[i*3+2]);
	   if (j > k)
	      {
	      k = j;
	      whitecol = i;
	      }
	   }

	for(i=0;i<MAXSECTORS;i++) sector[i].extra = -1;
	for(i=0;i<MAXWALLS;i++) wall[i].extra = -1;
	for(i=0;i<MAXSPRITES;i++) sprite[i].extra = -1;

	ExtPreLoadMap();
	i = loadboard(boardfilename,(!pathsearchmode&&grponlymode?2:0),&posx,&posy,&posz,&ang,&cursectnum);
	if (i == -2)
	    i = loadoldboard(boardfilename,(!pathsearchmode&&grponlymode?2:0),&posx,&posy,&posz,&ang,&cursectnum);

    initprintf("Loaded map: %s\n", boardfilename);

	if (i < 0)
	{
		initspritelists();
		posx = 32768;
		posy = 32768;
		posz = 0;
		ang = 1536;
		numsectors = 0;
		numwalls = 0;
		cursectnum = -1;
		overheadeditor();
		keystatus[buildkeys[14]] = 0;
	}
	else
	{
		ExtLoadMap(boardfilename);
	}

	updatenumsprites();

	startposx = posx;
	startposy = posy;
	startposz = posz;
	startang = ang;
	startsectnum = cursectnum;

	totalclock = 0;

	quitflag = 0;
	while (quitflag == 0)
	{
		if (handleevents())
		   {
			if (quitevent)
			   {
				keystatus[1] = 1;
				quitevent = 0;
			   }
		   }

		OSD_DispatchQueued();
		ExtPreCheckKeys();

		drawrooms(posx,posy,posz,ang,horiz,cursectnum);
#ifdef SUPERBUILD
		ExtAnalyzeSprites();
#endif
		drawmasks();
		ExtCheckKeys();
		nextpage();
		synctics = totalclock-lockclock;
		lockclock += synctics;

		if (keystatus[0x10] > 0)          //Q  - 3D                  // wxaq
		   {
	       keystatus[0x10] = 0;
	       if (keystatus[0x38] > 0)       //Alt
	          {
              keystatus[0x38] = 0;        // Direct no save quit
              asksave = 0;
              quitflag = 1;
              break;
              }
           }
        ch = bgetchar();
		if (ch == 27)
		   {
	       keystatus[1] = 0;
           ky = GetMenuOption();          // 3D
//           ResetEvent(VK_ESCAPE);
           if (ky > 0)
              {
              CheckSelections(ky);        // 3D
              if (ky < 3)   // quit
                 {
                 if (ky == 2)
                     asksave = 0;
                 quitflag = 1;
                 break;
                 }
              }
           bflushchars();
		   }
	   }

	if (asksave)
	{
		if (ky == 1)
			{
			updatesector(startposx,startposy,&startsectnum);
			ExtPreSaveMap();
			saveboard(boardfilename,&startposx,&startposy,&startposz,&startang,&startsectnum);
			ExtSaveMap(boardfilename);
			}
	}

	clearfilenames();
	ExtUnInit();
	uninitengine();

	Bprintf("Memory status: %ld(%ld) bytes\n",cachesize,artsize);
	Bprintf("%s\n",kensig);
	return(0);
}

void showmouse(void)
{
	long i;

	for(i=1;i<=4;i++)
	{
		plotpixel(searchx+i,searchy,whitecol);
		plotpixel(searchx-i,searchy,whitecol);
		plotpixel(searchx,searchy-i,whitecol);
		plotpixel(searchx,searchy+i,whitecol);
	}
}

void editinput(void)  // 3D
{
	char smooshyalign, repeatpanalign, *ptr, buffer[80];
	short sectnum, nextsectnum, startwall, endwall, dasector, daang;
	long mousx, mousy, mousz, bstatus;
	long i, j, k, cnt, templong=0, doubvel, changedir, wallfind[2], daz[2];
	long dashade[2], goalz, xvect, yvect, hiz, loz;
	short hitsect, hitwall, hitsprite;
	long hitx, hity, hitz, dax, day, hihit, lohit;
    short ky = 0;

//    keystatus[1] = 0;

	if (keystatus[0x57] > 0)  //F11 - 3D brightness
	{
		keystatus[0x57] = 0;
		brightness++;
		if (brightness >= 16) brightness = 0;
		setbrightness(brightness,palette,0);
	}

     if (keystatus[0x58] > 0)   //F12 3D
		{
			keystatus[0x58] = 0;
			screencapture("captxxxx.tga",keystatus[0x2a]|keystatus[0x36]);
			showframe(1);
		}

	mousz = 0;
	getmousevalues(&mousx,&mousy,&bstatus);
	mousx = (mousx<<16)+mousexsurp;
	mousy = (mousy<<16)+mouseysurp;
	{
	ldiv_t ld;
	ld = ldiv((long)((double)mousx*msens), (1<<16)); mousx = ld.quot; mousexsurp = ld.rem;
	ld = ldiv((long)((double)mousy*msens), (1<<16)); mousy = ld.quot; mouseysurp = ld.rem;
	}
	searchx += mousx;
	searchy += mousy;
	if (searchx < 4) searchx = 4;
	if (searchy < 4) searchy = 4;
	if (searchx > xdim-5) searchx = xdim-5;
	if (searchy > ydim-5) searchy = ydim-5;
	showmouse();

	if (keystatus[0x3b] > 0) posx--;
	if (keystatus[0x3c] > 0) posx++;
	if (keystatus[0x3d] > 0) posy--;
	if (keystatus[0x3e] > 0) posy++;
	if (keystatus[0x43] > 0) ang--;
	if (keystatus[0x44] > 0) ang++;

	if (angvel != 0)          //ang += angvel * constant
	{                         //ENGINE calculates angvel for you
		doubvel = synctics;
		if (keystatus[buildkeys[4]] > 0)  //Lt. shift makes turn velocity 50% faster
			doubvel += (synctics>>1);
		ang += ((angvel*doubvel)>>4);
		ang = (ang+2048)&2047;
	}
	if ((vel|svel) != 0)
	{
		doubvel = synctics;
		if (keystatus[buildkeys[4]] > 0)     //Lt. shift doubles forward velocity
			doubvel += synctics;
		xvect = 0, yvect = 0;
		if (vel != 0)
		{
			xvect += ((vel*doubvel*(long)sintable[(ang+2560)&2047])>>3);
			yvect += ((vel*doubvel*(long)sintable[(ang+2048)&2047])>>3);
		}
		if (svel != 0)
		{
			xvect += ((svel*doubvel*(long)sintable[(ang+2048)&2047])>>3);
			yvect += ((svel*doubvel*(long)sintable[(ang+1536)&2047])>>3);
		}
		clipmove(&posx,&posy,&posz,&cursectnum,xvect,yvect,128L,4L<<8,4L<<8,CLIPMASK0);
	}
	getzrange(posx,posy,posz,cursectnum,&hiz,&hihit,&loz,&lohit,128L,CLIPMASK0);

	if (keystatus[0x3a] > 0)
	{
		zmode++;
		if (zmode == 3) zmode = 0;
		if (zmode == 1) zlock = (loz-posz)&0xfffffc00;
		keystatus[0x3a] = 0;
	}

	if (zmode == 0)
	{
		goalz = loz-(kensplayerheight<<8);   //playerheight pixels above floor
		if (goalz < hiz+(16<<8))   //ceiling&floor too close
			goalz = ((loz+hiz)>>1);
		goalz += mousz;
		if (keystatus[buildkeys[8]] > 0)                            //A (stand high)
		{
			if (keystatus[0x1d] > 0)
				horiz = max(-100,horiz-((keystatus[buildkeys[4]]+1)*synctics*2));
			else
			{
				goalz -= (16<<8);
				if (keystatus[buildkeys[4]] > 0)    //Either shift key
					goalz -= (24<<8);
			}
		}
		if (keystatus[buildkeys[9]] > 0)                            //Z (stand low)
		{
			if (keystatus[0x1d] > 0)
				horiz = min(300,horiz+((keystatus[buildkeys[4]]+1)*synctics*2));
			else
			{
				goalz += (12<<8);
				if (keystatus[buildkeys[4]] > 0)    //Either shift key
					goalz += (12<<8);
			}
		}

		if (goalz != posz)
		{
			if (posz < goalz) hvel += 32;
			if (posz > goalz) hvel = ((goalz-posz)>>3);

			posz += hvel;
			if (posz > loz-(4<<8)) posz = loz-(4<<8), hvel = 0;
			if (posz < hiz+(4<<8)) posz = hiz+(4<<8), hvel = 0;
		}
	}
	else
	{
		goalz = posz;
		if (keystatus[buildkeys[8]] > 0)                            //A
		{
			if (keystatus[0x1d] > 0) {
				horiz = max(-100,horiz-((keystatus[buildkeys[4]]+1)*synctics*2));
			} else {
				if (zmode != 1)
					goalz -= (8<<8);
				else
				{
					zlock += (4<<8);
					keystatus[buildkeys[8]] = 0;
				}
			}
		}
		if (keystatus[buildkeys[9]] > 0)                            //Z (stand low)
		{
			if (keystatus[0x1d] > 0) {
				horiz = min(300,horiz+((keystatus[buildkeys[4]]+1)*synctics*2));
			} else {
				if (zmode != 1)
					goalz += (8<<8);
				else if (zlock > 0)
				{
					zlock -= (4<<8);
					keystatus[buildkeys[9]] = 0;
				}
			}
		}

		if (goalz < hiz+(4<<8)) goalz = hiz+(4<<8);
		if (goalz > loz-(4<<8)) goalz = loz-(4<<8);
		if (zmode == 1) goalz = loz-zlock;
		if (goalz < hiz+(4<<8)) goalz = ((loz+hiz)>>1);  //ceiling&floor too close
		if (zmode == 1) posz = goalz;

		if (goalz != posz)
		{
			//if (posz < goalz) hvel += (32<<keystatus[buildkeys[4]]);
			//if (posz > goalz) hvel -= (32<<keystatus[buildkeys[4]]);
			if (posz < goalz) hvel = ((synctics* 192)<<keystatus[buildkeys[4]]);
			if (posz > goalz) hvel = ((synctics*-192)<<keystatus[buildkeys[4]]);

			posz += hvel;

			if (posz > loz-(4<<8)) posz = loz-(4<<8), hvel = 0;
			if (posz < hiz+(4<<8)) posz = hiz+(4<<8), hvel = 0;
		}
		else
			hvel = 0;
	}

	searchit = 2;
	if (searchstat >= 0)
	{
		if ((bstatus&1) > 0)
			searchit = 0;
		if (keystatus[0x4a] > 0)  // -
		{
			keystatus[0x4a] = 0;
			if ((keystatus[0x38]|keystatus[0xb8]) > 0)  //ALT
			{
				if ((keystatus[0x1d]|keystatus[0x9d]) > 0)  //CTRL
				{
					if (visibility < 16384) visibility += visibility;
				}
				else
				{
					if ((keystatus[0x2a]|keystatus[0x36]) == 0)
						k = 16; else k = 1;

					if (highlightsectorcnt >= 0)
						for(i=0;i<highlightsectorcnt;i++)
							if (highlightsector[i] == searchsector)
							{
								while (k > 0)
								{
									for(i=0;i<highlightsectorcnt;i++)
									{
										sector[highlightsector[i]].visibility++;
										if (sector[highlightsector[i]].visibility == 240)
											sector[highlightsector[i]].visibility = 239;
									}
									k--;
								}
								break;
							}
					while (k > 0)
					{
						sector[searchsector].visibility++;
						if (sector[searchsector].visibility == 240)
							sector[searchsector].visibility = 239;
						k--;
					}
					asksave = 1;
				}
			}
			else
			{
				k = 0;
				if (highlightsectorcnt >= 0)
				{
					for(i=0;i<highlightsectorcnt;i++)
						if (highlightsector[i] == searchsector)
						{
							k = 1;
							break;
						}
				}

				if (k == 0)
				{
					if (searchstat == 0) wall[searchwall].shade++;
					if (searchstat == 1) sector[searchsector].ceilingshade++;
					if (searchstat == 2) sector[searchsector].floorshade++;
					if (searchstat == 3) sprite[searchwall].shade++;
					if (searchstat == 4) wall[searchwall].shade++;
				}
				else
				{
					for(i=0;i<highlightsectorcnt;i++)
					{
						dasector = highlightsector[i];

						sector[dasector].ceilingshade++;        //sector shade
						sector[dasector].floorshade++;

						startwall = sector[dasector].wallptr;   //wall shade
						endwall = startwall + sector[dasector].wallnum - 1;
						for(j=startwall;j<=endwall;j++)
							wall[j].shade++;

						j = headspritesect[dasector];           //sprite shade
						while (j != -1)
						{
							sprite[j].shade++;
							j = nextspritesect[j];
						}
					}
				}
				asksave = 1;
			}
		}
		if (keystatus[0x4e] > 0)  // +
		{
			keystatus[0x4e] = 0;
			if ((keystatus[0x38]|keystatus[0xb8]) > 0)  //ALT
			{
				if ((keystatus[0x1d]|keystatus[0x9d]) > 0)  //CTRL
				{
					if (visibility > 32) visibility >>= 1;
				}
				else
				{
					if ((keystatus[0x2a]|keystatus[0x36]) == 0)
						k = 16; else k = 1;

					if (highlightsectorcnt >= 0)
						for(i=0;i<highlightsectorcnt;i++)
							if (highlightsector[i] == searchsector)
							{
								while (k > 0)
								{
									for(i=0;i<highlightsectorcnt;i++)
									{
										sector[highlightsector[i]].visibility--;
										if (sector[highlightsector[i]].visibility == 239)
											sector[highlightsector[i]].visibility = 240;
									}
									k--;
								}
								break;
							}
					while (k > 0)
					{
						sector[searchsector].visibility--;
						if (sector[searchsector].visibility == 239)
							sector[searchsector].visibility = 240;
						k--;
					}
					asksave = 1;
				}
			}
			else
			{
				k = 0;
				if (highlightsectorcnt >= 0)
				{
					for(i=0;i<highlightsectorcnt;i++)
						if (highlightsector[i] == searchsector)
						{
							k = 1;
							break;
						}
				}

				if (k == 0)
				{
					if (searchstat == 0) wall[searchwall].shade--;
					if (searchstat == 1) sector[searchsector].ceilingshade--;
					if (searchstat == 2) sector[searchsector].floorshade--;
					if (searchstat == 3) sprite[searchwall].shade--;
					if (searchstat == 4) wall[searchwall].shade--;
				}
				else
				{
					for(i=0;i<highlightsectorcnt;i++)
					{
						dasector = highlightsector[i];

						sector[dasector].ceilingshade--;        //sector shade
						sector[dasector].floorshade--;

						startwall = sector[dasector].wallptr;   //wall shade
						endwall = startwall + sector[dasector].wallnum - 1;
						for(j=startwall;j<=endwall;j++)
							wall[j].shade--;

						j = headspritesect[dasector];           //sprite shade
						while (j != -1)
						{
							sprite[j].shade--;
							j = nextspritesect[j];
						}
					}
				}
				asksave = 1;
			}
		}
		if (keystatus[0xc9] > 0) // PGUP
		{
			k = 0;
			if (highlightsectorcnt >= 0)
			{
				for(i=0;i<highlightsectorcnt;i++)
					if (highlightsector[i] == searchsector)
					{
						k = 1;
						break;
					}
			}

			if ((searchstat == 0) || (searchstat == 1))
			{
				if (k == 0)
				{
					i = headspritesect[searchsector];
					while (i != -1)
					{
						templong = getceilzofslope(searchsector,sprite[i].x,sprite[i].y);
						templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<2);
						if (sprite[i].cstat&128) templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<1);
						if (sprite[i].z == templong)
							sprite[i].z -= 1024 << ((keystatus[0x1d]|keystatus[0x9d])<<1);	// JBF 20031128
						i = nextspritesect[i];
					}
					sector[searchsector].ceilingz -= 1024 << ((keystatus[0x1d]|keystatus[0x9d])<<1);	// JBF 20031128
				}
				else
				{
					for(j=0;j<highlightsectorcnt;j++)
					{
						i = headspritesect[highlightsector[j]];
						while (i != -1)
						{
							templong = getceilzofslope(highlightsector[j],sprite[i].x,sprite[i].y);
							templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<2);
							if (sprite[i].cstat&128) templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<1);
							if (sprite[i].z == templong)
								sprite[i].z -= 1024 << ((keystatus[0x1d]|keystatus[0x9d])<<1);	// JBF 20031128
							i = nextspritesect[i];
						}
						sector[highlightsector[j]].ceilingz -= 1024 << ((keystatus[0x1d]|keystatus[0x9d])<<1);	// JBF 20031128
					}
				}
			}
			if (searchstat == 2)
			{
				if (k == 0)
				{
					i = headspritesect[searchsector];
					while (i != -1)
					{
						templong = getflorzofslope(searchsector,sprite[i].x,sprite[i].y);
						if (sprite[i].cstat&128) templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<1);
						if (sprite[i].z == templong)
							sprite[i].z -= 1024 << ((keystatus[0x1d]|keystatus[0x9d])<<1);	// JBF 20031128
						i = nextspritesect[i];
					}
					sector[searchsector].floorz -= 1024 << ((keystatus[0x1d]|keystatus[0x9d])<<1);	// JBF 20031128
				}
				else
				{
					for(j=0;j<highlightsectorcnt;j++)
					{
						i = headspritesect[highlightsector[j]];
						while (i != -1)
						{
							templong = getflorzofslope(highlightsector[j],sprite[i].x,sprite[i].y);
							if (sprite[i].cstat&128) templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<1);
							if (sprite[i].z == templong)
								sprite[i].z -= 1024 << ((keystatus[0x1d]|keystatus[0x9d])<<1);	// JBF 20031128
							i = nextspritesect[i];
						}
						sector[highlightsector[j]].floorz -= 1024 << ((keystatus[0x1d]|keystatus[0x9d])<<1);	// JBF 20031128
					}
				}
			}
			if (sector[searchsector].floorz < sector[searchsector].ceilingz)
				sector[searchsector].floorz = sector[searchsector].ceilingz;
			if (searchstat == 3)
			{
				if ((keystatus[0x1d]|keystatus[0x9d]) > 0)  //CTRL - put sprite on ceiling
				{
					sprite[searchwall].z = getceilzofslope(searchsector,sprite[searchwall].x,sprite[searchwall].y);
					if (sprite[searchwall].cstat&128) sprite[searchwall].z -= ((tilesizy[sprite[searchwall].picnum]*sprite[searchwall].yrepeat)<<1);
					if ((sprite[searchwall].cstat&48) != 32)
						sprite[searchwall].z += ((tilesizy[sprite[searchwall].picnum]*sprite[searchwall].yrepeat)<<2);
				}
				else
				{
					k = 0;
					if (highlightcnt >= 0)
						for(i=0;i<highlightcnt;i++)
							if (highlight[i] == searchwall+16384)
							{
								k = 1;
								break;
							}

					if (k == 0)
						sprite[searchwall].z -= (4<<8);
					else
					{
						for(i=0;i<highlightcnt;i++)
							if ((highlight[i]&0xc000) == 16384)
								sprite[highlight[i]&16383].z -= (4<<8);
					}
				}
			}
			asksave = 1;
			keystatus[0xc9] = 0;
		}
		if (keystatus[0xd1] > 0) // PGDN
		{
			k = 0;
			if (highlightsectorcnt >= 0)
			{
				for(i=0;i<highlightsectorcnt;i++)
					if (highlightsector[i] == searchsector)
					{
						k = 1;
						break;
					}
			}

			if ((searchstat == 0) || (searchstat == 1))
			{
				if (k == 0)
				{
					i = headspritesect[searchsector];
					while (i != -1)
					{
						templong = getceilzofslope(searchsector,sprite[i].x,sprite[i].y);
						if (sprite[i].cstat&128) templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<1);
						templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<2);
						if (sprite[i].z == templong)
							sprite[i].z += 1024 << ((keystatus[0x1d]|keystatus[0x9d])<<1);	// JBF 20031128
						i = nextspritesect[i];
					}
					sector[searchsector].ceilingz += 1024 << ((keystatus[0x1d]|keystatus[0x9d])<<1);	// JBF 20031128
				}
				else
				{
					for(j=0;j<highlightsectorcnt;j++)
					{
						i = headspritesect[highlightsector[j]];
						while (i != -1)
						{
							templong = getceilzofslope(highlightsector[j],sprite[i].x,sprite[i].y);
							if (sprite[i].cstat&128) templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<1);
							templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<2);
							if (sprite[i].z == templong)
								sprite[i].z += 1024 << ((keystatus[0x1d]|keystatus[0x9d])<<1);	// JBF 20031128
							i = nextspritesect[i];
						}
						sector[highlightsector[j]].ceilingz += 1024 << ((keystatus[0x1d]|keystatus[0x9d])<<1);	// JBF 20031128
					}
				}
			}
			if (searchstat == 2)
			{
				if (k == 0)
				{
					i = headspritesect[searchsector];
					while (i != -1)
					{
						templong = getflorzofslope(searchsector,sprite[i].x,sprite[i].y);
						if (sprite[i].cstat&128) templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<1);
						if (sprite[i].z == templong)
							sprite[i].z += 1024 << ((keystatus[0x1d]|keystatus[0x9d])<<1);	// JBF 20031128
						i = nextspritesect[i];
					}
					sector[searchsector].floorz += 1024 << ((keystatus[0x1d]|keystatus[0x9d])<<1);	// JBF 20031128
				}
				else
				{
					for(j=0;j<highlightsectorcnt;j++)
					{
						i = headspritesect[highlightsector[j]];
						while (i != -1)
						{
							templong = getflorzofslope(highlightsector[j],sprite[i].x,sprite[i].y);
							if (sprite[i].cstat&128) templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<1);
							if (sprite[i].z == templong)
								sprite[i].z += 1024 << ((keystatus[0x1d]|keystatus[0x9d])<<1);	// JBF 20031128
							i = nextspritesect[i];
						}
						sector[highlightsector[j]].floorz += 1024 << ((keystatus[0x1d]|keystatus[0x9d])<<1);	// JBF 20031128
					}
				}
			}
			if (sector[searchsector].ceilingz > sector[searchsector].floorz)
				sector[searchsector].ceilingz = sector[searchsector].floorz;
			if (searchstat == 3)
			{
				if ((keystatus[0x1d]|keystatus[0x9d]) > 0)  //CTRL - put sprite on ground
				{
					sprite[searchwall].z = getflorzofslope(searchsector,sprite[searchwall].x,sprite[searchwall].y);
					if (sprite[searchwall].cstat&128) sprite[searchwall].z -= ((tilesizy[sprite[searchwall].picnum]*sprite[searchwall].yrepeat)<<1);
				}
				else
				{
					k = 0;
					if (highlightcnt >= 0)
						for(i=0;i<highlightcnt;i++)
							if (highlight[i] == searchwall+16384)
							{
								k = 1;
								break;
							}

					if (k == 0)
						sprite[searchwall].z += (4<<8);
					else
					{
						for(i=0;i<highlightcnt;i++)
							if ((highlight[i]&0xc000) == 16384)
								sprite[highlight[i]&16383].z += (4<<8);
					}
				}
			}
			asksave = 1;
			keystatus[0xd1] = 0;
		}
		if (keystatus[0x0f] > 0)  //TAB
		{
			if (searchstat == 0)
			{
				temppicnum = wall[searchwall].picnum;
				tempshade = wall[searchwall].shade;
				temppal = wall[searchwall].pal;
				tempxrepeat = wall[searchwall].xrepeat;
				tempyrepeat = wall[searchwall].yrepeat;
				tempcstat = wall[searchwall].cstat;
				templotag = wall[searchwall].lotag;
				temphitag = wall[searchwall].hitag;
				tempextra = wall[searchwall].extra;
			}
			if (searchstat == 1)
			{
				temppicnum = sector[searchsector].ceilingpicnum;
				tempshade = sector[searchsector].ceilingshade;
				temppal = sector[searchsector].ceilingpal;
				tempvis = sector[searchsector].visibility;
				tempxrepeat = sector[searchsector].ceilingxpanning;
				tempyrepeat = sector[searchsector].ceilingypanning;
				tempcstat = sector[searchsector].ceilingstat;
				templotag = sector[searchsector].lotag;
				temphitag = sector[searchsector].hitag;
				tempextra = sector[searchsector].extra;
			}
			if (searchstat == 2)
			{
				temppicnum = sector[searchsector].floorpicnum;
				tempshade = sector[searchsector].floorshade;
				temppal = sector[searchsector].floorpal;
				tempvis = sector[searchsector].visibility;
				tempxrepeat = sector[searchsector].floorxpanning;
				tempyrepeat = sector[searchsector].floorypanning;
				tempcstat = sector[searchsector].floorstat;
				templotag = sector[searchsector].lotag;
				temphitag = sector[searchsector].hitag;
				tempextra = sector[searchsector].extra;
			}
			if (searchstat == 3)
			{
				temppicnum = sprite[searchwall].picnum;
				tempshade = sprite[searchwall].shade;
				temppal = sprite[searchwall].pal;
				tempxrepeat = sprite[searchwall].xrepeat;
				tempyrepeat = sprite[searchwall].yrepeat;
				tempcstat = sprite[searchwall].cstat;
				templotag = sprite[searchwall].lotag;
				temphitag = sprite[searchwall].hitag;
				tempextra = sprite[searchwall].extra;
			}
			if (searchstat == 4)
			{
				temppicnum = wall[searchwall].overpicnum;
				tempshade = wall[searchwall].shade;
				temppal = wall[searchwall].pal;
				tempxrepeat = wall[searchwall].xrepeat;
				tempyrepeat = wall[searchwall].yrepeat;
				tempcstat = wall[searchwall].cstat;
				templotag = wall[searchwall].lotag;
				temphitag = wall[searchwall].hitag;
				tempextra = wall[searchwall].extra;
			}
			somethingintab = searchstat;
			keystatus[0x0f] = 0;
		}
		if (keystatus[0x1c] > 0) //Left ENTER
		{
			if ((keystatus[0x2a]|keystatus[0x36]) > 0)       //Either shift key
			{
				if (((searchstat == 0) || (searchstat == 4)) && ((keystatus[0x1d]|keystatus[0x9d]) > 0))  //Ctrl-shift Enter (auto-shade)
				{
					dashade[0] = 127;
					dashade[1] = -128;
					i = searchwall;
					do
					{
						if ((long)wall[i].shade < dashade[0]) dashade[0] = wall[i].shade;
						if ((long)wall[i].shade > dashade[1]) dashade[1] = wall[i].shade;

						i = wall[i].point2;
					}
					while (i != searchwall);

					daang = getangle(wall[wall[searchwall].point2].x-wall[searchwall].x,wall[wall[searchwall].point2].y-wall[searchwall].y);
					i = searchwall;
					do
					{
						j = getangle(wall[wall[i].point2].x-wall[i].x,wall[wall[i].point2].y-wall[i].y);
						k = ((j+2048-daang)&2047);
						if (k > 1024)
							k = 2048-k;
						wall[i].shade = dashade[0]+mulscale10(k,dashade[1]-dashade[0]);

						i = wall[i].point2;
					}
					while (i != searchwall);
				}
				else if (somethingintab < 255)
				{
					if (searchstat == 0) wall[searchwall].shade = tempshade, wall[searchwall].pal = temppal;
					if (searchstat == 1)
					{
						sector[searchsector].ceilingshade = tempshade, sector[searchsector].ceilingpal = temppal;
						if ((somethingintab == 1) || (somethingintab == 2))
							sector[searchsector].visibility = tempvis;
					}
					if (searchstat == 2)
					{
						sector[searchsector].floorshade = tempshade, sector[searchsector].floorpal = temppal;
						if ((somethingintab == 1) || (somethingintab == 2))
							sector[searchsector].visibility = tempvis;
					}
					if (searchstat == 3) sprite[searchwall].shade = tempshade, sprite[searchwall].pal = temppal;
					if (searchstat == 4) wall[searchwall].shade = tempshade, wall[searchwall].pal = temppal;
				}
			}
			else if (((searchstat == 0) || (searchstat == 4)) && ((keystatus[0x1d]|keystatus[0x9d]) > 0) && (somethingintab < 255))  //Either ctrl key
			{
				i = searchwall;
				do
				{
					wall[i].picnum = temppicnum;
					wall[i].shade = tempshade;
					wall[i].pal = temppal;
					if ((somethingintab == 0) || (somethingintab == 4))
					{
						wall[i].xrepeat = tempxrepeat;
						wall[i].yrepeat = tempyrepeat;
						wall[i].cstat = tempcstat;
					}
					fixrepeats((short)i);
					i = wall[i].point2;
				}
				while (i != searchwall);
			}
			else if (((searchstat == 1) || (searchstat == 2)) && ((keystatus[0x1d]|keystatus[0x9d]) > 0) && (somethingintab < 255))  //Either ctrl key
			{
				clearbuf(&pskysearch[0],(long)((numsectors+3)>>2),0L);
				if (searchstat == 1)
				{
					i = searchsector;
					if ((sector[i].ceilingstat&1) > 0)
						pskysearch[i] = 1;

					while (pskysearch[i] == 1)
					{
						sector[i].ceilingpicnum = temppicnum;
						sector[i].ceilingshade = tempshade;
						sector[i].ceilingpal = temppal;
						if ((somethingintab == 1) || (somethingintab == 2))
						{
							sector[i].ceilingxpanning = tempxrepeat;
							sector[i].ceilingypanning = tempyrepeat;
							sector[i].ceilingstat = tempcstat;
						}
						pskysearch[i] = 2;

						startwall = sector[i].wallptr;
						endwall = startwall + sector[i].wallnum - 1;
						for(j=startwall;j<=endwall;j++)
						{
							k = wall[j].nextsector;
							if (k >= 0)
								if ((sector[k].ceilingstat&1) > 0)
									if (pskysearch[k] == 0)
										pskysearch[k] = 1;
						}

						for(j=0;j<numsectors;j++)
							if (pskysearch[j] == 1)
								i = j;
					}
				}
				if (searchstat == 2)
				{
					i = searchsector;
					if ((sector[i].floorstat&1) > 0)
						pskysearch[i] = 1;

					while (pskysearch[i] == 1)
					{
						sector[i].floorpicnum = temppicnum;
						sector[i].floorshade = tempshade;
						sector[i].floorpal = temppal;
						if ((somethingintab == 1) || (somethingintab == 2))
						{
							sector[i].floorxpanning = tempxrepeat;
							sector[i].floorypanning = tempyrepeat;
							sector[i].floorstat = tempcstat;
						}
						pskysearch[i] = 2;

						startwall = sector[i].wallptr;
						endwall = startwall + sector[i].wallnum - 1;
						for(j=startwall;j<=endwall;j++)
						{
							k = wall[j].nextsector;
							if (k >= 0)
								if ((sector[k].floorstat&1) > 0)
									if (pskysearch[k] == 0)
										pskysearch[k] = 1;
						}

						for(j=0;j<numsectors;j++)
							if (pskysearch[j] == 1)
								i = j;
					}
				}
			}
			else if (somethingintab < 255)
			{
				if (searchstat == 0)
				{
					wall[searchwall].picnum = temppicnum;
					wall[searchwall].shade = tempshade;
					wall[searchwall].pal = temppal;
					if (somethingintab == 0)
					{
						wall[searchwall].xrepeat = tempxrepeat;
						wall[searchwall].yrepeat = tempyrepeat;
						wall[searchwall].cstat = tempcstat;
						wall[searchwall].lotag = templotag;
						wall[searchwall].hitag = temphitag;
						wall[searchwall].extra = tempextra;
					}
					fixrepeats(searchwall);
				}
				if (searchstat == 1)
				{
					sector[searchsector].ceilingpicnum = temppicnum;
					sector[searchsector].ceilingshade = tempshade;
					sector[searchsector].ceilingpal = temppal;
					if ((somethingintab == 1) || (somethingintab == 2))
					{
						sector[searchsector].ceilingxpanning = tempxrepeat;
						sector[searchsector].ceilingypanning = tempyrepeat;
						sector[searchsector].ceilingstat = tempcstat;
						sector[searchsector].visibility = tempvis;
						sector[searchsector].lotag = templotag;
						sector[searchsector].hitag = temphitag;
						sector[searchsector].extra = tempextra;
					}
				}
				if (searchstat == 2)
				{
					sector[searchsector].floorpicnum = temppicnum;
					sector[searchsector].floorshade = tempshade;
					sector[searchsector].floorpal = temppal;
					if ((somethingintab == 1) || (somethingintab == 2))
					{
						sector[searchsector].floorxpanning= tempxrepeat;
						sector[searchsector].floorypanning= tempyrepeat;
						sector[searchsector].floorstat = tempcstat;
						sector[searchsector].visibility = tempvis;
						sector[searchsector].lotag = templotag;
						sector[searchsector].hitag = temphitag;
						sector[searchsector].extra = tempextra;
					}
				}
				if (searchstat == 3)
				{
					sprite[searchwall].picnum = temppicnum;
					if ((tilesizx[temppicnum] <= 0) || (tilesizy[temppicnum] <= 0))
					{
						j = 0;
						for(k=0;k<MAXTILES;k++)
							if ((tilesizx[k] > 0) && (tilesizy[k] > 0))
							{
								j = k;
								break;
							}
						sprite[searchwall].picnum = j;
					}
					sprite[searchwall].shade = tempshade;
					sprite[searchwall].pal = temppal;
					if (somethingintab == 3)
					{
						sprite[searchwall].xrepeat = tempxrepeat;
						sprite[searchwall].yrepeat = tempyrepeat;
						if (sprite[searchwall].xrepeat < 1) sprite[searchwall].xrepeat = 1;
						if (sprite[searchwall].yrepeat < 1) sprite[searchwall].yrepeat = 1;
						sprite[searchwall].cstat = tempcstat;
						sprite[searchwall].lotag = templotag;
						sprite[searchwall].hitag = temphitag;
						sprite[searchwall].extra = tempextra;
					}
				}
				if (searchstat == 4)
				{
					wall[searchwall].overpicnum = temppicnum;
					if (wall[searchwall].nextwall >= 0)
						wall[wall[searchwall].nextwall].overpicnum = temppicnum;
					wall[searchwall].shade = tempshade;
					wall[searchwall].pal = temppal;
					if (somethingintab == 4)
					{
						wall[searchwall].xrepeat = tempxrepeat;
						wall[searchwall].yrepeat = tempyrepeat;
						wall[searchwall].cstat = tempcstat;
						wall[searchwall].lotag = templotag;
						wall[searchwall].hitag = temphitag;
						wall[searchwall].extra = tempextra;
					}
					fixrepeats(searchwall);
				}
			}
			asksave = 1;
			keystatus[0x1c] = 0;
		}
		if (keystatus[0x2e] > 0)      //C
		{
			keystatus[0x2e] = 0;
			if (keystatus[0x38] > 0)    //Alt-C
			{
				if (somethingintab < 255)
				{
					switch(searchstat)
					{
						case 0:
							j = wall[searchwall].picnum;
							for(i=0;i<numwalls;i++)
								if (wall[i].picnum == j) wall[i].picnum = temppicnum;
							break;
						 case 1:
							j = sector[searchsector].ceilingpicnum;
							for(i=0;i<numsectors;i++)
								if (sector[i].ceilingpicnum == j) sector[i].ceilingpicnum = temppicnum;
							break;
						 case 2:
							j = sector[searchsector].floorpicnum;
							for(i=0;i<numsectors;i++)
								if (sector[i].floorpicnum == j) sector[i].floorpicnum = temppicnum;
							break;
						 case 3:
							 j = sprite[searchwall].picnum;
							 for(i=0;i<MAXSPRITES;i++)
								 if (sprite[i].statnum < MAXSTATUS)
									 if (sprite[i].picnum == j) sprite[i].picnum = temppicnum;
							 break;
						 case 4:
							 j = wall[searchwall].overpicnum;
							 for(i=0;i<numwalls;i++)
								 if (wall[i].overpicnum == j) wall[i].overpicnum = temppicnum;
							 break;
					}
				}
			}
			else    //C
			{
				if (searchstat == 3)
				{
					sprite[searchwall].cstat ^= 128;
					asksave = 1;
				}
			}
		}
		if (keystatus[0x2f] > 0)  //V  from 3D mode
		{
			if (searchstat == 0) templong = wall[searchwall].picnum;
			if (searchstat == 1) templong = sector[searchsector].ceilingpicnum;
			if (searchstat == 2) templong = sector[searchsector].floorpicnum;
			if (searchstat == 3) templong = sprite[searchwall].picnum;
			if (searchstat == 4) templong = wall[searchwall].overpicnum;
			templong = gettile(templong);
			if (searchstat == 0) wall[searchwall].picnum = templong;
			if (searchstat == 1) sector[searchsector].ceilingpicnum = templong;
			if (searchstat == 2) sector[searchsector].floorpicnum = templong;
			if (searchstat == 3) sprite[searchwall].picnum = templong;
			if (searchstat == 4)
			{
				wall[searchwall].overpicnum = templong;
				if (wall[searchwall].nextwall >= 0)
					 wall[wall[searchwall].nextwall].overpicnum = templong;
			}
			asksave = 1;
			keystatus[0x2f] = 0;
            bflushchars();
		}

		if (keystatus[0x1a])  // [
		{
			keystatus[0x1a] = 0;
			if (keystatus[0x38]|keystatus[0xb8])
			{
				i = wall[searchwall].nextsector;
				if (i >= 0)
					switch(searchstat)
					{
						case 0: case 1: case 4:
							alignceilslope(searchsector,wall[searchwall].x,wall[searchwall].y,getceilzofslope(i,wall[searchwall].x,wall[searchwall].y));
							break;
						case 2:
							alignflorslope(searchsector,wall[searchwall].x,wall[searchwall].y,getflorzofslope(i,wall[searchwall].x,wall[searchwall].y));
							break;
					}
			}
			else
			{
				i = 512;
				if (keystatus[0x36]) i = 8;
				if (keystatus[0x2a]) i = 1;

				if (searchstat == 1)
				{
					if (!(sector[searchsector].ceilingstat&2))
						sector[searchsector].ceilingheinum = 0;
					sector[searchsector].ceilingheinum = max(sector[searchsector].ceilingheinum-i,-32768);
				}
				if (searchstat == 2)
				{
					if (!(sector[searchsector].floorstat&2))
						sector[searchsector].floorheinum = 0;
					sector[searchsector].floorheinum = max(sector[searchsector].floorheinum-i,-32768);
				}
			}

			if (sector[searchsector].ceilingheinum == 0)
				sector[searchsector].ceilingstat &= ~2;
			else
				sector[searchsector].ceilingstat |= 2;

			if (sector[searchsector].floorheinum == 0)
				sector[searchsector].floorstat &= ~2;
			else
				sector[searchsector].floorstat |= 2;
			asksave = 1;
		}
		if (keystatus[0x1b])  // ]
		{
			keystatus[0x1b] = 0;
			if (keystatus[0x38]|keystatus[0xb8])
			{
				i = wall[searchwall].nextsector;
				if (i >= 0)
					switch(searchstat)
					{
						case 1:
							alignceilslope(searchsector,wall[searchwall].x,wall[searchwall].y,getceilzofslope(i,wall[searchwall].x,wall[searchwall].y));
							break;
						case 0: case 2: case 4:
							alignflorslope(searchsector,wall[searchwall].x,wall[searchwall].y,getflorzofslope(i,wall[searchwall].x,wall[searchwall].y));
							break;
					}
			}
			else
			{
				i = 512;
				if (keystatus[0x36]) i = 8;
				if (keystatus[0x2a]) i = 1;

				if (searchstat == 1)
				{
					if (!(sector[searchsector].ceilingstat&2))
						sector[searchsector].ceilingheinum = 0;
					sector[searchsector].ceilingheinum = min(sector[searchsector].ceilingheinum+i,32767);
				}
				if (searchstat == 2)
				{
					if (!(sector[searchsector].floorstat&2))
						sector[searchsector].floorheinum = 0;
					sector[searchsector].floorheinum = min(sector[searchsector].floorheinum+i,32767);
				}
			}

			if (sector[searchsector].ceilingheinum == 0)
				sector[searchsector].ceilingstat &= ~2;
			else
				sector[searchsector].ceilingstat |= 2;

			if (sector[searchsector].floorheinum == 0)
				sector[searchsector].floorstat &= ~2;
			else
				sector[searchsector].floorstat |= 2;

			asksave = 1;
		}

		smooshyalign = keystatus[0x4c];
		repeatpanalign = (keystatus[0x2a]|keystatus[0x36]);
		if ((keystatus[0x4b]|keystatus[0x4d]) > 0)  // 4 & 6 (keypad)
		{
			if ((repeatcountx == 0) || (repeatcountx > 16))
			{
				changedir = 0;
				if (keystatus[0x4b] > 0) changedir = -1;
				if (keystatus[0x4d] > 0) changedir = 1;

				if ((searchstat == 0) || (searchstat == 4))
				{
					if (repeatpanalign == 0)
						wall[searchwall].xrepeat = changechar(wall[searchwall].xrepeat,changedir,smooshyalign,1);
					else
						wall[searchwall].xpanning = changechar(wall[searchwall].xpanning,changedir,smooshyalign,0);
				}
				if ((searchstat == 1) || (searchstat == 2))
				{
					if (searchstat == 1)
						sector[searchsector].ceilingxpanning = changechar(sector[searchsector].ceilingxpanning,changedir,smooshyalign,0);
					else
						sector[searchsector].floorxpanning = changechar(sector[searchsector].floorxpanning,changedir,smooshyalign,0);
				}
				if (searchstat == 3)
				{
					sprite[searchwall].xrepeat = changechar(sprite[searchwall].xrepeat,changedir,smooshyalign,1);
					if (sprite[searchwall].xrepeat < 4)
						sprite[searchwall].xrepeat = 4;
				}
				asksave = 1;
				repeatcountx = max(1,repeatcountx);
			}
			repeatcountx += (synctics>>1);
		}
		else
			repeatcountx = 0;

		if ((keystatus[0x48]|keystatus[0x50]) > 0)  // 2 & 8 (keypad)
		{
			if ((repeatcounty == 0) || (repeatcounty > 16))
			{
				changedir = 0;
				if (keystatus[0x48] > 0) changedir = -1;
				if (keystatus[0x50] > 0) changedir = 1;

				if ((searchstat == 0) || (searchstat == 4))
				{
					if (repeatpanalign == 0)
						wall[searchwall].yrepeat = changechar(wall[searchwall].yrepeat,changedir,smooshyalign,1);
					else
						wall[searchwall].ypanning = changechar(wall[searchwall].ypanning,changedir,smooshyalign,0);
				}
				if ((searchstat == 1) || (searchstat == 2))
				{
					if (searchstat == 1)
						sector[searchsector].ceilingypanning = changechar(sector[searchsector].ceilingypanning,changedir,smooshyalign,0);
					else
						sector[searchsector].floorypanning = changechar(sector[searchsector].floorypanning,changedir,smooshyalign,0);
				}
				if (searchstat == 3)
				{
					sprite[searchwall].yrepeat = changechar(sprite[searchwall].yrepeat,changedir,smooshyalign,1);
					if (sprite[searchwall].yrepeat < 4)
						sprite[searchwall].yrepeat = 4;
				}
				asksave = 1;
				repeatcounty = max(1,repeatcounty);
			}
			repeatcounty += (synctics>>1);
			//}
		}
		else
			repeatcounty = 0;

		if (keystatus[0x33] > 0) // , Search & fix panning to the left (3D)
		{
			if (searchstat == 3)
			{
				i = searchwall;
				if ((keystatus[0x2a]|keystatus[0x36]) > 0)
					sprite[i].ang = ((sprite[i].ang+2048-1)&2047);
				else
				{
					sprite[i].ang = ((sprite[i].ang+2048-128)&2047);
					keystatus[0x33] = 0;
				}
			}
		}
		if (keystatus[0x34] > 0) // . Search & fix panning to the right (3D)
		{
			if ((searchstat == 0) || (searchstat == 4))
			{
				AutoAlignWalls((long)searchwall,0L);

				/*wallfind[0] = searchwall;
				cnt = 4096;
				do
				{
					wallfind[1] = wall[wallfind[0]].point2;
					j = -1;
					if (wall[wallfind[1]].picnum == wall[searchwall].picnum)
						j = wallfind[1];
					k = wallfind[1];

					while ((wall[wallfind[1]].nextwall >= 0) && (wall[wall[wallfind[1]].nextwall].point2 != k))
					{
						i = wall[wall[wallfind[1]].nextwall].point2;   //break if going around in circles on red lines with same picture on both sides
						if (wallfind[1] == wall[wall[i].nextwall].point2)
							break;

						wallfind[1] = wall[wall[wallfind[1]].nextwall].point2;
						if (wall[wallfind[1]].picnum == wall[searchwall].picnum)
							j = wallfind[1];
					}
					wallfind[1] = j;

					if ((j >= 0) && (wallfind[1] != searchwall))
					{
						j = (wall[wallfind[0]].xpanning+(wall[wallfind[0]].xrepeat<<3)) % tilesizx[wall[wallfind[0]].picnum];
						wall[wallfind[1]].cstat &= ~8;    //Set to non-flip
						wall[wallfind[1]].cstat |= 4;     //Set y-orientation
						wall[wallfind[1]].xpanning = j;

						for(k=0;k<2;k++)
						{
							sectnum = sectorofwall((short)wallfind[k]);
							nextsectnum = wall[wallfind[k]].nextsector;

							if (nextsectnum == -1)
							{
								if ((wall[wallfind[k]].cstat&4) == 0)
									daz[k] = sector[sectnum].ceilingz;
								else
									daz[k] = sector[sectnum].floorz;
							}
							else                                      //topstep
							{
								if (sector[nextsectnum].ceilingz > sector[sectnum].ceilingz)
									daz[k] = sector[nextsectnum].ceilingz;
								else if (sector[nextsectnum].floorz < sector[sectnum].floorz)
									daz[k] = sector[nextsectnum].floorz;
							}
						}

						j = (picsiz[wall[searchwall].picnum]>>4);
						if ((1<<j) != tilesizy[wall[searchwall].picnum]) j++;

						j = ((wall[wallfind[0]].ypanning+(((daz[1]-daz[0])*wall[wallfind[0]].yrepeat)>>(j+3)))&255);
						wall[wallfind[1]].ypanning = j;
						wall[wallfind[1]].yrepeat = wall[wallfind[0]].yrepeat;
						if (nextsectnum >= 0)
							if (sector[nextsectnum].ceilingz >= sector[sectnum].ceilingz)
								if (sector[nextsectnum].floorz <= sector[sectnum].floorz)
								{
									if (wall[wall[wallfind[1]].nextwall].picnum == wall[searchwall].picnum)
									{
										wall[wall[wallfind[1]].nextwall].yrepeat = wall[wallfind[0]].yrepeat;
										if ((wall[wall[wallfind[1]].nextwall].cstat&4) == 0)
											daz[1] = sector[nextsectnum].floorz;
										else
											daz[1] = sector[sectnum].ceilingz;
										wall[wall[wallfind[1]].nextwall].ypanning = j;
									}
								}
					}
					wallfind[0] = wallfind[1];
					cnt--;
				}
				while ((wall[wallfind[0]].picnum == wall[searchwall].picnum) && (wallfind[0] != searchwall) && (cnt > 0));
				*/

				keystatus[0x34] = 0;
			}
			if (searchstat == 3)
			{
				i = searchwall;
				if ((keystatus[0x2a]|keystatus[0x36]) > 0)
					sprite[i].ang = ((sprite[i].ang+2048+1)&2047);
				else
				{
					sprite[i].ang = ((sprite[i].ang+2048+128)&2047);
					keystatus[0x34] = 0;
				}
			}
		}
		if (keystatus[0x35] > 0)  // /?     Reset panning&repeat to 0
		{
			if ((searchstat == 0) || (searchstat == 4))
			{
				wall[searchwall].xpanning = 0;
				wall[searchwall].ypanning = 0;
				wall[searchwall].xrepeat = 8;
				wall[searchwall].yrepeat = 8;
				wall[searchwall].cstat = 0;
				fixrepeats((short)searchwall);
			}
			if (searchstat == 1)
			{
				sector[searchsector].ceilingxpanning = 0;
				sector[searchsector].ceilingypanning = 0;
				sector[searchsector].ceilingstat &= ~2;
				sector[searchsector].ceilingheinum = 0;
			}
			if (searchstat == 2)
			{
				sector[searchsector].floorxpanning = 0;
				sector[searchsector].floorypanning = 0;
				sector[searchsector].floorstat &= ~2;
				sector[searchsector].floorheinum = 0;
			}
			if (searchstat == 3)
			{
				if ((keystatus[0x2a]|keystatus[0x36]) > 0)
				{
					sprite[searchwall].xrepeat = sprite[searchwall].yrepeat;
				}
				else
				{
					sprite[searchwall].xrepeat = 64;
					sprite[searchwall].yrepeat = 64;
				}
			}
			keystatus[0x35] = 0;
			asksave = 1;
		}


		if (keystatus[0x19] > 0)  // P (parallaxing sky)
		{
			if ((keystatus[0x1d]|keystatus[0x9d]) > 0)
			{
				parallaxtype++;
				if (parallaxtype == 3)
					parallaxtype = 0;
			}
			else if ((keystatus[0x38]|keystatus[0xb8]) > 0)
			{
				switch (searchstat)
				{
					case 0: case 4:
						Bstrcpy(buffer,"Wall pal: ");
						wall[searchwall].pal = getnumber256(buffer,wall[searchwall].pal,256L,0);
						break;
					case 1:
						Bstrcpy(buffer,"Ceiling pal: ");
						sector[searchsector].ceilingpal = getnumber256(buffer,sector[searchsector].ceilingpal,256L,0);
						break;
					case 2:
						Bstrcpy(buffer,"Floor pal: ");
						sector[searchsector].floorpal = getnumber256(buffer,sector[searchsector].floorpal,256L,0);
						break;
					case 3:
						Bstrcpy(buffer,"Sprite pal: ");
						sprite[searchwall].pal = getnumber256(buffer,sprite[searchwall].pal,256L,0);
						break;
				}
			}
			else
			{
				if ((searchstat == 0) || (searchstat == 1) || (searchstat == 4))
				{
					sector[searchsector].ceilingstat ^= 1;
					asksave = 1;
				}
				else if (searchstat == 2)
				{
					sector[searchsector].floorstat ^= 1;
					asksave = 1;
				}
			}
			keystatus[0x19] = 0;
		}

		if (keystatus[0x20] != 0)   //Alt-D  (adjust sprite[].clipdist)
		{
			keystatus[0x20] = 0;
			if ((keystatus[0x38]|keystatus[0xb8]) > 0)
			{
				if (searchstat == 3)
				{
					Bstrcpy(buffer,"Sprite clipdist: ");
					sprite[searchwall].clipdist = getnumber256(buffer,sprite[searchwall].clipdist,256L,0);
				}
			}
		}

		if (keystatus[0x30] > 0)  // B (clip Blocking xor) (3D)
		{
			if (searchstat == 3)
			{
				sprite[searchwall].cstat ^= 1;
				sprite[searchwall].cstat &= ~256;
				sprite[searchwall].cstat |= ((sprite[searchwall].cstat&1)<<8);
				asksave = 1;
			}
			else
			{
				wall[searchwall].cstat ^= 1;
				wall[searchwall].cstat &= ~64;
				if ((wall[searchwall].nextwall >= 0) && ((keystatus[0x2a]|keystatus[0x36]) == 0))
				{
					wall[wall[searchwall].nextwall].cstat &= ~(1+64);
					wall[wall[searchwall].nextwall].cstat |= (wall[searchwall].cstat&1);
				}
				asksave = 1;
			}
			keystatus[0x30] = 0;
		}
		if (keystatus[0x14] > 0)  // T (transluscence for sprites/masked walls)
		{
			/*if (searchstat == 1)   //Set masked/transluscent ceilings/floors
			{
				i = (sector[searchsector].ceilingstat&(128+256));
				sector[searchsector].ceilingstat &= ~(128+256);
				switch(i)
				{
					case 0: sector[searchsector].ceilingstat |= 128; break;
					case 128: sector[searchsector].ceilingstat |= 256; break;
					case 256: sector[searchsector].ceilingstat |= 384; break;
					case 384: sector[searchsector].ceilingstat |= 0; break;
				}
				asksave = 1;
			}
			if (searchstat == 2)
			{
				i = (sector[searchsector].floorstat&(128+256));
				sector[searchsector].floorstat &= ~(128+256);
				switch(i)
				{
					case 0: sector[searchsector].floorstat |= 128; break;
					case 128: sector[searchsector].floorstat |= 256; break;
					case 256: sector[searchsector].floorstat |= 384; break;
					case 384: sector[searchsector].floorstat |= 0; break;
				}
				asksave = 1;
			}*/
			if (searchstat == 3)
			{
				if ((sprite[searchwall].cstat&2) == 0)
					sprite[searchwall].cstat |= 2;
				else if ((sprite[searchwall].cstat&512) == 0)
					sprite[searchwall].cstat |= 512;
				else
					sprite[searchwall].cstat &= ~(2+512);
				asksave = 1;
			}
			if (searchstat == 4)
			{
				if ((wall[searchwall].cstat&128) == 0)
					wall[searchwall].cstat |= 128;
				else if ((wall[searchwall].cstat&512) == 0)
					wall[searchwall].cstat |= 512;
				else
					wall[searchwall].cstat &= ~(128+512);

				if (wall[searchwall].nextwall >= 0)
				{
					wall[wall[searchwall].nextwall].cstat &= ~(128+512);
					wall[wall[searchwall].nextwall].cstat |= (wall[searchwall].cstat&(128+512));
				}
				asksave = 1;
			}
			keystatus[0x14] = 0;
		}

		if (keystatus[0x2] > 0)  // 1 (make 1-way wall)
		{
			if (searchstat != 3)
			{
				wall[searchwall].cstat ^= 32;
				asksave = 1;
			}
			else
			{
				sprite[searchwall].cstat ^= 64;
				i = sprite[searchwall].cstat;
				if ((i&48) == 32)
				{
					sprite[searchwall].cstat &= ~8;
					if ((i&64) > 0)
						if (posz > sprite[searchwall].z)
							sprite[searchwall].cstat |= 8;
				}
				asksave = 1;
			}
			keystatus[0x2] = 0;
		}
		if (keystatus[0x3] > 0)  // 2 (bottom wall swapping)
		{
			if (searchstat != 3)
			{
				wall[searchwall].cstat ^= 2;
				asksave = 1;
			}
			keystatus[0x3] = 0;
		}
		if (keystatus[0x18] > 0)  // O (top/bottom orientation - for doors)
		{
			if ((searchstat == 0) || (searchstat == 4))
			{
				wall[searchwall].cstat ^= 4;
				asksave = 1;
			}
			if (searchstat == 3)   // O (ornament onto wall) (2D)
			{
				asksave = 1;
				i = searchwall;

				hitscan(sprite[i].x,sprite[i].y,sprite[i].z,sprite[i].sectnum,
					sintable[(sprite[i].ang+2560+1024)&2047],
					sintable[(sprite[i].ang+2048+1024)&2047],
					0,
					&hitsect,&hitwall,&hitsprite,&hitx,&hity,&hitz,CLIPMASK1);

				sprite[i].x = hitx;
				sprite[i].y = hity;
				sprite[i].z = hitz;
				changespritesect(i,hitsect);
				if (hitwall >= 0)
					sprite[i].ang = ((getangle(wall[wall[hitwall].point2].x-wall[hitwall].x,wall[wall[hitwall].point2].y-wall[hitwall].y)+512)&2047);

					//Make sure sprite's in right sector
				if (inside(sprite[i].x,sprite[i].y,sprite[i].sectnum) == 0)
				{
					j = wall[hitwall].point2;
					sprite[i].x -= ksgn(wall[j].y-wall[hitwall].y);
					sprite[i].y += ksgn(wall[j].x-wall[hitwall].x);
				}
			}
			keystatus[0x18] = 0;
		}
		if (keystatus[0x32] > 0)  // M (masking walls)
		{
			if (searchstat != 3)
			{
				i = wall[searchwall].nextwall;
				templong = (keystatus[0x2a]|keystatus[0x36]);
				if (i >= 0)
				{
					wall[searchwall].cstat ^= 16;
					if ((wall[searchwall].cstat&16) > 0)
					{
						wall[searchwall].cstat &= ~8;
						if (templong == 0)
						{
							wall[i].cstat |= 8;           //auto other-side flip
							wall[i].cstat |= 16;
							wall[i].overpicnum = wall[searchwall].overpicnum;
						}
					}
					else
					{
						wall[searchwall].cstat &= ~8;
						if (templong == 0)
						{
							wall[i].cstat &= ~8;         //auto other-side unflip
							wall[i].cstat &= ~16;
						}
					}
					wall[searchwall].cstat &= ~32;
					if (templong == 0) wall[i].cstat &= ~32;
					asksave = 1;
				}
			}
			keystatus[0x32] = 0;
		}
		if (keystatus[0x23] > 0)  // H (hitscan sensitivity) (help)
		{
			if (searchstat == 3)
			{
				sprite[searchwall].cstat ^= 256;
				asksave = 1;
			}
			else
			{
				wall[searchwall].cstat ^= 64;
				if ((wall[searchwall].nextwall >= 0) && ((keystatus[0x2a]|keystatus[0x36]) == 0))
				{
					wall[wall[searchwall].nextwall].cstat &= ~64;
					wall[wall[searchwall].nextwall].cstat |= (wall[searchwall].cstat&64);
				}
				asksave = 1;
			}
			keystatus[0x23] = 0;
		}
		if (keystatus[0x12] > 0)  // E (expand)
		{
			if (searchstat == 1)
			{
				sector[searchsector].ceilingstat ^= 8;
				asksave = 1;
			}
			if (searchstat == 2)
			{
				sector[searchsector].floorstat ^= 8;
				asksave = 1;
			}
			keystatus[0x12] = 0;
		}
		if (keystatus[0x13] > 0)  // R (relative alignment, rotation)
		{
			if (searchstat == 1)
			{
				sector[searchsector].ceilingstat ^= 64;
				asksave = 1;
			}
			if (searchstat == 2)
			{
				sector[searchsector].floorstat ^= 64;
				asksave = 1;
			}
			if (searchstat == 3)
			{
				i = sprite[searchwall].cstat;
				if ((i&48) < 32) i += 16; else i &= ~48;
				sprite[searchwall].cstat = i;
				asksave = 1;
			}
			keystatus[0x13] = 0;
		}
		if (keystatus[0x21] > 0)  //F (Flip)
		{
			keystatus[0x21] = 0;
			if ((keystatus[0x38]|keystatus[0xb8]) > 0)  //ALT-F (relative alignmment flip)
			{
				if (searchstat != 3)
				{
					setfirstwall(searchsector,searchwall);
					asksave = 1;
				}
			}
			else
			{
				if ((searchstat == 0) || (searchstat == 4))
				{
					i = wall[searchwall].cstat;
					i = ((i>>3)&1)+((i>>7)&2);    //3-x,8-y
					switch(i)
					{
						case 0: i = 1; break;
						case 1: i = 3; break;
						case 2: i = 0; break;
						case 3: i = 2; break;
					}
					i = ((i&1)<<3)+((i&2)<<7);
					wall[searchwall].cstat &= ~0x0108;
					wall[searchwall].cstat |= i;
					asksave = 1;
				}
				if (searchstat == 1)         //8-way ceiling flipping (bits 2,4,5)
				{
					i = sector[searchsector].ceilingstat;
					i = (i&0x4)+((i>>4)&3);
					switch(i)
					{
						case 0: i = 6; break;
						case 6: i = 3; break;
						case 3: i = 5; break;
						case 5: i = 1; break;
						case 1: i = 7; break;
						case 7: i = 2; break;
						case 2: i = 4; break;
						case 4: i = 0; break;
					}
					i = (i&0x4)+((i&3)<<4);
					sector[searchsector].ceilingstat &= ~0x34;
					sector[searchsector].ceilingstat |= i;
					asksave = 1;
				}
				if (searchstat == 2)         //8-way floor flipping (bits 2,4,5)
				{
					i = sector[searchsector].floorstat;
					i = (i&0x4)+((i>>4)&3);
					switch(i)
					{
						case 0: i = 6; break;
						case 6: i = 3; break;
						case 3: i = 5; break;
						case 5: i = 1; break;
						case 1: i = 7; break;
						case 7: i = 2; break;
						case 2: i = 4; break;
						case 4: i = 0; break;
					}
					i = (i&0x4)+((i&3)<<4);
					sector[searchsector].floorstat &= ~0x34;
					sector[searchsector].floorstat |= i;
					asksave = 1;
				}
				if (searchstat == 3)
				{
					i = sprite[searchwall].cstat;
					if (((i&48) == 32) && ((i&64) == 0))
					{
						sprite[searchwall].cstat &= ~0xc;
						sprite[searchwall].cstat |= (i&4)^4;
					}
					else
					{
						i = ((i>>2)&3);
						switch(i)
						{
							case 0: i = 1; break;
							case 1: i = 3; break;
							case 2: i = 0; break;
							case 3: i = 2; break;
						}
						i <<= 2;
						sprite[searchwall].cstat &= ~0xc;
						sprite[searchwall].cstat |= i;
					}
					asksave = 1;
				}
			}
		}
		if (keystatus[0x1f] > 0)  //S (insert sprite) (3D)
		{
			dax = 16384;
			day = divscale14(searchx-(xdim>>1),xdim>>1);
			rotatepoint(0,0,dax,day,ang,&dax,&day);

			hitscan(posx,posy,posz,cursectnum,               // Start position
				dax,day,(scale(searchy,200,ydim)-horiz)*2000, // vector of 3D ang
				&hitsect,&hitwall,&hitsprite,&hitx,&hity,&hitz,CLIPMASK1);

			if (hitsect >= 0)
			{
				dax = hitx;
				day = hity;
				if ((gridlock > 0) && (grid > 0))
				{
					if ((searchstat == 0) || (searchstat == 4))
					{
						hitz = (hitz&0xfffffc00);
					}
					else
					{
						dax = ((dax+(1024>>grid))&(0xffffffff<<(11-grid)));
						day = ((day+(1024>>grid))&(0xffffffff<<(11-grid)));
					}
				}

				i = insertsprite(hitsect,0);
				sprite[i].x = dax, sprite[i].y = day;
				sprite[i].cstat = defaultspritecstat;
				sprite[i].shade = 0;
				sprite[i].pal = 0;
				sprite[i].xrepeat = 64, sprite[i].yrepeat = 64;
				sprite[i].xoffset = 0, sprite[i].yoffset = 0;
				sprite[i].ang = 1536;
				sprite[i].xvel = 0; sprite[i].yvel = 0; sprite[i].zvel = 0;
				sprite[i].owner = -1;
				sprite[i].clipdist = 32;
				sprite[i].lotag = 0;
				sprite[i].hitag = 0;
				sprite[i].extra = -1;

				for(k=0;k<MAXTILES;k++)
					localartfreq[k] = 0;
				for(k=0;k<MAXSPRITES;k++)
					if (sprite[k].statnum < MAXSTATUS)
						localartfreq[sprite[k].picnum]++;
				j = 0;
				for(k=0;k<MAXTILES;k++)
					if (localartfreq[k] > localartfreq[j])
						j = k;
				if (localartfreq[j] > 0)
					sprite[i].picnum = j;
				else
					sprite[i].picnum = 0;

				if (somethingintab == 3)
				{
					sprite[i].picnum = temppicnum;
					if ((tilesizx[temppicnum] <= 0) || (tilesizy[temppicnum] <= 0))
					{
						j = 0;
						for(k=0;k<MAXTILES;k++)
							if ((tilesizx[k] > 0) && (tilesizy[k] > 0))
							{
								j = k;
								break;
							}
						sprite[i].picnum = j;
					}
					sprite[i].shade = tempshade;
					sprite[i].pal = temppal;
					sprite[i].xrepeat = tempxrepeat;
					sprite[i].yrepeat = tempyrepeat;
					if (sprite[i].xrepeat < 1) sprite[i].xrepeat = 1;
					if (sprite[i].yrepeat < 1) sprite[i].yrepeat = 1;
					sprite[i].cstat = tempcstat;
				}

				j = ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<1);
				if ((sprite[i].cstat&128) == 0)
					sprite[i].z = min(max(hitz,getceilzofslope(hitsect,hitx,hity)+(j<<1)),getflorzofslope(hitsect,hitx,hity));
				else
					sprite[i].z = min(max(hitz,getceilzofslope(hitsect,hitx,hity)+j),getflorzofslope(hitsect,hitx,hity)-j);

				if ((searchstat == 0) || (searchstat == 4))
				{
					sprite[i].cstat = (sprite[i].cstat&~48)|(16+64);
					if (hitwall >= 0)
						sprite[i].ang = ((getangle(wall[wall[hitwall].point2].x-wall[hitwall].x,wall[wall[hitwall].point2].y-wall[hitwall].y)+512)&2047);

						//Make sure sprite's in right sector
					if (inside(sprite[i].x,sprite[i].y,sprite[i].sectnum) == 0)
					{
						j = wall[hitwall].point2;
						sprite[i].x -= ksgn(wall[j].y-wall[hitwall].y);
						sprite[i].y += ksgn(wall[j].x-wall[hitwall].x);
					}
				}
				else
				{
					if (tilesizy[sprite[i].picnum] >= 32) sprite[i].cstat |= 1;
				}

				updatenumsprites();
				asksave = 1;
			}

			keystatus[0x1f] = 0;
		}
		if (keystatus[0xd3] > 0)
		{
			if (searchstat == 3)
			{
				deletesprite(searchwall);
				updatenumsprites();
				asksave = 1;
			}
			keystatus[0xd3] = 0;
		}

		if ((keystatus[0x3f]|keystatus[0x40]) > 0)  //F5,F6
		{
			switch(searchstat)
			{
				case 1: case 2: ExtShowSectorData(searchsector); break;
				case 0: case 4: ExtShowWallData(searchwall); break;
				case 3: ExtShowSpriteData(searchwall); break;
			}
			keystatus[0x3f] = 0, keystatus[0x40] = 0;
		}
		if ((keystatus[0x41]|keystatus[0x42]) > 0)  //F7,F8
		{
			switch(searchstat)
			{
				case 1: case 2: ExtEditSectorData(searchsector); break;
				case 0: case 4: ExtEditWallData(searchwall); break;
				case 3: ExtEditSpriteData(searchwall); break;
			}
			keystatus[0x41] = 0, keystatus[0x42] = 0;
		}

	}

	if (keystatus[buildkeys[14]] > 0)  // Enter from 3D
	{
		BackupMap();                                                 // wxbu
		overheadeditor();
		keystatus[buildkeys[14]] = 0;
	}
}

char changechar(char dachar, long dadir, char smooshyalign, char boundcheck)
{
	if (dadir < 0)
	{
		if ((dachar > 0) || (boundcheck == 0))
		{
			dachar--;
			if (smooshyalign > 0)
				dachar = (dachar&0xf8);
		}
	}
	else if (dadir > 0)
	{
		if ((dachar < 255) || (boundcheck == 0))
		{
			dachar++;
			if (smooshyalign > 0)
			{
				if (dachar >= 256-8) dachar = 255;
				else dachar = ((dachar+7)&0xf8);
			}
		}
	}
	return(dachar);
}

long gettile(long tilenum)
{
	char snotbuf[80], ch;
	long i, j, k, otilenum, topleft, gap, temp, templong;
	long xtiles, ytiles, tottiles;

	if (tilenum < 0)
	    tilenum = 0;

	xtiles = (xdim>>xsC);                                            // wxts
	ytiles = (ydim>>xsC);                                            // wxts
	tottiles = xtiles*ytiles;
	otilenum = tilenum;

	keystatus[0x2f] = 0;
	for(i=0;i<MAXTILES;i++)
	{
		localartfreq[i] = 0;
		localartlookup[i] = i;
	}
	if ((searchstat == 1) || (searchstat == 2))
		for(i=0;i<numsectors;i++)
		{
			localartfreq[sector[i].ceilingpicnum]++;
			localartfreq[sector[i].floorpicnum]++;
		}
	if (searchstat == 0)
		for(i=0;i<numwalls;i++)
			localartfreq[wall[i].picnum]++;
	if (searchstat == 4)
		for(i=0;i<numwalls;i++)
			localartfreq[wall[i].overpicnum]++;
	if (searchstat == 3)
		for(i=0;i<MAXSPRITES;i++)
			if (sprite[i].statnum < MAXSTATUS)
				localartfreq[sprite[i].picnum]++;
	gap = (MAXTILES>>1);

	do
	{
		for(i=0;i<MAXTILES-gap;i++)
		{
			temp = i;
			while ((localartfreq[temp] < localartfreq[temp+gap]) && (temp >= 0))
			{
				templong = localartfreq[temp];
				localartfreq[temp] = localartfreq[temp+gap];
				localartfreq[temp+gap] = templong;
				templong = localartlookup[temp];
				localartlookup[temp] = localartlookup[temp+gap];
				localartlookup[temp+gap] = templong;

				if (tilenum == temp)
					tilenum = temp+gap;
				else if (tilenum == temp+gap)
					tilenum = temp;

				temp -= gap;
			}
		}
		gap >>= 1;
	}
	while (gap > 0);
	localartlookupnum = 0;
	while (localartfreq[localartlookupnum] > 0)
		localartlookupnum++;

	if (localartfreq[0] == 0)
	{
		tilenum = otilenum;
		localartlookupnum = MAXTILES;
		for(i=0;i<MAXTILES;i++)
			localartlookup[i] = i;
	}

	topleft = ((tilenum/(xtiles<<gettilezoom))*(xtiles<<gettilezoom))-(xtiles<<gettilezoom);
	if (topleft < 0)
	    topleft = 0;
	if (topleft > MAXTILES-(tottiles<<(gettilezoom<<1)))
	    topleft = MAXTILES-(tottiles<<(gettilezoom<<1));

	while ((keystatus[0x1c]|keystatus[1]) == 0)          // enter | esc
	{
		drawtilescreen(topleft,tilenum);

		if (handleevents()) {
			if (quitevent) quitevent = 0;
		}

		synctics = totalclock-lockclock;
		lockclock += synctics;

		if ((keystatus[0x37] > 0) && (gettilezoom < 2))  // kp star
		{
			gettilezoom++;
			topleft = ((tilenum/(xtiles<<gettilezoom))*(xtiles<<gettilezoom))-(xtiles<<gettilezoom);
			if (topleft < 0) topleft = 0;
			if (topleft > MAXTILES-(tottiles<<(gettilezoom<<1))) topleft = MAXTILES-(tottiles<<(gettilezoom<<1));
			keystatus[0x37] = 0;
		}
		if ((keystatus[0xb5] > 0) && (gettilezoom > 0))  // kp slash
		{
			gettilezoom--;
			topleft = ((tilenum/(xtiles<<gettilezoom))*(xtiles<<gettilezoom))-(xtiles<<gettilezoom);
			if (topleft < 0) topleft = 0;
			if (topleft > MAXTILES-(tottiles<<(gettilezoom<<1))) topleft = MAXTILES-(tottiles<<(gettilezoom<<1));
			keystatus[0xb5] = 0;
		}
		if ((keystatus[0xcb] > 0) && (tilenum > 0))                               // left arrow
			tilenum--, keystatus[0xcb] = 0;
		if ((keystatus[0xcd] > 0) && (tilenum < MAXTILES-1))                      // right arrow
			tilenum++, keystatus[0xcd] = 0;
		if ((keystatus[0xc8] > 0) && (tilenum >= (xtiles<<gettilezoom)))          // up arrow
			tilenum-=(xtiles<<gettilezoom), keystatus[0xc8] = 0;
		if ((keystatus[0xd0] > 0) && (tilenum < MAXTILES-(xtiles<<gettilezoom)))  // down arrow
			tilenum+=(xtiles<<gettilezoom), keystatus[0xd0] = 0;
		if ((keystatus[0xc9] > 0) && (tilenum >= (xtiles<<gettilezoom)))          // pgup
		{
			tilenum-=(tottiles<<(gettilezoom<<1));
			if (tilenum < 0) tilenum = 0;
			keystatus[0xc9] = 0;
		}
		if ((keystatus[0xd1] > 0) && (tilenum < MAXTILES-(xtiles<<gettilezoom)))  // pgdn
		{
			tilenum+=(tottiles<<(gettilezoom<<1));
			if (tilenum >= MAXTILES) tilenum = MAXTILES-1;
			keystatus[0xd1] = 0;
		}

		if (keystatus[0x2f] > 0)   //V                                            // v
		{
			keystatus[0x2f] = 0;
			if (tilenum < localartlookupnum)
				tilenum = localartlookup[tilenum];
			else
				tilenum = 0;
			localartlookupnum = MAXTILES;       // displays all tiles
			for(i=0;i<MAXTILES;i++)
				localartlookup[i] = i;
		}
                                                                                  // g
		if (keystatus[0x22] > 0)       //G (goto)
		{
			if (tilenum < localartlookupnum)         //Automatically press 'V'
				tilenum = localartlookup[tilenum];
			else
				tilenum = 0;
			localartlookupnum = MAXTILES;
			for(i=0;i<MAXTILES;i++)
				localartlookup[i] = i;

			keystatus[0x22] = 0;
			bflushchars();

			j = tilenum;
			while (keystatus[1] == 0)
			{
				if (handleevents()) {
					if (quitevent) quitevent = 0;
				}

				ch = bgetchar();

				//drawtilescreen(topleft,tilenum);
				Bsprintf(snotbuf,"Goto tile: %ld_ ",j);
				printext256(0,0,whitecol,0,snotbuf,0);
				showframe(1);

				if (ch >= '0' && ch <= '9') {
					i = (j*10)+(ch-'0');
					if (i < MAXTILES) j = i;
				} else if (ch == 8) {
					j /= 10;
				} else if (ch == 13) {
					tilenum = j;
					break;
				}
			}
			clearkeys();
		}
		while (tilenum < topleft) topleft -= (xtiles<<gettilezoom);
		while (tilenum >= topleft+(tottiles<<(gettilezoom<<1))) topleft += (xtiles<<gettilezoom);
		if (topleft < 0) topleft = 0;
		if (topleft > MAXTILES-(tottiles<<(gettilezoom<<1))) topleft = MAXTILES-(tottiles<<(gettilezoom<<1));
	}

	if (keystatus[0x1c] == 0)                                                     // enter
	{
		tilenum = otilenum;
	}
	else
	{
		if (tilenum < localartlookupnum)
		{
			tilenum = localartlookup[tilenum];
			if ((tilesizx[tilenum] == 0) || (tilesizy[tilenum] == 0))
				tilenum = otilenum;
		}
		else
			tilenum = otilenum;
	}
	keystatus[0x1] = 0;
	keystatus[0x1c] = 0;
	return(tilenum);
}

long drawtilescreen(long pictopleft, long picbox)
{
	long i, j, vidpos, vidpos2, dat, wallnum, xdime, ydime, cnt, pinc;
	long dax, day, scaledown, xtiles, ytiles, tottiles;
	char *picptr, snotbuf[80];

	xtiles = (xdim>>xsC);        // 6 = 32 tiles   xdim = 1024 (screen size) // wxts
	ytiles = (ydim>>xsC);        // 6                                        // wxts
	tottiles = xtiles*ytiles;

	begindrawing(); //{{{

	setpolymost2dview();	// JBF 20040205: set to 2d rendering

	pinc = ylookup[1];
	clearview(0L);
	for(cnt=0;cnt<(tottiles<<(gettilezoom<<1));cnt++)         //draw the 5*3 grid of tiles
	{
		wallnum = cnt+pictopleft;
		if (wallnum < localartlookupnum)
		{
			wallnum = localartlookup[wallnum];
			if ((tilesizx[wallnum] != 0) && (tilesizy[wallnum] != 0))
			{
				if (waloff[wallnum] == 0) loadtile(wallnum);
				picptr = (char *)(waloff[wallnum]);
				xdime = tilesizx[wallnum];             // actual pic size
				ydime = tilesizy[wallnum];

				dax = ((cnt%(xtiles<<gettilezoom))<<(xsC-gettilezoom));   // wxts 6 dax increments by 32
				day = ((cnt/(xtiles<<gettilezoom))<<(xsC-gettilezoom));   // wxts

				if (polymost_drawtilescreen(dax, day, wallnum, xsA>>gettilezoom)) {      // wxts
					vidpos = ylookup[day]+dax+frameplace;
					if ((xdime <= (xsA>>gettilezoom)) && (ydime <= (xsA>>gettilezoom)))  // wxts
					{
						for(i=0;i<xdime;i++)
						{
							vidpos2 = vidpos+i;
							for(j=0;j<ydime;j++)
							{
								*(char *)vidpos2 = *picptr++;
								vidpos2 += pinc;
							}
						}
					}
					else                          //if 1 dimension > xsA
					{
						if (xdime > ydime)
							scaledown = ((xdime+(xsB>>gettilezoom))>>(xsC-gettilezoom));   // wxts
						else
							scaledown = ((ydime+(xsB>>gettilezoom))>>(xsC-gettilezoom));   // wxts

						for(i=0;i<xdime;i+=scaledown)
						{
							if (waloff[wallnum] == 0) loadtile(wallnum);
							picptr = (char *)(waloff[wallnum]) + ydime*i;
							vidpos2 = vidpos;
							for(j=0;j<ydime;j+=scaledown)
							{
								*(char *)vidpos2 = *picptr;
								picptr += scaledown;
								vidpos2 += pinc;
							}
							vidpos++;
						}
					}
				}
				if (localartlookupnum < MAXTILES)
				{
					dax = ((cnt%(xtiles<<gettilezoom))<<(xsC-gettilezoom));  // wxts
					day = ((cnt/(xtiles<<gettilezoom))<<(xsC-gettilezoom));  // wxts
					Bsprintf(snotbuf,"%d",localartfreq[cnt+pictopleft]);
					printext256(dax,day,whitecol,-1,snotbuf,1);
				}
			}
		}
	}

	cnt = picbox-pictopleft;    //draw open white box
	dax = ((cnt%(xtiles<<gettilezoom))<<(xsC-gettilezoom));          // wxts  horiz cursor size
	day = ((cnt/(xtiles<<gettilezoom))<<(xsC-gettilezoom));          // wxts

	for(i=0;i<(xsA>>gettilezoom);i++)                                // wxts
	{
		plotpixel(dax+i,day,whitecol);
		plotpixel(dax+i,day+(xsB>>gettilezoom),whitecol);            // wxts
		plotpixel(dax,day+i,whitecol);
		plotpixel(dax+(xsB>>gettilezoom),day+i,whitecol);            // wxts
	}

	i = localartlookup[picbox];
	Bsprintf(snotbuf,"%ld",i);
	j = CheckAnimations(i);
	if (j == -1)
	   {
	   printext256(0L,ydim-8,whitecol,-1,snotbuf,0);
	   }
	else
	   {
	   strcat(snotbuf, " - ");                                       // wxvc
	   strcat(snotbuf, sTile[j]);                                    // wxvc
	   printext256(0L,ydim-8,0,whitecol,snotbuf,0);
	   }
	printext256(xdim-(Bstrlen(names[i])<<3),ydim-8,whitecol,-1,names[i],0);
	Bsprintf(snotbuf,"%dx%d",tilesizx[i],tilesizy[i]);
    printext256(xdim>>2,ydim-8,whitecol,-1,snotbuf,0);

	enddrawing();	//}}}
	showframe(1);

	return(0);
}

int CheckAnimations(long iNum)                                        // wxvc
{
    int i;
    int x = -1;

    for (i=0; i<sTileMax; i++)
        {
        if (iTile[i] == iNum)
           {
           x = i;
           break;
           }
        }
    return x;
}

void overheadeditor(void)
{
	char buffer[80], *dabuffer, ch;
	long i, j, k, m=0, mousxplc, mousyplc, firstx=0, firsty=0, oposz, col;
	long templong, templong1, templong2, doubvel;
	long startwall, endwall, dax, day, daz, x1, y1, x2, y2, x3, y3, x4, y4;
	long highlightx1, highlighty1, highlightx2, highlighty2, xvect, yvect;
	short pag, suckwall=0, sucksect, newnumwalls, newnumsectors, split=0, bad;
	short splitsect=0, danumwalls, secondstartwall, joinsector[2], joinsectnum;
	short splitstartwall=0, splitendwall, loopnum;
	long mousx, mousy, bstatus;
	long centerx, centery, circlerad;
	short circlewall, circlepoints, circleang1, circleang2, circleangdir;
	long sectorhighlightx=0, sectorhighlighty=0;
	short cursectorhighlight, sectorhighlightstat;
	short hitsect, hitwall, hitsprite;
	long hitx, hity, hitz;
	short ky = 0;
	walltype *wal;

    keystatus[1] = 0;

	qsetmodeany(xdim2d,ydim2d);
	xdim2d = xdim;
	ydim2d = ydim;

	searchx = scale(searchx,xdim2d,xdimgame);
	searchy = scale(searchy,ydim2d-STATUS2DSIZ,ydimgame);
	oposz = posz;

	begindrawing();	//{{{                                       // draws window lines on 2D screen
	ydim16 = ydim;
//	drawline16(0,ydim-STATUS2DSIZ,xdim-1,ydim-STATUS2DSIZ,7);
//	drawline16(0,ydim-1,xdim-1,ydim-1,7);
//	drawline16(0,ydim-STATUS2DSIZ,0,ydim-1,7);
//	drawline16(xdim-1,ydim-STATUS2DSIZ,xdim-1,ydim-1,7);
//	drawline16(0,ydim-STATUS2DSIZ+24,xdim-1,ydim-STATUS2DSIZ+24,7);
	drawline16(192,ydim-STATUS2DSIZ,192,ydim-STATUS2DSIZ+24,7);
//    if (xdim2d > 1000)
//	     printext16(xdim2d-200, ydim-STATUS2DSIZ+128, 8, -1, kensig, 0);
	printext16(9L,ydim-STATUS2DSIZ+9L, 8, -1, kensig, 0);
//	printext16(8L,ydim-STATUS2DSIZ+8L,12,-1,kensig,0);
	// printmessage16(REVISION);
//	drawline16(0,ydim-1-24,xdim-1,ydim-1-24,7);
	drawline16(256,ydim-1-24,256,ydim-1,7);
	ydim16 = ydim-STATUS2DSIZ;
	enddrawing();	//}}}

    clearmidstatbar16();                                             // wxyz

	pag = 0;
	highlightcnt = -1;
	cursectorhighlight = -1;

		//White out all bordering lines of grab that are
		//not highlighted on both sides
	for(i=highlightsectorcnt-1;i>=0;i--)
	{
		startwall = sector[highlightsector[i]].wallptr;
		endwall = startwall + sector[highlightsector[i]].wallnum;
		for(j=startwall;j<endwall;j++)
		{
			if (wall[j].nextwall >= 0)
			{
				for(k=highlightsectorcnt-1;k>=0;k--)
					if (highlightsector[k] == wall[j].nextsector)
						break;
				if (k < 0)
				{
					wall[wall[j].nextwall].nextwall = -1;
					wall[wall[j].nextwall].nextsector = -1;
					wall[j].nextwall = -1;
					wall[j].nextsector = -1;
				}
			}
		}
	}

	for(i=0;i<(MAXWALLS>>3);i++)   //Clear all highlights
		show2dwall[i] = 0;
	for(i=0;i<(MAXSPRITES>>3);i++)
		show2dsprite[i] = 0;

	sectorhighlightstat = -1;
	newnumwalls = -1;
	joinsector[0] = -1;
	circlewall = -1;
	circlepoints = 7;
	bstatus = 0;
	keystatus[buildkeys[14]] = 0;

	while ((keystatus[buildkeys[14]]>>1) == 0)
	{
		if (handleevents()) {
			if (quitevent) {
				keystatus[1] = 1;
				quitevent = 0;
			}
		}

		OSD_DispatchQueued();

		oldmousebstatus = bstatus;
		getmousevalues(&mousx,&mousy,&bstatus);
		mousx = (mousx<<16)+mousexsurp;
		mousy = (mousy<<16)+mouseysurp;
		{
		ldiv_t ld;
		ld = ldiv((long)((double)mousx*msens), (1<<16)); mousx = ld.quot; mousexsurp = ld.rem;
		ld = ldiv((long)((double)mousy*msens), (1<<16)); mousy = ld.quot; mouseysurp = ld.rem;
		}
		searchx += mousx;
		searchy += mousy;
		if (searchx < 8) searchx = 8;
		if (searchx > xdim-8-1) searchx = xdim-8-1;
		if (searchy < 8) searchy = 8;
		if (searchy > ydim-8-1) searchy = ydim-8-1;

		if (keystatus[0x3b] > 0) posx--, keystatus[0x3b] = 0;
		if (keystatus[0x3c] > 0) posx++, keystatus[0x3c] = 0;
		if (keystatus[0x3d] > 0) posy--, keystatus[0x3d] = 0;
		if (keystatus[0x3e] > 0) posy++, keystatus[0x3e] = 0;
		if (keystatus[0x43] > 0) ang--, keystatus[0x43] = 0;
		if (keystatus[0x44] > 0) ang++, keystatus[0x44] = 0;
		if (angvel != 0)          //ang += angvel * constant
		{                         //ENGINE calculates angvel for you
			doubvel = synctics;
			if (keystatus[buildkeys[4]] > 0)  //Lt. shift makes turn velocity 50% faster
				doubvel += (synctics>>1);
			ang += ((angvel*doubvel)>>4);
			ang = (ang+2048)&2047;
		}
		if ((vel|svel) != 0)
		{
			doubvel = synctics;
			if (keystatus[buildkeys[4]] > 0)     //Lt. shift doubles forward velocity
				doubvel += synctics;
			xvect = 0, yvect = 0;
			if (vel != 0)
			{
				xvect += ((vel*doubvel*(long)sintable[(ang+2560)&2047])>>3);
				yvect += ((vel*doubvel*(long)sintable[(ang+2048)&2047])>>3);
			}
			if (svel != 0)
			{
				xvect += ((svel*doubvel*(long)sintable[(ang+2048)&2047])>>3);
				yvect += ((svel*doubvel*(long)sintable[(ang+1536)&2047])>>3);
			}
			clipmove(&posx,&posy,&posz,&cursectnum,xvect,yvect,128L,4L<<8,4L<<8,CLIPMASK0);
		}

		getpoint(searchx,searchy,&mousxplc,&mousyplc);
		linehighlight = getlinehighlight(mousxplc, mousyplc);

		if (newnumwalls >= numwalls)
		{
			dax = mousxplc;
			day = mousyplc;
			adjustmark(&dax,&day,newnumwalls);
			wall[newnumwalls].x = dax;
			wall[newnumwalls].y = day;
		}

		ydim16 = ydim - STATUS2DSIZ;

		templong = numwalls;
		numwalls = newnumwalls;
		if (numwalls < 0) numwalls = templong;

		clear2dscreen();
		draw2dgrid(posx,posy,ang,zoom,grid);

		x2 = mulscale14(startposx-posx,zoom);          //Draw brown arrow (start)
		y2 = mulscale14(startposy-posy,zoom);
		if (((halfxdim16+x2) >= 2) && ((halfxdim16+x2) <= xdim-3))
			if (((midydim16+y2) >= 2) && ((midydim16+y2) <= ydim16-3))
			{
				x1 = mulscale11(sintable[(startang+2560)&2047],zoom) / 768;
				y1 = mulscale11(sintable[(startang+2048)&2047],zoom) / 768;
				begindrawing();	//{{{
				drawline16((halfxdim16+x2)+x1,(midydim16+y2)+y1,(halfxdim16+x2)-x1,(midydim16+y2)-y1,6);
				drawline16((halfxdim16+x2)+x1,(midydim16+y2)+y1,(halfxdim16+x2)+y1,(midydim16+y2)-x1,6);
				drawline16((halfxdim16+x2)+x1,(midydim16+y2)+y1,(halfxdim16+x2)-y1,(midydim16+y2)+x1,6);
				enddrawing();	//}}}
			}

		draw2dscreen(posx,posy,ang,zoom,grid);

		begindrawing();	//{{{
		if ((showtags == 1) && (zoom >= 768))
		{
			for(i=0;i<numsectors;i++)
			{
				dabuffer = (char *)ExtGetSectorCaption(i);
				if (dabuffer[0] != 0)
				{
					dax = 0;   //Get average point of sector
					day = 0;
					startwall = sector[i].wallptr;
					endwall = startwall + sector[i].wallnum - 1;
					for(j=startwall;j<=endwall;j++)
					{
						dax += wall[j].x;
						day += wall[j].y;
					}
					if (endwall > startwall)
					{
						dax /= (endwall-startwall+1);
						day /= (endwall-startwall+1);
					}

					dax = mulscale14(dax-posx,zoom);
					day = mulscale14(day-posy,zoom);

					x1 = halfxdim16+dax-(Bstrlen(dabuffer)<<1);
					y1 = midydim16+day-4;
					x2 = x1 + (Bstrlen(dabuffer)<<2)+2;
					y2 = y1 + 7;
					if ((x1 >= 0) && (x2 < xdim) && (y1 >= 0) && (y2 < ydim16))
						printext16(x1,y1,0,7,dabuffer,1);
				}
			}

			x3 = divscale14(-halfxdim16,zoom)+posx;
			y3 = divscale14(-(midydim16-4),zoom)+posy;
			x4 = divscale14(halfxdim16,zoom)+posx;
			y4 = divscale14(ydim16-(midydim16-4),zoom)+posy;

			for(i=numwalls-1,wal=&wall[i];i>=0;i--,wal--)
			{
					//Get average point of wall
				dax = ((wal->x+wall[wal->point2].x)>>1);
				day = ((wal->y+wall[wal->point2].y)>>1);
				if ((dax > x3) && (dax < x4) && (day > y3) && (day < y4))
				{
					dabuffer = (char *)ExtGetWallCaption(i);
					if (dabuffer[0] != 0)
					{
						dax = mulscale14(dax-posx,zoom);
						day = mulscale14(day-posy,zoom);
						x1 = halfxdim16+dax-(Bstrlen(dabuffer)<<1);
						y1 = midydim16+day-4;
						x2 = x1 + (Bstrlen(dabuffer)<<2)+2;
						y2 = y1 + 7;
						if ((x1 >= 0) && (x2 < xdim) && (y1 >= 0) && (y2 < ydim16))
							printext16(x1,y1,0,4,dabuffer,1);
					}
				}
			}

			i = 0; j = numsprites;
			while ((j > 0) && (i < MAXSPRITES))
			{
				if (sprite[i].statnum < MAXSTATUS)
				{
					dabuffer = (char *)ExtGetSpriteCaption(i);
					if (dabuffer[0] != 0)
					{
							//Get average point of sprite
						dax = sprite[i].x;
						day = sprite[i].y;

						dax = mulscale14(dax-posx,zoom);
						day = mulscale14(day-posy,zoom);

						x1 = halfxdim16+dax-(Bstrlen(dabuffer)<<1);
						y1 = midydim16+day-4;
						x2 = x1 + (Bstrlen(dabuffer)<<2)+2;
						y2 = y1 + 7;
						if ((x1 >= 0) && (x2 < xdim) && (y1 >= 0) && (y2 < ydim16))
						{
							if ((sprite[i].cstat&1) == 0) col = 3; else col = 5;
							printext16(x1,y1,0,col,dabuffer,1);
						}
					}
					j--;
				}
				i++;
			}
		}

		printcoords16(posx,posy,ang);

		numwalls = templong;

		if (highlightsectorcnt > 0)
			for(i=0;i<highlightsectorcnt;i++)
				fillsector(highlightsector[i],2);

		col = 15-((gridlock<<1)+gridlock);
		drawline16(searchx,searchy-8,searchx,searchy-1,col);
		drawline16(searchx+1,searchy-8,searchx+1,searchy-1,col);
		drawline16(searchx,searchy+2,searchx,searchy+9,col);
		drawline16(searchx+1,searchy+2,searchx+1,searchy+9,col);
		drawline16(searchx-8,searchy,searchx-1,searchy,col);
		drawline16(searchx-8,searchy+1,searchx-1,searchy+1,col);
		drawline16(searchx+2,searchy,searchx+9,searchy,col);
		drawline16(searchx+2,searchy+1,searchx+9,searchy+1,col);

			//Draw the white pixel closest to mouse cursor on linehighlight
		getclosestpointonwall(mousxplc,mousyplc,(long)linehighlight,&dax,&day);
		x2 = mulscale14(dax-posx,zoom);
		y2 = mulscale14(day-posy,zoom);
		if (wall[linehighlight].nextsector >= 0)
			drawline16(halfxdim16+x2,midydim16+y2,halfxdim16+x2,midydim16+y2,15);
		else
			drawline16(halfxdim16+x2,midydim16+y2,halfxdim16+x2,midydim16+y2,5);
		enddrawing();	//}}}

		OSD_Draw();

        Display2DStats();                                            // wxst

        if (keystatus[0x58] > 0)   //F12 2D
	   	   {
			keystatus[0x58] = 0;
			screencapture("captxxxx.tga",keystatus[0x2a]|keystatus[0x36]);
			showframe(1);
		   }

		if (keystatus[0x30] > 0)  // B (clip Blocking xor) (2D)
		{
			pointhighlight = getpointhighlight(mousxplc, mousyplc);
			linehighlight = getlinehighlight(mousxplc, mousyplc);

			if ((pointhighlight&0xc000) == 16384)
			{
				sprite[pointhighlight&16383].cstat ^= 1;
				sprite[pointhighlight&16383].cstat &= ~256;
				sprite[pointhighlight&16383].cstat |= ((sprite[pointhighlight&16383].cstat&1)<<8);
				asksave = 1;
			}
			else if (linehighlight >= 0)
			{
				wall[linehighlight].cstat ^= 1;
				wall[linehighlight].cstat &= ~64;
				if ((wall[linehighlight].nextwall >= 0) && ((keystatus[0x2a]|keystatus[0x36]) == 0))
				{
					wall[wall[linehighlight].nextwall].cstat &= ~(1+64);
					wall[wall[linehighlight].nextwall].cstat |= (wall[linehighlight].cstat&1);
				}
				asksave = 1;
			}
			keystatus[0x30] = 0;
		}
		if (keystatus[0x21] > 0)  //F (F alone does nothing in 2D right now)
		{
			keystatus[0x21] = 0;
			if ((keystatus[0x38]|keystatus[0xb8]) > 0)  //ALT-F (relative alignmment flip)
			{
				linehighlight = getlinehighlight(mousxplc, mousyplc);
				if (linehighlight >= 0)
				{
					setfirstwall(sectorofwall(linehighlight),linehighlight);
					asksave = 1;
					printmessage16("This wall now sector's first wall (sector[].wallptr)");
				}
			}
		}

		if (keystatus[0x18] > 0)  // O (ornament onto wall) (2D)
		{
			keystatus[0x18] = 0;
			if ((pointhighlight&0xc000) == 16384)
			{
				asksave = 1;
				i = (pointhighlight&16383);

				hitscan(sprite[i].x,sprite[i].y,sprite[i].z,sprite[i].sectnum,
					sintable[(sprite[i].ang+2560+1024)&2047],
					sintable[(sprite[i].ang+2048+1024)&2047],
					0,
					&hitsect,&hitwall,&hitsprite,&hitx,&hity,&hitz,CLIPMASK1);

				sprite[i].x = hitx;
				sprite[i].y = hity;
				sprite[i].z = hitz;
				changespritesect(i,hitsect);
				if (hitwall >= 0)
					sprite[i].ang = ((getangle(wall[wall[hitwall].point2].x-wall[hitwall].x,wall[wall[hitwall].point2].y-wall[hitwall].y)+512)&2047);

					//Make sure sprite's in right sector
				if (inside(sprite[i].x,sprite[i].y,sprite[i].sectnum) == 0)
				{
					j = wall[hitwall].point2;
					sprite[i].x -= ksgn(wall[j].y-wall[hitwall].y);
					sprite[i].y += ksgn(wall[j].x-wall[hitwall].x);
				}
			}
		}

		if (keystatus[0x33] > 0)  // , (2D)
		{
			if (highlightsectorcnt > 0)
			{
				k = 0;
				dax = 0;
				day = 0;
				for(i=0;i<highlightsectorcnt;i++)
				{
					startwall = sector[highlightsector[i]].wallptr;
					endwall = startwall+sector[highlightsector[i]].wallnum-1;
					for(j=startwall;j<=endwall;j++)
					{
						dax += wall[j].x;
						day += wall[j].y;
						k++;
					}
				}
				if (k > 0)
				{
					dax /= k;
					day /= k;
				}

				k = (keystatus[0x2a]|keystatus[0x36]);

				if (k == 0)
				{
					if ((gridlock > 0) && (grid > 0))
					{
						dax = ((dax+(1024>>grid))&(0xffffffff<<(11-grid)));
						day = ((day+(1024>>grid))&(0xffffffff<<(11-grid)));
					}
				}

				for(i=0;i<highlightsectorcnt;i++)
				{
					startwall = sector[highlightsector[i]].wallptr;
					endwall = startwall+sector[highlightsector[i]].wallnum-1;
					for(j=startwall;j<=endwall;j++)
					{
						if (k == 0)
						{
							x3 = wall[j].x;
							y3 = wall[j].y;
							wall[j].x = dax+day-y3;
							wall[j].y = day+x3-dax;
						}
						else
						{
							rotatepoint(dax,day,wall[j].x,wall[j].y,1,&wall[j].x,&wall[j].y);
						}
					}

					j = headspritesect[highlightsector[i]];
					while (j != -1)
					{
						if (k == 0)
						{
							x3 = sprite[j].x;
							y3 = sprite[j].y;
							sprite[j].x = dax+day-y3;
							sprite[j].y = day+x3-dax;
							sprite[j].ang = ((sprite[j].ang+512)&2047);
						}
						else
						{
							rotatepoint(dax,day,sprite[j].x,sprite[j].y,1,&sprite[j].x,&sprite[j].y);
							sprite[j].ang = ((sprite[j].ang+1)&2047);
						}

						j = nextspritesect[j];
					}
				}
				if (k == 0) keystatus[0x33] = 0;
				asksave = 1;
			}
			else
			{
				if (pointhighlight >= 16384)
				{
					i = pointhighlight-16384;
					if ((keystatus[0x2a]|keystatus[0x36]) > 0)
						sprite[i].ang = ((sprite[i].ang+2048-1)&2047);
					else
					{
						sprite[i].ang = ((sprite[i].ang+2048-128)&2047);
						keystatus[0x33] = 0;
					}

					clearmidstatbar16();
					showspritedata((short)pointhighlight-16384);
				}
			}
		}
		if (keystatus[0x34] > 0)  // .  (2D)
		{
			if (highlightsectorcnt > 0)
			{
				k = 0;
				dax = 0;
				day = 0;
				for(i=0;i<highlightsectorcnt;i++)
				{
					startwall = sector[highlightsector[i]].wallptr;
					endwall = startwall+sector[highlightsector[i]].wallnum-1;
					for(j=startwall;j<=endwall;j++)
					{
						dax += wall[j].x;
						day += wall[j].y;
						k++;
					}
				}
				if (k > 0)
				{
					dax /= k;
					day /= k;
				}

				k = (keystatus[0x2a]|keystatus[0x36]);

				if (k == 0)
				{
					if ((gridlock > 0) && (grid > 0))
					{
						dax = ((dax+(1024>>grid))&(0xffffffff<<(11-grid)));
						day = ((day+(1024>>grid))&(0xffffffff<<(11-grid)));
					}
				}

				for(i=0;i<highlightsectorcnt;i++)
				{
					startwall = sector[highlightsector[i]].wallptr;
					endwall = startwall+sector[highlightsector[i]].wallnum-1;
					for(j=startwall;j<=endwall;j++)
					{
						if (k == 0)
						{
							x3 = wall[j].x;
							y3 = wall[j].y;
							wall[j].x = dax+y3-day;
							wall[j].y = day+dax-x3;
						}
						else
						{
							rotatepoint(dax,day,wall[j].x,wall[j].y,2047,&wall[j].x,&wall[j].y);
						}
					}

					j = headspritesect[highlightsector[i]];
					while (j != -1)
					{
						if (k == 0)
						{
							x3 = sprite[j].x;
							y3 = sprite[j].y;
							sprite[j].x = dax+y3-day;
							sprite[j].y = day+dax-x3;
							sprite[j].ang = ((sprite[j].ang+1536)&2047);
						}
						else
						{
							rotatepoint(dax,day,sprite[j].x,sprite[j].y,2047,&sprite[j].x,&sprite[j].y);
							sprite[j].ang = ((sprite[j].ang+2047)&2047);
						}

						j = nextspritesect[j];
					}
				}
				if (k == 0) keystatus[0x34] = 0;
				asksave = 1;
			}
			else
			{
				if (pointhighlight >= 16384)
				{
					i = pointhighlight-16384;
					if ((keystatus[0x2a]|keystatus[0x36]) > 0)
						sprite[i].ang = ((sprite[i].ang+2048+1)&2047);
					else
					{
						sprite[i].ang = ((sprite[i].ang+2048+128)&2047);
						keystatus[0x34] = 0;
					}

					clearmidstatbar16();
					showspritedata((short)pointhighlight-16384);
				}
			}
		}
		if (keystatus[0x46] > 0)  //Scroll lock (set starting position)
		{
			startposx = posx;
			startposy = posy;
			startposz = posz;
			startang = ang;
			startsectnum = cursectnum;
			keystatus[0x46] = 0;
			asksave = 1;
		}

		if (keystatus[0x3f] > 0)  //F5   2D
		{
			keystatus[0x3f] = 0;
            clearmidstatbar16();                                             // wxyz
			for (i=0;i<numsectors;i++)
				if (inside(mousxplc,mousyplc,i) == 1)
				{
					ydim16 = STATUS2DSIZ;
					ExtShowSectorData((short)i);
					ydim16 = ydim-STATUS2DSIZ;
					break;
				}
		}
		if (keystatus[0x40] > 0)  //F6
		{
			keystatus[0x40] = 0;

			if (pointhighlight >= 16384)
			{
				i = pointhighlight-16384;

				ydim16 = STATUS2DSIZ;
				ExtShowSpriteData((short)i);
				ydim16 = ydim-STATUS2DSIZ;
			}
			else if (linehighlight >= 0)
			{
				i = linehighlight;

				ydim16 = STATUS2DSIZ;
				ExtShowWallData((short)i);
				ydim16 = ydim-STATUS2DSIZ;
			}

		}
		if (keystatus[0x41] > 0)  //F7
		{
			keystatus[0x41] = 0;
            clearmidstatbar16();                                             // wxyz
			for (i=0;i<numsectors;i++)
				if (inside(mousxplc,mousyplc,i) == 1)
				{
					ydim16 = STATUS2DSIZ;
					ExtEditSectorData((short)i);
					ydim16 = ydim-STATUS2DSIZ;
					break;
				}
		}
		if (keystatus[0x42] > 0)  //F8
		{
			keystatus[0x42] = 0;

			if (pointhighlight >= 16384)
			{
				i = pointhighlight-16384;

				ydim16 = STATUS2DSIZ;
				ExtEditSpriteData((short)i);
				ydim16 = ydim-STATUS2DSIZ;
			}
			else if (linehighlight >= 0)
			{
				i = linehighlight;

				ydim16 = STATUS2DSIZ;
				ExtEditWallData((short)i);
				ydim16 = ydim-STATUS2DSIZ;
			}
		}

		if (keystatus[0x14] > 0)  // T (tag)
		{
			keystatus[0x14] = 0;
			if ((keystatus[0x1d]|keystatus[0x9d]) > 0)  //Ctrl-T
			{
				showtags ^= 1;
				if (showtags == 0)
					printmessage16("Show tags OFF");
				else
					printmessage16("Show tags ON");
			}
			else if ((keystatus[0x38]|keystatus[0xb8]) > 0)  //ALT
			{
				if (pointhighlight >= 16384)
				{
					i = pointhighlight-16384;
					Bsprintf(buffer,"Sprite (%ld) Lo-tag: ",i);
					sprite[i].lotag = getnumber16(buffer,sprite[i].lotag,65536L,0);
					clearmidstatbar16();
					showspritedata((short)i);
				}
				else if (linehighlight >= 0)
				{
					i = linehighlight;
					Bsprintf(buffer,"Wall (%ld) Lo-tag: ",i);
					wall[i].lotag = getnumber16(buffer,wall[i].lotag,65536L,0);
					clearmidstatbar16();
					showwalldata((short)i);
				}
				printmessage16("");
			}
			else
			{
				for (i=0;i<numsectors;i++)
					if (inside(mousxplc,mousyplc,i) == 1)
					{
						Bsprintf(buffer,"Sector (%ld) Lo-tag: ",i);
						sector[i].lotag = getnumber16(buffer,sector[i].lotag,65536L,0);
						clearmidstatbar16();
						showsectordata((short)i);
						break;
					}
				printmessage16("");
			}
		}
		if (keystatus[0x23] > 0)  //H (Hi 16 bits of tag)
		{
			keystatus[0x23] = 0;
			if ((keystatus[0x1d]|keystatus[0x9d]) > 0)  //Ctrl-H
			{
				pointhighlight = getpointhighlight(mousxplc, mousyplc);
				linehighlight = getlinehighlight(mousxplc, mousyplc);

				if ((pointhighlight&0xc000) == 16384)
				{
					sprite[pointhighlight&16383].cstat ^= 256;
					asksave = 1;
				}
				else if (linehighlight >= 0)
				{
					wall[linehighlight].cstat ^= 64;
					if ((wall[linehighlight].nextwall >= 0) && ((keystatus[0x2a]|keystatus[0x36]) == 0))
					{
						wall[wall[linehighlight].nextwall].cstat &= ~64;
						wall[wall[linehighlight].nextwall].cstat |= (wall[linehighlight].cstat&64);
					}
					asksave = 1;
				}
			}
			else if ((keystatus[0x38]|keystatus[0xb8]) > 0)  //ALT
			{
				if (pointhighlight >= 16384)
				{
					i = pointhighlight-16384;
					Bsprintf(buffer,"Sprite (%ld) Hi-tag: ",i);
					sprite[i].hitag = getnumber16(buffer,sprite[i].hitag,65536L,0);
					clearmidstatbar16();
					showspritedata((short)i);
				}
				else if (linehighlight >= 0)
				{
					i = linehighlight;
					Bsprintf(buffer,"Wall (%ld) Hi-tag: ",i);
					wall[i].hitag = getnumber16(buffer,wall[i].hitag,65536L,0);
					clearmidstatbar16();
					showwalldata((short)i);
				}
			}
			else
			{
				for (i=0;i<numsectors;i++)
					if (inside(mousxplc,mousyplc,i) == 1)
					{
						Bsprintf(buffer,"Sector (%ld) Hi-tag: ",i);
						sector[i].hitag = getnumber16(buffer,sector[i].hitag,65536L,0);
						clearmidstatbar16();
						showsectordata((short)i);
						break;
					}
			}
			printmessage16("");
		}
		if (keystatus[0x19] > 0)  // P (palookup #)
		{
			keystatus[0x19] = 0;

			for (i=0;i<numsectors;i++)
				if (inside(mousxplc,mousyplc,i) == 1)
				{
					Bsprintf(buffer,"Sector (%ld) Ceilingpal: ",i);
					sector[i].ceilingpal = getnumber16(buffer,sector[i].ceilingpal,256L,0);
					clearmidstatbar16();
					showsectordata((short)i);

					Bsprintf(buffer,"Sector (%ld) Floorpal: ",i);
					sector[i].floorpal = getnumber16(buffer,sector[i].floorpal,256L,0);
					clearmidstatbar16();
					showsectordata((short)i);

					printmessage16("");
					break;
				}
		}
		if (keystatus[0x12] > 0)  // E (status list)
		{
			if (pointhighlight >= 16384)
			{
				i = pointhighlight-16384;
				Bsprintf(buffer,"Sprite (%ld) Status list: ",i);
				changespritestat(i,getnumber16(buffer,sprite[i].statnum,65536L,0));
				clearmidstatbar16();
				showspritedata((short)i);
			}

			printmessage16("");

			keystatus[0x12] = 0;
		}

		if (keystatus[0x0f] > 0)  //TAB
		{
			clearmidstatbar16();

			if ((keystatus[0x38]|keystatus[0xb8]|keystatus[0x1d]|keystatus[0x9d]) > 0)  //ALT or CTRL
			{
				if (pointhighlight >= 16384)
					showspritedata((short)pointhighlight-16384);
				else if (linehighlight >= 0)
					showwalldata((short)linehighlight);
			}
			else
			{
				for (i=0;i<numsectors;i++)
					if (inside(mousxplc,mousyplc,i) == 1)
					{
						showsectordata((short)i);
						break;
					}
			}
			keystatus[0x0f] = 0;
		}


		if (highlightsectorcnt < 0)
		{
			if (keystatus[0x36] > 0)  //Right shift (point highlighting)
			{
				if (highlightcnt == 0)
				{
					highlightx2 = searchx, highlighty2 = searchy;
					ydim16 = ydim-STATUS2DSIZ;
					drawline16(highlightx2,highlighty1,highlightx1,highlighty1,5);
					drawline16(highlightx2,highlighty2,highlightx1,highlighty2,5);
					drawline16(highlightx1,highlighty2,highlightx1,highlighty1,5);
					drawline16(highlightx2,highlighty2,highlightx2,highlighty1,5);
				}
				if (highlightcnt != 0)
				{
					highlightx1 = searchx;
					highlighty1 = searchy;
					highlightx2 = searchx;
					highlighty2 = searchx;
					highlightcnt = 0;

					for(i=0;i<(MAXWALLS>>3);i++)   //Clear all highlights
						show2dwall[i] = 0;
					for(i=0;i<(MAXSPRITES>>3);i++)
						show2dsprite[i] = 0;
				}
			}
			else
			{
				if (highlightcnt == 0)
				{
					getpoint(highlightx1,highlighty1,&highlightx1,&highlighty1);
					getpoint(highlightx2,highlighty2,&highlightx2,&highlighty2);
					if (highlightx1 > highlightx2)
					{
						templong = highlightx1; highlightx1 = highlightx2; highlightx2 = templong;
					}
					if (highlighty1 > highlighty2)
					{
						templong = highlighty1; highlighty1 = highlighty2; highlighty2 = templong;
					}

					if ((keystatus[0x1d]|keystatus[0x9d]) > 0)
					{
						if ((linehighlight >= 0) && (linehighlight < MAXWALLS))
						{
							i = linehighlight;
							do
							{
								highlight[highlightcnt++] = i;
								show2dwall[i>>3] |= (1<<(i&7));

								for(j=0;j<numwalls;j++)
									if (wall[j].x == wall[i].x)
										if (wall[j].y == wall[i].y)
											if (i != j)
											{
												highlight[highlightcnt++] = j;
												show2dwall[j>>3] |= (1<<(j&7));
											}

								i = wall[i].point2;
							}
							while (i != linehighlight);
						}
					}
					else
					{
						for(i=0;i<numwalls;i++)
							if ((wall[i].x >= highlightx1) && (wall[i].x <= highlightx2))
								if ((wall[i].y >= highlighty1) && (wall[i].y <= highlighty2))
								{
									highlight[highlightcnt++] = i;
									show2dwall[i>>3] |= (1<<(i&7));
								}
						for(i=0;i<MAXSPRITES;i++)
							if (sprite[i].statnum < MAXSTATUS)
								if ((sprite[i].x >= highlightx1) && (sprite[i].x <= highlightx2))
									if ((sprite[i].y >= highlighty1) && (sprite[i].y <= highlighty2))
									{
										highlight[highlightcnt++] = i+16384;
										show2dsprite[i>>3] |= (1<<(i&7));
									}
					}

					if (highlightcnt <= 0)
						highlightcnt = -1;
				}
			}
		}
		if (highlightcnt < 0)
		{
			if (keystatus[0xb8] > 0)  //Right alt (sector highlighting)
			{
				if (highlightsectorcnt == 0)
				{
					highlightx2 = searchx, highlighty2 = searchy;
					ydim16 = ydim-STATUS2DSIZ;
					begindrawing();	//{{{
					drawline16(highlightx2,highlighty1,highlightx1,highlighty1,10);
					drawline16(highlightx2,highlighty2,highlightx1,highlighty2,10);
					drawline16(highlightx1,highlighty2,highlightx1,highlighty1,10);
					drawline16(highlightx2,highlighty2,highlightx2,highlighty1,10);
					enddrawing();	//}}}
				}
				if (highlightsectorcnt != 0)
				{
					for(i=0;i<highlightsectorcnt;i++)
					{
						startwall = sector[highlightsector[i]].wallptr;
						endwall = startwall+sector[highlightsector[i]].wallnum-1;
						for(j=startwall;j<=endwall;j++)
						{
							if (wall[j].nextwall >= 0)
								checksectorpointer(wall[j].nextwall,wall[j].nextsector);
							checksectorpointer((short)j,highlightsector[i]);
						}
					}
					highlightx1 = searchx;
					highlighty1 = searchy;
					highlightx2 = searchx;
					highlighty2 = searchx;
					highlightsectorcnt = 0;
				}
			}
			else
			{
				if (highlightsectorcnt == 0)
				{
					getpoint(highlightx1,highlighty1,&highlightx1,&highlighty1);
					getpoint(highlightx2,highlighty2,&highlightx2,&highlighty2);
					if (highlightx1 > highlightx2)
					{
						templong = highlightx1; highlightx1 = highlightx2; highlightx2 = templong;
					}
					if (highlighty1 > highlighty2)
					{
						templong = highlighty1; highlighty1 = highlighty2; highlighty2 = templong;
					}

					for(i=0;i<numsectors;i++)
					{
						startwall = sector[i].wallptr;
						endwall = startwall + sector[i].wallnum;
						bad = 0;
						for(j=startwall;j<endwall;j++)
						{
							if (wall[j].x < highlightx1) bad = 1;
							if (wall[j].x > highlightx2) bad = 1;
							if (wall[j].y < highlighty1) bad = 1;
							if (wall[j].y > highlighty2) bad = 1;
							if (bad == 1) break;
						}
						if (bad == 0)
							highlightsector[highlightsectorcnt++] = i;
					}
					if (highlightsectorcnt <= 0)
						highlightsectorcnt = -1;

						//White out all bordering lines of grab that are
						//not highlighted on both sides
					for(i=highlightsectorcnt-1;i>=0;i--)
					{
						startwall = sector[highlightsector[i]].wallptr;
						endwall = startwall + sector[highlightsector[i]].wallnum;
						for(j=startwall;j<endwall;j++)
						{
							if (wall[j].nextwall >= 0)
							{
								for(k=highlightsectorcnt-1;k>=0;k--)
									if (highlightsector[k] == wall[j].nextsector)
										break;
								if (k < 0)
								{
									wall[wall[j].nextwall].nextwall = -1;
									wall[wall[j].nextwall].nextsector = -1;
									wall[j].nextwall = -1;
									wall[j].nextsector = -1;
								}
							}
						}
					}

				}
			}
		}

		if (((bstatus&1) < (oldmousebstatus&1)) && (highlightsectorcnt < 0))  //after dragging
		{
			j = 1;
			if (highlightcnt > 0)
				for (i=0;i<highlightcnt;i++)
					if (pointhighlight == highlight[i])
					{
						j = 0;
						break;
					}

			if (j == 0)
			{
				for(i=0;i<highlightcnt;i++)
				{
					if ((highlight[i]&0xc000) == 16384)
					{
						j = (highlight[i]&16383);

						setsprite(j,sprite[j].x,sprite[j].y,sprite[j].z);

						templong = ((tilesizy[sprite[j].picnum]*sprite[j].yrepeat)<<2);
						sprite[j].z = max(sprite[j].z,getceilzofslope(sprite[j].sectnum,sprite[j].x,sprite[j].y)+templong);
						sprite[j].z = min(sprite[j].z,getflorzofslope(sprite[j].sectnum,sprite[j].x,sprite[j].y));
					}
				}
			}
			else if ((pointhighlight&0xc000) == 16384)
			{
				j = (pointhighlight&16383);

				setsprite(j,sprite[j].x,sprite[j].y,sprite[j].z);

				templong = ((tilesizy[sprite[j].picnum]*sprite[j].yrepeat)<<2);

				sprite[j].z = max(sprite[j].z,getceilzofslope(sprite[j].sectnum,sprite[j].x,sprite[j].y)+templong);
				sprite[j].z = min(sprite[j].z,getflorzofslope(sprite[j].sectnum,sprite[j].x,sprite[j].y));
			}

			if ((pointhighlight&0xc000) == 0)
			{
				dax = wall[pointhighlight].x;
				day = wall[pointhighlight].y;
			}
			else if ((pointhighlight&0xc000) == 16384)
			{
				dax = sprite[pointhighlight&16383].x;
				day = sprite[pointhighlight&16383].y;
			}

			for(i=numwalls-1;i>=0;i--)     //delete points
			{
				if (wall[i].x == wall[wall[i].point2].x)
					if (wall[i].y == wall[wall[i].point2].y)
					{
						deletepoint((short)i);
						printmessage16("Point deleted.");
						asksave = 1;
					}
			}
			for(i=0;i<numwalls;i++)        //make new red lines?
			{
				if ((wall[i].x == dax) && (wall[i].y == day))
				{
					checksectorpointer((short)i,sectorofwall((short)i));
					fixrepeats((short)i);
					asksave = 1;
				}
				else if ((wall[wall[i].point2].x == dax) && (wall[wall[i].point2].y == day))
				{
					checksectorpointer((short)i,sectorofwall((short)i));
					fixrepeats((short)i);
					asksave = 1;
				}
			}

		}

		if ((bstatus&1) > 0)                //drag points
		{
			if (highlightsectorcnt > 0)
			{
				if ((bstatus&1) > (oldmousebstatus&1))
				{
					newnumwalls = -1;
					sectorhighlightstat = -1;
					updatesector(mousxplc,mousyplc,&cursectorhighlight);

					if ((cursectorhighlight >= 0) && (cursectorhighlight < numsectors))
					{
						for (i=0;i<highlightsectorcnt;i++)
							if (cursectorhighlight == highlightsector[i])
							{
									//You clicked inside one of the flashing sectors!
								sectorhighlightstat = 1;

								dax = mousxplc;
								day = mousyplc;
								if ((gridlock > 0) && (grid > 0))
								{
									dax = ((dax+(1024>>grid))&(0xffffffff<<(11-grid)));
									day = ((day+(1024>>grid))&(0xffffffff<<(11-grid)));
								}
								sectorhighlightx = dax;
								sectorhighlighty = day;
								break;
							}
					}
				}
				else if (sectorhighlightstat == 1)
				{
					dax = mousxplc;
					day = mousyplc;
					if ((gridlock > 0) && (grid > 0))
					{
						dax = ((dax+(1024>>grid))&(0xffffffff<<(11-grid)));
						day = ((day+(1024>>grid))&(0xffffffff<<(11-grid)));
					}

					dax -= sectorhighlightx;
					day -= sectorhighlighty;
					sectorhighlightx += dax;
					sectorhighlighty += day;

					for(i=0;i<highlightsectorcnt;i++)
					{
						startwall = sector[highlightsector[i]].wallptr;
						endwall = startwall+sector[highlightsector[i]].wallnum-1;
						for(j=startwall;j<=endwall;j++)
							{ wall[j].x += dax; wall[j].y += day; }

						for(j=headspritesect[highlightsector[i]];j>=0;j=nextspritesect[j])
							{ sprite[j].x += dax; sprite[j].y += day; }
					}

					//for(i=0;i<highlightsectorcnt;i++)
					//{
					//   startwall = sector[highlightsector[i]].wallptr;
					//   endwall = startwall+sector[highlightsector[i]].wallnum-1;
					//   for(j=startwall;j<=endwall;j++)
					//   {
					//      if (wall[j].nextwall >= 0)
					//         checksectorpointer(wall[j].nextwall,wall[j].nextsector);
					//     checksectorpointer((short)j,highlightsector[i]);
					//   }
					//}
					asksave = 1;
				}

			}
			else
			{
				if ((bstatus&1) > (oldmousebstatus&1))
					pointhighlight = getpointhighlight(mousxplc, mousyplc);

				if (pointhighlight >= 0)
				{
					dax = mousxplc;
					day = mousyplc;
					if ((gridlock > 0) && (grid > 0))
					{
						dax = ((dax+(1024>>grid))&(0xffffffff<<(11-grid)));
						day = ((day+(1024>>grid))&(0xffffffff<<(11-grid)));
					}

					j = 1;
					if (highlightcnt > 0)
						for (i=0;i<highlightcnt;i++)
							if (pointhighlight == highlight[i])
								{ j = 0; break; }

					if (j == 0)
					{
						if ((pointhighlight&0xc000) == 0)
						{
							dax -= wall[pointhighlight].x;
							day -= wall[pointhighlight].y;
						}
						else
						{
							dax -= sprite[pointhighlight&16383].x;
							day -= sprite[pointhighlight&16383].y;
						}
						for(i=0;i<highlightcnt;i++)
						{
							if ((highlight[i]&0xc000) == 0)
							{
								wall[highlight[i]].x += dax;
								wall[highlight[i]].y += day;
							}
							else
							{
								sprite[highlight[i]&16383].x += dax;
								sprite[highlight[i]&16383].y += day;
							}
						}
					}
					else
					{
						if ((pointhighlight&0xc000) == 0)
							dragpoint(pointhighlight,dax,day);
						else if ((pointhighlight&0xc000) == 16384)
						{
							daz = ((tilesizy[sprite[pointhighlight&16383].picnum]*sprite[pointhighlight&16383].yrepeat)<<2);

							for(i=0;i<numsectors;i++)
								if (inside(dax,day,i) == 1)
									if (sprite[pointhighlight&16383].z >= getceilzofslope(i,dax,day))
										if (sprite[pointhighlight&16383].z-daz <= getflorzofslope(i,dax,day))
										{
											sprite[pointhighlight&16383].x = dax;
											sprite[pointhighlight&16383].y = day;
											if (sprite[pointhighlight&16383].sectnum != i)
												changespritesect(pointhighlight&16383,(short)i);
											break;
										}
						}
					}
					asksave = 1;
				}
			}
		}
		else
		{
			pointhighlight = getpointhighlight(mousxplc, mousyplc);
			sectorhighlightstat = -1;
		}

		if ((bstatus&6) > 0)
		{
			searchx = halfxdim16;
			searchy = midydim16;
			posx = mousxplc;
			posy = mousyplc;
		}

		if (((keystatus[buildkeys[8]] > 0) || (bstatus&16)) && (zoom < 16384)) zoom += synctics*(zoom>>4);
		if (((keystatus[buildkeys[9]] > 0) || (bstatus&32)) && (zoom > 24)) zoom -= synctics*(zoom>>4);

		if (keystatus[0x22] > 0)  // G (grid on/off)
		{
			grid++;
			if (grid == 7) grid = 0;
			keystatus[0x22] = 0;
		}
		if (keystatus[0x26] > 0)  // L (grid lock)
		{
			gridlock = 1-gridlock, keystatus[0x26] = 0;
			if (gridlock == 0)
				printmessage16("Grid locking OFF");
			else
				printmessage16("Grid locking ON");
		}

		if (keystatus[0x24] > 0)  // J (join sectors)
		{
			if (joinsector[0] >= 0)
			{
				joinsector[1] = -1;
				for(i=0;i<numsectors;i++)
					if (inside(mousxplc,mousyplc,i) == 1)
					{
						joinsector[1] = i;
						break;
					}
				if ((joinsector[1] >= 0) && (joinsector[0] != joinsector[1]))
				{
					newnumwalls = numwalls;

					for(k=0;k<2;k++)
					{
						startwall = sector[joinsector[k]].wallptr;
						endwall = startwall + sector[joinsector[k]].wallnum - 1;
						for(j=startwall;j<=endwall;j++)
						{
							if (wall[j].cstat == 255)
								continue;
							joinsectnum = k;
							if (wall[j].nextsector == joinsector[1-joinsectnum])
							{
								wall[j].cstat = 255;
								continue;
							}

							i = j;
							m = newnumwalls;
							do
							{
								Bmemcpy(&wall[newnumwalls],&wall[i],sizeof(walltype));
								wall[newnumwalls].point2 = newnumwalls+1;
								newnumwalls++;
								wall[i].cstat = 255;

								i = wall[i].point2;
								if (wall[i].nextsector == joinsector[1-joinsectnum])
								{
									i = wall[wall[i].nextwall].point2;
									joinsectnum = 1 - joinsectnum;
								}
							}
							while ((wall[i].cstat != 255) && (wall[i].nextsector != joinsector[1-joinsectnum]));
							wall[newnumwalls-1].point2 = m;
						}
					}

					if (newnumwalls > numwalls)
					{
						Bmemcpy(&sector[numsectors],&sector[joinsector[0]],sizeof(sectortype));
						sector[numsectors].wallptr = numwalls;
						sector[numsectors].wallnum = newnumwalls-numwalls;

						//fix sprites
						for(i=0;i<2;i++)
						{
							j = headspritesect[joinsector[i]];
							while (j != -1)
							{
								k = nextspritesect[j];
								changespritesect(j,numsectors);
								j = k;
							}
						}

						numsectors++;

						for(i=numwalls;i<newnumwalls;i++)
						{
							if (wall[i].nextwall >= 0)
							{
								wall[wall[i].nextwall].nextwall = i;
								wall[wall[i].nextwall].nextsector = numsectors-1;
							}
						}

						numwalls = newnumwalls;
						newnumwalls = -1;

						for(k=0;k<2;k++)
						{
							startwall = sector[joinsector[k]].wallptr;
							endwall = startwall + sector[joinsector[k]].wallnum - 1;
							for(j=startwall;j<=endwall;j++)
							{
								wall[j].nextwall = -1;
								wall[j].nextsector = -1;
							}
						}

						deletesector((short)joinsector[0]);
						if (joinsector[0] < joinsector[1])
							joinsector[1]--;
						deletesector((short)joinsector[1]);
						printmessage16("Sectors joined.");
					}
				}
				joinsector[0] = -1;
			}
			else
			{
				joinsector[0] = -1;
				for(i=0;i<numsectors;i++)
					if (inside(mousxplc,mousyplc,i) == 1)
					{
						joinsector[0] = i;
						printmessage16("Join sector - press J again on sector to join with.");
						break;
					}
			}
			keystatus[0x24] = 0;
		}

		if (((keystatus[0x38]|keystatus[0xb8])&keystatus[0x1f]) > 0) //ALT-S
		{
			if ((linehighlight >= 0) && (wall[linehighlight].nextwall == -1))
			{
				if ((newnumwalls = whitelinescan(linehighlight)) < numwalls)
				{
					printmessage16("Can't make a sector out there.");
				}
				else
				{
					for(i=numwalls;i<newnumwalls;i++)
					{
						wall[wall[i].nextwall].nextwall = i;
						wall[wall[i].nextwall].nextsector = numsectors;
					}
					numwalls = newnumwalls;
					newnumwalls = -1;
					numsectors++;
					printmessage16("Inner loop made into new sector.");
				}
			}
			keystatus[0x1f] = 0;
		}
		else if (keystatus[0x1f] > 0)  //S
		{
			sucksect = -1;
			for(i=0;i<numsectors;i++)
				if (inside(mousxplc,mousyplc,i) == 1)
				{
					sucksect = i;
					break;
				}

			if (sucksect >= 0)
			{
				dax = mousxplc;
				day = mousyplc;
				if ((gridlock > 0) && (grid > 0))
				{
					dax = ((dax+(1024>>grid))&(0xffffffff<<(11-grid)));
					day = ((day+(1024>>grid))&(0xffffffff<<(11-grid)));
				}

				i = insertsprite(sucksect,0);
				sprite[i].x = dax, sprite[i].y = day;
				sprite[i].cstat = defaultspritecstat;
				sprite[i].shade = 0;
				sprite[i].pal = 0;
				sprite[i].xrepeat = 64, sprite[i].yrepeat = 64;
				sprite[i].xoffset = 0, sprite[i].yoffset = 0;
				sprite[i].ang = 1536;
				sprite[i].xvel = 0; sprite[i].yvel = 0; sprite[i].zvel = 0;
				sprite[i].owner = -1;
				sprite[i].clipdist = 32;
				sprite[i].lotag = 0;
				sprite[i].hitag = 0;
				sprite[i].extra = -1;

				sprite[i].z = getflorzofslope(sucksect,dax,day);
				if ((sprite[i].cstat&128) != 0)
					sprite[i].z -= ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<1);

				for(k=0;k<MAXTILES;k++)
					localartfreq[k] = 0;
				for(k=0;k<MAXSPRITES;k++)
					if (sprite[k].statnum < MAXSTATUS)
						localartfreq[sprite[k].picnum]++;
				j = 0;
				for(k=0;k<MAXTILES;k++)
					if (localartfreq[k] > localartfreq[j])
						j = k;
				if (localartfreq[j] > 0)
					sprite[i].picnum = j;
				else
					sprite[i].picnum = 0;

				if (somethingintab == 3)
				{
					sprite[i].picnum = temppicnum;
					if ((tilesizx[temppicnum] <= 0) || (tilesizy[temppicnum] <= 0))
					{
						j = 0;
						for(k=0;k<MAXTILES;k++)
							if ((tilesizx[k] > 0) && (tilesizy[k] > 0))
							{
								j = k;
								break;
							}
						sprite[i].picnum = j;
					}
					sprite[i].shade = tempshade;
					sprite[i].pal = temppal;
					sprite[i].xrepeat = tempxrepeat;
					sprite[i].yrepeat = tempyrepeat;
					if (sprite[i].xrepeat < 1) sprite[i].xrepeat = 1;
					if (sprite[i].yrepeat < 1) sprite[i].yrepeat = 1;
					sprite[i].cstat = tempcstat;
				}

				if (tilesizy[sprite[i].picnum] >= 32)
					sprite[i].cstat |= 1;

				printmessage16("Sprite inserted.");
				updatenumsprites();
				asksave = 1;
			}

			keystatus[0x1f] = 0;
		}

		if (keystatus[0x2e] > 0)  // C (make circle of points)
		{
			if (circlewall >= 0)
			{
				circlewall = -1;
			}
			else
			{
				if (linehighlight >= 0)
					circlewall = linehighlight;
			}
			keystatus[0x2e] = 0;
		}
		if (keystatus[0x4a] > 0)  // -
		{
			if (circlepoints > 1)
				circlepoints--;
			keystatus[0x4a] = 0;
		}
		if (keystatus[0x4e] > 0)  // +
		{
			if (circlepoints < 63)
				circlepoints++;
			keystatus[0x4e] = 0;
		}

		bad = (keystatus[0x39] > 0);  //Gotta do this to save lots of 3 spaces!

		if (circlewall >= 0)
		{
			x1 = wall[circlewall].x;
			y1 = wall[circlewall].y;
			x2 = wall[wall[circlewall].point2].x;
			y2 = wall[wall[circlewall].point2].y;
			x3 = mousxplc;
			y3 = mousyplc;
			adjustmark(&x3,&y3,newnumwalls);
			templong1 = dmulscale4(x3-x2,x1-x3,y1-y3,y3-y2);
			templong2 = dmulscale4(y1-y2,x1-x3,y1-y3,x2-x1);
			if (templong2 != 0)
			{
				centerx = (((x1+x2) + scale(y1-y2,templong1,templong2))>>1);
				centery = (((y1+y2) + scale(x2-x1,templong1,templong2))>>1);

				dax = mulscale14(centerx-posx,zoom);
				day = mulscale14(centery-posy,zoom);
				drawline16(halfxdim16+dax-2,midydim16+day-2,halfxdim16+dax+2,midydim16+day+2,14);
				drawline16(halfxdim16+dax-2,midydim16+day+2,halfxdim16+dax+2,midydim16+day-2,14);

				circleang1 = getangle(x1-centerx,y1-centery);
				circleang2 = getangle(x2-centerx,y2-centery);

				circleangdir = 1;
				k = ((circleang2-circleang1)&2047);
				if (mulscale4(x3-x1,y2-y1) < mulscale4(x2-x1,y3-y1))
				{
					circleangdir = -1;
					k = -((circleang1-circleang2)&2047);
				}

				circlerad = (ksqrt(dmulscale4(centerx-x1,centerx-x1,centery-y1,centery-y1))<<2);

				for(i=circlepoints;i>0;i--)
				{
					j = ((circleang1 + scale(i,k,circlepoints+1))&2047);
					dax = centerx+mulscale14(sintable[(j+512)&2047],circlerad);
					day = centery+mulscale14(sintable[j],circlerad);

					if (dax <= -editorgridextent) dax = -editorgridextent;
					if (dax >= editorgridextent) dax = editorgridextent;
					if (day <= -editorgridextent) day = -editorgridextent;
					if (day >= editorgridextent) day = editorgridextent;

					if (bad > 0)
					{
						m = 0;
						if (wall[circlewall].nextwall >= 0)
							if (wall[circlewall].nextwall < circlewall) m = 1;
						insertpoint(circlewall,dax,day);
						circlewall += m;
					}
					dax = mulscale14(dax-posx,zoom);
					day = mulscale14(day-posy,zoom);
					drawline16(halfxdim16+dax-2,midydim16+day-2,halfxdim16+dax+2,midydim16+day-2,14);
					drawline16(halfxdim16+dax+2,midydim16+day-2,halfxdim16+dax+2,midydim16+day+2,14);
					drawline16(halfxdim16+dax+2,midydim16+day+2,halfxdim16+dax-2,midydim16+day+2,14);
					drawline16(halfxdim16+dax-2,midydim16+day+2,halfxdim16+dax-2,midydim16+day-2,14);
				}
				if (bad > 0)
				{
					bad = 0;
					keystatus[0x39] = 0;
					asksave = 1;
					printmessage16("Circle points inserted.");
					circlewall = -1;
				}
			}
		}

		if (bad > 0)   //Space bar test
		{
			keystatus[0x39] = 0;
			adjustmark(&mousxplc,&mousyplc,newnumwalls);
			if (checkautoinsert(mousxplc,mousyplc,newnumwalls) == 1)
			{
				printmessage16("You must insert a point there first.");
				bad = 0;
			}
		}

		if (bad > 0)  //Space
		{
			if ((newnumwalls < numwalls) && (numwalls < MAXWALLS-1))
			{
				firstx = mousxplc, firsty = mousyplc;  //Make first point
				newnumwalls = numwalls;
				suckwall = -1;
				split = 0;

				//clearbufbyte(&wall[newnumwalls],sizeof(walltype),0L);
				memset(&wall[newnumwalls],0,sizeof(walltype));
				wall[newnumwalls].extra = -1;

				wall[newnumwalls].x = mousxplc;
				wall[newnumwalls].y = mousyplc;
				wall[newnumwalls].nextsector = -1;
				wall[newnumwalls].nextwall = -1;
				for(i=0;i<numwalls;i++)
					if ((wall[i].x == mousxplc) && (wall[i].y == mousyplc))
						suckwall = i;
				wall[newnumwalls].point2 = newnumwalls+1;
				printmessage16("Sector drawing started.");
				newnumwalls++;
			}
			else
			{  //if not back to first point
				if ((firstx != mousxplc) || (firsty != mousyplc))  //nextpoint
				{
					j = 0;
					for(i=numwalls;i<newnumwalls;i++)
						if ((mousxplc == wall[i].x) && (mousyplc == wall[i].y))
							j = 1;
					if (j == 0)
					{
							//check if starting to split a sector
						if (newnumwalls == numwalls+1)
						{
							dax = ((wall[numwalls].x+mousxplc)>>1);
							day = ((wall[numwalls].y+mousyplc)>>1);
							for(i=0;i<numsectors;i++)
								if (inside(dax,day,i) == 1)
								{    //check if first point at point of sector
									m = -1;
									startwall = sector[i].wallptr;
									endwall = startwall + sector[i].wallnum - 1;
									for(k=startwall;k<=endwall;k++)
										if (wall[k].x == wall[numwalls].x)
											if (wall[k].y == wall[numwalls].y)
											{
												m = k;
												break;
											}
									if (m >= 0)
										if ((wall[wall[k].point2].x != mousxplc) || (wall[wall[k].point2].y != mousyplc))
											if ((wall[lastwall((short)k)].x != mousxplc) || (wall[lastwall((short)k)].y != mousyplc))
											{
												split = 1;
												splitsect = i;
												splitstartwall = m;
												break;
											}
								}
						}

							//make new point

						//make sure not drawing over old red line
						bad = 0;
						for(i=0;i<numwalls;i++)
						{
							if (wall[i].nextwall >= 0)
							{
								if ((wall[i].x == mousxplc) && (wall[i].y == mousyplc))
									if ((wall[wall[i].point2].x == wall[newnumwalls-1].x) && (wall[wall[i].point2].y == wall[newnumwalls-1].y))
										bad = 1;
								if ((wall[i].x == wall[newnumwalls-1].x) && (wall[i].y == wall[newnumwalls-1].y))
									if ((wall[wall[i].point2].x == mousxplc) && (wall[wall[i].point2].y == mousyplc))
										bad = 1;
							}
						}

						if (bad == 0)
						{
							//clearbufbyte(&wall[newnumwalls],sizeof(walltype),0L);
							memset(&wall[newnumwalls],0,sizeof(walltype));
							wall[newnumwalls].extra = -1;

							wall[newnumwalls].x = mousxplc;
							wall[newnumwalls].y = mousyplc;
							wall[newnumwalls].nextsector = -1;
							wall[newnumwalls].nextwall = -1;
							for(i=0;i<numwalls;i++)
								if ((wall[i].x == mousxplc) && (wall[i].y == mousyplc))
									suckwall = i;
							wall[newnumwalls].point2 = newnumwalls+1;
							newnumwalls++;
						}
						else
						{
							printmessage16("You can't draw new lines over red lines.");
						}
					}
				}

					//if not split and back to first point
				if ((split == 0) && (firstx == mousxplc) && (firsty == mousyplc) && (newnumwalls >= numwalls+3))
				{
					wall[newnumwalls-1].point2 = numwalls;

					if (suckwall == -1)  //if no connections to other sectors
					{
						k = -1;
						for(i=0;i<numsectors;i++)
							if (inside(firstx,firsty,i) == 1)
								k = i;
						if (k == -1)   //if not inside another sector either
						{              //add island sector
							if (clockdir(numwalls) == 1)
								flipwalls(numwalls,newnumwalls);

							//clearbufbyte(&sector[numsectors],sizeof(sectortype),0L);
							memset(&sector[numsectors],0,sizeof(sectortype));
							sector[numsectors].extra = -1;

							sector[numsectors].wallptr = numwalls;
							sector[numsectors].wallnum = newnumwalls-numwalls;
							sector[numsectors].ceilingz = (-32<<8);
							sector[numsectors].floorz = (32<<8);
							for(i=numwalls;i<newnumwalls;i++)
							{
								wall[i].cstat = 0;
								wall[i].shade = 0;
								wall[i].yrepeat = 8;
								fixrepeats((short)i);
								wall[i].picnum = 0;
								wall[i].overpicnum = 0;
								wall[i].nextsector = -1;
								wall[i].nextwall = -1;
							}
							headspritesect[numsectors] = -1;
							numsectors++;
						}
						else       //else add loop to sector
						{
							if (clockdir(numwalls) == 0)
								flipwalls(numwalls,newnumwalls);

							j = newnumwalls-numwalls;

							sector[k].wallnum += j;
							for(i=k+1;i<numsectors;i++)
								sector[i].wallptr += j;
							suckwall = sector[k].wallptr;

							for(i=0;i<numwalls;i++)
							{
								if (wall[i].nextwall >= suckwall)
									wall[i].nextwall += j;
								if (wall[i].point2 >= suckwall)
									wall[i].point2 += j;
							}

							for(i=newnumwalls-1;i>=suckwall;i--)
								Bmemcpy(&wall[i+j],&wall[i],sizeof(walltype));
							for(i=0;i<j;i++)
								Bmemcpy(&wall[i+suckwall],&wall[i+newnumwalls],sizeof(walltype));

							for(i=suckwall;i<suckwall+j;i++)
							{
								wall[i].point2 += (suckwall-numwalls);

								wall[i].cstat = wall[suckwall+j].cstat;
								wall[i].shade = wall[suckwall+j].shade;
								wall[i].yrepeat = wall[suckwall+j].yrepeat;
								fixrepeats((short)i);
								wall[i].picnum = wall[suckwall+j].picnum;
								wall[i].overpicnum = wall[suckwall+j].overpicnum;

								wall[i].nextsector = -1;
								wall[i].nextwall = -1;
							}
						}
					}
					else
					{
						  //add new sector with connections
						if (clockdir(numwalls) == 1)
							flipwalls(numwalls,newnumwalls);

						//clearbufbyte(&sector[numsectors],sizeof(sectortype),0L);
						memset(&sector[numsectors],0,sizeof(sectortype));
						sector[numsectors].extra = -1;

						sector[numsectors].wallptr = numwalls;
						sector[numsectors].wallnum = newnumwalls-numwalls;
						sucksect = sectorofwall(suckwall);
						sector[numsectors].ceilingstat = sector[sucksect].ceilingstat;
						sector[numsectors].floorstat = sector[sucksect].floorstat;
						sector[numsectors].ceilingxpanning = sector[sucksect].ceilingxpanning;
						sector[numsectors].floorxpanning = sector[sucksect].floorxpanning;
						sector[numsectors].ceilingshade = sector[sucksect].ceilingshade;
						sector[numsectors].floorshade = sector[sucksect].floorshade;
						sector[numsectors].ceilingz = sector[sucksect].ceilingz;
						sector[numsectors].floorz = sector[sucksect].floorz;
						sector[numsectors].ceilingpicnum = sector[sucksect].ceilingpicnum;
						sector[numsectors].floorpicnum = sector[sucksect].floorpicnum;
						sector[numsectors].ceilingheinum = sector[sucksect].ceilingheinum;
						sector[numsectors].floorheinum = sector[sucksect].floorheinum;
						for(i=numwalls;i<newnumwalls;i++)
						{
							wall[i].cstat = wall[suckwall].cstat;
							wall[i].shade = wall[suckwall].shade;
							wall[i].yrepeat = wall[suckwall].yrepeat;
							fixrepeats((short)i);
							wall[i].picnum = wall[suckwall].picnum;
							wall[i].overpicnum = wall[suckwall].overpicnum;
							checksectorpointer((short)i,(short)numsectors);
						}
						headspritesect[numsectors] = -1;
						numsectors++;
					}
					numwalls = newnumwalls;
					newnumwalls = -1;
					asksave = 1;
				}
				if (split == 1)
				{
						 //split sector
					startwall = sector[splitsect].wallptr;
					endwall = startwall + sector[splitsect].wallnum - 1;
					for(k=startwall;k<=endwall;k++)
						if (wall[k].x == wall[newnumwalls-1].x)
							if (wall[k].y == wall[newnumwalls-1].y)
							{
								bad = 0;
								if (loopnumofsector(splitsect,splitstartwall) != loopnumofsector(splitsect,(short)k))
									bad = 1;

								if (bad == 0)
								{
									//SPLIT IT!
									//Split splitsect given: startwall,
									//   new points: numwalls to newnumwalls-2

									splitendwall = k;
									newnumwalls--;  //first fix up the new walls
									for(i=numwalls;i<newnumwalls;i++)
									{
										wall[i].cstat = wall[startwall].cstat;
										wall[i].shade = wall[startwall].shade;
										wall[i].yrepeat = wall[startwall].yrepeat;
										fixrepeats((short)i);
										wall[i].picnum = wall[startwall].picnum;
										wall[i].overpicnum = wall[startwall].overpicnum;

										wall[i].nextwall = -1;
										wall[i].nextsector = -1;
										wall[i].point2 = i+1;
									}

									danumwalls = newnumwalls;  //where to add more walls
									m = splitendwall;          //copy rest of loop next
									while (m != splitstartwall)
									{
										Bmemcpy(&wall[danumwalls],&wall[m],sizeof(walltype));
										wall[danumwalls].point2 = danumwalls+1;
										danumwalls++;
										m = wall[m].point2;
									}
									wall[danumwalls-1].point2 = numwalls;

										//Add other loops for 1st sector
									loopnum = loopnumofsector(splitsect,splitstartwall);
									i = loopnum;
									for(j=startwall;j<=endwall;j++)
									{
										k = loopnumofsector(splitsect,(short)j);
										if ((k != i) && (k != loopnum))
										{
											i = k;
											if (loopinside(wall[j].x,wall[j].y,numwalls) == 1)
											{
												m = j;          //copy loop
												k = danumwalls;
												do
												{
													Bmemcpy(&wall[danumwalls],&wall[m],sizeof(walltype));
													wall[danumwalls].point2 = danumwalls+1;
													danumwalls++;
													m = wall[m].point2;
												}
												while (m != j);
												wall[danumwalls-1].point2 = k;
											}
										}
									}

									secondstartwall = danumwalls;
										//copy split points for other sector backwards
									for(j=newnumwalls;j>numwalls;j--)
									{
										Bmemcpy(&wall[danumwalls],&wall[j],sizeof(walltype));
										wall[danumwalls].nextwall = -1;
										wall[danumwalls].nextsector = -1;
										wall[danumwalls].point2 = danumwalls+1;
										danumwalls++;
									}
									m = splitstartwall;     //copy rest of loop next
									while (m != splitendwall)
									{
										Bmemcpy(&wall[danumwalls],&wall[m],sizeof(walltype));
										wall[danumwalls].point2 = danumwalls+1;
										danumwalls++;
										m = wall[m].point2;
									}
									wall[danumwalls-1].point2 = secondstartwall;

										//Add other loops for 2nd sector
									loopnum = loopnumofsector(splitsect,splitstartwall);
									i = loopnum;
									for(j=startwall;j<=endwall;j++)
									{
										k = loopnumofsector(splitsect,(short)j);
										if ((k != i) && (k != loopnum))
										{
											i = k;
											if (loopinside(wall[j].x,wall[j].y,secondstartwall) == 1)
											{
												m = j;          //copy loop
												k = danumwalls;
												do
												{
													Bmemcpy(&wall[danumwalls],&wall[m],sizeof(walltype));
													wall[danumwalls].point2 = danumwalls+1;
													danumwalls++;
													m = wall[m].point2;
												}
												while (m != j);
												wall[danumwalls-1].point2 = k;
											}
										}
									}

										//fix all next pointers on old sector line
									for(j=numwalls;j<danumwalls;j++)
									{
										if (wall[j].nextwall >= 0)
										{
											wall[wall[j].nextwall].nextwall = j;
											if (j < secondstartwall)
												wall[wall[j].nextwall].nextsector = numsectors;
											else
												wall[wall[j].nextwall].nextsector = numsectors+1;
										}
									}
										//set all next pointers on split
									for(j=numwalls;j<newnumwalls;j++)
									{
										m = secondstartwall+(newnumwalls-1-j);
										wall[j].nextwall = m;
										wall[j].nextsector = numsectors+1;
										wall[m].nextwall = j;
										wall[m].nextsector = numsectors;
									}
										//copy sector attributes & fix wall pointers
									Bmemcpy(&sector[numsectors],&sector[splitsect],sizeof(sectortype));
									Bmemcpy(&sector[numsectors+1],&sector[splitsect],sizeof(sectortype));
									sector[numsectors].wallptr = numwalls;
									sector[numsectors].wallnum = secondstartwall-numwalls;
									sector[numsectors+1].wallptr = secondstartwall;
									sector[numsectors+1].wallnum = danumwalls-secondstartwall;

										//fix sprites
									j = headspritesect[splitsect];
									while (j != -1)
									{
										k = nextspritesect[j];
										if (loopinside(sprite[j].x,sprite[j].y,numwalls) == 1)
											changespritesect(j,numsectors);
										//else if (loopinside(sprite[j].x,sprite[j].y,secondstartwall) == 1)
										else  //Make sure no sprites get left out & deleted!
											changespritesect(j,numsectors+1);
										j = k;
									}

									numsectors+=2;

										//Back of number of walls of new sector for later
									k = danumwalls-numwalls;

										//clear out old sector's next pointers for clean deletesector
									numwalls = danumwalls;
									for(j=startwall;j<=endwall;j++)
									{
										wall[j].nextwall = -1;
										wall[j].nextsector = -1;
									}
									deletesector(splitsect);

										//Check pointers
									for(j=numwalls-k;j<numwalls;j++)
									{
										if (wall[j].nextwall >= 0)
											checksectorpointer(wall[j].nextwall,wall[j].nextsector);
										checksectorpointer((short)j,sectorofwall((short)j));
									}

										//k now safe to use as temp

									for(m=numsectors-2;m<numsectors;m++)
									{
										j = headspritesect[m];
										while (j != -1)
										{
											k = nextspritesect[j];
											setsprite(j,sprite[j].x,sprite[j].y,sprite[j].z);
											j = k;
										}
									}

									newnumwalls = -1;
									printmessage16("Sector split.");
									break;
								}
								else
								{
										//Sector split - actually loop joining

									splitendwall = k;
									newnumwalls--;  //first fix up the new walls
									for(i=numwalls;i<newnumwalls;i++)
									{
										wall[i].cstat = wall[startwall].cstat;
										wall[i].shade = wall[startwall].shade;
										wall[i].yrepeat = wall[startwall].yrepeat;
										fixrepeats((short)i);
										wall[i].picnum = wall[startwall].picnum;
										wall[i].overpicnum = wall[startwall].overpicnum;

										wall[i].nextwall = -1;
										wall[i].nextsector = -1;
										wall[i].point2 = i+1;
									}

									danumwalls = newnumwalls;  //where to add more walls
									m = splitendwall;          //copy rest of loop next
									do
									{
										Bmemcpy(&wall[danumwalls],&wall[m],sizeof(walltype));
										wall[danumwalls].point2 = danumwalls+1;
										danumwalls++;
										m = wall[m].point2;
									} while (m != splitendwall);

									//copy split points for other sector backwards
									for(j=newnumwalls;j>numwalls;j--)
									{
										Bmemcpy(&wall[danumwalls],&wall[j],sizeof(walltype));
										wall[danumwalls].nextwall = -1;
										wall[danumwalls].nextsector = -1;
										wall[danumwalls].point2 = danumwalls+1;
										danumwalls++;
									}

									m = splitstartwall;     //copy rest of loop next
									do
									{
										Bmemcpy(&wall[danumwalls],&wall[m],sizeof(walltype));
										wall[danumwalls].point2 = danumwalls+1;
										danumwalls++;
										m = wall[m].point2;
									} while (m != splitstartwall);
									wall[danumwalls-1].point2 = numwalls;

										//Add other loops to sector
									loopnum = loopnumofsector(splitsect,splitstartwall);
									i = loopnum;
									for(j=startwall;j<=endwall;j++)
									{
										k = loopnumofsector(splitsect,(short)j);
										if ((k != i) && (k != loopnumofsector(splitsect,splitstartwall)) && (k != loopnumofsector(splitsect,splitendwall)))
										{
											i = k;
											m = j; k = danumwalls;     //copy loop
											do
											{
												Bmemcpy(&wall[danumwalls],&wall[m],sizeof(walltype));
												wall[danumwalls].point2 = danumwalls+1;
												danumwalls++;
												m = wall[m].point2;
											} while (m != j);
											wall[danumwalls-1].point2 = k;
										}
									}

										//fix all next pointers on old sector line
									for(j=numwalls;j<danumwalls;j++)
									{
										if (wall[j].nextwall >= 0)
										{
											wall[wall[j].nextwall].nextwall = j;
											wall[wall[j].nextwall].nextsector = numsectors;
										}
									}

										//copy sector attributes & fix wall pointers
									Bmemcpy(&sector[numsectors],&sector[splitsect],sizeof(sectortype));
									sector[numsectors].wallptr = numwalls;
									sector[numsectors].wallnum = danumwalls-numwalls;

										//fix sprites
									j = headspritesect[splitsect];
									while (j != -1)
									{
										k = nextspritesect[j];
										changespritesect(j,numsectors);
										j = k;
									}

									numsectors++;

										//Back of number of walls of new sector for later
									k = danumwalls-numwalls;

										//clear out old sector's next pointers for clean deletesector
									numwalls = danumwalls;
									for(j=startwall;j<=endwall;j++)
									{
										wall[j].nextwall = -1;
										wall[j].nextsector = -1;
									}
									deletesector(splitsect);

										//Check pointers
									for(j=numwalls-k;j<numwalls;j++)
									{
										if (wall[j].nextwall >= 0)
											checksectorpointer(wall[j].nextwall,wall[j].nextsector);
										checksectorpointer((short)j,numsectors-1);
									}

									newnumwalls = -1;
									printmessage16("Loops joined.");
									break;
								}
							}
				}
			}
		}

		if (keystatus[0x1c] > 0) //Left Enter
		{
			keystatus[0x1c] = 0;
			if (keystatus[0x2a]&keystatus[0x1d])
			{
				printmessage16("CHECKING ALL POINTERS!");
				for(i=0;i<numsectors;i++)
				{
					startwall = sector[i].wallptr;
					for(j=startwall;j<numwalls;j++)
						if (wall[j].point2 < startwall) startwall = wall[j].point2;
					sector[i].wallptr = startwall;
				}
				for(i=numsectors-2;i>=0;i--)
					sector[i].wallnum = sector[i+1].wallptr-sector[i].wallptr;
				sector[numsectors-1].wallnum = numwalls-sector[numsectors-1].wallptr;

				for(i=0;i<numwalls;i++)
				{
					wall[i].nextsector = -1;
					wall[i].nextwall = -1;
				}
				for(i=0;i<numsectors;i++)
				{
					startwall = sector[i].wallptr;
					endwall = startwall + sector[i].wallnum;
					for(j=startwall;j<endwall;j++)
						checksectorpointer((short)j,(short)i);
				}
				printmessage16("ALL POINTERS CHECKED!");
				asksave = 1;
			}
			else
			{
				if (linehighlight >= 0)
				{
					checksectorpointer(linehighlight,sectorofwall(linehighlight));
					printmessage16("Highlighted line pointers checked.");
					asksave = 1;
				}
			}
		}

		if ((keystatus[0x0e] > 0) && (newnumwalls >= numwalls)) //Backspace
		{
			if (newnumwalls > numwalls)
			{
				newnumwalls--;
				asksave = 1;
				keystatus[0x0e] = 0;
			}
			if (newnumwalls == numwalls)
			{
				newnumwalls = -1;
				asksave = 1;
				keystatus[0x0e] = 0;
			}
		}

		if ((keystatus[0xd3] > 0) && (keystatus[0x9d] > 0) && (numwalls >= 0))
		{                                                      //sector delete
			keystatus[0xd3] = 0;

			sucksect = -1;
			for(i=0;i<numsectors;i++)
				if (inside(mousxplc,mousyplc,i) == 1)
				{
					k = 0;
					if (highlightsectorcnt >= 0)
						for(j=0;j<highlightsectorcnt;j++)
							if (highlightsector[j] == i)
							{
								for(j=highlightsectorcnt-1;j>=0;j--)
								{
									deletesector(highlightsector[j]);
									for(k=j-1;k>=0;k--)
										if (highlightsector[k] >= highlightsector[j])
											highlightsector[k]--;
								}
								printmessage16("Highlighted sectors deleted.");
								newnumwalls = -1;
								k = 1;
								highlightsectorcnt = -1;
								break;
							}
					if (k == 0)
					{
						deletesector((short)i);
						highlightsectorcnt = -1;
						printmessage16("Sector deleted.");
					}
					newnumwalls = -1;
					asksave = 1;
					break;
				}
		}

		if ((keystatus[0xd3] > 0) && (pointhighlight >= 0))
		{
			if ((pointhighlight&0xc000) == 16384)   //Sprite Delete
			{
				deletesprite(pointhighlight&16383);
				printmessage16("Sprite deleted.");
				updatenumsprites();
				asksave = 1;
			}
			keystatus[0xd3] = 0;
		}

		if (keystatus[0xd2] > 0)  //InsertPoint
		{
			if (highlightsectorcnt >= 0)
			{
				newnumsectors = numsectors;
				newnumwalls = numwalls;
				for(i=0;i<highlightsectorcnt;i++)
				{
					copysector(highlightsector[i],newnumsectors,newnumwalls,1);
					newnumsectors++;
					newnumwalls += sector[highlightsector[i]].wallnum;
				}

				for(i=0;i<highlightsectorcnt;i++)
				{
					startwall = sector[highlightsector[i]].wallptr;
					endwall = startwall+sector[highlightsector[i]].wallnum-1;
					for(j=startwall;j<=endwall;j++)
					{
						if (wall[j].nextwall >= 0)
							checksectorpointer(wall[j].nextwall,wall[j].nextsector);
						checksectorpointer((short)j,highlightsector[i]);
					}
					highlightsector[i] = numsectors+i;
				}
				numsectors = newnumsectors;
				numwalls = newnumwalls;

				newnumwalls = -1;
				newnumsectors = -1;

				updatenumsprites();
				printmessage16("Sectors duplicated and stamped.");
				asksave = 1;
			}
			else if (highlightcnt >= 0)
			{
				for(i=0;i<highlightcnt;i++)
					if ((highlight[i]&0xc000) == 16384)
					{
							//duplicate sprite
						k = (highlight[i]&16383);
						j = insertsprite(sprite[k].sectnum,sprite[k].statnum);
						Bmemcpy(&sprite[j],&sprite[k],sizeof(spritetype));
						sprite[j].sectnum = sprite[k].sectnum;   //Don't let memcpy overwrite sector!
						setsprite(j,sprite[j].x,sprite[j].y,sprite[j].z);
					}
				updatenumsprites();
				printmessage16("Sprites duplicated and stamped.");
				asksave = 1;
			}
			else if (linehighlight >= 0)
			{
				getclosestpointonwall(mousxplc,mousyplc,(long)linehighlight,&dax,&day);
				adjustmark(&dax,&day,newnumwalls);
				insertpoint(linehighlight,dax,day);
				printmessage16("Point inserted.");

				j = 0;
					//Check to see if point was inserted over another point
				for(i=numwalls-1;i>=0;i--)     //delete points
					if (wall[i].x == wall[wall[i].point2].x)
						if (wall[i].y == wall[wall[i].point2].y)
						{
							deletepoint((short)i);
							j++;
						}
				for(i=0;i<numwalls;i++)        //make new red lines?
				{
					if ((wall[i].x == dax) && (wall[i].y == day))
					{
						checksectorpointer((short)i,sectorofwall((short)i));
						fixrepeats((short)i);
					}
					else if ((wall[wall[i].point2].x == dax) && (wall[wall[i].point2].y == day))
					{
						checksectorpointer((short)i,sectorofwall((short)i));
						fixrepeats((short)i);
					}
				}
				//if (j != 0)
				//{
				//   dax = ((wall[linehighlight].x + wall[wall[linehighlight].point2].x)>>1);
				//   day = ((wall[linehighlight].y + wall[wall[linehighlight].point2].y)>>1);
				//   if ((dax != wall[linehighlight].x) || (day != wall[linehighlight].y))
				//      if ((dax != wall[wall[linehighlight].point2].x) || (day != wall[wall[linehighlight].point2].y))
				//      {
				//         insertpoint(linehighlight,dax,day);
				//         printmessage16("Point inserted at midpoint.");
				//      }
				//}

				asksave = 1;
			}
			keystatus[0xd2] = 0;
		}

		ExtCheckKeys();

		/*j = 0;
		for(i=22-1;i>=0;i--) updatecrc16(j,kensig[i]);
		if ((j&0xffff) != 0xebf)
		{
			printf("Don't screw with my name.\n");
			exit(0);
		}*/
		//printext16(9L,336+9L,4,-1,kensig,0);
		//printext16(8L,336+8L,12,-1,kensig,0);

		showframe(1);
		synctics = totalclock-lockclock;
		lockclock += synctics;

		if (keystatus[buildkeys[14]] > 0)  // enter from 2D
		{
			updatesector(posx,posy,&cursectnum);
			if (cursectnum >= 0)
			   {
			   BackupMap();                                          // wxbu
			   keystatus[buildkeys[14]] = 2;
			   }
			else
			   printmessage16("Arrow must be inside a sector before entering 3D mode.");
		}

		if (keystatus[1] > 0)
		{
			keystatus[1] = 0;
			printmessage16("(N)ew, (L)oad, (S)ave, Save (A)s, (P)lay, (M)enu, (Q)uit ");
			showframe(1);
			bflushchars();
			bad = 1;
			while (bad == 1)
			{
				if (handleevents()) {
					if (quitevent) {
						quitevent = 0;
					}
				}

				ch = bgetchar();

				if (keystatus[1] > 0)
				{
					keystatus[1] = 0;
					bad = 0;
					printmessage16("");
				}
				else if (ch == 'n' || ch == 'N')  //N
				{
					bad = 0;
					printmessage16("Are you sure you want to start a new board? (Y/N)");
					showframe(1);
					bflushchars(); ch = 0;
					while (keystatus[1] == 0)
					{
						if (handleevents()) {
							if (quitevent) {
								quitevent = 0;
							}
						}

						ch = bgetchar();

						if (ch == 'Y' || ch == 'y')
						{
							highlightsectorcnt = -1;
							highlightcnt = -1;

							for(i=0;i<(MAXWALLS>>3);i++)   //Clear all highlights
								show2dwall[i] = 0;
							for(i=0;i<(MAXSPRITES>>3);i++)
								show2dsprite[i] = 0;

							for(i=0;i<MAXSECTORS;i++) sector[i].extra = -1;
							for(i=0;i<MAXWALLS;i++) wall[i].extra = -1;
							for(i=0;i<MAXSPRITES;i++) sprite[i].extra = -1;

							sectorhighlightstat = -1;
							newnumwalls = -1;
							joinsector[0] = -1;
							circlewall = -1;
							circlepoints = 7;

							posx = 32768;          //new board!
							posy = 32768;
							posz = 0;
							ang = 1536;
							numsectors = 0;
							numwalls = 0;
							cursectnum = -1;
							initspritelists();
							Bstrcpy(boardfilename,"newboard.map");
							DoMainCaption();
							break;
						} else if (ch == 'N' || ch == 'n' || ch == 13 || ch == ' ') {
							break;
						}
					}
					printmessage16("");
					showframe(1);
				}
				else if (ch == 'l' || ch == 'L')  //L
				{
					bad = 0;
					i = menuselect();
					if (i < 0)
					{
						if (i == -2)
							printmessage16("No .MAP files found.");
					}
					else
					{
						Bstrcpy(boardfilename, selectedboardfilename);
						DoMainCaption();
                        clearmidstatbar16();                                             // wxyz
						if (highlightsectorcnt >= 0)
						{
							j = 0; k = 0;
							for(i=0;i<highlightsectorcnt;i++)
							{
								j += sector[highlightsector[i]].wallnum;

								m = headspritesect[highlightsector[i]];
								while (m != -1)
								{
									k++;
									m = nextspritesect[m];
								}
							}

							updatenumsprites();
							if ((numsectors+highlightsectorcnt > MAXSECTORS) || (numwalls+j > MAXWALLS) || (numsprites+k > MAXSPRITES))
							{
								highlightsectorcnt = -1;
							}
							else
							{
									//Put sectors&walls to end of lists
								j = MAXWALLS;
								for(i=0;i<highlightsectorcnt;i++)
								{
									j -= sector[highlightsector[i]].wallnum;
									copysector(highlightsector[i],(short)(MAXSECTORS-highlightsectorcnt+i),(short)j,0);
								}

									//Put sprites to end of list
									//DONT USE m BETWEEN HERE AND SPRITE RE-ATTACHING!
								m = MAXSPRITES;
								for(i=MAXSPRITES-1;i>=0;i--)
									if (sprite[i].statnum < MAXSTATUS)
									{
										k = sprite[i].sectnum;
										for(j=0;j<highlightsectorcnt;j++)
											if (highlightsector[j] == k)
											{
												m--;
												if (i != m)
													Bmemcpy(&sprite[m],&sprite[i],sizeof(spritetype));

													//HACK - THESE 2 buffers back up .sectnum and .statnum
													//for initspritelists() inside the loadboard call
												//tsprite[m].picnum = MAXSECTORS-highlightsectorcnt+j;
												//tsprite[m].owner = sprite[i].statnum;

												// JBF: I see your hack and raise you another
												spriteext[m].mdanimcur = MAXSECTORS-highlightsectorcnt+j;
												spriteext[m].angoff = sprite[i].statnum;

												break;
											}
									}
							}
						}

						highlightcnt = -1;
						sectorhighlightstat = -1;
						newnumwalls = -1;
						joinsector[0] = -1;
						circlewall = -1;
						circlepoints = 7;

						for(i=0;i<MAXSECTORS;i++) sector[i].extra = -1;
						for(i=0;i<MAXWALLS;i++) wall[i].extra = -1;
						for(i=0;i<MAXSPRITES;i++) sprite[i].extra = -1;

						ExtPreLoadMap();
						i = loadboard(boardfilename,(!pathsearchmode&&grponlymode?2:0),&posx,&posy,&posz,&ang,&cursectnum);
						if (i == -2) i = loadoldboard(boardfilename,(!pathsearchmode&&grponlymode?2:0),&posx,&posy,&posz,&ang,&cursectnum);
						if (i < 0)
						{
							printmessage16("Invalid map format.");
						}
						else
						{
							ExtLoadMap(boardfilename);

							if (highlightsectorcnt >= 0)
							{
								if ((numsectors+highlightsectorcnt > MAXSECTORS) || (sector[MAXSECTORS-highlightsectorcnt].wallptr < numwalls))
								{
									highlightsectorcnt = -1;
								}
								else
								{
										//Re-attach sectors&walls
									for(i=0;i<highlightsectorcnt;i++)
									{
										copysector((short)(MAXSECTORS-highlightsectorcnt+i),numsectors,numwalls,0);
										highlightsector[i] = numsectors;
										numwalls += sector[numsectors].wallnum;
										numsectors++;
									}
										//Re-attach sprites
									while (m < MAXSPRITES)
									{
										//HACK - THESE 2 buffers back up .sectnum and .statnum
										//for initspritelists() inside the loadboard call
										//tsprite[m].picnum = sprite[i].sectnum;
										//tsprite[m].owner = sprite[i].statnum;

										j = insertsprite(tsprite[m].picnum+(numsectors-MAXSECTORS),tsprite[m].owner);
										Bmemcpy(&sprite[j],&sprite[m],sizeof(spritetype));
										//sprite[j].sectnum = tsprite[m].picnum+(numsectors-MAXSECTORS);
										//sprite[j].statnum = tsprite[m].owner;

										// JBF: I see your hack and raise you another
										sprite[j].sectnum = spriteext[m].mdanimcur+(numsectors-MAXSECTORS);
										sprite[j].statnum = spriteext[m].angoff;
										spriteext[m].mdanimcur = spriteext[m].angoff = 0;

										m++;
									}

									for(i=0;i<highlightsectorcnt;i++)
									{
										startwall = sector[highlightsector[i]].wallptr;
										endwall = startwall+sector[highlightsector[i]].wallnum-1;
										for(j=startwall;j<=endwall;j++)
										{
											if (wall[j].nextwall >= 0)
												checksectorpointer(wall[j].nextwall,wall[j].nextsector);
											checksectorpointer((short)j,highlightsector[i]);
										}
									}

								}
							}

							if (mapversion < 7) printmessage16("Map loaded successfully and autoconverted to V7!");
							else printmessage16("Map loaded successfully.");
						}
						updatenumsprites();
						startposx = posx;      //this is same
						startposy = posy;
						startposz = posz;
						startang = ang;
						startsectnum = cursectnum;
					}
					showframe(1);
					keystatus[0x1c] = 0;
				}
				else if (ch == 'a' || ch == 'A')  //A
				{
					bad = 0;

					Bstrcpy(selectedboardfilename, boardfilename);
					if (Bstrrchr(boardfilename, '/'))
						Bstrcpy(boardfilename, Bstrrchr(boardfilename, '/')+1);

					i = 0;
					while ((boardfilename[i] != 0) && (i < 64))
						i++;
					if (boardfilename[i-4] == '.')
						i -= 4;
					boardfilename[i] = 0;

					bflushchars();
					while (bad == 0)
					    {
						Bsprintf(buffer,"Save as: %s_", boardfilename);
						printmessage16(buffer);
						showframe(1);
						if (handleevents())
						   {
							if (quitevent)
							    quitevent = 0;
						   }
						ch = bgetchar();
						if (keystatus[1] > 0)
						    bad = 1;
						else
						if (ch == 13)
						    bad = 2;
						else
						if (ch > 0)
						   {
							if (i > 0 && (ch == 8 || ch == 127))
							   {
								i--;
								boardfilename[i] = 0;
							   }
							else
							if (i < 32 && ch > 31 && ch < 123)       // wxlf  i < 8
							   {
								if (ch == 32)
								    ch = '_';
								boardfilename[i++] = ch;
								boardfilename[i] = 0;
							   }
						    }
					    }
					if (bad == 1)
					   {
						Bstrcpy(boardfilename, selectedboardfilename);
						keystatus[1] = 0;
						printmessage16("Operation cancelled");
						showframe(1);
					   }
					if (bad == 2)
					   {
						char *f;
						keystatus[0x1c] = 0;
						if (boardfilename[i-4] != '.')
						   {
						   boardfilename[i] = '.';
						   boardfilename[i+1] = 'm';
						   boardfilename[i+2] = 'a';
						   boardfilename[i+3] = 'p';
						   boardfilename[i+4] = 0;
						   }
						else
						   {
						   boardfilename[i] = 0;
						   }
						if (Bstrrchr(selectedboardfilename,'/'))
							Bstrcpy(Bstrrchr(selectedboardfilename, '/')+1, boardfilename);
						else
							Bstrcpy(selectedboardfilename, boardfilename);
						if (pathsearchmode) f = selectedboardfilename;
						else
						   {
							// virtual filesystem mode can't save to directories so drop the file into
							// the current directory
							f = strrchr(selectedboardfilename, '/');
							if (!f) f = selectedboardfilename; else f++;
						   }
						Bsprintf(buffer,"Saving to %s...",f);
						printmessage16(buffer);
						showframe(1);
						fixspritesectors();   //Do this before saving!
						updatesector(startposx,startposy,&startsectnum);
						ExtPreSaveMap();
						saveboard(f,&startposx,&startposy,&startposz,&startang,&startsectnum);
						ExtSaveMap(f);
						printmessage16("Board saved.");
						Bstrcpy(boardfilename, selectedboardfilename);
					   }
					bad = 0;
				    DoMainCaption();
				   }
				else
				if (ch == 's' || ch == 'S')  //S
				{
					char *f;

					bad = 0;
					printmessage16("Saving board...");
					showframe(1);
					fixspritesectors();   //Do this before saving!
					updatesector(startposx,startposy,&startsectnum);
					if (pathsearchmode) f = boardfilename;
					else {
						// virtual filesystem mode can't save to directories so drop the file into
						// the current directory
						f = strrchr(boardfilename, '/');
						if (!f) f = boardfilename; else f++;
					}
					ExtPreSaveMap();
					saveboard(f,&startposx,&startposy,&startposz,&startang,&startsectnum);
					ExtSaveMap(f);
					printmessage16("Board saved.");
					asksave = 0;
					showframe(1);
				}
				else if (ch == 'p' || ch == 'P')  // Play               wxpy
				{
				// printmessage16(REVISION);
                PlayTestMap();
				bad = 0;
				}
				else if (ch == 'm' || ch == 'M')  // Menu               wxpy
				{
                ky = GetMenuOption();             // 2D
                if (ky > 0)
                   {
                   CheckSelections(ky);           // 2D
                   if (ky < 3)   // quit
                      {
                      if (ky == 2)
                          asksave = 0;
                      quitflag = 1;
                      return;
                      }
                   }
                bflushchars();
                clearkeys();
               //  printmessage16(REVISION);
                bad = 0;
  		        }
				else if (ch == 'q' || ch == 'Q')  //Q
				{
					bad = 0;
					printmessage16("Are you sure you want to quit?");
					showframe(1);
					bflushchars();
					while (keystatus[1] == 0)
					    {
						if (handleevents())
						   {
						   if (quitevent)
						       quitevent = 0;
						   }

						ch = bgetchar();
						if (ch == 'y' || ch == 'Y' || ch == 13)
						   {
						   //QUIT!
                           if (asksave == 1)                        // wxyz
                              {
						       printmessage16("Save changes?");
							   showframe(1);
							   while (keystatus[1] == 0)
							      {
							  	if (handleevents())
							  	   {
							  	   if (quitevent) break;	// like saying no
							  	   }

							  	ch = bgetchar();
							  	if (ch == 'y' || ch == 'Y')
							  	   {
							  		char *f;
							  		fixspritesectors();   //Do this before saving!
							  		updatesector(startposx,startposy,&startsectnum);
							  		ExtPreSaveMap();
							  		if (pathsearchmode)
							  		    f = boardfilename;
							  		else
							  		   {
							  			// virtual filesystem mode can't save to directories so drop the file into
							  			// the current directory
							  			f = strrchr(boardfilename, '/');
							  			if (!f) f = boardfilename; else f++;
							  		   }
							  		saveboard(f,&startposx,&startposy,&startposz,&startang,&startsectnum);
							  		ExtSaveMap(f);
							  		break;
							  	    }
							  	else
							  	if (ch == 'n' || ch == 'N' || ch == 13 || ch == ' ')
							  	   {
							  		break;
							  	   }
							      }
							  }
							clearfilenames();
							uninittimer();
							uninitinput();
							ExtUnInit();
							uninitengine();
							printf("Memory status: %ld(%ld) bytes\n",cachesize,artsize);
							printf("%s\n",kensig);
							exit(0);
						    }
						 else
						 if (ch == 'n' || ch == 'N' || ch == 13 || ch == ' ')
						    {
							break;
						    }
					}
					printmessage16("");
					showframe(1);
				}
			}
			clearkeys();
		}

		//nextpage();
	}

	for(i=0;i<highlightsectorcnt;i++)
	{
		startwall = sector[highlightsector[i]].wallptr;
		endwall = startwall+sector[highlightsector[i]].wallnum-1;
		for(j=startwall;j<=endwall;j++)
		{
			if (wall[j].nextwall >= 0)
				checksectorpointer(wall[j].nextwall,wall[j].nextsector);
			checksectorpointer((short)j,highlightsector[i]);
		}
	}

	fixspritesectors();

	if (setgamemode(fullscreen,xdimgame,ydimgame,bppgame) < 0)
	{
		ExtUnInit();
		uninitinput();
		uninittimer();
		uninitsystem();
		clearfilenames();
		printf("%ld * %ld not supported in this graphics mode\n",xdim,ydim);
		exit(0);
	}

	posz = oposz;
	searchx = scale(searchx,xdimgame,xdim2d);
	searchy = scale(searchy,ydimgame,ydim2d-STATUS2DSIZ);
}

void getpoint(long searchxe, long searchye, long *x, long *y)
{
	if (posx <= -editorgridextent) posx = -editorgridextent;
	if (posx >= editorgridextent) posx = editorgridextent;
	if (posy <= -editorgridextent) posy = -editorgridextent;
	if (posy >= editorgridextent) posy = editorgridextent;

	*x = posx + divscale14(searchxe-halfxdim16,zoom);
	*y = posy + divscale14(searchye-midydim16,zoom);

	if (*x <= -editorgridextent) *x = -editorgridextent;
	if (*x >= editorgridextent) *x = editorgridextent;
	if (*y <= -editorgridextent) *y = -editorgridextent;
	if (*y >= editorgridextent) *y = editorgridextent;
}

long getlinehighlight(long xplc, long yplc)
{
	long i, dst, dist, closest, x1, y1, x2, y2, nx, ny;

	if (numwalls == 0)
		return(-1);
	dist = 0x7fffffff;
	closest = numwalls-1;
	for(i=0;i<numwalls;i++)
	{
		getclosestpointonwall(xplc,yplc,i,&nx,&ny);
		dst = klabs(xplc-nx)+klabs(yplc-ny);
		if (dst <= dist)
			dist = dst, closest = i;
	}

	if (wall[closest].nextwall >= 0)
	{    //if red line, allow highlighting of both sides
		x1 = wall[closest].x;
		y1 = wall[closest].y;
		x2 = wall[wall[closest].point2].x;
		y2 = wall[wall[closest].point2].y;
		if (dmulscale32(xplc-x1,y2-y1,-(x2-x1),yplc-y1) >= 0)
			closest = wall[closest].nextwall;
	}

	return(closest);
}

long getpointhighlight(long xplc, long yplc)
{
	long i, dst, dist, closest;

	if (numwalls == 0)
		return(-1);

	dist = 0;
	if (grid > 0)
		dist = 1024;

	closest = -1;
	for(i=0;i<numwalls;i++)
	{
		dst = klabs(xplc-wall[i].x) + klabs(yplc-wall[i].y);
		if (dst <= dist)
			dist = dst, closest = i;
	}
	for(i=0;i<MAXSPRITES;i++)
		if (sprite[i].statnum < MAXSTATUS)
		{
			dst = klabs(xplc-sprite[i].x) + klabs(yplc-sprite[i].y);
			if (dst <= dist)
				dist = dst, closest = i+16384;
		}
	return(closest);
}

long adjustmark(long *xplc, long *yplc, short danumwalls)
{
	long i, dst, dist, dax, day, pointlockdist;

	if (danumwalls < 0)
		danumwalls = numwalls;

	pointlockdist = 0;
	if ((grid > 0) && (gridlock > 0))
		pointlockdist = (128>>grid);

	dist = pointlockdist;
	dax = *xplc;
	day = *yplc;
	for(i=0;i<danumwalls;i++)
	{
		dst = klabs((*xplc)-wall[i].x) + klabs((*yplc)-wall[i].y);
		if (dst < dist)
		{
			dist = dst;
			dax = wall[i].x;
			day = wall[i].y;
		}
	}
	if (dist == pointlockdist)
		if ((gridlock > 0) && (grid > 0))
		{
			dax = ((dax+(1024>>grid))&(0xffffffff<<(11-grid)));
			day = ((day+(1024>>grid))&(0xffffffff<<(11-grid)));
		}

	*xplc = dax;
	*yplc = day;
	return(0);
}

long checkautoinsert(long dax, long day, short danumwalls)
{
	long i, x1, y1, x2, y2;

	if (danumwalls < 0)
		danumwalls = numwalls;
	for(i=0;i<danumwalls;i++)       // Check if a point should be inserted
	{
		x1 = wall[i].x;
		y1 = wall[i].y;
		x2 = wall[wall[i].point2].x;
		y2 = wall[wall[i].point2].y;

		if ((x1 != dax) || (y1 != day))
			if ((x2 != dax) || (y2 != day))
				if (((x1 <= dax) && (dax <= x2)) || ((x2 <= dax) && (dax <= x1)))
					if (((y1 <= day) && (day <= y2)) || ((y2 <= day) && (day <= y1)))
						if ((dax-x1)*(y2-y1) == (day-y1)*(x2-x1))
							return(1);          //insertpoint((short)i,dax,day);
	}
	return(0);
}

long clockdir(short wallstart)   //Returns: 0 is CW, 1 is CCW
{
	short i, themin;
	long minx, templong, x0, x1, x2, y0, y1, y2;

	minx = 0x7fffffff;
	themin = -1;
	i = wallstart-1;
	do
	{
		i++;
		if (wall[wall[i].point2].x < minx)
		{
			minx = wall[wall[i].point2].x;
			themin = i;
		}
	}
	while ((wall[i].point2 != wallstart) && (i < MAXWALLS));

	x0 = wall[themin].x;
	y0 = wall[themin].y;
	x1 = wall[wall[themin].point2].x;
	y1 = wall[wall[themin].point2].y;
	x2 = wall[wall[wall[themin].point2].point2].x;
	y2 = wall[wall[wall[themin].point2].point2].y;

	if ((y1 >= y2) && (y1 <= y0)) return(0);
	if ((y1 >= y0) && (y1 <= y2)) return(1);

	templong = (x0-x1)*(y2-y1) - (x2-x1)*(y0-y1);
	if (templong < 0)
		return(0);
	else
		return(1);
}

void flipwalls(short numwalls, short newnumwalls)
{
	long i, j, nume, templong;

	nume = newnumwalls-numwalls;

	for(i=numwalls;i<numwalls+(nume>>1);i++)
	{
		j = numwalls+newnumwalls-i-1;
		templong = wall[i].x; wall[i].x = wall[j].x; wall[j].x = templong;
		templong = wall[i].y; wall[i].y = wall[j].y; wall[j].y = templong;
	}
}

void insertpoint(short linehighlight, long dax, long day)
{
	short sucksect;
	long i, j, k;

	j = linehighlight;
	sucksect = sectorofwall((short)j);

	sector[sucksect].wallnum++;
	for(i=sucksect+1;i<numsectors;i++)
		sector[i].wallptr++;

	movewalls((long)j+1,+1L);
	Bmemcpy(&wall[j+1],&wall[j],sizeof(walltype));

	wall[j].point2 = j+1;
	wall[j+1].x = dax;
	wall[j+1].y = day;
	fixrepeats((short)j);
	fixrepeats((short)j+1);

	if (wall[j].nextwall >= 0)
	{
		k = wall[j].nextwall;

		sucksect = sectorofwall((short)k);

		sector[sucksect].wallnum++;
		for(i=sucksect+1;i<numsectors;i++)
			sector[i].wallptr++;

		movewalls((long)k+1,+1L);
		Bmemcpy(&wall[k+1],&wall[k],sizeof(walltype));

		wall[k].point2 = k+1;
		wall[k+1].x = dax;
		wall[k+1].y = day;
		fixrepeats((short)k);
		fixrepeats((short)k+1);

		j = wall[k].nextwall;
		wall[j].nextwall = k+1;
		wall[j+1].nextwall = k;
		wall[k].nextwall = j+1;
		wall[k+1].nextwall = j;
	}
}

void deletepoint(short point)
{
	long i, j, k, sucksect;

	sucksect = sectorofwall(point);

	sector[sucksect].wallnum--;
	for(i=sucksect+1;i<numsectors;i++)
		sector[i].wallptr--;

	j = lastwall(point);
	k = wall[point].point2;
	wall[j].point2 = k;

	if (wall[j].nextwall >= 0)
	{
		wall[wall[j].nextwall].nextwall = -1;
		wall[wall[j].nextwall].nextsector = -1;
	}
	if (wall[point].nextwall >= 0)
	{
		wall[wall[point].nextwall].nextwall = -1;
		wall[wall[point].nextwall].nextsector = -1;
	}
	movewalls((long)point,-1L);

	checksectorpointer((short)j,(short)sucksect);
}

long deletesector(short sucksect)
{
	long i, j, k, nextk, startwall, endwall;

	while (headspritesect[sucksect] >= 0)
		deletesprite(headspritesect[sucksect]);
	updatenumsprites();

	startwall = sector[sucksect].wallptr;
	endwall = startwall + sector[sucksect].wallnum - 1;
	j = sector[sucksect].wallnum;

	for(i=sucksect;i<numsectors-1;i++)
	{
		k = headspritesect[i+1];
		while (k != -1)
		{
			nextk = nextspritesect[k];
			changespritesect((short)k,(short)i);
			k = nextk;
		}

		Bmemcpy(&sector[i],&sector[i+1],sizeof(sectortype));
		sector[i].wallptr -= j;
	}
	numsectors--;

	j = endwall-startwall+1;
	for (i=startwall;i<=endwall;i++)
		if (wall[i].nextwall != -1)
		{
			wall[wall[i].nextwall].nextwall = -1;
			wall[wall[i].nextwall].nextsector = -1;
		}
	movewalls(startwall,-j);
	for(i=0;i<numwalls;i++)
		if (wall[i].nextwall >= startwall)
			wall[i].nextsector--;
	return(0);
}

void fixspritesectors(void)
{
	long i, j, dax, day, daz;

	for(i=numsectors-1;i>=0;i--)
		if ((sector[i].wallnum <= 0) || (sector[i].wallptr >= numwalls))
			deletesector((short)i);

	for(i=0;i<MAXSPRITES;i++)
		if (sprite[i].statnum < MAXSTATUS)
		{
			dax = sprite[i].x;
			day = sprite[i].y;
			if (inside(dax,day,sprite[i].sectnum) != 1)
			{
				daz = ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<2);

				for(j=0;j<numsectors;j++)
					if (inside(dax,day,(short)j) == 1)
						if (sprite[i].z >= getceilzofslope(j,dax,day))
							if (sprite[i].z-daz <= getflorzofslope(j,dax,day))
							{
								changespritesect((short)i,(short)j);
								break;
							}
			}
		}
}

long movewalls(long start, long offs)
{
	long i;

	if (offs < 0)  //Delete
	{
		for(i=start;i<numwalls+offs;i++)
			Bmemcpy(&wall[i],&wall[i-offs],sizeof(walltype));
	}
	else if (offs > 0)  //Insert
	{
		for(i=numwalls+offs-1;i>=start+offs;i--)
			Bmemcpy(&wall[i],&wall[i-offs],sizeof(walltype));
	}
	numwalls += offs;
	for(i=0;i<numwalls;i++)
	{
		if (wall[i].nextwall >= start) wall[i].nextwall += offs;
		if (wall[i].point2 >= start) wall[i].point2 += offs;
	}
	return(0);
}

long checksectorpointer(short i, short sectnum)
{
	long j, k, startwall, endwall, x1, y1, x2, y2;

	x1 = wall[i].x;
	y1 = wall[i].y;
	x2 = wall[wall[i].point2].x;
	y2 = wall[wall[i].point2].y;

	if (wall[i].nextwall >= 0)          //Check for early exit
	{
		k = wall[i].nextwall;
		if ((wall[k].x == x2) && (wall[k].y == y2))
			if ((wall[wall[k].point2].x == x1) && (wall[wall[k].point2].y == y1))
				return(0);
	}

	wall[i].nextsector = -1;
	wall[i].nextwall = -1;
	for(j=0;j<numsectors;j++)
	{
		startwall = sector[j].wallptr;
		endwall = startwall + sector[j].wallnum - 1;
		for(k=startwall;k<=endwall;k++)
		{
			if ((wall[k].x == x2) && (wall[k].y == y2))
				if ((wall[wall[k].point2].x == x1) && (wall[wall[k].point2].y == y1))
					if (j != sectnum)
					{
						wall[i].nextsector = j;
						wall[i].nextwall = k;
						wall[k].nextsector = sectnum;
						wall[k].nextwall = i;
					}
		}
	}
	return(0);
}

void fixrepeats(short i)
{
	long dax, day, dist;

	dax = wall[wall[i].point2].x-wall[i].x;
	day = wall[wall[i].point2].y-wall[i].y;
	dist = ksqrt(dax*dax+day*day);
	dax = wall[i].xrepeat; day = wall[i].yrepeat;
	wall[i].xrepeat = (char)min(max(mulscale10(dist,day),1),255);
}

void clearmidstatbar16(void) // clears main center of text area
{
	begindrawing();
	ydim16 = ydim;                                                                                          //0x080808081  wxyz
	clearbuf((char *)(frameplace + (bytesperline*(ydim-STATUS2DSIZ+26L))),(bytesperline*(STATUS2DSIZ-1-(25<<1))) >> 2, 0x07070707l);
	drawline16(0,ydim-STATUS2DSIZ,0,ydim-1,7);
	drawline16(xdim-1,ydim-STATUS2DSIZ,xdim-1,ydim-1,7);
	ydim16 = ydim-STATUS2DSIZ;
	enddrawing();
}

short loopinside(long x, long y, short startwall)
{
	long x1, y1, x2, y2, templong;
	short i, cnt;

	cnt = clockdir(startwall);
	i = startwall;
	do
	{
		x1 = wall[i].x; x2 = wall[wall[i].point2].x;
		if ((x1 >= x) || (x2 >= x))
		{
			y1 = wall[i].y; y2 = wall[wall[i].point2].y;
			if (y1 > y2)
			{
				templong = x1, x1 = x2, x2 = templong;
				templong = y1, y1 = y2, y2 = templong;
			}
			if ((y1 <= y) && (y2 > y))
				if (x1*(y-y2)+x2*(y1-y) <= x*(y1-y2))
					cnt ^= 1;
		}
		i = wall[i].point2;
	}
	while (i != startwall);
	return(cnt);
}

long numloopsofsector(short sectnum)
{
	long i, numloops, startwall, endwall;

	numloops = 0;
	startwall = sector[sectnum].wallptr;
	endwall = startwall + sector[sectnum].wallnum;
	for(i=startwall;i<endwall;i++)
		if (wall[i].point2 < i) numloops++;
	return(numloops);
}

short getnumber16(char namestart[80], short num, long maxnumber, char sign)
{
	char buffer[80], ch;
	long j, k, n, danum, oldnum;

	danum = (long)num;
	oldnum = danum;
	bflushchars();
	while (keystatus[0x1] == 0)
	{
		if (handleevents()) {
			if (quitevent) quitevent = 0;
		}

		ch = bgetchar();

		Bsprintf(buffer,"%s%ld_ ",namestart,danum);
		printmessage16(buffer);
		showframe(1);

		if (ch >= '0' && ch <= '9') {
			if (sign && danum<0)
				n = (danum*10)-(ch-'0');
			else
				n = (danum*10)+(ch-'0');
			if (n < maxnumber) danum = n;
		} else if (ch == 8 || ch == 127) {	// backspace
			danum /= 10;
		} else if (ch == 13) {
			oldnum = danum;
			asksave = 1;
			break;
		} else if (ch == '-' && sign) {	// negate
			danum = -danum;
		}
	}
	clearkeys();
	return((short)oldnum);
}

short getnumber256(char namestart[80], short num, long maxnumber, char sign)
{
	char buffer[80], ch;
	long j, k, n, danum, oldnum;

	danum = (long)num;
	oldnum = danum;
	bflushchars();
	while (keystatus[0x1] == 0)
	{
		if (handleevents()) {
			if (quitevent) quitevent = 0;
		}

		drawrooms(posx,posy,posz,ang,horiz,cursectnum);
#ifdef SUPERBUILD
		ExtAnalyzeSprites();
#endif
		drawmasks();

		ch = bgetchar();

		Bsprintf(buffer,"%s%ld_ ",namestart,danum);
		printmessage256(buffer);
		showframe(1);

		if (ch >= '0' && ch <= '9') {
			if (sign && danum<0)
				n = (danum*10)-(ch-'0');
			else
				n = (danum*10)+(ch-'0');
			if (n < maxnumber) danum = n;
		} else if (ch == 8 || ch == 127) {	// backspace
			danum /= 10;
		} else if (ch == 13) {
			oldnum = danum;
			asksave = 1;
			break;
		} else if (ch == '-' && sign) {	// negate
			danum = -danum;
		}
	}
	clearkeys();

	lockclock = totalclock;  //Reset timing

	return((short)oldnum);
}


void clearfilenames(void)
{
	klistfree(finddirs);
	klistfree(findfiles);
	finddirs = findfiles = NULL;
	numfiles = numdirs = 0;
}

long getfilenames(char *path, char *kind)
{
	CACHE1D_FIND_REC *r;

	clearfilenames();
	finddirs = klistpath(path,"*",CACHE1D_FIND_DIR|CACHE1D_FIND_DRIVE|(!pathsearchmode&&grponlymode?CACHE1D_OPT_NOSTACK:0));
	findfiles = klistpath(path,kind,CACHE1D_FIND_FILE|(!pathsearchmode&&grponlymode?CACHE1D_OPT_NOSTACK:0));
	for (r = finddirs; r; r=r->next) numdirs++;
	for (r = findfiles; r; r=r->next) numfiles++;

	finddirshigh = finddirs;
	findfileshigh = findfiles;
	currentlist = 0;
	if (findfileshigh) currentlist = 1;

	return(0);
}

long menuselect(void)
{
	int listsize;
	long i, j, topplc;
	char ch, buffer[78], *sb;
	CACHE1D_FIND_REC *dir;

	int bakpathsearchmode = pathsearchmode;

	listsize = (ydim16-32)/8;

	Bstrcpy(selectedboardfilename, boardfilename);
	if (pathsearchmode)
			Bcanonicalisefilename(selectedboardfilename, 1);		// clips off the last token and compresses relative path
	else
			Bcorrectfilename(selectedboardfilename, 1);

	getfilenames(selectedboardfilename, "*.map");

	begindrawing();
	printmessage16("Select .MAP file with arrows&enter.");
	enddrawing();

	do {
		begindrawing();
		clearbuf((char *)frameplace, (bytesperline*ydim16) >> 2, 0l);

		if (pathsearchmode) {
			strcpy(buffer,"Local filesystem mode. Press F for game filesystem.");
		} else {
			sprintf(buffer,"Game filesystem %smode. Press F for local filesystem, G for %s.",
					grponlymode?"GRP-only ":"", grponlymode?"all files":"GRP files only");
		}
		printext16(halfxdim16-(8*strlen(buffer)/2), 4, cBlak,0,buffer,0);      // wxcl

		Bsnprintf(buffer,78,"(%d dirs, %d files) %s",numdirs,numfiles,selectedboardfilename);
		buffer[sizeof(buffer)-1] = 0;
		printext16(1,ydim16-8-1,8,0,buffer,0);

		if (finddirshigh) {
			dir = finddirshigh;
			for(i=listsize/2-1; i>=0; i--) if (!dir->prev) break; else dir=dir->prev;
			for(i=0; i<listsize && dir; i++, dir=dir->next) {
				int c = dir->type == CACHE1D_FIND_DIR ? 4 : 3;
				memset(buffer,0,sizeof(buffer));
				strncpy(buffer,dir->name,25);
				if (strlen(buffer) == 25)
					buffer[21] = buffer[22] = buffer[23] = '.', buffer[24] = 0;
				if (dir == finddirshigh) {
					if (currentlist == 0) printext16(8,16+8*i,c|8,0,"->",0);
					printext16(32,16+8*i,c|8,0,buffer,0);
				} else {
					printext16(32,16+8*i,c,0,buffer,0);
				}
			}
		}

		if (findfileshigh) {
			dir = findfileshigh;
			for(i=listsize/2-1; i>=0; i--) if (!dir->prev) break; else dir=dir->prev;
			for(i=0; i<listsize && dir; i++, dir=dir->next) {
				if (dir == findfileshigh) {
					if (currentlist == 1) printext16(240,16+8*i,7|8,0,"->",0);
					printext16(240+24,16+8*i,7|8,0,dir->name,0);
				} else {
					printext16(240+24,16+8*i,7,0,dir->name,0);
				}
			}
		}
		enddrawing();
		showframe(1);

		keystatus[0xcb] = 0;
		keystatus[0xcd] = 0;
		keystatus[0xc8] = 0;
		keystatus[0xd0] = 0;
		keystatus[0x1c] = 0;	//enter
		keystatus[0xf] = 0;		//tab
		keystatus[1] = 0;		//esc
		ch = 0;                      //Interesting fakery of ch = getch()
		while (ch == 0)
		{
			if (handleevents()) {
				if (quitevent) {
					keystatus[1] = 1;
					quitevent = 0;
				}
			}
			ch = bgetchar();
			if (keystatus[0xcb] > 0) ch = 9;		// left arr
			if (keystatus[0xcd] > 0) ch = 9;		// right arr
			if (keystatus[0xc8] > 0) ch = 72;		// up arr
			if (keystatus[0xd0] > 0) ch = 80;		// down arr

		}

		if (ch == 'f' || ch == 'F') {
			currentlist = 0;
			pathsearchmode = 1-pathsearchmode;
			if (pathsearchmode) {
				strcpy(selectedboardfilename, "");
				Bcanonicalisefilename(selectedboardfilename, 0);
			} else strcpy(selectedboardfilename, "/");
			getfilenames(selectedboardfilename, "*.map");
		} else if (ch == 'g' || ch == 'G') {
			if (!pathsearchmode) grponlymode = 1-grponlymode;
			getfilenames(selectedboardfilename, "*.map");
		} else if (ch == 9) {
			if ((currentlist == 0 && findfiles) || (currentlist == 1 && finddirs))
				currentlist = 1-currentlist;
		} else if ((ch == 75) || (ch == 72)) {
			if (currentlist == 0) {
				if (finddirshigh && finddirshigh->prev) finddirshigh = finddirshigh->prev;
			} else {
				if (findfileshigh && findfileshigh->prev) findfileshigh = findfileshigh->prev;
			}
		} else if ((ch == 77) || (ch == 80)) {
			if (currentlist == 0) {
				if (finddirshigh && finddirshigh->next) finddirshigh = finddirshigh->next;
			} else {
				if (findfileshigh && findfileshigh->next) findfileshigh = findfileshigh->next;
			}
		} else if ((ch == 13) && (currentlist == 0)) {
			if (finddirshigh->type == CACHE1D_FIND_DRIVE) {
				strcpy(selectedboardfilename, finddirshigh->name);
			} else {
				strcat(selectedboardfilename, finddirshigh->name);
			}
			strcat(selectedboardfilename, "/");
			if (pathsearchmode)
				Bcanonicalisefilename(selectedboardfilename, 1);
			else
				Bcorrectfilename(selectedboardfilename, 1);

			//printf("Changing directories to: %s\n", selectedboardfilename);

			getfilenames(selectedboardfilename, "*.map");
			ch = 0;

			begindrawing();
			clearbuf((char *)frameplace, (bytesperline*ydim16) >> 2, 0l);
			enddrawing();
			showframe(1);
		}
		if (ch == 13 && !findfileshigh) ch = 0;
	}
	while ((ch != 13) && (ch != 27));
	if (ch == 13)
	{
		Bstrcat(selectedboardfilename, findfileshigh->name);
		//printf("Selected file: %s\n", selectedboardfilename);

		return(0);
	}
	pathsearchmode = bakpathsearchmode;
	return(-1);
}

long fillsector(short sectnum, char fillcolor)
{
	long x1, x2, y1, y2, sy, y, templong;
	long lborder, rborder, uborder, dborder, miny, maxy, dax;
	short z, zz, startwall, endwall, fillcnt;

	lborder = 0; rborder = xdim;
	uborder = 0; dborder = ydim16;

	if (sectnum == -1)
		return(0);
	miny = dborder-1;
	maxy = uborder;
	startwall = sector[sectnum].wallptr;
	endwall = startwall + sector[sectnum].wallnum - 1;
	for(z=startwall;z<=endwall;z++)
	{
		y1 = (((wall[z].y-posy)*zoom)>>14)+midydim16;
		y2 = (((wall[wall[z].point2].y-posy)*zoom)>>14)+midydim16;
		if (y1 < miny) miny = y1;
		if (y2 < miny) miny = y2;
		if (y1 > maxy) maxy = y1;
		if (y2 > maxy) maxy = y2;
	}
	if (miny < uborder) miny = uborder;
	if (maxy >= dborder) maxy = dborder-1;

	for(sy=miny+((totalclock>>2)&3);sy<=maxy;sy+=3)	// JBF 20040116: numframes%3 -> (totalclock>>2)&3
	{
		y = posy+(((sy-midydim16)<<14)/zoom);

		fillist[0] = lborder; fillcnt = 1;
		for(z=startwall;z<=endwall;z++)
		{
			x1 = wall[z].x; x2 = wall[wall[z].point2].x;
			y1 = wall[z].y; y2 = wall[wall[z].point2].y;
			if (y1 > y2)
			{
				templong = x1; x1 = x2; x2 = templong;
				templong = y1; y1 = y2; y2 = templong;
			}
			if ((y1 <= y) && (y2 > y))
				//if (x1*(y-y2) + x2*(y1-y) <= 0)
				{
					dax = x1+scale(y-y1,x2-x1,y2-y1);
					dax = (((dax-posx)*zoom)>>14)+halfxdim16;
					if (dax >= lborder)
						fillist[fillcnt++] = dax;
				}
		}
		if (fillcnt > 0)
		{
			for(z=1;z<fillcnt;z++)
				for (zz=0;zz<z;zz++)
					if (fillist[z] < fillist[zz])
					{
						templong = fillist[z]; fillist[z] = fillist[zz]; fillist[zz] = templong;
					}

			for (z=(fillcnt&1);z<fillcnt-1;z+=2)
			{
				if (fillist[z] > rborder) break;
				if (fillist[z+1] > rborder)
					fillist[z+1] = rborder;
				drawline16(fillist[z],sy,fillist[z+1],sy,fillcolor);
			}
		}
	}
	return(0);
}

short whitelinescan(short dalinehighlight)
{
	long i, j, k;
	short sucksect, newnumwalls;

	sucksect = sectorofwall(dalinehighlight);

	Bmemcpy(&sector[numsectors],&sector[sucksect],sizeof(sectortype));
	sector[numsectors].wallptr = numwalls;
	sector[numsectors].wallnum = 0;
	i = dalinehighlight;
	newnumwalls = numwalls;
	do
	{
		j = lastwall((short)i);
		if (wall[j].nextwall >= 0)
		{
			j = wall[j].point2;
			for(k=0;k<numwalls;k++)
			{
				if (wall[wall[k].point2].x == wall[j].x)
					if (wall[wall[k].point2].y == wall[j].y)
						if (wall[k].nextwall == -1)
						{
							j = k;
							break;
						}
			}
		}

		Bmemcpy(&wall[newnumwalls],&wall[i],sizeof(walltype));

		wall[newnumwalls].nextwall = j;
		wall[newnumwalls].nextsector = sectorofwall((short)j);

		newnumwalls++;
		sector[numsectors].wallnum++;

		i = j;
	}
	while (i != dalinehighlight);

	for(i=numwalls;i<newnumwalls-1;i++)
		wall[i].point2 = i+1;
	wall[newnumwalls-1].point2 = numwalls;

	if (clockdir(numwalls) == 1)
		return(-1);
	else
		return(newnumwalls);
}
/*
#define loadbyte(fil,tempbuf,bufplc,dat)        \
{                                               \
	if (bufplc == 0)                             \
	{                                            \
		for(bufplc=0;bufplc<4096;bufplc++)        \
			tempbuf[bufplc] = 0;                   \
		bufplc = 0;                               \
		read(fil,tempbuf,4096);                   \
	}                                            \
	dat = tempbuf[bufplc];                       \
	bufplc = ((bufplc+1)&4095);                  \
}                                               \

long loadnames(void)
{
	char buffer[80], firstch, ch;
	long fil, i, num, buffercnt, bufplc;

	if ((fil = open("names.h",O_BINARY|O_RDWR,S_IREAD)) == -1) return(-1);
	bufplc = 0;
	do { loadbyte(fil,tempbuf,bufplc,firstch); } while (firstch != '#');

	while ((firstch == '#') || (firstch == '/'))
	{
		do { loadbyte(fil,tempbuf,bufplc,ch); } while (ch > 32);

		buffercnt = 0;
		do
		{
			loadbyte(fil,tempbuf,bufplc,ch);
			if (ch > 32) buffer[buffercnt++] = ch;
		}
		while (ch > 32);

		num = 0;
		do
		{
			loadbyte(fil,tempbuf,bufplc,ch);
			if ((ch >= 48) && (ch <= 57)) num = num*10+(ch-48);
		}
		while (ch != 13);
		for(i=0;i<buffercnt;i++) names[num][i] = buffer[i];
		names[num][buffercnt] = 0;

		loadbyte(fil,tempbuf,bufplc,firstch);
		if (firstch == 10) loadbyte(fil,tempbuf,bufplc,firstch);
	}
	close(fil);
	return(0);
}
*/

long loadnames(void)
{
	char buffer[1024], *p, *name, *number, *endptr;
	int num, syms=0, line=0, a;
	BFILE *fp;

	fp = fopenfrompath("NAMES.H","r");
	if (!fp) {
		if ((fp = fopenfrompath("names.h","r")) == NULL) {
			printf("Failed to open NAMES.H\n");
			return -1;
		}
	}

	//clearbufbyte(names, sizeof(names), 0);
	memset(names,0,sizeof(names));

	printf("Loading NAMES.H\n");

	while (Bfgets(buffer, 1024, fp)) {
		a = Bstrlen(buffer);
		if (a >= 1) {
			if (a > 1)
				if (buffer[a-2] == '\r') buffer[a-2] = 0;
			if (buffer[a-1] == '\n') buffer[a-1] = 0;
		}

		p = buffer;
		line++;
		while (*p == 32) p++;
		if (*p == 0) continue;	// blank line

		if (*p == '#') {
			p++;
			while (*p == 32) p++;
			if (*p == 0) continue;	// null directive

			if (!Bstrncmp(p, "define ", 7)) {
				// #define_...
				p += 7;
				while (*p == 32) p++;
				if (*p == 0) {
					printf("Error: Malformed #define at line %d\n", line-1);
					continue;
				}

				name = p;
				while (*p != 32 && *p != 0) p++;
				if (*p == 32) {
					*(p++) = 0;
					while (*p == 32) p++;
					if (*p == 0) {	// #define_NAME with no number
						printf("Error: No number given for name \"%s\" (line %d)\n", name, line-1);
						continue;
					}

					number = p;
					while (*p != 0) p++;
					if (*p != 0) *p = 0;

					// add to list
					num = Bstrtol(number, &endptr, 10);
					if (*endptr != 0) {
						p = endptr;
						goto badline;
					}
					//printf("Grokked \"%s\" -> \"%d\"\n", name, num);
					if (num < 0 || num >= MAXTILES) {
						printf("Error: Constant %d for name \"%s\" out of range (line %d)\n", num, name, line-1);
						continue;
					}

					if (Bstrlen(name) > 24)
						printf("Warning: Name \"%s\" longer than 24 characters (line %d). Truncating.\n", name, line-1);

					Bstrncpy(names[num], name, 24);
					names[num][24] = 0;

					syms++;

					continue;

				} else {	// #define_NAME with no number
					printf("Error: No number given for name \"%s\" (line %d)\n", name, line-1);
					continue;
				}
			} else goto badline;
		} else if (*p == '/') {
			if (*(p+1) == '/') continue;	// comment
		}
badline:
		printf("Error: Invalid statement found at character %d on line %d\n", (p-buffer), line-1);
	}
	printf("Read %d lines, loaded %d names.\n", line, syms);

	Bfclose(fp);
	return 0;
}

void printcoords16(long posxe, long posye, short ange)
{
	char snotbuf[80];
	long i,m;

	Bsprintf(snotbuf,"x=%ld y=%ld ang=%d",posxe,posye,ange);
	i = 0;
	while ((snotbuf[i] != 0) && (i < 30))
		i++;
	while (i < 30)
	{
		snotbuf[i] = 32;
		i++;
	}
	snotbuf[30] = 0;

	m = (numsectors > MAXSECTORSV7 || numwalls > MAXWALLSV7 || numsprites > MAXSPRITESV7);

	printext16(8, ydim-STATUS2DSIZ+128, cWhit, cBlue, snotbuf,0);            // wxcl wxbg

	Bsprintf(snotbuf,"%d/%d sectors %d/%d walls %ld/%d sprites",
					numsectors,m?MAXSECTORSV8:MAXSECTORSV7,
					numwalls,m?MAXWALLSV8:MAXWALLSV7,
					numsprites,m?MAXSPRITESV8:MAXSPRITESV7);
	i = 0;
	while ((snotbuf[i] != 0) && (i < 56))
		i++;
	while (i < 56)
	{
		snotbuf[i] = 32;
		i++;
	}
	snotbuf[56] = 0;

	printext16(264, ydim-STATUS2DSIZ+128, cWhit, cBlue, snotbuf,0);          // wxcl wxbg
}

void updatenumsprites(void)
{
	long i;

	numsprites = 0;
	for(i=0;i<MAXSPRITES;i++)
		if (sprite[i].statnum < MAXSTATUS)
			numsprites++;
}

void copysector(short soursector, short destsector, short deststartwall, char copystat)
{
	short j, k, m, newnumwalls, startwall, endwall;

	newnumwalls = deststartwall;  //erase existing sector fragments

		//duplicate walls
	startwall = sector[soursector].wallptr;
	endwall = startwall + sector[soursector].wallnum;
	for(j=startwall;j<endwall;j++)
	{
		Bmemcpy(&wall[newnumwalls],&wall[j],sizeof(walltype));
		wall[newnumwalls].point2 += deststartwall-startwall;
		if (wall[newnumwalls].nextwall >= 0)
		{
			wall[newnumwalls].nextwall += deststartwall-startwall;
			wall[newnumwalls].nextsector += destsector-soursector;
		}
		newnumwalls++;
	}

	//for(j=deststartwall;j<newnumwalls;j++)
	//{
	//   if (wall[j].nextwall >= 0)
	//      checksectorpointer(wall[j].nextwall,wall[j].nextsector);
	//   checksectorpointer((short)j,destsector);
	//}

	if (newnumwalls > deststartwall)
	{
			//duplicate sectors
		Bmemcpy(&sector[destsector],&sector[soursector],sizeof(sectortype));
		sector[destsector].wallptr = deststartwall;
		sector[destsector].wallnum = newnumwalls-deststartwall;

		if (copystat == 1)
		{
				//duplicate sprites
			j = headspritesect[soursector];
			while (j >= 0)
			{
				k = nextspritesect[j];

				m = insertsprite(destsector,sprite[j].statnum);
				Bmemcpy(&sprite[m],&sprite[j],sizeof(spritetype));
				sprite[m].sectnum = destsector;   //Don't let memcpy overwrite sector!

				j = k;
			}
		}

	}
}

void showsectordata(short sectnum)
{
	char snotbuf[80];

	Bsprintf(snotbuf,"Sector %d",sectnum);
	printext16(8,ydim-STATUS2DSIZ+32,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Firstwall: %d",sector[sectnum].wallptr);
	printext16(8,ydim-STATUS2DSIZ+48,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Numberofwalls: %d",sector[sectnum].wallnum);
	printext16(8,ydim-STATUS2DSIZ+56,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Firstsprite: %d",headspritesect[sectnum]);
	printext16(8,ydim-STATUS2DSIZ+64,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Tags: %d, %d",sector[sectnum].hitag,sector[sectnum].lotag);
	printext16(8,ydim-STATUS2DSIZ+72,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"     (0x%x), (0x%x)",sector[sectnum].hitag,sector[sectnum].lotag);
	printext16(8,ydim-STATUS2DSIZ+80,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Extra: %d",sector[sectnum].extra);
	printext16(8,ydim-STATUS2DSIZ+88,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Visibility: %d",sector[sectnum].visibility);
	printext16(8,ydim-STATUS2DSIZ+96,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Pixel height: %ld",(sector[sectnum].floorz-sector[sectnum].ceilingz)>>8);
	printext16(8,ydim-STATUS2DSIZ+104,cBlak,-1,snotbuf,0);

	printext16(200,ydim-STATUS2DSIZ+32,cBlak,-1,"CEILINGS:",0);
	Bsprintf(snotbuf,"Flags (hex): %x",sector[sectnum].ceilingstat);
	printext16(200,ydim-STATUS2DSIZ+48,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"(X,Y)pan: %d, %d",sector[sectnum].ceilingxpanning,sector[sectnum].ceilingypanning);
	printext16(200,ydim-STATUS2DSIZ+56,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Shade byte: %d",sector[sectnum].ceilingshade);
	printext16(200,ydim-STATUS2DSIZ+64,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Z-coordinate: %ld",sector[sectnum].ceilingz);
	printext16(200,ydim-STATUS2DSIZ+72,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Tile number: %d",sector[sectnum].ceilingpicnum);
	printext16(200,ydim-STATUS2DSIZ+80,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Ceiling heinum: %d",sector[sectnum].ceilingheinum);
	printext16(200,ydim-STATUS2DSIZ+88,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Palookup number: %d",sector[sectnum].ceilingpal);
	printext16(200,ydim-STATUS2DSIZ+96,cBlak,-1,snotbuf,0);

	printext16(400,ydim-STATUS2DSIZ+32,cBlak,-1,"FLOORS:",0);
	Bsprintf(snotbuf,"Flags (hex): %x",sector[sectnum].floorstat);
	printext16(400,ydim-STATUS2DSIZ+48,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"(X,Y)pan: %d, %d",sector[sectnum].floorxpanning,sector[sectnum].floorypanning);
	printext16(400,ydim-STATUS2DSIZ+56,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Shade byte: %d",sector[sectnum].floorshade);
	printext16(400,ydim-STATUS2DSIZ+64,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Z-coordinate: %ld",sector[sectnum].floorz);
	printext16(400,ydim-STATUS2DSIZ+72,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Tile number: %d",sector[sectnum].floorpicnum);
	printext16(400,ydim-STATUS2DSIZ+80,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Floor heinum: %d",sector[sectnum].floorheinum);
	printext16(400,ydim-STATUS2DSIZ+88,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Palookup number: %d",sector[sectnum].floorpal);
	printext16(400,ydim-STATUS2DSIZ+96,cBlak,-1,snotbuf,0);
}

void showwalldata(short wallnum)
{
	long dax, day, dist;
	char snotbuf[80];

	Bsprintf(snotbuf,"Wall %d",wallnum);
	printext16(8,ydim-STATUS2DSIZ+32,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"X-coordinate: %ld",wall[wallnum].x);
	printext16(8,ydim-STATUS2DSIZ+48,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Y-coordinate: %ld",wall[wallnum].y);
	printext16(8,ydim-STATUS2DSIZ+56,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Point2: %d",wall[wallnum].point2);
	printext16(8,ydim-STATUS2DSIZ+64,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Sector: %ld",sectorofwall(wallnum));
	printext16(8,ydim-STATUS2DSIZ+72,cBlak,-1,snotbuf,0);

	Bsprintf(snotbuf,"Tags: %d, %d",wall[wallnum].hitag,wall[wallnum].lotag);
	printext16(8,ydim-STATUS2DSIZ+88,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"     (0x%x), (0x%x)",wall[wallnum].hitag,wall[wallnum].lotag);
	printext16(8,ydim-STATUS2DSIZ+96,cBlak,-1,snotbuf,0);

	printext16(200,ydim-STATUS2DSIZ+32,cBlak,-1,names[wall[wallnum].picnum],0);
	Bsprintf(snotbuf,"Flags (hex): %x",wall[wallnum].cstat);
	printext16(200,ydim-STATUS2DSIZ+48,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Shade: %d",wall[wallnum].shade);
	printext16(200,ydim-STATUS2DSIZ+56,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Pal: %d",wall[wallnum].pal);
	printext16(200,ydim-STATUS2DSIZ+64,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"(X,Y)repeat: %d, %d",wall[wallnum].xrepeat,wall[wallnum].yrepeat);
	printext16(200,ydim-STATUS2DSIZ+72,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"(X,Y)pan: %d, %d",wall[wallnum].xpanning,wall[wallnum].ypanning);
	printext16(200,ydim-STATUS2DSIZ+80,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Tile number: %d",wall[wallnum].picnum);
	printext16(200,ydim-STATUS2DSIZ+88,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"OverTile number: %d",wall[wallnum].overpicnum);
	printext16(200,ydim-STATUS2DSIZ+96,cBlak,-1,snotbuf,0);

	Bsprintf(snotbuf,"nextsector: %d",wall[wallnum].nextsector);
	printext16(400,ydim-STATUS2DSIZ+48,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"nextwall: %d",wall[wallnum].nextwall);
	printext16(400,ydim-STATUS2DSIZ+56,cBlak,-1,snotbuf,0);

	Bsprintf(snotbuf,"Extra: %d",wall[wallnum].extra);
	printext16(400,ydim-STATUS2DSIZ+72,cBlak,-1,snotbuf,0);

	dax = wall[wallnum].x-wall[wall[wallnum].point2].x;
	day = wall[wallnum].y-wall[wall[wallnum].point2].y;
	dist = ksqrt(dax*dax+day*day);
	Bsprintf(snotbuf,"Wall length: %ld",dist>>4);
	printext16(400,ydim-STATUS2DSIZ+96,cBlak,-1,snotbuf,0);

	dax = (long)sectorofwall(wallnum);
	Bsprintf(snotbuf,"Pixel height: %ld",(sector[dax].floorz-sector[dax].ceilingz)>>8);
	printext16(400,ydim-STATUS2DSIZ+104,cBlak,-1,snotbuf,0);
}

void showspritedata(short spritenum)
{
	char snotbuf[80];

	Bsprintf(snotbuf,"Sprite %d",spritenum);
	printext16(8,ydim-STATUS2DSIZ+32,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"X-coordinate: %ld",sprite[spritenum].x);
	printext16(8,ydim-STATUS2DSIZ+48,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Y-coordinate: %ld",sprite[spritenum].y);
	printext16(8,ydim-STATUS2DSIZ+56,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Z-coordinate: %ld",sprite[spritenum].z);
	printext16(8,ydim-STATUS2DSIZ+64,cBlak,-1,snotbuf,0);

	Bsprintf(snotbuf,"Sectnum: %d",sprite[spritenum].sectnum);
	printext16(8,ydim-STATUS2DSIZ+72,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Statnum: %d",sprite[spritenum].statnum);
	printext16(8,ydim-STATUS2DSIZ+80,cBlak,-1,snotbuf,0);

	Bsprintf(snotbuf,"Tags: %d, %d",sprite[spritenum].hitag,sprite[spritenum].lotag);
	printext16(8,ydim-STATUS2DSIZ+96,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"     (0x%x), (0x%x)",sprite[spritenum].hitag,sprite[spritenum].lotag);
	printext16(8,ydim-STATUS2DSIZ+104,cBlak,-1,snotbuf,0);

	printext16(200,ydim-STATUS2DSIZ+32,cBlak,-1,names[sprite[spritenum].picnum],0);
	Bsprintf(snotbuf,"Flags (hex): %x",sprite[spritenum].cstat);
	printext16(200,ydim-STATUS2DSIZ+48,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Shade: %d",sprite[spritenum].shade);
	printext16(200,ydim-STATUS2DSIZ+56,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Pal: %d",sprite[spritenum].pal);
	printext16(200,ydim-STATUS2DSIZ+64,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"(X,Y)repeat: %d, %d",sprite[spritenum].xrepeat,sprite[spritenum].yrepeat);
	printext16(200,ydim-STATUS2DSIZ+72,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"(X,Y)offset: %d, %d",sprite[spritenum].xoffset,sprite[spritenum].yoffset);
	printext16(200,ydim-STATUS2DSIZ+80,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Tile number: %d",sprite[spritenum].picnum);
	printext16(200,ydim-STATUS2DSIZ+88,cBlak,-1,snotbuf,0);

	Bsprintf(snotbuf,"Angle (2048 degrees): %d",sprite[spritenum].ang);
	printext16(400,ydim-STATUS2DSIZ+48,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"X-Velocity: %d",sprite[spritenum].xvel);
	printext16(400,ydim-STATUS2DSIZ+56,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Y-Velocity: %d",sprite[spritenum].yvel);
	printext16(400,ydim-STATUS2DSIZ+64,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Z-Velocity: %d",sprite[spritenum].zvel);
	printext16(400,ydim-STATUS2DSIZ+72,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Owner: %d",sprite[spritenum].owner);
	printext16(400,ydim-STATUS2DSIZ+80,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Clipdist: %d",sprite[spritenum].clipdist);
	printext16(400,ydim-STATUS2DSIZ+88,cBlak,-1,snotbuf,0);
	Bsprintf(snotbuf,"Extra: %d",sprite[spritenum].extra);
	printext16(400,ydim-STATUS2DSIZ+96,cBlak,-1,snotbuf,0);
}

void keytimerstuff(void)
{
	static long ltotalclock=0;
	if (totalclock == ltotalclock) return;
	ltotalclock=totalclock;

	if (keystatus[buildkeys[5]] == 0)
	{
		if (keystatus[buildkeys[2]] > 0) angvel = max(angvel-16,-128);
		if (keystatus[buildkeys[3]] > 0) angvel = min(angvel+16,127);
	}
	else
	{
		if (keystatus[buildkeys[2]] > 0) svel = min(svel+8,127);
		if (keystatus[buildkeys[3]] > 0) svel = max(svel-8,-128);
	}
	if (keystatus[buildkeys[0]] > 0) vel = min(vel+8,127);
	if (keystatus[buildkeys[1]] > 0) vel = max(vel-8,-128);
	if (keystatus[buildkeys[12]] > 0) svel = min(svel+8,127);
	if (keystatus[buildkeys[13]] > 0) svel = max(svel-8,-128);

	if (angvel < 0) angvel = min(angvel+12,0);
	if (angvel > 0) angvel = max(angvel-12,0);
	if (svel < 0) svel = min(svel+2,0);
	if (svel > 0) svel = max(svel-2,0);
	if (vel < 0) vel = min(vel+2,0);
	if (vel > 0) vel = max(vel-2,0);
}

void printmessage16(char name[128])     // extended from 82
{
	char snotbuf[100];                  // 60
	short x = 96;
	long i;

    if (xdim2d < 1000)
        x = 72;
    if (xdim2d < 790)
        x = 54;

	i = 0;
	while ((name[i] != 0) && (i < x))   // 54
	{
		snotbuf[i] = name[i];
		i++;
	}

	while (i < x)                       // 54
	{
		snotbuf[i] = 32;
		i++;
	}

	snotbuf[x] = 0;                     // 54

	begindrawing();
	printext16(200L, ydim-STATUS2DSIZ+8L, cWhit, cBlue, snotbuf, 0);  // (S)ave etc line wxbg
	enddrawing();
}

void printmessage256(char name[82])
{
	char snotbuf[40];
	long i;

	i = 0;
	while ((name[i] != 0) && (i < 38))
	{
		snotbuf[i] = name[i];
		i++;
	}
	while (i < 38)
	{
		snotbuf[i] = 32;
		i++;
	}
	snotbuf[38] = 0;

	printext256(0L,0L,whitecol,0,snotbuf,0);
}

	//Find closest point (*dax, *day) on wall (dawall) to (x, y)
void getclosestpointonwall(long x, long y, long dawall, long *nx, long *ny)
{
	walltype *wal;
	long i, j, dx, dy;

	wal = &wall[dawall];
	dx = wall[wal->point2].x-wal->x;
	dy = wall[wal->point2].y-wal->y;
	i = dx*(x-wal->x) + dy*(y-wal->y);
	if (i <= 0) { *nx = wal->x; *ny = wal->y; return; }
	j = dx*dx+dy*dy;
	if (i >= j) { *nx = wal->x+dx; *ny = wal->y+dy; return; }
	i = divscale30(i,j);
	*nx = wal->x + mulscale30(dx,i);
	*ny = wal->y + mulscale30(dy,i);
}

void initcrc(void)
{
	long i, j, k, a;

	for(j=0;j<256;j++)      //Calculate CRC table
	{
		k = (j<<8); a = 0;
		for(i=7;i>=0;i--)
		{
			if (((k^a)&0x8000) > 0)
				a = ((a<<1)&65535) ^ 0x1021;   //0x1021 = genpoly
			else
				a = ((a<<1)&65535);
			k = ((k<<1)&65535);
		}
		crctable[j] = (a&65535);
	}
}

static char visited[8192];

long GetWallZPeg(long nWall)
{
	long z=0, nSector, nNextSector;

	nSector = sectorofwall((short)nWall);
	nNextSector = wall[nWall].nextsector;
	if (nNextSector == -1)
	{
			//1-sided wall
		if (wall[nWall].cstat&4) z = sector[nSector].floorz;
								  else z = sector[nSector].ceilingz;
	}
	else
	{
			//2-sided wall
		if (wall[nWall].cstat&4)
			z = sector[nSector].ceilingz;
		else
		{
			if (sector[nNextSector].ceilingz > sector[nSector].ceilingz)
				z = sector[nNextSector].ceilingz;   //top step
			if (sector[nNextSector].floorz < sector[nSector].floorz)
				z = sector[nNextSector].floorz;   //bottom step
		}
	}
	return(z);
}

void AlignWalls(long nWall0, long z0, long nWall1, long z1, long nTile)
{
	long n;

		//do the x alignment
	wall[nWall1].cstat &= ~0x0108;    //Set to non-flip
	wall[nWall1].xpanning = (char)((wall[nWall0].xpanning+(wall[nWall0].xrepeat<<3))%tilesizx[nTile]);

	z1 = GetWallZPeg(nWall1);

	for(n=(picsiz[nTile]>>4);((1<<n)<tilesizy[nTile]);n++);

	wall[nWall1].yrepeat = wall[nWall0].yrepeat;
	wall[nWall1].ypanning = (char)(wall[nWall0].ypanning+(((z1-z0)*wall[nWall0].yrepeat)>>(n+3)));
}

void AutoAlignWalls(long nWall0, long ply)
{
	long z0, z1, nTile, nWall1, branch, visible, nNextSector, nSector;

	nTile = wall[nWall0].picnum;
	branch = 0;
	if (ply == 0)
	{
			//clear visited bits
		memset(visited,0,sizeof(visited));
		visited[nWall0] = 1;
	}

	z0 = GetWallZPeg(nWall0);

	nWall1 = wall[nWall0].point2;

		//loop through walls at this vertex in CCW order
	while (1)
	{
			//break if this wall would connect us in a loop
		if (visited[nWall1]) break;

		visited[nWall1] = 1;

			//break if reached back of left wall
		if (wall[nWall1].nextwall == nWall0) break;

		if (wall[nWall1].picnum == nTile)
		{
			z1 = GetWallZPeg(nWall1);
			visible = 0;

			nNextSector = wall[nWall1].nextsector;
			if (nNextSector < 0)
				visible = 1;
			else
			{
					//ignore two sided walls that have no visible face
				nSector = wall[wall[nWall1].nextwall].nextsector;
				if (getceilzofslope((short)nSector,wall[nWall1].x,wall[nWall1].y) <
					getceilzofslope((short)nNextSector,wall[nWall1].x,wall[nWall1].y))
					visible = 1;

				if (getflorzofslope((short)nSector,wall[nWall1].x,wall[nWall1].y) >
					getflorzofslope((short)nNextSector,wall[nWall1].x,wall[nWall1].y))
					visible = 1;
			}

			if (visible)
			{
				branch++;
				AlignWalls(nWall0,z0,nWall1,z1,nTile);

					//if wall was 1-sided, no need to recurse
				if (wall[nWall1].nextwall < 0)
				{
					nWall0 = nWall1;
					z0 = GetWallZPeg(nWall0);
					nWall1 = wall[nWall0].point2;
					branch = 0;
					continue;
				}
				else
					AutoAlignWalls(nWall1,ply+1);
			}
		}

		if (wall[nWall1].nextwall < 0) break;
		nWall1 = wall[wall[nWall1].nextwall].point2;
	}
}

//=============================================================================

void CheckSelections(short iKey)
{
    if (iKey == 1)
        SaveTestMap(1);
    else
    if (iKey == 2)
        return;
    else
    if (iKey == 3)
       {
       PlayTestMap();
	   }
    else
    if (iKey == 4)
       {
       SaveTestMap(0);
	   }
    else
    if (iKey == 5)
       {
       xsA = 64;
       xsB = 63;
       xsC = 6;
	   }
    else
    if (iKey == 6)
       {
       xsA = 128;
       xsB = 127;
       xsC = 7;
	   }
    else
    if (iKey == 7)
       {
       xsA = 256;
       xsB = 255;
       xsC = 8;
	   }
    else
    if (iKey == 20)
       {
       SWPBUILDHelp();
	   }
    else
    if (iKey == 99)         // config save and exit
       {
       SaveConfigs();
	   }
}

void PlayTestMap(void)
{
    int k;

    if (asksave > 0)
       {
       if (wm_ynbox("SWPBUILD", "Map not saved!\nSave before playing - Y/N ?"))
           SaveTestMap(1);
       }
    k = MessageBox((HWND)win_gethwnd(),"Play with monsters - Y/N ?", "SWPBUILD - Play Map",MB_YESNOCANCEL|MB_ICONQUESTION|MB_TASKMODAL);
    if (k == IDYES)
	    k = 2; // y
    else
    if (k == IDNO)
	    k = 1; // N
    else
        return;

  	if (Bstrrchr(boardfilename, '/'))
	    Bstrcpy(boardfilename, Bstrrchr(boardfilename, '/')+1);
	Bstrcpy(selectedboardfilename, boardfilename);
	if (keystatus[1] == 0)   // Esc
	   {
	   if (k == 1)           // No
           strcat(selectedboardfilename, " -monsters");
       spawnlp( P_NOWAIT, "SWP.exe -z -map", "", selectedboardfilename, NULL );
       bflushchars();
       }
}

void SaveTestMap(int iforce)
{
    char *f;

    fixspritesectors();   //Do this before saving!
    updatesector(startposx,startposy,&startsectnum);
    if (pathsearchmode)
        f = boardfilename;
    else
       {
       // virtual filesystem mode can't save to directories so drop the file into
       // the current directory
       f = strrchr(boardfilename, '/');
       if (!f)
           f = boardfilename; else f++;
       }
    ExtPreSaveMap();
    saveboard(f,&startposx,&startposy,&startposz,&startang,&startsectnum);
    ExtSaveMap(f);
    asksave = 0;
    if (iforce == 0)
 	    MessageBeep(64);;
}

void SWPBUILDHelp(void)
{
//    HWND myHandle;

//    myHandle = win_gethwnd();
    //spawnlp( P_NOWAIT, "SWPBUILDHelp.exe", "", "", NULL );
    spawnlp( P_NOWAIT, "Notepad.exe", "", "SWPBUILDHelp.txt", NULL );
    //SetFocus(myHandle);
    SetFocus((HWND)win_gethwnd());
}

void BackupMap(void)                                                 // wxas
{
    char fn[BMAX_PATH];
    short i;

    fn[0] = 0;
    i = strlen(boardfilename);
    strncpy(fn, boardfilename, i);

    if (fn[i-4] == '.')
       {
       fn[i-3] = 'b';
       fn[i-2] = 'a';
       fn[i-1] = 'k';
       fn[i] = 0;
       }

    updatesector(startposx,startposy,&startsectnum);
    saveboard(fn,&startposx,&startposy,&startposz,&startang,&startsectnum);
    //initprintf("Saving map: %s\n", fn);
}

void SaveConfigs(void)
{
    quitflag = 1;
}

void Display2DStats(void)                                                     // wxst
{
    char sTemp[128], *sTyp = "";
    short i, nm = 0, ht = 0, lt = 0, tx = 0, x, snm = 0, sht = 0, slt = 0;
    long msx, msy;

    if (pointhighlight >= 16384)
       {
       nm = pointhighlight-16384;
       tx = sprite[nm].picnum;
	   ht = sprite[nm].hitag;
	   lt = sprite[nm].lotag;
       x = CheckAnimations(tx);
       if (x > 0)
           sTyp = sTile[x];
       strcpy(sTemp, "Sprite : %d");
       }
    else
    if (linehighlight >= 0)
       {
       nm = linehighlight;
       tx = wall[nm].picnum;
	   ht = wall[nm].hitag;
	   lt = wall[nm].lotag;
       strcpy(sTemp, "Wall   : %d");
       }

    getpoint(searchx,searchy,&msx,&msy);
    for (i=0; i<numsectors; i++)
        {
        if (inside(msx,msy,i) == 1)
           {
           snm = i;
           sht = sector[i].hitag;
           slt = sector[i].lotag;
           break;
           }
        }

    sprintf(tempbuf, sTyp);
    printext16(8L, ydim-STATUS2DSIZ-75, cYell, -1, tempbuf, 0);
    sprintf(tempbuf, sTemp, nm);
    printext16(8L, ydim-STATUS2DSIZ-60, cYell, -1, tempbuf, 0);
    sprintf(tempbuf, "Picnum : %d", tx);
    printext16(8L, ydim-STATUS2DSIZ-45, cYell, -1, tempbuf, 0);
    sprintf(tempbuf, "Hitag  : %d", ht);
    printext16(8L, ydim-STATUS2DSIZ-30, cYell, -1, tempbuf, 0);
    sprintf(tempbuf, "Lotag  : %d", lt);
    printext16(8L, ydim-STATUS2DSIZ-15, cYell, -1, tempbuf, 0);


    if (snm >= 0 && snm <= numsectors && sht != 25958 && sht >= 0 && slt != 25958 && slt >= 0)
       {
       sprintf(tempbuf, "Sector: %d", snm);
       printext16(xdim2d-120, ydim-STATUS2DSIZ-45, cYell, -1, tempbuf, 0);
       sprintf(tempbuf, "Hitag : %d", sht);
       printext16(xdim2d-120, ydim-STATUS2DSIZ-30, cYell, -1, tempbuf, 0);
       sprintf(tempbuf, "Lotag : %d", slt);
       printext16(xdim2d-120, ydim-STATUS2DSIZ-15, cYell, -1, tempbuf, 0);
       }
}

void Display3DStats(void)                                                // wxst
{
    char *sTemp = "", *sTyp = "";
    short nm = 0, tx = 0, lt = 0, ht = 0, x;

	if (searchstat == 0)
	   {
	   tx = wall[searchwall].picnum;
	   ht = wall[searchwall].hitag;
	   lt = wall[searchwall].lotag;
	   sTemp = "Wall   : %d";
       nm = searchwall;
	   }
	else
	if (searchstat == 1)
	   {
	   tx = sector[searchsector].ceilingpicnum;
	   ht = sector[searchsector].hitag;
	   lt = sector[searchsector].lotag;
	   sTemp = "Sector : %d";
	   nm = searchsector;
	   }
	else
	if (searchstat == 2)
	   {
	   tx = sector[searchsector].floorpicnum;
	   ht = sector[searchsector].hitag;
	   lt = sector[searchsector].lotag;
	   sTemp = "Sector : %d";
	   nm = searchsector;
	   }
	else
	if (searchstat == 3)
	   {
	   tx = sprite[searchwall].picnum;
	   lt = sprite[searchwall].lotag;
	   ht = sprite[searchwall].hitag;
	   sTemp = "Sprite : %d";
	   nm = searchwall;
       x = CheckAnimations(tx);
       if (x > 0)
           sTyp = sTile[x];
	   }
	else
	if (searchstat == 4)
	   {
	   tx = wall[searchwall].overpicnum;
	   ht = wall[searchwall].hitag;
	   lt = wall[searchwall].lotag;
	   sTemp = "Wall   : %d";
	   nm = searchwall;
	   }

    sprintf(tempbuf, sTyp);
    printext256(10L, ydim-90, whitecol, -1, tempbuf, 0);
    sprintf(tempbuf, sTemp, nm);
    printext256(10L, ydim-75, whitecol, -1, tempbuf, 0);
    sprintf(tempbuf, "Picnum : %d", tx);
    printext256(10L, ydim-60, whitecol, -1, tempbuf, 0);
    sprintf(tempbuf, "Hitag  : %d", ht);
    printext256(10L, ydim-45, whitecol, -1, tempbuf, 0);
    sprintf(tempbuf, "Lotag  : %d", lt);
    printext256(10L, ydim-30, whitecol, -1, tempbuf, 0);
/*
    #define AVERAGEFRAMES 16
    static long frameval[AVERAGEFRAMES], framecnt = 0;
    extern short f_c;
    long i;

    if (i != frameval[framecnt])
       {
       i = totalclock;
       sprintf(tempbuf, "Fps    : %ld", ((120 * AVERAGEFRAMES) / (i - frameval[framecnt])) + f_c);
       printext256(10L, ydim-15, whitecol, -1, tempbuf, 0);
       frameval[framecnt] = i;
       framecnt = ((framecnt + 1) & (AVERAGEFRAMES - 1));
       }
*/
}

void DoMainCaption(void)                                             // wxcp
{
    char xbuf[128];

    ExtractFilename(boardfilename);
    snprintf(xbuf, 128, VERSION " | %s", myMapname);
    wm_setapptitle(xbuf);
}

void ExtractFilename(char *filename)                                 // wxcp
{
    int i, j, k, x;
    int Ln = strlen(filename);

    for (i=0; i<10; i++)
         myMapname[i] = 0;

    j = 0;
    k = 0;
    for (i=Ln; i>1; i--)
        {
        if (boardfilename[i] == 0x2e)  // '.'
            k = i;
        if (boardfilename[i] == 0x2f || boardfilename[i] == 0x5c) // '/' or '\'
           {
           j = i+1;
           break;
           }
        }

    if (j > 0)
       {
       x = 0;
       if (k > 0)
           Ln = k;
       for (i=j; i<Ln; i++)
           {
           myMapname[x] = boardfilename[i];
           x++;
           }
       strcat(myMapname, ".map");
       }
    else
       strcpy(myMapname,boardfilename);
}

/*
 * vim:ts=4:
 */
