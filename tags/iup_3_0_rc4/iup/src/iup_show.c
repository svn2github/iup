/** \file
 * \brief show/popup/hide/map
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdlib.h>
#include <stdarg.h>

#include "iup.h"

#include "iup_object.h"
#include "iup_layout.h"
#include "iup_attrib.h"
#include "iup_class.h"
#include "iup_dialog.h"
#include "iup_menu.h"
#include "iup_assert.h"
#include "iup_str.h"
#include "iup_drv.h"
#include "iup_drvfont.h"


void IupUnmap(Ihandle *ih)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  /* Invalid if it is NOT mapped. */
  if (!ih->handle)
    return;

  /* unmap children */
  {
    Ihandle* child = ih->firstchild;
    while (child)
    {
      IupUnmap(child);
      child = child->brother;
    }
  }

  /* only call UNMAP_CB for controls that have a native representation */
  if (ih->iclass->nativetype != IUP_TYPEVOID)
  {
    Icallback unmap_cb = IupGetCallback(ih, "UNMAP_CB");
    if (unmap_cb) unmap_cb(ih);
  }

  /* unmap from the native system */
  iupClassObjectUnMap(ih);
  ih->handle = NULL;
}

static char* iShowGetVisible(Ihandle* ih)
{
  char* value = iupAttribGet(ih, "VISIBLE");   /* Check on the element first */
  while (!value)
  {
    ih = ih->parent;   /* iheritance here independs on the attribute */
    if (!ih)
      return NULL;

    value = iupAttribGet(ih, "VISIBLE");

    /* only recursive up to the native parent */ 
    if (ih->iclass->nativetype != IUP_TYPEVOID)
      return value; /* can be NULL */
  }

  return value;
}

static void iShowUpdateVisible(Ihandle* ih)
{
  int inherit;
  /* although default is VISIBLE=YES, 
     when mapped the element is still hidden. 
     So we must manually update the visible state. */
  char* value = iShowGetVisible(ih);
  iupClassObjectSetAttribute(ih, "VISIBLE", value, &inherit);
}

int IupMap(Ihandle* ih)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return IUP_INVALID;

  /* calculate position and size for all children */
  if (ih->iclass->nativetype == IUP_TYPEDIALOG)
    iupLayoutCompute(ih);

  /* already mapped */
  if (ih->handle)
  {
    if (ih->iclass->nativetype == IUP_TYPEDIALOG)
      iupLayoutUpdate(ih);

    return IUP_NOERROR;
  }
    
  /* map to the native system */
  if (iupClassObjectMap(ih) == IUP_ERROR)
  {
    iupERROR("Error during IupMap.");
    return IUP_ERROR;
  }

  /* update FONT, must be the before several others */
  if (ih->iclass->nativetype != IUP_TYPEVOID &&
      ih->iclass->nativetype != IUP_TYPEIMAGE &&
      ih->iclass->nativetype != IUP_TYPEMENU)
    iupUpdateStandardFontAttrib(ih);

  /* ensure attributes default values, at this time only the ones that need to be set after map */
  iupClassObjectEnsureDefaultAttributes(ih);

  /* check visible state if not a dialog */
  if (ih->iclass->nativetype == IUP_TYPECANVAS || 
      ih->iclass->nativetype == IUP_TYPECONTROL)
    iShowUpdateVisible(ih);

  /* updates the defined attributes in the native system. */
  iupAttribUpdate(ih); 

  /* updates attributes defined in the parent tree */
  iupAttribUpdateFromParent(ih);

  /* map children */
  {
    Ihandle* child = ih->firstchild;
    while (child)
    {
      if (IupMap(child) == IUP_ERROR)
        return IUP_ERROR;

      child = child->brother;
    }
  }

  /* moves and resizes the elements to reflect the layout computation */
  /* if the dialog is visible will be reflected in the user interface */
  if (ih->iclass->nativetype == IUP_TYPEDIALOG)
    iupLayoutUpdate(ih);

  /* only call MAP_CB for controls that have a native representation */
  if (ih->iclass->nativetype != IUP_TYPEVOID)
  {
    Icallback map_cb = IupGetCallback(ih, "MAP_CB");
    if (map_cb) map_cb(ih);
  }

  return IUP_NOERROR;
}

int IupPopup(Ihandle *ih, int x, int y)
{
  int ret;

  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return IUP_INVALID;

  if (ih->iclass->nativetype != IUP_TYPEDIALOG && 
      ih->iclass->nativetype != IUP_TYPEMENU)
  {
    iupERROR("Must be a menu or dialog in IupPopup.");
    return IUP_INVALID;
  }

  ret = IupMap(ih);
  if (ret == IUP_ERROR) 
    return ret;

  if (ih->iclass->nativetype == IUP_TYPEDIALOG)
    ret = iupDialogPopup(ih, x, y);
  else
    ret = iupMenuPopup(ih, x, y);

  if (ret != IUP_NOERROR) 
  {
    iupERROR("Error during IupPopup.");
    return ret;
  }

  return IUP_NOERROR;
}

int IupShowXY(Ihandle *ih, int x, int y)
{
  int ret;

  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return IUP_INVALID;

  if (ih->iclass->nativetype != IUP_TYPEDIALOG)
  {
    iupERROR("Must be a dialog in IupShowXY.");
    return IUP_INVALID;
  }

  ret = IupMap(ih);
  if (ret == IUP_ERROR) 
    return ret;

  ret = iupDialogShowXY(ih, x, y);
  if (ret != IUP_NOERROR) 
  {
    iupERROR("Error during IupShowXY.");
    return ret;
  }

  return IUP_NOERROR;
}

int IupShow(Ihandle* ih)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return IUP_INVALID;

  if (ih->iclass->nativetype != IUP_TYPEDIALOG)
    IupSetAttribute(ih, "VISIBLE", "YES");
  else   
  {
    int ret = IupMap(ih);
    if (ret == IUP_ERROR) 
      return ret;
      
    ret = iupDialogShowXY(ih, IUP_CURRENT, IUP_CURRENT);
    if (ret != IUP_NOERROR) 
    {
      iupERROR("Error during IupShow.");
      return ret;
    }
  }

  return IUP_NOERROR;
}

int IupHide(Ihandle* ih)
{
  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return IUP_INVALID; 

  if (ih->iclass->nativetype != IUP_TYPEDIALOG)
    IupSetAttribute(ih, "VISIBLE", "NO");
  else if (ih->handle)
    iupDialogHide(ih);

  return IUP_NOERROR;
}
