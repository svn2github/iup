/** \file
 * \brief IupMatrix Expansion Library.
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "iup.h"
#include "iupcbs.h"
#include "iupcontrols.h"
#include "iupmatrixex.h"

#include "iup_object.h"
#include "iup_childtree.h"
#include "iup_register.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_assert.h"
#include "iup_matrixex.h"


static int iMatrixExBusyProgressCancel_CB(Ihandle* ih)
{
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
  matex_data->busy_progress_abort = 1;
  return IUP_DEFAULT;
}

void iupMatrixExBusyStart(ImatExData* matex_data, int count, const char* busyname)
{
  /* can not start a new one if already busy */
  iupASSERT(matex_data->busy);
  if (matex_data->busy)
    return;

  matex_data->busy = 1;
  matex_data->busy_count = 0;

  matex_data->busy_cb = (IFniis)IupGetCallback(matex_data->ih, "BUSY_CB");
  if (matex_data->busy_cb)
    matex_data->busy_cb(matex_data->ih, 1, count, (char*)busyname);

  if (iupAttribGetBoolean(matex_data->ih, "BUSYPROGRESS"))
  {
    int x, y;

    if (!matex_data->busy_progress)
    {
      matex_data->busy_progress = IupProgressDlg();
      IupSetCallback(matex_data->busy_progress, "CANCEL_CB", iMatrixExBusyProgressCancel_CB);
      IupSetAttributeHandle(matex_data->busy_progress, "PARENTDIALOG", IupGetDialog(matex_data->ih));
      IupSetAttribute(matex_data->busy_progress, "_IUP_MATEX_DATA", (char*)matex_data);

      IupMap(matex_data->busy_progress); /* to compute dialog size */
    }
  
    IupStoreAttribute(matex_data->busy_progress, "DESCRIPTION", busyname);
    IupSetfAttribute(matex_data->busy_progress, "TOTALCOUNT", "%d", count);
    IupSetAttribute(matex_data->busy_progress, "COUNT", "0");

    IupRefresh(matex_data->busy_progress);

    x = IupGetInt(matex_data->ih, "X") + (matex_data->ih->currentwidth-matex_data->busy_progress->currentwidth)/2;
    y = IupGetInt(matex_data->ih, "Y") + (matex_data->ih->currentheight-matex_data->busy_progress->currentheight)/2;
    IupShowXY(matex_data->busy_progress, x, y);

    matex_data->busy_progress_abort = 0;
    matex_data->busy = 2;
  }
}

int iupMatrixExBusyInc(ImatExData* matex_data)
{
  if (matex_data->busy)
  {
    matex_data->busy_count++;

    if (matex_data->busy_cb)
    {
      int ret = matex_data->busy_cb(matex_data->ih, 2, matex_data->busy_count, NULL);
      if (ret == IUP_IGNORE)
      {
        iupMatrixExBusyEnd(matex_data);
        return 0;
      }
    }

    if (matex_data->busy == 2)
    {
      IupSetAttribute(matex_data->busy_progress, "INC", NULL);

      if (matex_data->busy_progress_abort)
      {
        iupMatrixExBusyEnd(matex_data);
        return 0;
      }
    }
  }
  return 1;
}

void iupMatrixExBusyEnd(ImatExData* matex_data)
{
  if (matex_data->busy)
  {
    if (matex_data->busy_cb)
      matex_data->busy_cb(matex_data->ih, 0, 0, NULL);

    if (matex_data->busy == 2)
      IupHide(matex_data->busy_progress);

    matex_data->busy_count = 0;
    matex_data->busy_cb = NULL;
    matex_data->busy = 0;
  }
}

static int iMatrixSetBusyAttrib(Ihandle* ih, const char* value)
{
  /* can only be canceled */
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
  if (matex_data->busy && !iupStrBoolean(value))
    iupMatrixExBusyEnd(matex_data);
  return 0;
}

static char* iMatrixGetBusyAttrib(Ihandle* ih)
{
  ImatExData* matex_data = (ImatExData*)iupAttribGet(ih, "_IUP_MATEX_DATA");
  if (matex_data->busy)
    return "Yes";
  else
    return "No";
}

void iupMatrixExRegisterBusy(Iclass* ic)
{
  iupClassRegisterCallback(ic, "BUSY_CB", "iis");

  iupClassRegisterAttribute(ic, "BUSY", iMatrixGetBusyAttrib, iMatrixSetBusyAttrib, NULL, NULL, IUPAF_NO_SAVE|IUPAF_NO_DEFAULTVALUE|IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "BUSYPROGRESS", NULL, NULL, NULL, NULL, IUPAF_NO_INHERIT);
}

