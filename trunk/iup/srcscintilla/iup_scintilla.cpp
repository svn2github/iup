/*
* IupScintilla component
*
* Description : A source code editing component, 
* derived from Scintilla (http://www.scintilla.org/)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <Scintilla.h>
#include <SciLexer.h>

#ifdef GTK
#include <gtk/gtk.h>
#include <ScintillaWidget.h>
#else
#include <windows.h>
#endif

#include "iup.h"
#include "iup_scintilla.h"
#include "iupcbs.h"
#include "iup_mask.h"

#include "iup_class.h"
#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_register.h"
#include "iup_stdcontrols.h"
#include "iup_layout.h"
#include "iup_assert.h"

#ifdef GTK
#include "iupgtk_drv.h"
#else
#include "iupwin_drv.h"
#endif

#include "iupsci_folding.h"
#include "iupsci_markers.h"
#include "iupsci_margin.h"
#include "iupsci_lexer.h"
#include "iupsci_style.h"
#include "iupsci_text.h"
#include "iupsci_selection.h"
#include "iupsci_clipboard.h"
#include "iupsci_overtype.h"
#include "iupsci_scrolling.h"
#include "iupsci_tab.h"
#include "iupsci_wordwrap.h"
#include "iupsci.h"


#define WM_IUPCARET WM_APP+1   /* Custom IUP message */


sptr_t iupScintillaSendMessage(Ihandle* ih, unsigned int iMessage, uptr_t wParam, sptr_t lParam)
{
#ifdef GTK
  return scintilla_send_message(SCINTILLA(ih->handle), iMessage, wParam, lParam);
#else
  return SendMessage(ih->handle, iMessage, wParam, lParam);
#endif
}


/***** AUXILIARY ATTRIBUTES *****/

void IupScintillaConvertLinColToPos(Ihandle* ih, int lin, int col, int *pos)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (!ih->handle)
    return;
    
  if (IupClassMatch(ih, "scintilla"))
  {
    *pos = iupScintillaSendMessage(ih, SCI_POSITIONFROMLINE, lin, 0);
    
    if(*pos != -1)
    {
      int line_length = iupScintillaSendMessage(ih, SCI_GETLINEENDPOSITION, lin, 0) - iupScintillaSendMessage(ih, SCI_POSITIONFROMLINE, lin, 0);
      if(col <= line_length)
        *pos += col;
      else
        *pos += line_length;
    }
    else
    {
      /* "lin" is greater than the lines in the document */
      *pos = iupScintillaSendMessage(ih, SCI_GETLINECOUNT, 0, 0);
    }
  }
}

void IupScintillaConvertPosToLinCol(Ihandle* ih, int pos, int *lin, int *col)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  if (!ih->handle)
    return;
    
  if (IupClassMatch(ih, "scintilla"))
  {
    *lin = iupScintillaSendMessage(ih, SCI_LINEFROMPOSITION, pos, 0);
    *col = iupScintillaSendMessage(ih, SCI_GETCOLUMN, pos, 0);
  }
}

static int iScintillaConvertXYToPos(Ihandle* ih, int x, int y)
{
  return iupScintillaSendMessage(ih, SCI_POSITIONFROMPOINT, x, y);
}


/***** GENERAL FUNCTIONS *****/
static char* iScintillaGetMaskDataAttrib(Ihandle* ih)
{
  /* Used only by the OLD iupmask API */
  return (char*)ih->data->mask;
}

static char* iScintillaGetMaskAttrib(Ihandle* ih)
{
  if (ih->data->mask)
    return iupMaskGetStr((Imask*)ih->data->mask);
  else
    return NULL;
}

static int iScintillaSetValueMaskedAttrib(Ihandle* ih, const char* value)
{
  if (value)
  {
    if (ih->data->mask && iupMaskCheck((Imask*)ih->data->mask, value)==0)
      return 0; /* abort */
    IupStoreAttribute(ih, "VALUE", value);
  }
  return 0;
}

