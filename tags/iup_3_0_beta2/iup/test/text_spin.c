#include <stdlib.h>
#include <stdio.h>

#include "iup.h"
#include "iupkey.h"


static int spin_cb(Ihandle* ih, int pos)
{
  (void)ih;
  printf("SPIN_CB(%d)\n", pos);
  if (!IupGetInt(ih, "SPINAUTO"))
    IupSetfAttribute(ih, "VALUE", "%s(%d)", "Test", pos);
  return IUP_DEFAULT;
}

static int action_cb(Ihandle* ih, int c, char* after)
{
  printf("ACTION(%d, %s)\n", c, after);
  return IUP_DEFAULT;
}

static int setspinvalue(Ihandle* ih)
{
  IupSetAttribute(IupGetDialogChild(ih, "spin"), "SPINVALUE", "25");
  return IUP_DEFAULT;
}

int main(int argc, char* argv[])
{
  Ihandle *dlg, *text;

  IupOpen(&argc, &argv);

  text = IupText(NULL);
  IupSetAttribute(text, "SIZE", "60x");
//  IupSetAttribute(text, "EXPAND", "HORIZONTAL");

  IupSetAttribute(text, "SPIN", "YES");
//  IupSetAttribute(text, "SPINMIN", "10");
//  IupSetAttribute(text, "SPINMAX", "60");
//  IupSetAttribute(text, "SPININC", "5");
//  IupSetAttribute(text, "SPINWRAP", "YES");
//  IupSetAttribute(text, "SPINALIGN", "LEFT");
//  IupSetAttribute(text, "SPINVALUE", "25");
//  IupSetAttribute(text, "SPINAUTO", "NO");
  IupSetAttribute(text, "NAME", "spin");

  IupSetCallback(text, "SPIN_CB", (Icallback)spin_cb);
  IupSetCallback(text, "ACTION", (Icallback)action_cb);

  dlg = IupDialog(IupVbox(text, IupButton("SPINVALUE", "setspinvalue"), NULL));
  IupSetAttribute(dlg, "GAP", "20");
  IupSetAttribute(dlg, "MARGIN", "20x20");
//  IupSetAttribute(dlg, "BGCOLOR", "173 177 194");  // Motif BGCOLOR for documentation

  IupSetFunction("setspinvalue", (Icallback)setspinvalue);

  IupSetAttribute(dlg, "TITLE", "Text Spin Test");
  IupShow(dlg);

  IupMainLoop();

  IupDestroy(dlg);

  IupClose();  
  return 0;
}
