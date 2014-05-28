/** \file
 * \brief GLLink Button Control
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iup.h"
#include "iupcbs.h"
#include "iupkey.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_register.h"

#include "iup_glcontrols.h"


static int iGLLinkButton_CB(Ihandle* ih, int button, int pressed, int x, int y, char* status)
{
  Ihandle* gl_parent = (Ihandle*)iupAttribGet(ih, "GL_CANVAS");
  iupGLSubCanvasRestoreState(gl_parent);

  if (button == IUP_BUTTON1 && pressed)
  {
    IFns cb = (IFns)IupGetCallback(ih, "ACTION");
    char* url = iupAttribGetStr(ih, "URL");
    if (cb)
    {
      int ret = cb(ih, url);
      if (ret == IUP_CLOSE) 
        IupExitLoop();
      else if (ret == IUP_DEFAULT && url)
        IupHelp(url);
    }
    else
      IupHelp(url);
  }

  (void)x;
  (void)y;
  (void)status;
  return IUP_DEFAULT;
}

static int iGLLinkEnterWindow_CB(Ihandle* ih)
{
  Ihandle* gl_parent = (Ihandle*)iupAttribGet(ih, "GL_CANVAS");
  IupSetAttribute(gl_parent, "CURSOR", "HAND");
  return IUP_DEFAULT;
}

static int iGLLinkLeaveWindow_CB(Ihandle* ih)
{
  Ihandle* gl_parent = (Ihandle*)iupAttribGet(ih, "GL_CANVAS");
  IupSetAttribute(gl_parent, "CURSOR", "ARROW");
  return IUP_DEFAULT;
}

static int iGLLinkCreateMethod(Ihandle* ih, void **params)
{
  if (params)
  {
    if (params[0]) iupAttribSetStr(ih, "URL", (char*)(params[0]));
    if (params[1]) iupAttribSetStr(ih, "TITLE", (char*)(params[1]));
  }

  IupSetCallback(ih, "GL_BUTTON_CB", (Icallback)iGLLinkButton_CB);
  IupSetCallback(ih, "GL_ENTERWINDOW_CB", iGLLinkEnterWindow_CB);
  IupSetCallback(ih, "GL_LEAVEWINDOW_CB", iGLLinkLeaveWindow_CB);

  IupSetAttribute(ih, "FONTSTYLE", "Underline");

  return IUP_NOERROR; 
}

Iclass* iupGLLinkNewClass(void)
{
  Iclass* ic = iupClassNew(iupRegisterFindClass("gllabel"));

  ic->name = "gllink";
  ic->format = "ss"; /* two strings */
  ic->nativetype = IUP_TYPEVOID;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 1;

  /* Class functions */
  ic->New = iupGLLinkNewClass;
  ic->Create = iGLLinkCreateMethod;

  /* Callbacks */
  iupClassRegisterCallback(ic, "ACTION", "s");

  /* attributes */
  iupClassRegisterAttribute(ic, "URL", NULL, NULL, NULL, NULL, IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CURSOR", NULL, iupdrvBaseSetCursorAttrib, IUPAF_SAMEASSYSTEM, "ARROW", IUPAF_IHANDLENAME|IUPAF_NO_INHERIT);

  iupClassRegisterReplaceAttribDef(ic, "FGCOLOR", "LINKFGCOLOR", NULL);

  return ic;
}

Ihandle* IupGLLink(const char *url, const char * title)
{
  void *params[3];
  params[0] = (void*)url;
  params[1] = (void*)title;
  params[2] = NULL;
  return IupCreatev("gllink", params);
}
