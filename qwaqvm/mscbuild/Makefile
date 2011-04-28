#############################################################################
# Makefile for Win32 Squeak using MingW32 make and Visual C++
# Do make init to allow make -n to function.
#############################################################################

#############################################################################
# Base definitions:

# The name of the VM to build
VM:=Teleplace

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
include $(SRCDIR)/plugins.ext.msc
include $(SRCDIR)/plugins.int

LIBS:= $(addprefix $(OBJDIR)/, $(addsuffix .lib, $(INTERNAL_PLUGINS)))
DLLS:= $(addprefix $(OBJDIR)/, $(addsuffix .dll, $(EXTERNAL_PLUGINS)))

#############################################################################
# Generic VM source file definitions
#
VMDIR:= $(SRCDIR)/vm
VMOBJ:=	$(VMSRC:.c=.o)
VMOBJ:=	$(filter-out gnu-interp.o sqFilePrims.o, $(VMOBJ))
VMOBJ:= $(addprefix $(OBJDIR)/,$(VMOBJ))
BTOBJ:= $(OBJDIR)/btext.o
ETOBJ:= $(OBJDIR)/etext.o

#LIBSRC:= $(wildcard *.c)
#LIBOBJ:= $(LIBSRC:.c=.o)

#############################################################################
# SqueakVM definitions
#
VMEXE:=  $(OBJDIR)/$(VM).exe
VMDEF:=	$(VM).def
VMLIB:=	$(VM).lib
VMRES:=	$(OBJDIR)/$(VM).res
VMDEFIN:=$(WIN32MISCDIR)/$(VM).def.in


.PRECIOUS: eitheror.exe

include Makefile.CFLAGS

#############################################################################
# Linker settings
#
LD:=	 link
LDFLAGS:= /DEBUG /subsystem:windows
STDLIBS:= advapi32.lib ddraw.lib dinput.lib opengl32.lib wsock32.lib comdlg32.lib ole32.lib winmm.lib version.lib shell32.lib user32.lib gdi32.lib kernel32.lib
#STDLIBS:= -lddraw -ldinput -lopengl32 -lwsock32 -lcomdlg32 -lole32 -lwinmm \
#	 -lversion -lwininet -luser32 -lgdi32 -lkernel32 -lcrtdll

#############################################################################
# Gnu/windows tools to use
#
CP:= cp
RM:= rm
MD:= mkdir
MAKE:=make # On e.g. Win98 this expands to C:/GNUTOOLS/BIN/MAKE.EXE which fails

#############################################################################
# RC settings
#
# Note: RC compiles the .rc files into linkable .o files (with .res extensions)
#
RC=	 rc
RCFLAGS= 

.SUFFIXES:
.SUFFIXES:	.ccg .cc .c .o .s .i .rc .res .cg .hg .ccg .cpp



#############################################################################
# Common build rules
#
default: print-settings init eitheror.exe $(OBJDIR) $(VMEXE) $(DLLS)

# Do make init to allow make -n to function.
init:	$(OBJDIR) eitheror.exe

print-settings:
	@echo ---------------- Makefile settings ------------------
	@echo VPATH=$(VPATH)
	@echo INCLUDES=$(INCLUDES)
	@echo CFLAGS=$(CFLAGS)
	@echo -----------------------------------------------------

eitheror.exe: $(WIN32UTILDIR)/eitheror.c
	$(CC) /O1 /Zi $< -o $@

$(BTOBJ):	$(WIN32MISCDIR)/btext.c
	$(CC) -c /Fo$(BTOBJ) /O1 $<

$(ETOBJ):	$(WIN32MISCDIR)/etext.c
	$(CC) -c /Fo$(ETOBJ) /O1 $<

vm:	$(VMEXE)

$(VMEXE): interp.c $(VMOBJ) $(LIBS) $(VMRES) $(BTOBJ) $(ETOBJ)
	$(LD) $(LDFLAGS) /out:$(VMEXE) $(BTOBJ) $(VMOBJ) $(VMRES) $(LIBS) $(STDLIBS) $(ETOBJ) /stack:0x20000 /map
	$(RM) $(OBJDIR)/version.o

$(OBJDIR):
	@-mkdir $(subst /,\\,$(BLDDIR))
	mkdir $(subst /,\\,$(OBJDIR))
	mkdir $(subst /,\\,$(addprefix $(BLDDIR)/, $(INTERNAL_PLUGINS)))
	mkdir $(subst /,\\,$(addprefix $(BLDDIR)/, $(EXTERNAL_PLUGINS)))

#############################################################################
# The exports for named primitives from Squeak (required by VM)
#
#$(VMDEF) $(VMLIB): $(VMOBJ)
#	$(DLLTOOL) --input-def $(VMDEFIN) --output-def $(OBJDIR)/$(VMDEF) --output-exp --output-lib $(OBJDIR)/$(VMLIB) $(VMOBJ)

#############################################################################
# Building plugins
#

.PHONY: FORCE

# Internal plugin.  Build as lib then link in lib
# Check for Makefile.msvc in win32 plugins directory otherwise use default Makefile
# N.B.  PLATDIR *must* be a relative path for this to work
$(OBJDIR)/%.lib:	FORCE
	@-mkdir $(subst /,\\,$(BLDDIR)/$(*F))
	$(MAKE) $(MFLAGS) -C $(BLDDIR)/$(*F) \
		-f../../$(shell eitheror.exe $(PLATDIR)/win32/plugins/$(*F)/Makefile.msvc Makefile.plugin) \
		PLATDIR=../../$(PLATDIR) BASEDIR=../../$(BASEDIR) SRCDIR=../../$(SRCDIR) LIBNAME=$(*F) \
		XDEFS=-DSQUEAK_BUILTIN_PLUGIN \
		../../$(OBJDIR)/$(*F).lib

# External plugin.  Build as dll and copy to vm dir ($(OBJDIR)).
# Check for Makefile.msvc in win32 plugins directory otherwise use default Makefile
$(OBJDIR)/%.dll:	FORCE
	@-mkdir $(subst /,\\,$(BLDDIR)/$(*F))
	$(MAKE) $(MFLAGS) -C $(BLDDIR)/$(*F) \
		-f../../$(shell eitheror.exe $(PLATDIR)/win32/plugins/$(*F)/Makefile.msvc Makefile.plugin) \
		PLATDIR=../../$(PLATDIR) BASEDIR=../../$(BASEDIR) SRCDIR=../../$(SRCDIR) LIBNAME=$(*F) \
		../../$(OBJDIR)/$(*F).dll

#############################################################################
# Basic rules

$(OBJDIR)/%.o:	%.c $(wildcard $(CROSSDIR)/*.h) $(wildcard $(WIN32DIR)/*.h) $(wildcard $(VMDIR)/*.h)
	$(CC) /Fo$@ $(CFLAGS) $(INCLUDES) $(DEFS) -c $<

$(OBJDIR)/%.res: $(WIN32MISCDIR)/%.rc
	$(RC) $(RCFLAGS) /fo $@ /i $(dir $<) $<