static int iScintillaSetMaskAttrib(Ihandle* ih, const char* value)
{
  if (!value)
  {
    if (ih->data->mask)
    {
      iupMaskDestroy((Imask*)ih->data->mask);
      ih->data->mask = NULL;
    }
  }
  else
  {
    int casei = iupAttribGetInt(ih, "MASKCASEI");
    Imask* mask = iupMaskCreate(value,casei);
    if (mask)
    {
      if (ih->data->mask)
        iupMaskDestroy((Imask*)ih->data->mask);

      ih->data->mask = mask;
      return 0;
    }
  }

  return 0;
}

static int iScintillaSetMaskIntAttrib(Ihandle* ih, const char* value)
{
  if (!value)
  {
    if (ih->data->mask)
    {
      iupMaskDestroy((Imask*)ih->data->mask);
      ih->data->mask = NULL;
    }
  }
  else
  {
    Imask* mask;
    int min, max;

    if (iupStrToIntInt(value, &min, &max, ':')!=2)
      return 0;

    mask = iupMaskCreateInt(min,max);

    if (ih->data->mask)
      iupMaskDestroy((Imask*)ih->data->mask);

    ih->data->mask = mask;
  }

  return 0;
}

static int iScintillaSetMaskFloatAttrib(Ihandle* ih, const char* value)
{
  if (!value)
  {
    if (ih->data->mask)
    {
      iupMaskDestroy((Imask*)ih->data->mask);
      ih->data->mask = NULL;
    }
  }
  else
  {
    Imask* mask;
    float min, max;

    if (iupStrToFloatFloat(value, &min, &max, ':')!=2)
      return 0;

    mask = iupMaskCreateFloat(min,max);

    if (ih->data->mask)
      iupMaskDestroy((Imask*)ih->data->mask);

    ih->data->mask = mask;
  }

  return 0;
}

static int iScintillaSetAppendNewlineAttrib(Ihandle* ih, const char* value)
{
  if (iupStrBoolean(value))
    ih->data->append_newline = 1;
  else
    ih->data->append_newline = 0;
  return 0;
}

static char* iScintillaGetAppendNewlineAttrib(Ihandle* ih)
{
  if (ih->data->append_newline)
    return "YES";
  else
    return "NO";
}

static char* iScintillaGetScrollbarAttrib(Ihandle* ih)
{
  if (ih->data->sb == (IUP_SB_HORIZ | IUP_SB_VERT))
    return "YES";
  if (ih->data->sb & IUP_SB_HORIZ)
    return "HORIZONTAL";
  if (ih->data->sb & IUP_SB_VERT)
    return "VERTICAL";
  
  return "NO";
}

static int iScintillaSetScrollbarAttrib(Ihandle* ih, const char* value)
{
  if (!value)
    value = "YES";    /* default is YES */

  if (iupStrEqualNoCase(value, "YES"))
    ih->data->sb = IUP_SB_HORIZ | IUP_SB_VERT;
  else if (iupStrEqualNoCase(value, "HORIZONTAL"))
    ih->data->sb = IUP_SB_HORIZ;
  else if (iupStrEqualNoCase(value, "VERTICAL"))
    ih->data->sb = IUP_SB_VERT;
  else
    ih->data->sb = IUP_SB_NONE;

  return 0;
}

