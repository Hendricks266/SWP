@echo off
del obj_win\game.*
rem del obj_win\menus.*
rem del obj_win\multivoc.*
rem del obj_win\vorbis.*
rem del obj_win\sounds.*
rem del obj_win\cd.*
rem del obj_win\config.*
rem del obj_win\startwin.game.*
rem del obj_win\border.*
rem del obj_win\gameres.*
rem del obj_win\panel.*
rem del obj_win\player.*
rem del obj_win\inv.*
rem del obj_win\cheats.*
rem del eobj_win\engine.o
del eobj_win\winlayer.o
rem del eobj_win\defs.o
rem del eobj_win\polymost.o
rem del obj_win\*.o
rem del eobj_win\*.o
rem make -B -f makefile
wmake -f Makefile.watcom
pause
