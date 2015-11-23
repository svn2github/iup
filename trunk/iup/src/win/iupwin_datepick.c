/** \file
 * \brief DatePick Control
 *
 * See Copyright Notice in "iup.h"
 */

#include <windows.h>
#include <commctrl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <memory.h>
#include <stdarg.h>
#include <limits.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"

#include "iupwin_drv.h"
#include "iupwin_handle.h"
#include "iupwin_str.h"
#include "iupwin_info.h"


#ifndef DTM_SETMCSTYLE
#define DTM_SETMCSTYLE   (DTM_FIRST + 11)
#define DTM_GETIDEALSIZE (DTM_FIRST + 15)
#endif
#ifndef MCS_NOSELCHANGEONNAV
#define MCS_NOSELCHANGEONNAV 0x0100
#endif


static int winDatePickSetValueAttrib(Ihandle* ih, const char* value)
{
  int year, month, day;
  if (sscanf(value, "%d/%d/%d", &day, &month, &year) == 3)
  {
    SYSTEMTIME st;

    if (month < 1) month = 1;
    if (month > 12) month = 12;
    if (day < 1) day = 1;
    if (day > 31) day = 31;

    st.wYear = (WORD)year;
    st.wMonth = (WORD)month;
    st.wDay = (WORD)day;

    SendMessage(ih->handle, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&st);
  }
  return 0; /* do not store value in hash table */
}

static char* winDatePickGetValueAttrib(Ihandle* ih)
{
  SYSTEMTIME st;
  SendMessage(ih->handle, DTM_GETSYSTEMTIME, 0, (LPARAM)&st);
  return iupStrReturnStrf("%02d/%02d/%d", st.wDay, st.wMonth, st.wYear);
}

static int winDatePickSetFormatAttrib(Ihandle* ih, const char* value)
{
  SendMessage(ih->handle, DTM_SETFORMAT, 0, (LPARAM)value);
  return 1;
}

static int winDatePickSetStandardFontAttrib(Ihandle* ih, const char* value)
{
  iupdrvSetStandardFontAttrib(ih, value);

  if (ih->handle)
  {
    HFONT hFont = (HFONT)SendMessage(ih->handle, WM_GETFONT, 0, 0);
    SendMessage(ih->handle, DTM_SETMCFONT, (WPARAM)hFont, (LPARAM)TRUE); /* not working in Windows 10 (not tested in 7 or 8) - works in XP */
  }

  return 1;
}

static char* winDatePickGetTodayAttrib(Ihandle* ih)
{
  SYSTEMTIME st;
  (void)ih;
  GetLocalTime(&st);
  return iupStrReturnStrf("%02d/%02d/%d", st.wDay, st.wMonth, st.wYear);
}


/*********************************************************************************************/


static int winDatePickWmNotify(Ihandle* ih, NMHDR* msg_info, int *result)
{
  if (msg_info->code == DTN_DATETIMECHANGE)
  {
    char* old_value = iupAttribGet(ih, "_IUP_OLDVALUE");
    char* value = winDatePickGetValueAttrib(ih);
    if (!iupStrEqual(old_value, value))
    {
      /* when user uses the dropdown, notification is called twice */
      iupBaseCallValueChangedCb(ih);
      iupAttribSetStr(ih, "_IUP_OLDVALUE", value);
    }
  }

  (void)result;
  return 0; /* result not used */
}


/*********************************************************************************************/

void iupdrvTextAddBorders(int *w, int *h);

static void winDatePickComputeNaturalSizeMethod(Ihandle* ih, int *w, int *h, int *children_expand)
{
  (void)children_expand; /* unset if not a container */

  if (ih->handle && iupwinIsVistaOrNew() && iupwin_comctl32ver6)
  {
    SIZE size;
    SendMessage(ih->handle, DTM_GETIDEALSIZE, 0, (LPARAM)&size);
    *w = size.cx;
    *h = size.cy;
  }
  else
  {
    iupdrvFontGetMultiLineStringSize(ih, "WW/WW/WWWW", w, h);
    iupdrvTextAddBorders(w, h);
    *w += iupdrvGetScrollbarSize();
  }
}

static int winDatePickMapMethod(Ihandle* ih)
{
  DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | DTS_SHORTDATECENTURYFORMAT;

  if (!ih->parent)
    return IUP_ERROR;

  if (iupAttribGetBoolean(ih, "CANFOCUS"))
    dwStyle |= WS_TABSTOP;

  if (!iupwinCreateWindow(ih, DATETIMEPICK_CLASS, 0, dwStyle, NULL))
    return IUP_ERROR;

  /* Process WM_NOTIFY */
  IupSetCallback(ih, "_IUPWIN_NOTIFY_CB", (Icallback)winDatePickWmNotify);
  
  SendMessage(ih->handle, DTM_SETFORMAT, 0, (LPARAM)TEXT("dd/MMM/yyyy"));

  if (iupwinIsVistaOrNew())
  {
    dwStyle = MCS_NOTODAY | MCS_NOSELCHANGEONNAV;

    if (iupAttribGetBoolean(ih, "CALENDARWEEKNUMBERS"))
      dwStyle |= MCS_WEEKNUMBERS;

    SendMessage(ih->handle, DTM_SETMCSTYLE, 0, (LPARAM)dwStyle);
  }

  return IUP_NOERROR;
}

Iclass* iupDatePickNewClass(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->name = "datepick";
  ic->format = NULL;  /* no parameters */
  ic->nativetype = IUP_TYPECONTROL;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 1;

  /* Class functions */
  ic->New = iupDatePickNewClass;

  ic->Map = winDatePickMapMethod;
  ic->ComputeNaturalSize = winDatePickComputeNaturalSizeMethod;

  ic->LayoutUpdate = iupdrvBaseLayoutUpdateMethod;
  ic->UnMap = iupdrvBaseUnMapMethod;

  /* Callbacks */
  iupClassRegisterCallback(ic, "VALUECHANGED_CB", "");

  /* Common Callbacks */
  iupBaseRegisterCommonCallbacks(ic);

  /* Common */
  iupBaseRegisterCommonAttrib(ic);

  /* Visual */
  iupBaseRegisterVisualAttrib(ic);

  iupClassRegisterAttribute(ic, "STANDARDFONT", NULL, winDatePickSetStandardFontAttrib, IUPAF_SAMEASSYSTEM, "DEFAULTFONT", IUPAF_NO_SAVE | IUPAF_NOT_MAPPED);

  iupClassRegisterAttribute(ic, "VALUE", winDatePickGetValueAttrib, winDatePickSetValueAttrib, NULL, NULL, IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TODAY", winDatePickGetTodayAttrib, NULL, NULL, NULL, IUPAF_READONLY | IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "FORMAT", NULL, winDatePickSetFormatAttrib, IUPAF_SAMEASSYSTEM, "dd/MMM/yyyy", IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "CALENDARWEEKNUMBERS", NULL, NULL, NULL, NULL, IUPAF_NO_DEFAULTVALUE | IUPAF_NO_INHERIT);

  return ic;
}

Ihandle *IupDatePick(void)
{
  return IupCreate("datepick");
}