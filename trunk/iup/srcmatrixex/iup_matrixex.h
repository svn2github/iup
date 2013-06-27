/** \file
 * \brief IupMatrix Expansion Library.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUP_MATRIXEX_H 
#define __IUP_MATRIXEX_H 

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _ImatExData
{
  Ihandle* ih;  /* self reference */

  int busy, busy_count, busy_progress_abort;
  IFniis busy_cb;
  Ihandle* busy_progress;

  /* Temporary callbacks, valid only after iupMatrixExInitCellAccess */
  sIFnii value_cb;
  IFniis value_edit_cb;
  IFniiii edition_cb;
} ImatExData;

void iupMatrixExBusyStart(ImatExData* matex_data, int count, const char* busyname);
int iupMatrixExBusyInc(ImatExData* matex_data);
void iupMatrixExBusyEnd(ImatExData* matex_data);

int iupMatrixExIsColumnVisible(Ihandle* ih, int col);
int iupMatrixExIsLineVisible(Ihandle* ih, int lin);

void iupMatrixExInitCellAccess(ImatExData* matex_data);
char* iupMatrixExGetCell(ImatExData* matex_data, int lin, int col);
void iupMatrixExSetCell(ImatExData* matex_data, int lin, int col, const char* value);

void iupMatrixExRegisterClipboard(Iclass* ic);
void iupMatrixExRegisterBusy(Iclass* ic);
void iupMatrixExRegisterVisible(Iclass* ic);
void iupMatrixExRegisterExport(Iclass* ic);
void iupMatrixExRegisterCopy(Iclass* ic);


#ifdef __cplusplus
}
#endif

#endif