/***** NOTIFICATIONS *****
Mapping callbacks!
Notifications not implemented yet:
SCN_STYLENEEDED
SCN_CHARADDED
SCN_SAVEPOINTREACHED
SCN_SAVEPOINTLEFT
SCN_MODIFYATTEMPTRO
SCN_KEY
SCN_UPDATEUI
SCN_MACRORECORD
SCN_NEEDSHOWN
SCN_PAINTED
SCN_USERLISTSELECTION
SCN_URIDROPPED
SCN_ZOOM
SCN_HOTSPOTDOUBLECLICK
SCN_HOTSPOTRELEASECLICK
SCN_INDICATORCLICK
SCN_INDICATORRELEASE
SCN_CALLTIPCLICK
SCN_AUTOCSELECTION
SCN_AUTOCCANCELLED
SCN_AUTOCCHARDELETED
*/
static void iScintillaNotify(Ihandle *ih, struct SCNotification* pMsg)
{
  int lineClick = iupScintillaSendMessage(ih, SCI_LINEFROMPOSITION, pMsg->position, 0);

  switch(pMsg->nmhdr.code)
  {
    case SCN_MARGINCLICK:
    {
      IFnii cb = (IFnii)IupGetCallback(ih, "MARGINCLICK_CB");
      if (cb)
        cb(ih, pMsg->margin, lineClick);
    }
    break;
    case SCN_DOUBLECLICK:
    {
      IFnii cb = (IFnii)IupGetCallback(ih, "DBLCLICK_CB");
      if (cb)
        cb(ih, pMsg->modifiers, lineClick);
    }
    break;
    case SCN_HOTSPOTCLICK:
    {
      IFnii cb = (IFnii)IupGetCallback(ih, "HOTSPOTCLICK_CB");
      if (cb)
        cb(ih, pMsg->modifiers, lineClick);
    }
    break;
    case SCN_MODIFIED:
    {
      /* Not mapped here:
      SC_MOD_CHANGESTYLE, SC_MOD_CHANGEFOLD, SC_MOD_CHANGEMARKER, SC_MOD_CHANGEINDICATOR, SC_MOD_CHANGELINESTATE,
      SC_MOD_LEXERSTATE, SC_MOD_CHANGEMARGIN, SC_MOD_CHANGEANNOTATION, SC_MOD_CONTAINER, SC_MODEVENTMASKALL,
      SC_PERFORMED_USER, SC_STARTACTION */

      if((pMsg->modificationType & SC_PERFORMED_UNDO) || (pMsg->modificationType & SC_PERFORMED_REDO) ||
         (pMsg->modificationType & SC_MOD_BEFOREINSERT) || (pMsg->modificationType & SC_MOD_BEFOREDELETE) ||
         (pMsg->modificationType & SC_MULTISTEPUNDOREDO) || (pMsg->modificationType & SC_MULTILINEUNDOREDO))
      {
        IFnis cb = (IFnis)IupGetCallback(ih, "ACTION");
        if (cb && pMsg->text)
        {
          if(pMsg->length != 1)
            cb(ih, 0, (char*)pMsg->text);
          else
            cb(ih, pMsg->text[0], (char*)pMsg->text);

#ifndef GTK
          PostMessage(ih->handle, WM_IUPCARET, 0, 0L);
#endif
        }
      }

      if((pMsg->modificationType & SC_MOD_INSERTTEXT) || (pMsg->modificationType & SC_MOD_DELETETEXT) ||
         (pMsg->modificationType & SC_LASTSTEPINUNDOREDO))
      {
        IFn cb = (IFn)IupGetCallback(ih, "VALUECHANGED_CB");
        if (cb)
          cb(ih);
      }
    }
    break;
  }
}

static void iScintillaCallCaretCb(Ihandle* ih)
{
  int col, lin, pos;

  IFniii cb = (IFniii)IupGetCallback(ih, "CARET_CB");
  if (!cb)
    return;

  iupStrToIntInt(iupScintillaGetCaretAttrib(ih), &lin, &col, ',');
  iupStrToInt(iupScintillaGetCaretPosAttrib(ih), &pos);

  if (pos != ih->data->last_caret_pos)
  {
    ih->data->last_caret_pos = pos;

    cb(ih, lin, col, pos);
  }
}

#ifdef GTK
static void gtkScintillaNotify(GtkWidget *w, gint wp, gpointer lp, Ihandle *ih)
{
  struct SCNotification *pMsg =(struct SCNotification *)lp;

  iScintillaNotify(ih, pMsg);

  (void)w;
  (void)wp;
}

static void gtkScintillaMoveCursor(GtkWidget *w, GtkMovementStep step, gint count, gboolean extend_selection, Ihandle* ih)
{
  iScintillaCallCaretCb(ih);
  (void)w;
  (void)step;
  (void)count;
  (void)extend_selection;
}

static gboolean gtkScintillaButtonEvent(GtkWidget *widget, GdkEventButton *evt, Ihandle *ih)
{
  gtkScintillaMoveCursor(NULL, (GtkMovementStep)0, 0, 0, ih);
  return iupgtkButtonEvent(widget, evt, ih);
}
#else
static int winScintillaWmNotify(Ihandle* ih, NMHDR* msg_info, int *result)
{
  SCNotification *pMsg = (SCNotification*)msg_info;

  iScintillaNotify(ih, pMsg);

  (void)result;
  return 0; /* result not used */
}

