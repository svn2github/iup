PROJNAME = iup
APPNAME = iuplua
APPTYPE = console

SRCLUA = console5.lua

STRIP = 
OPT = YES      
NO_SCRIPTS = Yes
# IM and IupPPlot uses C++
LINKER = $(CPPC)

NO_LUAOBJECT = Yes
USE_BIN2C_LUA = Yes

ifdef USE_LUA52
  LUASFX = 52
else
  USE_LUA51 = Yes
  LUASFX = 51
endif

APPNAME := $(APPNAME)$(LUASFX)
SRC = iuplua$(LUASFX).c

DEFINES += IUPLUA_USELH
USE_LH_SUBDIR = Yes
LHDIR = lh

ifdef DBG
  USE_STATIC = Yes
endif

ifdef USE_STATIC
  # Statically link everything only when debugging
  IUP := ..
  USE_IUPLUA = Yes
  USE_IUP = Yes
  
  DEFINES = USE_STATIC

  ifeq "$(TEC_UNAME)" "SunOS510x86"
    IUPLUA_NO_GL = Yes
  endif
    
  #IUPLUA_NO_GL = Yes
  ifndef IUPLUA_NO_GL 
    USE_OPENGL = Yes
  else
    DEFINES += IUPLUA_NO_GL
  endif

  #IUPLUA_NO_CD = Yes
  ifndef IUPLUA_NO_CD 
    USE_CDLUA = Yes
    USE_IUPCONTROLS = Yes
    ifneq ($(findstring Win, $(TEC_SYSNAME)), )
      LIBS += iuplua_pplot$(LUASFX) iup_pplot
    else
      IUPLIB = $(IUP)/lib/$(TEC_UNAME)
      SLIB += $(IUPLIB)/libiuplua_pplot$(LUASFX).a $(IUPLIB)/libiup_pplot.a
    endif
      
    ifndef IUPLUA_NO_IM
      ifneq ($(findstring Win, $(TEC_SYSNAME)), )
        LIBS += cdluaim$(LUASFX)
      else
        CDLIB = $(CD)/lib/$(TEC_UNAME)
        SLIB += $(CDLIB)/libcdluaim$(LUASFX).a
      endif
    endif
    ifneq ($(findstring Win, $(TEC_SYSNAME)), )
      USE_GDIPLUS=Yes
    else
  #    USE_XRENDER=Yes
    endif
  else
    DEFINES += IUPLUA_NO_CD
  endif

  #IUPLUA_NO_IM = Yes
  ifndef IUPLUA_NO_IM
    USE_IMLUA = Yes
    
    ifneq ($(findstring Win, $(TEC_SYSNAME)), )
      LIBS += imlua_process$(LUASFX) iupluaim$(LUASFX) im_process iupim
    else
      IUPLIB = $(IUP)/lib/$(TEC_UNAME)
      IMLIB = $(IM)/lib/$(TEC_UNAME)
      SLIB +=  $(IMLIB)/libimlua_process$(LUASFX).a $(IUPLIB)/libiupluaim$(LUASFX).a $(IMLIB)/libim_process.a $(IUPLIB)/libiupim.a
    endif
    
  else
    DEFINES += IUPLUA_NO_IM
  endif

  IUPLUA_IMGLIB = Yes
  ifdef IUPLUA_IMGLIB
    DEFINES += IUPLUA_IMGLIB
    ifneq ($(findstring Win, $(TEC_SYSNAME)), )
      LIBS += iupluaimglib$(LUASFX) iupimglib
    else
      IUPLIB = $(IUP)/lib/$(TEC_UNAME)
      SLIB += $(IUPLIB)/libiupluaimglib$(LUASFX).a $(IUPLIB)/libiupimglib.a
    endif
  endif
else
  ifneq ($(findstring Win, $(TEC_SYSNAME)), )
    # Dinamically link in Windows, when not debugging
    # Must call "tecmake dll8"
    USE_DLL = Yes
    GEN_MANIFEST = No
  else
    # In UNIX Lua is always statically linked, late binding is used.
    USE_STATIC = Yes
  endif
endif


ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  SLIB += setargv.obj
  SRC += iuplua5.rc
endif

ifneq ($(findstring cygw, $(TEC_UNAME)), )
  LDFLAGS = -s
  LIBS += readline history
endif

ifneq ($(findstring Linux, $(TEC_UNAME)), )
  LIBS += dl 
  #To allow late binding
  LFLAGS = -Wl,-E
  LIBS += readline history curses ncurses
endif

ifneq ($(findstring BSD, $(TEC_UNAME)), )
  #To allow late binding
  LFLAGS = -Wl,-E
  LIBS += readline history curses ncurses
endif

ifneq ($(findstring SunOS, $(TEC_UNAME)), )
  LIBS += dl
endif

#ifneq ($(findstring AIX, $(TEC_UNAME)), )
#  FLAGS  += -mminimal-toc
#  OPTFLAGS = -mminimal-toc -ansi -pedantic 
#  LFLAGS = -Xlinker "-bbigtoc"
#endif
