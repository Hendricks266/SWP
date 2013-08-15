# SWP Makefile for GNU Make

# SDK locations - adjust to match your setup
DXROOT=dx7

# Engine options
SUPERBUILD = 1
POLYMOST = 1
USE_OPENGL = 1
DYNAMIC_OPENGL = 1
USE_A_C = 0
NOASM = 0
LINKED_GTK = 0

# Debugging options
RELEASE?=1

# build locations
SRC=source
RSRC=rsrc
OBJ=obj_win
EROOT=Build
ESRC=$(EROOT)/src
EINC=$(EROOT)/include
EOBJ=eobj_win
INC=$(SRC)
o=o

ifneq (0,$(RELEASE))
  # debugging disabled
  debug=-fomit-frame-pointer -O1
else
  # debugging enabled
  debug=-ggdb -O0
endif

CC=gcc
CXX=g++
OURCFLAGS=$(debug) -W -Wall -Wimplicit -Wno-char-subscripts -Wno-unused \
	-funsigned-char -fno-strict-aliasing -DNO_GCC_BUILTINS \
	-I$(INC) -I$(EINC) -I$(SRC)/jmact -I$(SRC)/jaudiolib
OURCXXFLAGS=-fno-exceptions -fno-rtti
LIBS=-lm

NASMFLAGS=-s
EXESUFFIX=

JMACTOBJ=$(OBJ)/util_lib.$o \
	$(OBJ)/file_lib.$o \
	$(OBJ)/control.$o \
	$(OBJ)/keyboard.$o \
	$(OBJ)/mouse.$o \
	$(OBJ)/mathutil.$o \
	$(OBJ)/scriplib.$o \
	$(OBJ)/animlib.$o

AUDIOLIB_FX_STUB=$(OBJ)/audiolib_fxstub.$o
AUDIOLIB_MUSIC_STUB=$(OBJ)/audiolib_musicstub.$o
#AUDIOLIB_FX=$(OBJ)/audiolib_fx_fmod.$o
AUDIOLIB_FX=$(OBJ)/mv_mix.$o \
	  $(OBJ)/mv_mix16.$o \
	  $(OBJ)/mvreverb.$o \
	  $(OBJ)/pitch.$o \
	  $(OBJ)/multivoc.$o \
	  $(OBJ)/ll_man.$o \
	  $(OBJ)/fx_man.$o \
	  $(OBJ)/dsoundout.$o
AUDIOLIB_MUSIC=$(OBJ)/midi.$o \
	  $(OBJ)/mpu401.$o \
	  $(OBJ)/music.$o

GAMEOBJS= \
	$(OBJ)/actor.$o \
	$(OBJ)/ai.$o \
	$(OBJ)/anim.$o \
	$(OBJ)/border.$o \
	$(OBJ)/break.$o \
	$(OBJ)/bunny.$o \
	$(OBJ)/cache.$o \
	$(OBJ)/cd.$o \
	$(OBJ)/cheats.$o \
	$(OBJ)/colormap.$o \
	$(OBJ)/config.$o \
	$(OBJ)/console.$o \
	$(OBJ)/coolg.$o \
	$(OBJ)/coolie.$o \
	$(OBJ)/copysect.$o \
	$(OBJ)/demo.$o \
	$(OBJ)/draw.$o \
	$(OBJ)/eel.$o \
	$(OBJ)/game.$o \
	$(OBJ)/girlninj.$o \
	$(OBJ)/goro.$o \
	$(OBJ)/hornet.$o \
	$(OBJ)/interp.$o \
	$(OBJ)/interpsh.$o \
	$(OBJ)/inv.$o \
	$(OBJ)/jplayer.$o \
	$(OBJ)/jsector.$o \
	$(OBJ)/jweapon.$o \
	$(OBJ)/lava.$o \
	$(OBJ)/light.$o \
	$(OBJ)/mclip.$o \
	$(OBJ)/mdastr.$o \
	$(OBJ)/menus.$o \
	$(OBJ)/miscactr.$o \
	$(OBJ)/morph.$o \
	$(OBJ)/net.$o \
	$(OBJ)/ninja.$o \
	$(OBJ)/panel.$o \
	$(OBJ)/player.$o \
	$(OBJ)/predict.$o \
	$(OBJ)/quake.$o \
	$(OBJ)/ripper.$o \
	$(OBJ)/ripper2.$o \
	$(OBJ)/rooms.$o \
	$(OBJ)/rotator.$o \
	$(OBJ)/rts.$o \
	$(OBJ)/save.$o \
	$(OBJ)/scrip2.$o \
	$(OBJ)/sector.$o \
	$(OBJ)/serp.$o \
	$(OBJ)/setup.$o \
	$(OBJ)/skel.$o \
	$(OBJ)/skull.$o \
	$(OBJ)/slidor.$o \
	$(OBJ)/sounds.$o \
	$(OBJ)/spike.$o \
	$(OBJ)/sprite.$o \
	$(OBJ)/sumo.$o \
	$(OBJ)/swconfig.$o \
	$(OBJ)/sync.$o \
	$(OBJ)/text.$o \
	$(OBJ)/track.$o \
	$(OBJ)/usrhooks.$o \
	$(OBJ)/vator.$o \
	$(OBJ)/vis.$o \
	$(OBJ)/wallmove.$o \
	$(OBJ)/warp.$o \
	$(OBJ)/weapon.$o \
	$(OBJ)/zilla.$o \
	$(OBJ)/zombie.$o \
	$(OBJ)/saveable.$o \
	$(JMACTOBJ)

