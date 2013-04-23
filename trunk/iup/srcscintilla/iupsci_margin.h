/** \file
 * \brief Scintilla control: Margin
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPSCI_MARGIN_H 
#define __IUPSCI_MARGIN_H

#ifdef __cplusplus
extern "C" {
#endif


char* iupScintillaGetMarginTypeAttribId(Ihandle* ih, int margin);
int iupScintillaSetMarginTypeAttribId(Ihandle* ih, int margin, const char* value);
char* iupScintillaGetMarginWidthAttribId(Ihandle* ih, int margin);
int iupScintillaSetMarginWidthAttribId(Ihandle* ih, int margin, const char* value);
char* iupScintillaGetMarginMaskFoldersAttribId(Ihandle* ih, int margin);
int iupScintillaSetMarginMaskFoldersAttribId(Ihandle* ih, int margin, const char* value);
char* iupScintillaGetMarginSensitiveAttribId(Ihandle* ih, int margin);
int iupScintillaSetMarginSensitiveAttribId(Ihandle* ih, int margin, const char* value);
char* iupScintillaGetMarginLeftAttrib(Ihandle* ih);
int iupScintillaSetMarginLeftAttrib(Ihandle* ih, const char* value);
char* iupScintillaGetMarginRightAttrib(Ihandle* ih);
int iupScintillaSetMarginRightAttrib(Ihandle* ih, const char* value);
char* iupScintillaGetMarginTextAttribId(Ihandle* ih, int line);
int iupScintillaSetMarginTextAttribId(Ihandle* ih, int line, const char* value);
int iupScintillaSetMarginTextClearAllAttrib(Ihandle* ih, const char* value);
char* iupScintillaGetMarginTextStyleAttribId(Ihandle* ih, int line);
int iupScintillaSetMarginTextStyleAttribId(Ihandle* ih, int line, const char* value);


#ifdef __cplusplus
}
#endif

#endif
