/** \file
 * \brief iupmatrix control
 * attributes set and get.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPMAT_GETSET_H 
#define __IUPMAT_GETSET_H

#ifdef __cplusplus
extern "C" {
#endif

void  iupMatrixSetValue(Ihandle* ih, int lin, int col, const char* value, int edited);
char* iupMatrixGetValue(Ihandle* ih, int lin, int col);
void  iupMatrixCopyValue(Ihandle* ih, int lin1, int col1, int lin2, int col2);

double iupMatrixGetValueNumber(Ihandle* ih, int lin, int col);
char* iupMatrixGetValueText(Ihandle* ih, int lin, int col);
char* iupMatrixGetValueString(Ihandle* ih, int lin, int col);

void iupMatrixSetCellFlag(Ihandle* ih, int lin, int col, unsigned char attr, int set);
int iupMatrixGetCellOffset(Ihandle* ih, int lin, int col, int *x, int *y);

void  iupMatrixPrepareDrawData(Ihandle* ih);
char* iupMatrixGetFgColor(Ihandle* ih, int lin, int col);
char* iupMatrixGetBgColor(Ihandle* ih, int lin, int col);
char* iupMatrixGetFont   (Ihandle* ih, int lin, int col);
void iupMatrixGetBgRGB(Ihandle* ih, int lin, int col, unsigned char *r, unsigned char *g, unsigned char *b);
void iupMatrixGetFgRGB(Ihandle* ih, int lin, int col, unsigned char *r, unsigned char *g, unsigned char *b);

void iupMatrixCellUpdateValue(Ihandle* ih);

char* iupMatrixGetSize(Ihandle* ih, int index, int m, int pixels_unit);
void  iupMatrixGetVisibleCellDim(Ihandle* ih, int lin, int col, int* x, int* y, int* w, int* h);
int   iupMatrixGetColumnWidth(Ihandle* ih, int col, int use_value);
int   iupMatrixGetLineHeight (Ihandle* ih, int lin, int use_value);
int   iupMatrixGetCellFromXY(Ihandle* ih, int x, int y, int* l, int* c);

int iupMatrixCheckCellPos(Ihandle* ih, int lin, int col);

#ifdef __cplusplus
}
#endif

#endif
