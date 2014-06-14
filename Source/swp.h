
// created by Hendricks266 to mitigate implicit function declaration warnings

#include "jmact/scriplib.h"
#include "jmact/file_lib.h"
#include "jaudiolib/multivoc.h"

#define SWPVOID void
#define SWPBOOL unsigned char

void CONFIG_WriteSetup( void );
int SpawnBlood(short SpriteNum, short Weapon, short hitang, long hitx, long hity, long hitz);
int getdeffile(char *fn);
void SetLastPlayed(char *smap, short x);
int SetCommandLine(char *argvbufx);
void kpgetdim (const char *, long, long *, long *);
long kprender (const char *, long, long, long, long, long, long, long);
void extractname(char *pth);
void CreateFolder(char *folder);
void Showhelp(short x);
void GetRootFolder(char *folder);
int DeleteAutosave(short iEnd, char *gsv);
int CheckAutosave(void);
int GetUserMusic(void);
void SetVSync(int sync);
int addsearchpath(const char *p);
SWPVOID checkforhrp(char *pth, short iMap);
void LoadDefFiles(short iTyp);
void GetName(char *sPath, char *sTemp);
long initgroupfile(char *filename);
int checksearchpath(char *pth);
void AutoSaveGame(short disp, int iTime);
void GetGameFiles(char *gpath);
void SetDefGames(void);
void GetMapFiles(char *mpath, short wb);
void LoadMusicFiles(char *mpath);
void ListSavedGames(void);
void CheckShareware(void);
void checkforsaves(char *pth);
void GetSavedGames(void);
void resetgame(void);
void resetmaps(void);
void resetsave(short def);
void resetwbmaps(void);
void dolower(char *cpath);
void GetMapNames(char *fname);
void tabhelp(void);
void GetMapsPlayed(void);
void GetLastPlayed(char *smap, int x);
void CheckIPAddress(void);
SWPBOOL CheckMapsPlayed(char *smap);
void LoadTracks(void);
int DetectShareware(void);

