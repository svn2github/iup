/** \file
 * \brief Windows Focus
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdlib.h>
#include <stdio.h>

#include <windows.h>

#include "iup.h"
#include "iup_object.h"
#include "iup_focus.h"
#include "iup_assert.h" 
#include "iup_drv.h" 
#include "iup_attrib.h" 

#include "iupwin_drv.h"
#include "iupwin_handle.h"

/* not defined for gcc */
#ifndef WM_CHANGEUISTATE
#define WM_CHANGEUISTATE                0x0127
#endif
#ifndef UIS_CLEAR
#define UIS_CLEAR                       2
#endif
#ifndef UISF_HIDEFOCUS
#define UISF_HIDEFOCUS                  0x1
#endif

/* Since Windows XP, the focus feedback only appears after the user press a key.
   Except for the IupText where the feedback is the caret.
   Before that if you click in a control the focus feedback will be hidden.

   We manually send WM_CHANGEUISTATE because we do not use IsDialogMessage anymore,
   and the focus feedback was not shown even after the used press a key.

   TODO: I would like a form to always show the feedback, but could not understand how WM_CHANGEUISTATE works.
         Neither SystemParametersInfo(SPI_SETKEYBOARDCUES, TRUE) or SystemParametersInfo(SPI_SETKEYBOARDPREF, TRUE) worked.
*/
void iupdrvSetFocus(Ihandle *ih)
{
  SetFocus(ih->handle);
  SendMessage(ih->handle, WM_CHANGEUISTATE, UIS_CLEAR|UISF_HIDEFOCUS, 0);
}

void iupwinWmSetFocus(Ihandle *ih)
{
  Ihandle* dialog = IupGetDialog(ih);
  if (ih != dialog)
    iupAttribSetStr(dialog, "_IUPWIN_LASTFOCUS", (char*)ih);  /* used by IupMenu */
  else
  {
    /* if a control inside that dialog had the focus, then reset to it when the dialog gets the focus */
    Ihandle* lastfocus = (Ihandle*)iupAttribGet(dialog, "_IUPWIN_LASTFOCUS");
    if (lastfocus) IupSetFocus(lastfocus);
  }

  iupCallGetFocusCb(ih);
}
