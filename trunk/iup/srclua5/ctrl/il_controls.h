/** \file
 * \brief IUPLua5 Controls internal Functions
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IL_CONTROLS_H 
#define __IL_CONTROLS_H

#ifdef __cplusplus
extern "C" {
#endif

int iupcolorbrowserlua_open (lua_State * L);
int iupcellslua_open (lua_State * L);
int iupmatrixlua_open (lua_State * L);
int iupmatrixlistlua_open(lua_State * L);

#ifdef __cplusplus
}
#endif

#endif
