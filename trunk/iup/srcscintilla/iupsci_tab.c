/** \file
 * \brief Scintilla control: Tabs and Indentation Guides
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#undef SCI_NAMESPACE
#include <Scintilla.h>

#include "iup.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"

#include "iupsci_tab.h"
#include "iupsci.h"

/***** TABS AND INDENTATION GUIDES ****
SCI_SETTABWIDTH(int widthInChars)
SCI_GETTABWIDTH
SCI_SETUSETABS(bool useTabs)
SCI_GETUSETABS
--SCI_SETINDENT(int widthInChars)
--SCI_GETINDENT
--SCI_SETTABINDENTS(bool tabIndents)
--SCI_GETTABINDENTS
--SCI_SETBACKSPACEUNINDENTS(bool bsUnIndents)
--SCI_GETBACKSPACEUNINDENTS
--SCI_SETLINEINDENTATION(int line, int indentation)
--SCI_GETLINEINDENTATION(int line)
--SCI_GETLINEINDENTPOSITION(int line)
SCI_SETINDENTATIONGUIDES(int indentView)
SCI_GETINDENTATIONGUIDES
SCI_SETHIGHLIGHTGUIDE(int column)
SCI_GETHIGHLIGHTGUIDE
*/

char* iupScintillaGetTabSizeAttrib(Ihandle *ih)
{
  int widthInChars;
  widthInChars = iupScintillaSendMessage(ih, SCI_GETTABWIDTH, 0, 0);
  return iupStrReturnInt(widthInChars);
}

int iupScintillaSetTabSizeAttrib(Ihandle *ih, const char *value)
{
  int widthInChars;
  iupStrToInt(value, &widthInChars);

  if(widthInChars < 2)
    widthInChars = 2;

  iupScintillaSendMessage(ih, SCI_SETTABWIDTH, widthInChars, 0);

  return 0;
}

char* iupScintillaGetHighlightGuideAttrib(Ihandle *ih)
{
  int col = iupScintillaSendMessage(ih, SCI_GETHIGHLIGHTGUIDE, 0, 0);
  return iupStrReturnInt(col);
}

int iupScintillaSetHighlightGuideAttrib(Ihandle *ih, const char *value)
{
  int col;
  if (!iupStrToInt(value, &col))
    return 0;

  iupScintillaSendMessage(ih, SCI_SETHIGHLIGHTGUIDE, col, 0);

  return 0;
}

char* iupScintillaGetIndentationGuidesAttrib(Ihandle *ih)
{
  int indentView = iupScintillaSendMessage(ih, SCI_GETINDENTATIONGUIDES, 0, 0);
  char* str[] = {"NONE", "REAL", "LOOKFORWARD", "LOOKBOTH"};
  return str[indentView];
}

int iupScintillaSetIndentationGuidesAttrib(Ihandle *ih, const char *value)
{
  int indentView;

  if (iupStrEqualNoCase(value, "REAL"))
    indentView = SC_IV_REAL;
  else if (iupStrEqualNoCase(value, "LOOKFORWARD"))
    indentView = SC_IV_LOOKFORWARD;
  else if (iupStrEqualNoCase(value, "LOOKBOTH"))
    indentView = SC_IV_LOOKBOTH;
  else /* NONE */
    indentView = SC_IV_NONE;

  iupScintillaSendMessage(ih, SCI_SETINDENTATIONGUIDES, indentView, 0);

  return 0;
}

int iupScintillaSetUseTabsAttrib(Ihandle* ih, const char* value)
{
  iupScintillaSendMessage(ih, SCI_SETUSETABS, iupStrBoolean(value), 0);
  return 0;
}

char* iupScintillaGetUseTabsAttrib(Ihandle* ih)
{
  return iupStrReturnBoolean (iupScintillaSendMessage(ih, SCI_GETUSETABS, 0, 0)); 
}