static int winScintillaProc(Ihandle* ih, UINT msg, WPARAM wp, LPARAM lp, LRESULT *result)
{
  switch (msg)
  {
  case WM_LBUTTONDBLCLK:
  case WM_MBUTTONDBLCLK:
  case WM_RBUTTONDBLCLK:
  case WM_LBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_RBUTTONDOWN:
    {
      if (iupwinButtonDown(ih, msg, wp, lp)==-1)
      {
        *result = 0;
        return 1;
      }
      PostMessage(ih->handle, WM_IUPCARET, 0, 0L);
      break;
    }
  case WM_MBUTTONUP:
  case WM_RBUTTONUP:
  case WM_LBUTTONUP:
    {
      if (iupwinButtonUp(ih, msg, wp, lp)==-1)
      {
        *result = 0;
        return 1;
      }
      PostMessage(ih->handle, WM_IUPCARET, 0, 0L);
      break;
    }
  case WM_IUPCARET:
    {
      iScintillaCallCaretCb(ih);
      break;
    }
  case WM_MOUSEMOVE:
    {
      iupwinMouseMove(ih, msg, wp, lp);
      break;
    }
  }

  return iupwinBaseProc(ih, msg, wp, lp, result);
}

static int winScintillaWmCommand(Ihandle* ih, WPARAM wp, LPARAM lp)
{
  int cmd = HIWORD(wp);
  switch (cmd)
  {
  case SCEN_CHANGE:
    {
      iupBaseCallValueChangedCb(ih);
      break;
    }
  }

  (void)lp;
  return 0; /* not used */
}
#endif

/*****************************************************************************/

