/** \file
 * \brief list of all created dialogs
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUP_DLGLIST_H 
#define __IUP_DLGLIST_H

#ifdef __cplusplus
extern "C" {
#endif


/** \defgroup dlglist List of Dialogs
 * \par
 * See \ref iup_dlglist.h
 * \ingroup cpi */


/** Adds a dialog to the list. Used only in IupDialog.
 * \ingroup dlglist */
void iupDlgListAdd(Ihandle *ih);

/** Removes a dialog from the list. Used only in IupDestroy.
 * \ingroup dlglist */
void iupDlgListRemove(Ihandle *ih);

/** Starts a loop for all the created dialogs. 
 * \ingroup dlglist */
Ihandle* iupDlgListFirst(void);

/** Retrieve the next dialog on the list. Must call iupDlgListFirst first.
 * \ingroup dlglist */
Ihandle* iupDlgListNext(void);

/** Increments the number of visible dialogs. 
 * \ingroup dlglist */
void iupDlgListVisibleInc(void);

/** Decrements the number of visible dialogs.
 * \ingroup dlglist */
void iupDlgListVisibleDec(void);

/** Returns the number of visible dialogs.
 * \ingroup dlglist */
int iupDlgListVisibleCount(void);

/* Destroy all dialogs and the list.
   Called only from IupClose. */
void iupDlgListDestroyAll(void);


#ifdef __cplusplus
}
#endif

#endif
