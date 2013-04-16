/** \file
 * \brief Scintilla control: Line wrapping
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <Scintilla.h>

#include "iup.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"

#include "iupsci_wordwrap.h"
#include "iupsci.h"

/***** LINE WRAPPING ****
Attributes not implement yet:
SCI_SETWRAPVISUALFLAGS(int wrapVisualFlags)
SCI_GETWRAPVISUALFLAGS
SCI_SETWRAPVISUALFLAGSLOCATION(int wrapVisualFlagsLocation)
SCI_GETWRAPVISUALFLAGSLOCATION
SCI_SETWRAPINDENTMODE(int indentMode)
SCI_GETWRAPINDENTMODE
SCI_SETWRAPSTARTINDENT(int indent)
SCI_GETWRAPSTARTINDENT
SCI_SETLAYOUTCACHE(int cacheMode)
SCI_GETLAYOUTCACHE
SCI_SETPOSITIONCACHE(int size)
SCI_GETPOSITIONCACHE
SCI_LINESSPLIT(int pixelWidth)
SCI_LINESJOIN
SCI_WRAPCOUNT(int docLine)
*/

char* iupScintillaGetWordWrapAttrib(Ihandle *ih)
{
  int type = iupScintillaSendMessage(ih, SCI_GETWRAPMODE, 0, 0);

  if(type == SC_WRAP_WORD)
    return "WORD";
  else if(type == SC_WRAP_CHAR)
    return "CHAR";
  else
    return "NONE";
}

int iupScintillaSetWordWrapAttrib(Ihandle *ih, const char *value)
{
  if (iupStrEqualNoCase(value, "WORD"))
    iupScintillaSendMessage(ih, SCI_SETWRAPMODE, SC_WRAP_WORD, 0);
  else if (iupStrEqualNoCase(value, "CHAR"))
    iupScintillaSendMessage(ih, SCI_SETWRAPMODE, SC_WRAP_CHAR, 0);
  else
    iupScintillaSendMessage(ih, SCI_SETWRAPMODE, SC_WRAP_NONE, 0);

  return 0;
}
