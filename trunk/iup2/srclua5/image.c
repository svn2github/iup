/******************************************************************************
 * Automatically generated file (iuplua5). Please don't change anything.                *
 *****************************************************************************/

#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iuplua.h"
#include "il.h"


 
#include<stdlib.h>
static int Image (lua_State * L)
{
  int w, h, i, j;
  unsigned char *img;
  Ihandle *image;
  
  h = iuplua_getn(L, 1);
  lua_pushnumber(L, 1);
  lua_gettable(L, 1);
  w = iuplua_getn(L, -1);
  lua_pop(L, 1);
  
  img = (unsigned char *) malloc (h*w);

  for (i=1; i<=h; i++)
  {
    lua_pushnumber(L, i);
    lua_gettable(L, 1);
    for (j=1; j<=w; j++)
    {
      int idx = (i-1)*w+(j-1);
      lua_pushnumber(L, j);
      lua_gettable(L, -2);
      img[idx] = (unsigned char)lua_tonumber(L, -1);
      lua_pop(L, 1);
    }
    lua_pop(L, 1);
  }
  
  image = IupImage(w,h,img);  
  free(img);

  w = iuplua_getn(L, 2);

  for(i=1; i<=w; i++)
  {
    lua_pushnumber(L,i);
    lua_pushnumber(L,i);
    lua_gettable(L, 2);
    IupStoreAttribute(image, (char *) lua_tostring(L,-2), (char *) lua_tostring(L,-1));
    lua_pop(L, 2);
  }
  
  iuplua_plugstate(L, image);
  iuplua_pushihandle_raw(L, image);
  return 1;
} 
 
int iupimagelua_open(lua_State * L)
{
  iuplua_register(L, Image, "Image");


#ifdef IUPLUA_USELH
#include "image.lh"
#else
  iuplua_dofile(L, "image.lua");
#endif

  return 0;
}

