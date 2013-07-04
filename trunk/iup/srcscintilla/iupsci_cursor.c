/** \file
 * \brief Scintilla control: Cursor and Zooming
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

#include "iupsci_cursor.h"
#include "iupsci.h"

/***** CURSOR ****
SCI_SETCURSOR(int curType)
SCI_GETCURSOR
*/

char* iupScintillaGetCursorAttrib(Ihandle *ih)
{
  if(iupScintillaSendMessage(ih, SCI_GETCURSOR, 0, 0) == SC_CURSORWAIT)
    return "WAIT";
  else
    return "NORMAL";
}

int iupScintillaSetCursorAttrib(Ihandle *ih, const char *value)
{
  if (iupStrEqualNoCase(value, "WAIT"))
    iupScintillaSendMessage(ih, SCI_SETCURSOR, (uptr_t)SC_CURSORWAIT, 0);
  else  /* NORMAL */
    iupScintillaSendMessage(ih, SCI_SETCURSOR, (uptr_t)SC_CURSORNORMAL, 0);

  return 0;
}

/***** ZOOMING ****
SCI_ZOOMIN
SCI_ZOOMOUT
SCI_SETZOOM(int zoomInPoints)
SCI_GETZOOM
*/

int iupScintillaSetZoomInAttrib(Ihandle *ih, const char *value)
{
  (void)value;
  iupScintillaSendMessage(ih, SCI_ZOOMIN, 0, 0);
  return 0;
}

int iupScintillaSetZoomOutAttrib(Ihandle *ih, const char *value)
{
  (void)value;
  iupScintillaSendMessage(ih, SCI_ZOOMOUT, 0, 0);
  return 0;
}

int iupScintillaSetZoomAttrib(Ihandle *ih, const char *value)
{
  int points;
  if (!iupStrToInt(value, &points))
    return 0;

  if(points >  20) points =  20;
  if(points < -10) points = -10;

  iupScintillaSendMessage(ih, SCI_SETZOOM, points, 0);

  return 0;
}

char* iupScintillaGetZoomAttrib(Ihandle* ih)
{
  int points = iupScintillaSendMessage(ih, SCI_GETZOOM, 0, 0);
  return iupStrReturnInt(points);
}
