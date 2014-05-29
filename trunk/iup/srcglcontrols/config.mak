PROJNAME = iup
LIBNAME = iupglcontrols
OPT = YES

INCLUDES = ../include ../src . ./freetype

SRC = iup_glcontrols.c iup_glcanvasbox.c iup_glsubcanvas.c iup_gllabel.c \
      iup_glimage.c iup_glfont.c iup_gldraw.c iup_glicon.c iup_glseparator.c \
      iup_glbutton.c iup_gltoggle.c iup_gllink.c iup_glprogressbar.c

LIBS = iupgl iup 
LDIR = ../lib/$(TEC_UNAME)
#LIBS += iupcontrols iupcd cdgl
#USE_CD = Yes
USE_OPENGL = Yes
USE_MACOS_OPENGL = Yes

DEFINES = FTGL_LIBRARY_STATIC
ifneq ($(findstring dll, $(TEC_UNAME)), )
  DEFINES = FTGL_LIBRARY
endif

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
#  INCLUDES += $(X11_INC)
  ifdef USE_MACOS_OPENGL
    LFLAGS = -framework OpenGL
    USE_OPENGL :=
  endif
  ifneq ($(TEC_SYSMINOR), 4)
    BUILD_DYLIB=Yes
  endif
endif
