PROJNAME = iup
LIBNAME = iupluaole

IUP := ..

OPT = YES
NO_LUALINK = Yes
USE_BIN2C_LUA = Yes
NO_LUAOBJECT = Yes

USE_IUPLUA = Yes

LIBS = iupole
DEF_FILE = ctrl/iupluaole.def

ifdef USE_LUA52
  LUASFX = 52
  DEFINES += LUA_COMPAT_MODULE
else
  USE_LUA51 = Yes
  LUASFX = 51
endif

LIBNAME := $(LIBNAME)$(LUASFX)
ifdef NO_LUAOBJECT
  DEFINES += IUPLUA_USELH
  USE_LH_SUBDIR = Yes
  LHDIR = lh
else
  DEFINES += IUPLUA_USELOH
  USE_LOH_SUBDIR = Yes
  LOHDIR = loh$(LUASFX)
endif

SRCLUA = olecontrol.lua

GC = $(addsuffix .c, $(basename $(SRCLUA)))
GC := $(addprefix ctrl/il_, $(GC))

$(GC) : ctrl/il_%.c : ctrl/%.lua generator.lua
	$(LUABIN) generator.lua $<

SRC	= $(GC)
