##################################################################
# Makefile for Pisco1 program 
#
# JLP
# Version 07-01-2015
##################################################################

# To check it, type "echo %OS%" on Windows command terminal
ifeq ($(OS),Windows_NT)
JLP_SYSTEM=Windows
else
JLP_SYSTEM=Linux
endif

##############################################################
# Linux CENTOS6:
##############################################################
ifeq ($(JLP_SYSTEM),Linux)

CPP = c++

JLPLIB_DIR = $(JLPSRC)/jlplib
CFITSIO_INCL_DIR = $(JLPLIB_DIR)/cfitsio/incl

CXFLAGS = -Wall `wx-config --cppflags` -DLINUX $(MY_INCL)

mylib = $(JLPLIB)/jlp
MATHLIB = $(JLPLIB)/jlp/jlp_numeric.a $(JLPLIB)/math/libfftw3.a

PATHSEP1 = /

#WX_LIB = -L/usr/local/lib -pthread   -lwx_baseu-2.9 -lwx_gtk2u_core-2.9
WX_LIB = `wx-config --libs base,core`

else

##############################################################
# Windows operating system:
##############################################################

CPP = c++
JLPSRC = c:/jlp_src
JLPLIB_DIR = c:/jlp_src/jlplib
CFITSIO_INCL_DIR = $(JLPLIB_DIR)/cfitsio/incl

# C++ flags to use with wxWidgets code 
WX_INCLUDE_DIR = c:/wxWidgets-3.0.2/include

# Windows: I add USE_XPM (USE_XPM is needed to have the icon)
CXFLAGS = -Wall -DUSE_XPM -I$(WX_INCLUDE_DIR) $(MY_INCL) -DWIN32

# To avoid console apparition:
NO_CONSOLE = -mwindows 

# Need two back slashes for Linux compatibility:
#PATHSEP1 = \\
# Not true: hence:
PATHSEP1 = /

EXEC = c:/EXEC

mylib = c:/EXEC/MyLib
MATHLIB = $(mylib)$(PATHSEP)jlp_numeric.a $(mylib)$(PATHSEP)libfftw3.a
WX_LIB_DIR = c:/wxWidgets-3.0.2/lib/gcc_dll

WX_LIB := -L$(WX_LIB_DIR) -lwxbase30u -lwxmsw30u_core

endif

#Remove extra blank before $(PATHSEP1):
PATHSEP=$(strip $(PATHSEP1))

EXEC_DIR = $(EXEC)$(PATHSEP)

MY_INCL=-I. -I$(JLPLIB_DIR)$(PATHSEP)jlp_splot \
	-I$(JLPLIB_DIR)$(PATHSEP)jlp_fits \
	-I$(JLPLIB_DIR)$(PATHSEP)jlp_wxplot$(PATHSEP)jlp_wxplot_include \
	-I$(JLPLIB_DIR)$(PATHSEP)jlp_gseg_wxwid \
	-I$(JLPLIB_DIR)$(PATHSEP)jlp_gsegraf$(PATHSEP)jlp_gsegraf_include \
	-I$(JLPLIB_DIR)$(PATHSEP)jlp_numeric \
	-I$(CFITSIO_INCL_DIR)

FITSLIB=$(mylib)$(PATHSEP)jlp_fits.a $(mylib)$(PATHSEP)libcfitsio.a 
MY_LIB= $(mylib)$(PATHSEP)jlp_wxplot.a $(mylib)$(PATHSEP)jlp_splot.a  \
	$(MATHLIB) $(FITSLIB) -lm

PISCO1_SRC = psc1_frame_logbook.cpp psc1_frame_menu.cpp psc1_language.cpp \
	psc1_main.cpp psc1_param_panel.cpp psc1_pisco_filters.cpp \
	psc1_pisco_logbook.cpp psc1_pisco_panel.cpp \
	psc1_pisco_panel_onclick.cpp \
	psc1_pisco_panel_rs232.cpp psc1_pisco_risley.cpp psc1_rs232_mutex.cpp \
	psc1_rw_config_files.cpp psc1_target_catalogs.cpp \
	psc1_target_panel.cpp \
	psc1_target_panel_onclick.cpp read_WDS_catalog.cpp \
	jlp_celmeca1.cpp jlp_language_dlg.cpp jlp_risley.cpp \
	jlp_rs232_dlg.cpp jlp_rs232_in_c.cpp \
	jlp_time0.cpp jlp_utime.cpp

# jlp_rs232_thread.cpp
# tav_tav2.cpp jlp_rs232_display.cpp jlp_rs232_in_cpp.cpp jlp_rs232_in_c.c
PISCO1_OBJ = $(PISCO1_SRC:.cpp=.o)
 
PISCO1_DEP = Pisco1.h WDS_catalog_utils.h \
	jlp_language_dlg.h jlp_rs232_dlg.h \
	jlp_rs232_in_c.h jlp_rs232_thread.h \
	jlp_time0.h \
	psc1_defs.h psc1_frame.h psc1_frame_id.h psc1_language.h \
	psc1_param_panel.h psc1_pisco_panel.h psc1_rs232_mutex.h \
	psc1_rw_config_files.h psc1_target_panel.h psc1_typedef.h \
	read_WDS_catalog.h tav_utils.h \
	WDS_catalog_utils.h

.SUFFIXES:
.SUFFIXES: .o .cpp .exe $(SUFFIXES) 

.cpp.o:
	$(CPP) -c -g $(CXFLAGS) $*.cpp

.o.exe:
	$(CPP) -o $(EXEC_DIR)$*.exe $*.o $(PISCO1_OBJ) \
	$(MY_LIB) $(WX_LIB) -lz

.cpp.exe:
	$(CPP) -c -g $(CXFLAGS) $*.cpp
	$(CPP) -o $(EXEC_DIR)$*.exe $*.o $(PISCO1_OBJ) \
	$(MY_LIB) $(WX_LIB) -lz

# /usr/bin/ld: /home/build/liblnx/jlp/libcfitsio.a(zcompress.o): undefined reference to symbol 'inflateEnd'
#/usr/bin/ld: //lib/x86_64-linux-gnu/libz.so.1: error adding symbols: DSO manquant dans la ligne de commande
# SOLVED BY ADDING -lz at the end !
### Targets: ###
#all: $(PISCO1_OBJ)
all: $(PISCO1_OBJ) Pisco1.exe

Pisco1.o: Pisco1.cpp $(PISCO1_DEP)

########################################################
# Linux: 
ifeq ($(JLP_SYSTEM),Linux)

Pisco1.exe: Pisco1.cpp  $(PISCO1_DEP)

else
########################################################
# Windows:
Pisco1_rc.o: Pisco1.rc
	windres --use-temp-file --define USE_XPM -I$(WX_INCLUDE_DIR) Pisco1.rc \
	-o Pisco1_rc.o --define __WXMSW__ --define NOPCH

Pisco1.exe: $(PISCO1_OBJ) Pisco1.cpp Pisco1_rc.o
	$(CPP) $(CXFLAGS) $(NO_CONSOLE) -o $(EXEC_DIR)Pisco1.exe \
	Pisco1.cpp Pisco1_rc.o $(PISCO1_OBJ) $(MY_LIB) \
	$(WX_LIB) atmcd32m.lib -lm 
# Missing DLL's:
# libgcc_s_dw2-1.dll, libstdc++-6.dll 
# I also tried adding the linker options: -static-libgcc -static-libstdc++
# but it did not work.
########################################################
endif

clear: clean

clean: 
	rm -f Pisco1_rc.o Pisco1.o $(PISCO1_OBJ)