EDITOROBJS=$(OBJ)/jnstub.$o \
	$(OBJ)/brooms.$o \
	$(OBJ)/bldscript.$o \
	$(OBJ)/jbhlp.$o \
	$(OBJ)/colormap.$o

include $(EROOT)/Makefile.shared

SWP ?= SWP$(EXESUFFIX)

SWP_TARGET:=$(SWP)

ifndef EBACKTRACEDLL
    EBACKTRACEDLL = swpbacktrace1.dll
    ifeq ($(findstring x86_64,$(COMPILERTARGET)),x86_64)
        EBACKTRACEDLL = swpbacktrace1-64.dll
    endif
endif
EBACKTRACEDLL_TARGET:=$(EBACKTRACEDLL)

ifeq ($(PLATFORM),LINUX)
	NASMFLAGS+= -f elf
endif
ifeq ($(PLATFORM),WINDOWS)
	OURCFLAGS+= -DUNDERSCORES -I$(DXROOT)/include
	NASMFLAGS+= -DUNDERSCORES -f win32
	GAMEOBJS+= $(OBJ)/cda_win32.$o $(OBJ)/gameres.$o $(OBJ)/startdlg.$o $(OBJ)/startwin.game.$o
	EDITOROBJS+= $(OBJ)/buildres.$o
endif

ifeq ($(RENDERTYPE),SDL)
	OURCFLAGS+= $(subst -Dmain=SDL_main,,$(shell sdl-config --cflags))
	AUDIOLIBOBJ=$(AUDIOLIB_MUSIC_STUB) $(AUDIOLIB_FX_STUB)

	ifeq (1,$(HAVE_GTK2))
		OURCFLAGS+= -DHAVE_GTK2 $(shell pkg-config --cflags gtk+-2.0)
		GAMEOBJS+= $(OBJ)/game_banner.$o $(OBJ)/startdlg.$o $(OBJ)/startgtk.game.$o
		EDITOROBJS+= $(OBJ)/editor_banner.$o
	endif

	GAMEOBJS+= $(OBJ)/game_icon.$o
	EDITOROBJS+= $(OBJ)/build_icon.$o
endif
ifeq ($(RENDERTYPE),WIN)
	AUDIOLIBOBJ=$(AUDIOLIB_MUSIC) $(AUDIOLIB_FX)
endif

GAMEOBJS+= $(AUDIOLIBOBJ)
OURCFLAGS+= $(BUILDCFLAGS)

.SILENT:

.PHONY: clean all engine $(EOBJ)/$(ENGINELIB) $(EOBJ)/$(EDITORLIB)

# TARGETS

# Invoking Make from the terminal in OSX just chains the build on to xcode
ifeq ($(PLATFORM),DARWIN)
ifeq ($(RELEASE),0)
style=Development
else
style=Deployment
endif
.PHONY: alldarwin
alldarwin:
	cd osx && xcodebuild -target All -buildstyle $(style)
