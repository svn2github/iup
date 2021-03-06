/******************************************************************************
 * Automatically generated file (iuplua5). Please don't change anything.                *
 *****************************************************************************/

#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iuplua.h"
#include "il.h"


static int dialog_map_cb(Ihandle *self)
{
  lua_State *L = iuplua_call_start(self, "map_cb");
  return iuplua_call(L, 0);
}

static int dialog_unmap_cb(Ihandle *self)
{
  lua_State *L = iuplua_call_start(self, "unmap_cb");
  return iuplua_call(L, 0);
}

static int dialog_dropfiles_cb(Ihandle *self, char * p0, int p1, int p2, int p3)
{
  lua_State *L = iuplua_call_start(self, "dropfiles_cb");
  lua_pushstring(L, p0);
  lua_pushnumber(L, p1);
  lua_pushnumber(L, p2);
  lua_pushnumber(L, p3);
  return iuplua_call(L, 4);
}

static int dialog_show_cb(Ihandle *self, int p0)
{
  lua_State *L = iuplua_call_start(self, "show_cb");
  lua_pushnumber(L, p0);
  return iuplua_call(L, 1);
}

static int dialog_trayclick_cb(Ihandle *self, int p0, int p1, int p2)
{
  lua_State *L = iuplua_call_start(self, "trayclick_cb");
  lua_pushnumber(L, p0);
  lua_pushnumber(L, p1);
  lua_pushnumber(L, p2);
  return iuplua_call(L, 3);
}

static int dialog_close_cb(Ihandle *self)
{
  lua_State *L = iuplua_call_start(self, "close_cb");
  return iuplua_call(L, 0);
}

static int Dialog(lua_State *L)
{
  Ihandle *ih = IupDialog(iuplua_checkihandleornil(L, 1));
  iuplua_plugstate(L, ih);
  iuplua_pushihandle_raw(L, ih);
  return 1;
}

int iupdialoglua_open(lua_State * L)
{
  iuplua_register(L, Dialog, "Dialog");

  iuplua_register_cb(L, "MAP_CB", (lua_CFunction)dialog_map_cb, NULL);
  iuplua_register_cb(L, "UNMAP_CB", (lua_CFunction)dialog_unmap_cb, NULL);
  iuplua_register_cb(L, "DROPFILES_CB", (lua_CFunction)dialog_dropfiles_cb, NULL);
  iuplua_register_cb(L, "SHOW_CB", (lua_CFunction)dialog_show_cb, NULL);
  iuplua_register_cb(L, "TRAYCLICK_CB", (lua_CFunction)dialog_trayclick_cb, NULL);
  iuplua_register_cb(L, "CLOSE_CB", (lua_CFunction)dialog_close_cb, NULL);

#ifdef IUPLUA_USELOH
#ifdef TEC_BIGENDIAN
#ifdef TEC_64
#include "loh/dialog_be64.loh"
#else
#include "loh/dialog_be32.loh"
#endif
#else
#ifdef TEC_64
#ifdef WIN64
#include "loh/dialog_le64w.loh"
#else
#include "loh/dialog_le64.loh"
#endif
#else
#include "loh/dialog.loh"
#endif
#endif
#else
  iuplua_dofile(L, "dialog.lua");
#endif

  return 0;
}

