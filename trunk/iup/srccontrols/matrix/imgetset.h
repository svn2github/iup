/** \file
 * \brief iupmatrix control
 * attributes set and get.
 *
 * See Copyright Notice in iup.h
 * $Id: imgetset.h,v 1.2 2008-11-03 20:03:28 scuri Exp $
 */
 
#ifndef __IMGETSET_H 
#define __IMGETSET_H

#ifdef __cplusplus
extern "C" {
#endif

void* iMatrixSetCell(Ihandle* ih, int lin, int col, const char* v);
char* iMatrixGetCell(Ihandle* ih, int lin, int col);

void  iMatrixSetFocusPosition (Ihandle* ih, const char* v, int call_cb);
char* iMatrixGetFocusPosition (Ihandle* ih);

void  iMatrixSetColAlign(Ihandle* ih, int col);

void  iMatrixSetTitleLine  (Ihandle* ih, int lin);
void  iMatrixSetTitleColumn(Ihandle* ih, int col);

void  iMatrixSetOrigin(Ihandle* ih, const char* value);
char* iMatrixGetOrigin(Ihandle* ih);

void  iMatrixSetRedraw(Ihandle* ih, const char* value);
void  iMatrixSetRedrawCell (Ihandle* ih, int lin, int col);

void  iMatrixSetActive(Ihandle* ih, int mode, int lincol, const char* val);

#ifdef __cplusplus
}
#endif

#endif