endif

all: $(SWP_TARGET)

all:
	$(BUILD_FINISHED)
ifneq (,$(SWP_TARGET))
	@ls -l $(SWP)
endif

ebacktrace: $(EBACKTRACEDLL_TARGET)
ifneq (,$(EBACKTRACEDLL_TARGET))
	@ls -l $(EBACKTRACEDLL)
endif

$(SWP): $(GAMEOBJS) $(EOBJ)/$(ENGINELIB)
	$(CC) $(CFLAGS) $(OURCFLAGS) -o $@ $^ $(LIBS) # -Wl,-Map=SWP.dat
ifeq ($(RELEASE),1)
	strip SWP$(EXESUFFIX)
endif

include Makefile.deps

.PHONY: enginelib editorlib
enginelib editorlib:
	$(MAKE) -C $(EROOT) "OBJ=$(CURDIR)/$(EOBJ)" SETSPRITEZ=1 \
	SUPERBUILD=$(SUPERBUILD) POLYMOST=$(POLYMOST) \
	USE_OPENGL=$(USE_OPENGL) DYNAMIC_OPENGL=$(DYNAMIC_OPENGL) \
	USE_A_C=$(USE_A_C) NOASM=$(NOASM) RELEASE=$(RELEASE) $@

$(EOBJ)/$(ENGINELIB): enginelib
$(EOBJ)/$(EDITORLIB): editorlib

# RULES
$(EBACKTRACEDLL): backtrace/backtrace.c
	$(CC) $(CFLAGS) $(OURCFLAGS) -O2 -shared -Wall -Wextra -I$(EINC) -o $@ $^ -lbfd -liberty -limagehlp

$(OBJ)/%.$o: $(SRC)/%.nasm
	nasm $(NASMFLAGS) $< -o $@
$(OBJ)/%.$o: $(SRC)/jaudiolib/%.nasm
	nasm $(NASMFLAGS) $< -o $@

$(OBJ)/%.$o: $(SRC)/%.c
	$(CC) $(CFLAGS) $(OURCFLAGS) -c $< -o $@ 2>&1

$(OBJ)/%.$o: $(SRC)/jmact/%.c
	$(CC) $(CFLAGS) $(OURCFLAGS) -c $< -o $@ 2>&1

$(OBJ)/%.$o: $(SRC)/jaudiolib/%.c
	$(CC) $(CFLAGS) $(OURCFLAGS) -c $< -o $@ 2>&1

$(OBJ)/%.$o: $(SRC)/misc/%.rc
	windres -i $< -o $@ --include-dir=$(EINC) --include-dir=$(SRC)

$(OBJ)/%.$o: $(SRC)/util/%.c
	$(CC) $(CFLAGS) $(OURCFLAGS) -c $< -o $@ 2>&1

$(OBJ)/%.$o: $(RSRC)/%.c
	$(CC) $(CFLAGS) $(OURCFLAGS) -c $< -o $@ 2>&1

$(OBJ)/game_banner.$o: $(RSRC)/game_banner.c
$(OBJ)/editor_banner.$o: $(RSRC)/editor_banner.c
$(RSRC)/game_banner.c: $(RSRC)/game.bmp
	echo "#include <gdk-pixbuf/gdk-pixdata.h>" > $@
	gdk-pixbuf-csource --extern --struct --raw --name=startbanner_pixdata $^ | sed 's/load_inc//' >> $@
$(RSRC)/editor_banner.c: $(RSRC)/build.bmp
	echo "#include <gdk-pixbuf/gdk-pixdata.h>" > $@
	gdk-pixbuf-csource --extern --struct --raw --name=startbanner_pixdata $^ | sed 's/load_inc//' >> $@

# PHONIES
clean:
ifeq ($(PLATFORM),DARWIN)
	cd osx && xcodebuild -target All clean
else
	-rm -f $(OBJ)/*
endif

veryclean: clean
ifeq ($(PLATFORM),DARWIN)
else
	-rm -f $(EOBJ)/* SWP$(EXESUFFIX) build$(EXESUFFIX) core* $(EBACKTRACEDLL)
endif

