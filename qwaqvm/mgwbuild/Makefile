#############################################################################
# Makefile for Win32 Squeak using gcc-3.4.x and MingW32 5.1.4
# Do make init to allow mingw32-make -n to function.
#############################################################################

#############################################################################
# Base definitions:

# The name of the VM to build
VM:=OpenQwaq

# Is this a Croquet VM (defaults to OGL instead of D3D)?
CROQUET:=-DCROQUET

#############################################################################
# Default locations
#
ifndef BASEDIR
BASEDIR:=.
endif

SRCDIR:= ../winbuild/src
BLDDIR:= $(BASEDIR)/build
OBJDIR:= $(BLDDIR)/vm

# N.B. PLATDIR *must* be a relative path, see -f../../$(shell eitheror ... below
PLATDIR:=../platforms
CROSSDIR:=$(PLATDIR)/Cross/vm
WIN32DIR:=$(PLATDIR)/win32/vm
MAKERDIR:=$(SRCDIR)/vm
WIN32MISCDIR:=$(PLATDIR)/win32/misc
WIN32UTILDIR:=$(PLATDIR)/win32/util

CROSSSRC:= $(wildcard $(CROSSDIR)/*.c) $(wildcard $(CROSSDIR)/*.cpp)
WIN32SRC:= $(wildcard $(WIN32DIR)/*.c) $(wildcard $(WIN32DIR)/*.cpp)
MAKERSRC:= $(wildcard $(MAKERDIR)/*.c) $(wildcard $(MAKERDIR)/*.cpp)
VMSRC:= $(notdir $(MAKERSRC) $(WIN32SRC) $(CROSSSRC))

VPATH:= $(MAKERDIR) $(WIN32DIR) $(CROSSDIR)

#############################################################################
# The internal (.lib) and external (.dll) plugins
#
include $(SRCDIR)/plugins.ext
include $(SRCDIR)/plugins.int

LIBS:= $(addprefix $(OBJDIR)/, $(addsuffix .lib, $(INTERNAL_PLUGINS)))
DLLS:= $(addprefix $(OBJDIR)/, $(addsuffix .dll, $(EXTERNAL_PLUGINS)))

#############################################################################
# Generic VM source file definitions
#
VMDIR:= $(SRCDIR)/vm
VMOBJ:=	$(VMSRC:.c=.o)
VMOBJ:=	$(filter-out cointerp.o interp.o gnu-interp.o sqFilePrims.o, $(VMOBJ))
VMOBJ:= $(addprefix $(OBJDIR)/,$(VMOBJ))

#LIBSRC:= $(wildcard *.c)
#LIBOBJ:= $(LIBSRC:.c=.o)

#############################################################################
# SqueakVM definitions
#
VMEXE:=  $(OBJDIR)/$(VM).exe
CONSOLEVMEXE:=  $(OBJDIR)/$(VM)Console.exe
STRIPEXE:=  $(OBJDIR)/$(VM)Stripped.exe
STRIPCONSOLEEXE:=  $(OBJDIR)/$(VM)ConsoleStripped.exe
VMDEF:=	$(VM).def
VMEXP:=	$(OBJDIR)/$(VM).exp
VMMAP:=	$(OBJDIR)/$(VM).map
VMLIB:=	$(VM).lib
VMRES:=	$(VM).res
VMDEFIN:=$(BASEDIR)/$(VM).def.in
BTOBJ:= $(OBJDIR)/btext.o
ETOBJ:= $(OBJDIR)/etext.o


.PRECIOUS: eitheror.exe

#############################################################################
# DirectX definitions
#
DXDIR:= c:/dx7sdk/include

#############################################################################
# C compiler settings (gcc-3.4.5 MingW32 5.4)
#
# Define NDEBUG for production.  Do not define it for debugging.  It causes
# a catch-all exception handler to be defined which is good for production but
# gets in the way of debugging.  For debugging also define NODBGPRINT to prevent
# printf being redefined to print to the debug console
NDEBUG:=-DNDEBUG
# Set minimum version to WindowsXP (see \MinGW\include\w32api.h)
WINVER:=-D_WIN32_WINNT=0x0501 -DWINVER=0x0501
CC:=	gcc
OFLAGS:= -O2 -march=pentium4 -momit-leaf-frame-pointer -fno-math-errno -frounding-math
CFLAGS:= -ggdb2 -mwindows -mwin32 \
	-mno-rtd -mms-bitfields -mno-accumulate-outgoing-args $(OFLAGS)
DEFS:=	$(WINVER) -DWIN32 -DWIN32_FILE_SUPPORT -DNO_ISNAN \
		-DNO_SERVICE -DNO_STD_FILE_SUPPORT \
		$(NDEBUG) -DLSB_FIRST -DVM_NAME=\"$(VM)\" -DX86 $(XDEFS) $(CROQUET)
XDEFS:=	-DSQUEAK_BUILTIN_PLUGIN
INCLUDES:= -I. -I$(VMDIR) -I$(WIN32DIR) -I$(CROSSDIR) -I$(DXDIR) $(XINC)

#############################################################################
# Linker settings
#
# Note: I had to use 'gcc' instead of 'ld' to prevent unresolved symbols
#       The switch '-mwindows' gives us a GUI app instead of a console app.
#
LD:=	 gcc
LDFLAGS:= -mwindows
#LDFLAGS:= -mconsole
CONSOLELDFLAGS:= -mconsole
STDLIBS:= -lddraw -ldinput -lopengl32 -lwsock32 -lcomdlg32 -lole32 -lwinmm \
	 -lversion -lwininet -luser32 -lgdi32 -lpsapi -lkernel32 -lcrtdll -ldsound

#############################################################################
# Tools to use
#
AR:= ar rc
CP:= cp
RM:= rm
DLLTOOL:=	dlltool
DLLWRAP:=	dllwrap
#OPTSTRIP:=	strip # for production
OPTSTRIP:=	echo not doing strip
STRIP:=	strip

#############################################################################
# RC settings
#
# Note: RC compiles the .rc files into linkable .o files
#
RC:=	 windres
RCFLAGS:=

.SUFFIXES:
.SUFFIXES:	.ccg .cc .c .o .s .i .rc .res .cg .hg .ccg .cpp



#############################################################################
# Common build rules
#
default: print-settings init eitheror.exe $(OBJDIR) $(VMEXE) $(DLLS) $(STRIPEXE) $(STRIPCONSOLEEXE)

# Do make init to allow make -n to function.
init:	$(OBJDIR) eitheror.exe

print-settings:
	@echo ---------------- Makefile settings ------------------
	@echo VPATH=$(VPATH)
	@echo INCLUDES=$(INCLUDES)
	@echo CFLAGS=$(CFLAGS)
	@echo INTERNAL_PLUGINS=$(INTERNAL_PLUGINS)
	@echo EXTERNAL_PLUGINS=$(EXTERNAL_PLUGINS)
	@echo LIBS=$(LIBS)
	@echo DLLS=$(DLLS)
	@echo -----------------------------------------------------

print-objects:
	@echo ---------------- Makefile objects ------------------
	@echo VMOBJ=$(VMOBJ)
	@echo -----------------------------------------------------

eitheror.exe: $(WIN32UTILDIR)/eitheror.c
	$(CC) -o $@ -O1 $<

$(BTOBJ):	$(WIN32MISCDIR)/btext.c
	$(CC) -c -o $@ -fomit-frame-pointer -O2 $<

$(ETOBJ):	$(WIN32MISCDIR)/etext.c
	$(CC) -c -o $@ -fomit-frame-pointer -O2 $<

vm:	$(VMEXE)

$(VMEXE): $(VMOBJ) $(LIBS) $(VMEXP) resource.o $(BTOBJ) $(ETOBJ)
	$(CC) -o $(OBJDIR)/version.o $(CFLAGS) $(INCLUDES) $(DEFS) -c $(WIN32DIR)/version.c
	$(LD) $(LDFLAGS) -o $(VMEXE) \
			$(BTOBJ) $(VMOBJ) $(VMEXP) $(OBJDIR)/resource.o $(LIBS) $(STDLIBS) $(ETOBJ)
	nm --numeric-sort --defined-only -f bsd $(VMEXE) >$(VMMAP)
	$(OPTSTRIP) --strip-all $(VMEXE)

$(CONSOLEVMEXE): $(VMOBJ) $(LIBS) $(VMEXP) resource.o $(BTOBJ) $(ETOBJ)
	$(CC) -o $(OBJDIR)/version.o $(CFLAGS) $(INCLUDES) $(DEFS) -c $(WIN32DIR)/version.c
	$(LD) $(CONSOLELDFLAGS) -o $(CONSOLEVMEXE) \
			$(BTOBJ) $(VMOBJ) $(VMEXP) $(OBJDIR)/resource.o $(LIBS) $(STDLIBS) $(ETOBJ)
	nm --numeric-sort --defined-only -f bsd $(CONSOLEVMEXE) >$(VMMAP)
	$(OPTSTRIP) --strip-all $(CONSOLEVMEXE)

$(STRIPEXE): $(VMEXE)
	$(STRIP) --strip-all -o $(STRIPEXE) $(VMEXE)

$(STRIPCONSOLEEXE): $(CONSOLEVMEXE)
	$(STRIP) --strip-all -o $(STRIPCONSOLEEXE) $(CONSOLEVMEXE)

$(OBJDIR):
	@-mkdir $(subst /,\\,$(BLDDIR))
	mkdir $(subst /,\\,$(OBJDIR))
	mkdir $(subst /,\\,$(addprefix $(BLDDIR)/, $(INTERNAL_PLUGINS)))
	mkdir $(subst /,\\,$(addprefix $(BLDDIR)/, $(EXTERNAL_PLUGINS)))

#############################################################################
# The exports for named primitives from Squeak (required by VM)
#
$(VMDEF) $(VMEXP) $(VMLIB): $(VMOBJ)
	$(DLLTOOL) --input-def $(VMDEFIN) --output-def $(OBJDIR)/$(VMDEF) --output-exp $(VMEXP) --output-lib $(OBJDIR)/$(VMLIB) $(VMOBJ)

#############################################################################
# Building plugins
#

.PHONY: FORCE

# Internal plugin.  Build as lib then link in lib
# Check for Makefile in win32 plugins directory otherwise use default Makefile
# N.B.  PLATDIR *must* be a relative path for this to work
$(OBJDIR)/%.lib:	FORCE
	@-mkdir $(subst /,\\,$(BLDDIR)/$(*F))
	$(MAKE) $(MFLAGS) -C $(BLDDIR)/$(*F) \
		-f../../$(shell eitheror.exe $(PLATDIR)/win32/plugins/$(*F)/Makefile Makefile.plugin) \
		PLATDIR=../../$(PLATDIR) BASEDIR=../../$(BASEDIR) LIBNAME=$(*F) \
		XDEFS=-DSQUEAK_BUILTIN_PLUGIN \
		../../$(OBJDIR)/$(*F).lib

# External plugin.  Build as dll and copy to vm dir ($(OBJDIR)).
# Check for Makefile in win32 plugins directory otherwise use default Makefile
$(OBJDIR)/%.dll:	FORCE
	@-mkdir $(subst /,\\,$(BLDDIR)/$(*F))
	$(MAKE) $(MFLAGS) -C $(BLDDIR)/$(*F) \
		-f../../$(shell eitheror.exe $(PLATDIR)/win32/plugins/$(*F)/Makefile Makefile.plugin) \
		PLATDIR=../../$(PLATDIR) BASEDIR=../../$(BASEDIR) LIBNAME=$(*F) \
		../../$(OBJDIR)/$(*F).dll

FORCE:

#############################################################################
# Basic rules

$(OBJDIR)/%.o: %.c $(wildcard $(CROSSDIR)/*.h) $(wildcard $(WIN32DIR)/*.h) $(wildcard $(MAKERDIR)/*.h)
		$(CC) -o $@ $(CFLAGS) $(INCLUDES) $(DEFS) -c $<

.c.s:
		$(CC) -S -o $@ -fverbose-asm -Wa,ah $(CFLAGS) $(INCLUDES) $(DEFS) -c $<

.rc.res:
		$(RC) $(RCFLAGS) -i $< -o $(OBJDIR)/$@

resource.o:	$(VMRES)
		$(RC) $(RCFLAGS) -i $(OBJDIR)/$< -o $(OBJDIR)/$@

#############################################################################
# Extra specific dependencies
#

sqNamedPrims.o:	sqNamedPrims.c sqNamedPrims.h

Squeak.res: misc/Squeak.rc
	$(RC) $(RCFLAGS) -i $< -o $(OBJDIR)/$@

Croquet.res: misc/Croquet.rc
	$(RC) $(RCFLAGS) -i $< -o $(OBJDIR)/$@