static int iScintillaMapMethod(Ihandle* ih)
{
#ifdef GTK
  ih->handle = scintilla_new();
  if (!ih->handle)
    return IUP_ERROR;

  //TODO: why????
//  scintilla_set_id(SCINTILLA(ih->handle), 0);

  gtk_widget_show(ih->handle);

  /* add to the parent, all GTK controls must call this. */
  iupgtkAddToParent(ih);

  if (!iupAttribGetBoolean(ih, "CANFOCUS"))
    iupgtkSetCanFocus(ih->handle, 0);

  g_signal_connect(G_OBJECT(ih->handle), "enter-notify-event", G_CALLBACK(iupgtkEnterLeaveEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "leave-notify-event", G_CALLBACK(iupgtkEnterLeaveEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "focus-in-event",     G_CALLBACK(iupgtkFocusInOutEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "focus-out-event",    G_CALLBACK(iupgtkFocusInOutEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "show-help",          G_CALLBACK(iupgtkShowHelp), ih);

  g_signal_connect_after(G_OBJECT(ih->handle), "move-cursor", G_CALLBACK(gtkScintillaMoveCursor), ih);  /* only report some caret movements */
  g_signal_connect(G_OBJECT(ih->handle), "button-press-event", G_CALLBACK(gtkScintillaButtonEvent), ih);  /* if connected "after" then it is ignored */
  g_signal_connect(G_OBJECT(ih->handle), "button-release-event", G_CALLBACK(gtkScintillaButtonEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "motion-notify-event", G_CALLBACK(iupgtkMotionNotifyEvent), ih);

  g_signal_connect(ih->handle, "sci-notify", G_CALLBACK(gtkScintillaNotify), ih);

  gtk_widget_realize(ih->handle);
#else
  DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS;
  DWORD dwExStyle = 0;

  if (!ih->parent)
    return IUP_ERROR;

  if (iupAttribGetBoolean(ih, "CANFOCUS"))
    dwStyle |= WS_TABSTOP;

  if (iupAttribGetBoolean(ih, "BORDER"))
    dwExStyle |= WS_EX_CLIENTEDGE;
  
  if (!iupwinCreateWindowEx(ih, "Scintilla", dwExStyle, dwStyle))
    return IUP_ERROR;

  /* Process Scintilla Notifications */
  IupSetCallback(ih, "_IUPWIN_NOTIFY_CB", (Icallback)winScintillaWmNotify);

  /* Process ACTION_CB and CARET_CB */
  IupSetCallback(ih, "_IUPWIN_CTRLPROC_CB", (Icallback)winScintillaProc);

  /* Process WM_COMMAND */
  IupSetCallback(ih, "_IUPWIN_COMMAND_CB", (Icallback)winScintillaWmCommand);
#endif

  /* configure for DROP of files */
  if (IupGetCallback(ih, "DROPFILES_CB"))
    iupAttribSetStr(ih, "DROPFILESTARGET", "YES");

  /* add scrollbar */
  if (ih->data->sb & IUP_SB_HORIZ)
    iupScintillaSendMessage(ih, SCI_SETHSCROLLBAR, 1, 0);
  else
    iupScintillaSendMessage(ih, SCI_SETHSCROLLBAR, 0, 0);

  if (ih->data->sb & IUP_SB_VERT)
    iupScintillaSendMessage(ih, SCI_SETVSCROLLBAR, 1, 0);
  else
    iupScintillaSendMessage(ih, SCI_SETVSCROLLBAR, 0, 0);

  IupSetCallback(ih, "_IUP_XY2POS_CB", (Icallback)iScintillaConvertXYToPos);

  return IUP_NOERROR;
}

static void iScintillaComputeNaturalSizeMethod(Ihandle* ih, int *w, int *h, int *expand)
{
  int natural_w = 0, 
      natural_h = 0,
      visiblecolumns = iupAttribGetInt(ih, "VISIBLECOLUMNS"),
      visiblelines = iupAttribGetInt(ih, "VISIBLELINES");
  (void)expand; /* unset if not a container */

  iupdrvFontGetCharSize(ih, NULL, &natural_h);  /* one line height */
  natural_w = iupdrvFontGetStringWidth(ih, "WWWWWWWWWW");
  natural_w = (visiblecolumns*natural_w)/10;
  natural_h = visiblelines*natural_h;

  /* compute the borders space */
  if (iupAttribGetBoolean(ih, "BORDER"))
  {
#ifdef GTK
    int border_size = 2*5;
#else
    int border_size = 2*3;
#endif
    natural_w += border_size;
    natural_h += border_size;
  }

  /* compute scrollbar */
  if (ih->data->sb != IUP_SB_NONE)
  {
    int sb_size = iupdrvGetScrollbarSize();
    if (ih->data->sb & IUP_SB_HORIZ)
      natural_h += sb_size;  /* sb horizontal affects vertical size */
    if (ih->data->sb & IUP_SB_VERT)
      natural_w += sb_size;  /* sb vertical affects horizontal size */
  }
  
  *w = natural_w;
  *h = natural_h;
}

static int iScintillaCreateMethod(Ihandle* ih, void **params)
{
  (void)params;
  ih->data = iupALLOCCTRLDATA();
  ih->data->sb = IUP_SB_HORIZ | IUP_SB_VERT;
  ih->data->append_newline = 1;
  return IUP_NOERROR;
}

static void iScintillaReleaseMethod(Iclass* ic)
{
  (void)ic;
#ifndef GTK
  Scintilla_ReleaseResources();
#endif
}

static Iclass* iupScintillaNewClass(void)
{
  Iclass* ic = iupClassNew(NULL);

  ic->name = "scintilla";
  ic->format = NULL; /* no parameters */
  ic->nativetype = IUP_TYPECONTROL;
  ic->childtype  = IUP_CHILDNONE;
  ic->is_interactive = 1;
  ic->has_attrib_id  = 2;   /* has attributes with IDs that must be parsed */

  /* Class functions */
  ic->New     = iupScintillaNewClass;
  ic->Release = iScintillaReleaseMethod;
  ic->Create  = iScintillaCreateMethod;
  ic->Map     = iScintillaMapMethod;
  ic->ComputeNaturalSize = iScintillaComputeNaturalSizeMethod;
  ic->LayoutUpdate = iupdrvBaseLayoutUpdateMethod;

  /* Callbacks */
  iupClassRegisterCallback(ic, "MARGINCLICK_CB", "ii");
  iupClassRegisterCallback(ic, "DBLCLICK_CB", "ii");
  iupClassRegisterCallback(ic, "HOTSPOTCLICK_CB", "ii");
  iupClassRegisterCallback(ic, "BUTTON_CB", "iiiis");
  iupClassRegisterCallback(ic, "MOTION_CB", "iis");
  iupClassRegisterCallback(ic, "CARET_CB", "iii");
  iupClassRegisterCallback(ic, "VALUECHANGED_CB", "");
  iupClassRegisterCallback(ic, "ACTION", "is");

  /* Common Callbacks */
  iupBaseRegisterCommonCallbacks(ic);

  /* Common */
  iupBaseRegisterCommonAttrib(ic);

  /* Visual */
  iupBaseRegisterVisualAttrib(ic);

  /* Drag&Drop */
  iupdrvRegisterDragDropAttrib(ic);

  /* Text retrieval and modification */
  iupClassRegisterAttribute(ic, "APPENDNEWLINE", iScintillaGetAppendNewlineAttrib, iScintillaSetAppendNewlineAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "APPEND", NULL, iupScintillaSetAppendTextAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "PREPEND", NULL, iupScintillaSetPrependTextAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "VALUE", iupScintillaGetValueAttrib, iupScintillaSetValueAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "INSERT", NULL, iupScintillaSetInsertTextAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "LINE", iupScintillaGetLineAttrib, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "CHARAT", iupScintillaGetCharAtAttrib, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "DELETERANGE", NULL, iupScintillaSetDeleteRangeAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "READONLY", iupScintillaGetReadOnlyAttrib, iupScintillaSetReadOnlyAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "CLEARALL", NULL, iupScintillaSetClearAllAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "CLEARDOCUMENTSTYLE", NULL, iupScintillaSetClearDocumentAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  /* Selection and information */
  iupClassRegisterAttribute(ic, "CARET", iupScintillaGetCaretAttrib, iupScintillaSetCaretAttrib, NULL, NULL, IUPAF_NO_SAVE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CARETPOS", iupScintillaGetCaretPosAttrib, iupScintillaSetCaretPosAttrib, IUPAF_SAMEASSYSTEM, "0", IUPAF_NO_SAVE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "COUNT", iupScintillaGetCountAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "LINECOUNT", iupScintillaGetLineCountAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "LINEVALUE", iupScintillaGetCurrentLineAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SELECTEDTEXT", iupScintillaGetSelectedTextAttrib, iupScintillaSetSelectedTextAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SELECTION", iupScintillaGetSelectionAttrib, iupScintillaSetSelectionAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SELECTIONPOS", iupScintillaGetSelectionPosAttrib, iupScintillaSetSelectionPosAttrib, NULL, NULL, IUPAF_NO_INHERIT);

  /* Cut, Copy and Paste */
  iupClassRegisterAttribute(ic, "CLIPBOARD", NULL, iupScintillaSetClipboardAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  /* Overtype */
  iupClassRegisterAttribute(ic, "OVERWRITE", iupScintillaGetOvertypeAttrib, iupScintillaSetOvertypeAttrib, NULL, NULL, IUPAF_NO_INHERIT);

  /* Tabs and Indentation Guides */
  iupClassRegisterAttribute(ic, "TABSIZE", iupScintillaGetTabSizeAttrib, iupScintillaSetTabSizeAttrib, IUPAF_SAMEASSYSTEM, "8", IUPAF_DEFAULT);

  /* Line wrapping */
  iupClassRegisterAttribute(ic, "WORDWRAP", iupScintillaGetWordWrapAttrib, iupScintillaSetWordWrapAttrib, NULL, NULL, IUPAF_NO_INHERIT);

  /* Style Definition Attributes */
  iupClassRegisterAttribute(ic,   "STYLERESET", NULL, iupScintillaSetResetDefaultStyleAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "STYLECLEARALL", NULL, iupScintillaSetClearAllStyleAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLEFONT", iupScintillaGetFontStyleAttrib, iupScintillaSetFontStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLEFONTSIZE", iupScintillaGetFontSizeStyleAttrib, iupScintillaSetFontSizeStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLEFONTSIZEFRAC", iupScintillaGetFontSizeFracStyleAttrib, iupScintillaSetFontSizeFracStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLEBOLD", iupScintillaGetBoldStyleAttrib, iupScintillaSetBoldStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLEWEIGHT", iupScintillaGetWeightStyleAttrib, iupScintillaSetWeightStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLEITALIC", iupScintillaGetItalicStyleAttrib, iupScintillaSetItalicStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLEUNDERLINE", iupScintillaGetUnderlineStyleAttrib, iupScintillaSetUnderlineStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLEFGCOLOR", iupScintillaGetFgColorStyleAttrib, iupScintillaSetFgColorStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLEBGCOLOR", iupScintillaGetBgColorStyleAttrib, iupScintillaSetBgColorStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLEEOLFILLED", iupScintillaGetEolFilledStyleAttrib, iupScintillaSetEolFilledStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLECHARSET", iupScintillaGetCharSetStyleAttrib, iupScintillaSetCharSetStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLECASE", iupScintillaGetCaseStyleAttrib, iupScintillaSetCaseStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLEVISIBLE", iupScintillaGetVisibleStyleAttrib, iupScintillaSetVisibleStyleAttrib, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "STYLEHOTSPOT", iupScintillaGetHotSpotStyleAttrib, iupScintillaSetHotSpotStyleAttrib, IUPAF_NO_INHERIT);

  /* Lexer Attributes */
  iupClassRegisterAttribute(ic,   "LEXERLANGUAGE", iupScintillaGetLexerLanguageAttrib, iupScintillaSetLexerLanguageAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "PROPERTYNAME", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "PROPERTY", iupScintillaGetPropertyAttrib, iupScintillaSetPropertyAttrib, NULL, NULL, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "KEYWORDS", NULL, iupScintillaSetKeyWordsAttrib, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  /* Folding Attributes */
  iupClassRegisterAttribute(ic,   "FOLDFLAGS", NULL, iupScintillaSetFoldFlagsAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic,   "FOLDTOGGLE", NULL, iupScintillaSetFoldToggleAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "FOLDLEVEL", iupScintillaGetFoldLevelAttrib, iupScintillaSetFoldLevelAttrib, IUPAF_NO_INHERIT);

  /* Margin Attributes */
  iupClassRegisterAttributeId(ic, "MARGINTYPE", iupScintillaGetMarginTypeAttribId, iupScintillaSetMarginTypeAttribId, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "MARGINWIDTH", iupScintillaGetMarginWidthAttribId, iupScintillaSetMarginWidthAttribId, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "MARGINMASK", iupScintillaGetMarginMaskAttribId, iupScintillaSetMarginMaskAttribId, IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "MARGINSENSITIVE", iupScintillaGetMarginSensitiveAttribId, iupScintillaSetMarginSensitiveAttribId, IUPAF_NO_INHERIT);

  /* Marker Attributes */
  iupClassRegisterAttribute(ic, "MARKERDEFINE", NULL, iupScintillaSetMarkerDefineAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  /* Scrolling and automatic scrolling */
  iupClassRegisterAttribute(ic, "SCROLLBAR", iScintillaGetScrollbarAttrib, iScintillaSetScrollbarAttrib, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SCROLLTO", NULL, iupScintillaSetScrollToAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SCROLLTOPOS", NULL, iupScintillaSetScrollToPosAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  /* General */
  iupClassRegisterAttribute(ic, "VISIBLECOLUMNS", NULL, NULL, IUPAF_SAMEASSYSTEM, "30", IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "VISIBLELINES",   NULL, NULL, IUPAF_SAMEASSYSTEM, "10", IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "VALUEMASKED", NULL, iScintillaSetValueMaskedAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "MASKCASEI", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "MASK", iScintillaGetMaskAttrib, iScintillaSetMaskAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "MASKINT", NULL, iScintillaSetMaskIntAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "MASKFLOAT", NULL, iScintillaSetMaskFloatAttrib, NULL, NULL, IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "OLD_MASK_DATA", iScintillaGetMaskDataAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NOT_MAPPED|IUPAF_NO_INHERIT);

  iupClassRegisterAttribute(ic, "BORDER", NULL, NULL, IUPAF_SAMEASSYSTEM, "YES", IUPAF_NO_INHERIT);

  return ic;
}

void IupScintillaOpen(void)
{
  if (!IupGetGlobal("_IUP_SCINTILLA_OPEN"))
  {
    iupRegisterClass(iupScintillaNewClass());
    IupSetGlobal("_IUP_SCINTILLA_OPEN", "1");

#ifndef GTK
    Scintilla_RegisterClasses(IupGetGlobal("HINSTANCE"));
#endif
  }
}

Ihandle *IupScintilla(void)
{
  return IupCreate("scintilla");
}
