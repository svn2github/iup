#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "iup.h"
#include "iupcbs.h"
#include "iup_scintilla.h"
#include "iup_config.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_strmessage.h"
#include "iup_register.h"
#include "iup_childtree.h"
#include "iup_predialogs.h"


#define LINENUMBER_MARGIN "50"
#define BOOKMARK_MARGIN "20"

/********************************** Utilities *****************************************/

static void saveMarkers(Ihandle* config, Ihandle* multitext)
{
  int lin = 0, i = 1;
  char mark[10240];
  Icallback cb;

  char* filename = IupGetAttribute(multitext, "FILENAME");

  while (lin >= 0)
  {
    IupSetAttributeId(multitext, "MARKERNEXT", lin, "1");  /* 0001 - marker=0 */
    lin = IupGetInt(multitext, "LASTMARKERFOUND");
    if (lin >= 0)
    {
      sprintf(mark, "%s#%d", filename, lin);
      IupConfigSetVariableStrId(config, "Bookmarks", "FileLine", i, mark);

      lin++;
      i++;
    }
  }

  IupConfigSetVariableStrId(config, "Bookmarks", "FileLine", i, NULL);

  cb = IupGetCallback(IupGetDialog(multitext), "SAVEMARKERS_CB");
  if (cb)
    cb(IupGetDialog(multitext));
}

static void restoreMarkers(Ihandle* config, Ihandle* multitext)
{
  const char* mark;
  int i = 1;
  char filename_str[10240];
  char line_str[10];
  Icallback cb;

  char* filename = IupGetAttribute(multitext, "FILENAME");

  IupSetInt(multitext, "MARKERDELETEALL", 0);

  do
  {
    mark = IupConfigGetVariableStrId(config, "Bookmarks", "FileLine", i);
    if (mark)
    {
      iupStrToStrStr(mark, filename_str, line_str, '#');
      if (iupStrEqual(filename, filename_str))
      {
        int lin;
        iupStrToInt(line_str, &lin);
        IupSetIntId(multitext, "MARKERADD", lin, 0);
      }
    }
    i++;
  } while (mark != NULL);

  cb = IupGetCallback(IupGetDialog(multitext), "RESTOREMARKERS_CB");
  if (cb)
    cb(IupGetDialog(multitext));
}

static void addBookmark(Ihandle* multitext, int lin)
{
  IupSetIntId(multitext, "MARKERADD", lin, 0);
}

static void removeBookmark(Ihandle* multitext, int lin)
{
  IupSetIntId(multitext, "MARKERDELETE", lin, 0);
}

static void removeAllBookmark(Ihandle* multitext)
{
  IupSetInt(multitext, "MARKERDELETEALL", 0);
}

static void toggleMarker(Ihandle* multitext, int lin, int margin)
{
  Ihandle* ih = IupGetDialog(multitext);
  IFnii cb;

  /* bookmarks */
  if (margin == 1)
  {
    unsigned int markerMask = (unsigned int)IupGetIntId(multitext, "MARKERGET", lin);
    int has_bookmark = markerMask & 0x0001; /* 0001 - marker=0 */

    if (has_bookmark)
      removeBookmark(multitext, lin);
    else
      addBookmark(multitext, lin);
  }

  cb = (IFnii)IupGetCallback(ih, "MARKERCHANGED_CB");
  if (cb)
    cb(ih, lin, margin);
}

static void copyMarkedLines(Ihandle *multitext)
{
  int size = IupGetInt(multitext, "COUNT");
  char *buffer = (char *)malloc(size);
  char *text;
  int lin = 0;

  buffer[0] = 0;
  while (lin >= 0)
  {
    IupSetAttributeId(multitext, "MARKERNEXT", lin, "1");  /* 0001 - marker=0 */
    lin = IupGetInt(multitext, "LASTMARKERFOUND");
    if (lin >= 0)
    {
      text = IupGetAttributeId(multitext, "LINE", lin);
      strcat(buffer, text);  size -= (int)strlen(text);
      lin++;
    }
  }

  if (strlen(buffer) > 0)
  {
    Ihandle *clipboard = IupClipboard();
    IupSetAttribute(clipboard, "TEXT", buffer);
    IupDestroy(clipboard);
  }

  free(buffer);
}

static int multitext_valuechanged_cb(Ihandle* multitext);

static void cutMarkedLines(Ihandle *multitext)
{
  int size = IupGetInt(multitext, "COUNT");
  char *buffer = (char *)malloc(size);
  char *text;
  int lin = 0, pos, len;

  buffer[0] = 0;
  while (lin >= 0 && size)
  {
    IupSetAttributeId(multitext, "MARKERNEXT", lin, "1");  /* 0001 - marker=0 */
    lin = IupGetInt(multitext, "LASTMARKERFOUND");
    if (lin >= 0)
    {
      text = IupGetAttributeId(multitext, "LINE", lin);
      len = (int)strlen(text);
      IupTextConvertLinColToPos(multitext, lin, 0, &pos);
      IupSetStrf(multitext, "DELETERANGE", "%d,%d", pos, len);
      strcat(buffer, text);  size -= len;
      removeBookmark(multitext, lin);
      lin--;
    }
  }

  if (strlen(buffer) > 0)
  {
    Ihandle *clipboard = IupClipboard();
    IupSetAttribute(clipboard, "TEXT", buffer);
    IupDestroy(clipboard);

    multitext_valuechanged_cb(multitext); /* INSERT, DELETERANGE do NOT triggers the callback */
  }

  free(buffer);
}

static void pasteToMarkedLines(Ihandle *multitext)
{
  char *text;
  int lin = 0, pos, len, changed = 0;
  Ihandle *clipboard;

  while (lin >= 0)
  {
    IupSetAttributeId(multitext, "MARKERNEXT", lin, "1");  /* 0001 - marker=0 */
    lin = IupGetInt(multitext, "LASTMARKERFOUND");
    if (lin >= 0)
    {
      changed = 1;
      text = IupGetAttributeId(multitext, "LINE", lin);
      len = (int)strlen(text);
      IupTextConvertLinColToPos(multitext, lin, 0, &pos);
      IupSetStrf(multitext, "DELETERANGE", "%d,%d", pos, len);
      removeBookmark(multitext, lin);
      clipboard = IupClipboard();
      IupSetAttributeId(multitext, "INSERT", pos, IupGetAttribute(clipboard, "TEXT"));
      IupDestroy(clipboard);
      lin--;
    }
  }

  if (changed)
    multitext_valuechanged_cb(multitext); /* INSERT, DELETERANGE do NOT triggers the callback */
}

static void invertMarkedLines(Ihandle *multitext)
{
  int lin;
  for (lin = 0; lin < IupGetInt(multitext, "LINECOUNT"); lin++)
  {
    toggleMarker(multitext, lin, 1);
  }
}

static void removeMarkedLines(Ihandle *multitext)
{
  char *text;
  int lin = 0, pos, len, changed = 0;

  while (lin >= 0)
  {
    IupSetAttributeId(multitext, "MARKERNEXT", lin, "1");  /* 0001 - marker=0 */
    lin = IupGetInt(multitext, "LASTMARKERFOUND");
    if (lin >= 0)
    {
      changed = 1;
      text = IupGetAttributeId(multitext, "LINE", lin);
      len = (int)strlen(text);
      IupTextConvertLinColToPos(multitext, lin, 0, &pos);
      IupSetStrf(multitext, "DELETERANGE", "%d,%d", pos, len);
      removeBookmark(multitext, lin);
      lin--;
    }
  }

  if (changed)
    multitext_valuechanged_cb(multitext); /* INSERT, DELETERANGE do NOT triggers the callback */
}

static void changeTabsToSpaces(Ihandle *multitext)
{
  char *text = IupGetAttribute(multitext, "VALUE");
  int count = IupGetInt(multitext, "COUNT");
  int tabSize = IupGetInt(multitext, "TABSIZE");
  int lin, col, i, j, changed = 0;

  for (i = count - 1; i >= 0; i--)
  {
    int spacesToNextTab;
    char c = text[i];

    if (c != '\t')
      continue;

    changed = 1;

    IupTextConvertPosToLinCol(multitext, i, &lin, &col);

    spacesToNextTab = tabSize - (col + 1) % tabSize + 1;

    IupSetStrf(multitext, "DELETERANGE", "%d,%d", i, 1);

    for (j = 0; j < spacesToNextTab; j++)
      IupSetAttributeId(multitext, "INSERT", i + j, " ");
  }

  if (changed)
    multitext_valuechanged_cb(multitext); /* INSERT, DELETERANGE do NOT triggers the callback */
}

static void changeSpacesToTabs(Ihandle *multitext)
{
  char *text = IupGetAttribute(multitext, "VALUE");
  int count = IupGetInt(multitext, "COUNT");
  int tabSize = IupGetInt(multitext, "TABSIZE");
  int lin, col, i, changed = 0;

  for (i = count - 1; i >= 0; i--)
  {
    int nSpaces, tabStop;
    char c = text[i];

    IupTextConvertPosToLinCol(multitext, i, &lin, &col);

    tabStop = (col + 1) % tabSize == tabSize - 1 ? 1 : 0;

    if (!tabStop || c != ' ')
      continue;

    changed = 1;

    IupSetStrf(multitext, "DELETERANGE", "%d,%d", i + 1, 1);
    IupSetAttributeId(multitext, "INSERT", i + 1, "\t");

    nSpaces = 0;

    while (text[i - nSpaces] == ' ' && nSpaces < tabSize - 1)
      nSpaces++;

    if (nSpaces == 0)
      continue;

    i -= nSpaces;

    IupSetStrf(multitext, "DELETERANGE", "%d,%d", i + 1, nSpaces);
  }

  if (changed)
    multitext_valuechanged_cb(multitext); /* INSERT, DELETERANGE do NOT triggers the callback */
}

static void changeLeadingSpacesToTabs(Ihandle *multitext)
{
  int lineCount = IupGetInt(multitext, "LINECOUNT");
  int tabSize = IupGetInt(multitext, "TABSIZE");
  int pos, i, j, changed = 0;

  for (i = 0; i < lineCount; i++)
  {
    int tabCount = 0;
    int spaceCount = 0;
    char *text = IupGetAttributeId(multitext, "LINE", i);

    int len = (int)strspn(text, " \t");
    if (len == 0)
      continue;

    changed = 1;

    for (j = 0; j < len; j++)
    {
      if (text[j] == '\t')
      {
        tabCount++;
        spaceCount = 0;
      }
      else
        spaceCount++;

      if (spaceCount == tabSize)
      {
        tabCount++;
        spaceCount = 0;
      }
    }
    IupTextConvertLinColToPos(multitext, i, 0, &pos);
    IupSetStrf(multitext, "DELETERANGE", "%d,%d", pos, len);
    for (j = 0; j < spaceCount; j++)
      IupSetAttributeId(multitext, "INSERT", pos, " ");
    for (j = 0; j < tabCount; j++)
      IupSetAttributeId(multitext, "INSERT", pos, "\t");
  }

  if (changed)
    multitext_valuechanged_cb(multitext); /* INSERT, DELETERANGE do NOT triggers the callback */
}

static void removeLeadingSpaces(Ihandle *multitext)
{
  int lineCount = IupGetInt(multitext, "LINECOUNT");
  int pos, i, changed = 0;

  for (i = 0; i < lineCount; i++)
  {
    char *text = IupGetAttributeId(multitext, "LINE", i);
    int len = (int)strspn(text, " \t");
    if (len == 0)
      continue;

    changed = 1;

    IupTextConvertLinColToPos(multitext, i, 0, &pos);
    IupSetStrf(multitext, "DELETERANGE", "%d,%d", pos, len);
  }

  if (changed)
    multitext_valuechanged_cb(multitext); /* INSERT, DELETERANGE do NOT triggers the callback */
}

static void removeTrailingSpaces(Ihandle *multitext)
{
  int lineCount = IupGetInt(multitext, "LINECOUNT");
  int pos, i, j, changed = 0;

  for (i = 0; i < lineCount; i++)
  {
    int count = 0;
    char *text = IupGetAttributeId(multitext, "LINE", i);
    int len = (int)strlen(text);
    if (len == 0)
      continue;

    if (text[len - 1] == '\n')
      len--;

    for (j = len - 1; j >= 0; j--)
    {
      if (text[j] != ' ' && text[j] != '\t')
        break;
      count++;
    }

    if (count == 0)
      continue;

    changed = 1;

    IupTextConvertLinColToPos(multitext, i, len - count, &pos);
    IupSetStrf(multitext, "DELETERANGE", "%d,%d", pos, count);
  }

  if (changed)
    multitext_valuechanged_cb(multitext); /* INSERT, DELETERANGE do NOT triggers the callback */
}

static void changeEolToSpace(Ihandle *multitext)
{
  char *c;
  int changed = 0;

  do
  {
    char *text = IupGetAttribute(multitext, "VALUE");
    c = strchr(text, '\n');
    if (c)
    {
      int pos = (int)(c - text);
      IupSetStrf(multitext, "DELETERANGE", "%d,%d", pos, 1);
      IupSetAttributeId(multitext, "INSERT", pos, " ");
      changed = 1;
    }
  } while (c);

  if (changed)
    multitext_valuechanged_cb(multitext); /* INSERT, DELETERANGE do NOT triggers the callback */
}

static const char* str_filetitle(const char *filename)
{
  /* Start at the last character */
  int len = (int)strlen(filename);
  int offset = len - 1;
  while (offset != 0)
  {
    if (filename[offset] == '\\' || filename[offset] == '/')
    {
      offset++;
      break;
    }
    offset--;
  }
  return filename + offset;
}

static char* read_file(const char* filename)
{
  int size;
  char* str;
  FILE* file = fopen(filename, "rb");
  if (!file)
    return NULL;

  /* calculate file size */
  fseek(file, 0, SEEK_END);
  size = ftell(file);
  fseek(file, 0, SEEK_SET);

  if (size == 0)
    return NULL;

  /* allocate memory for the file contents + nul terminator */
  str = malloc(size + 1);
  /* read all data at once */
  fread(str, size, 1, file);
  /* set the nul terminator */
  str[size] = 0;

  fclose(file);
  return str;
}

static int write_file(const char* filename, const char* str, int count)
{
  FILE* file = fopen(filename, "wb");
  if (!file)
    return 0;

  fwrite(str, 1, count, file);
  fclose(file);
  return 1;
}


/********************************** Interface Utilities *****************************************/

static void update_title(Ihandle* ih, const char* filename, int is_dirty)
{
  char* subtitle = IupGetAttribute(ih, "SUBTITLE");
  char* dirty_sign = "";

  if (is_dirty)
    dirty_sign = "*";

  if (!filename) filename = "Untitled";

  IupSetfAttribute(ih, "TITLE", "%s%s - %s", str_filetitle(filename), dirty_sign, subtitle);
}

static void new_file(Ihandle* ih_item)
{
  Ihandle* ih = IupGetDialog(ih_item);
  Ihandle* multitext = IupGetDialogChild(ih, "MULTITEXT");

  IupSetAttribute(multitext, "FILENAME", NULL);
  IupSetAttribute(multitext, "DIRTY", "NO");
  IupSetAttribute(multitext, "VALUE", "");

  update_title(ih, NULL, 0);
}

static void open_file(Ihandle* ih_item, const char* filename)
{
  char* str = read_file(filename);
  if (str)
  {
    Ihandle* ih = IupGetDialog(ih_item);
    Ihandle* multitext = IupGetDialogChild(ih, "MULTITEXT");
    Ihandle* config = IupGetAttributeHandle(multitext, "CONFIG");

    IupSetStrAttribute(multitext, "FILENAME", filename);
    IupSetAttribute(multitext, "DIRTY", "NO");
    IupSetStrAttribute(multitext, "VALUE", str);

    update_title(ih, filename, 0);

    if (config)
    {
      IupConfigRecentUpdate(config, filename);
      restoreMarkers(config, multitext);
    }

    free(str);
  }
  else
    IupMessageError(IupGetDialog(ih_item), "IUP_ERRORFILEOPEN");
}

static int item_saveas_action_cb(Ihandle* ih_item);

static void save_file(Ihandle* multitext)
{
  char* filename = IupGetAttribute(multitext, "FILENAME");
  if (!filename)
    item_saveas_action_cb(multitext);
  else
  {
    char* str = IupGetAttribute(multitext, "VALUE");
    int count = IupGetInt(multitext, "COUNT");
    if (write_file(filename, str, count))
    {
      Ihandle* config = IupGetAttributeHandle(multitext, "CONFIG");

      IupSetAttribute(multitext, "DIRTY", "NO");
      update_title(IupGetDialog(multitext), filename, 0);

      if (config)
        saveMarkers(config, multitext);
    }
    else
      IupMessageError(IupGetDialog(multitext), "IUP_ERRORFILESAVE");
  }
}

static void saveas_file(Ihandle* multitext, const char* filename)
{
  char* str = IupGetAttribute(multitext, "VALUE");
  int count = IupGetInt(multitext, "COUNT");
  if (write_file(filename, str, count))
  {
    Ihandle* config = IupGetAttributeHandle(multitext, "CONFIG");

    IupSetStrAttribute(multitext, "FILENAME", filename);
    IupSetAttribute(multitext, "DIRTY", "NO");
    update_title(IupGetDialog(multitext), filename, 0);

    if (config)
    {
      IupConfigRecentUpdate(config, filename);

      saveMarkers(config, multitext);
    }
  }
  else
    IupMessageError(IupGetDialog(multitext), "IUP_ERRORFILESAVE");
}

static int save_check(Ihandle* ih_item)
{
  Ihandle* multitext = IupGetDialogChild(ih_item, "MULTITEXT");
  if (IupGetInt(multitext, "DIRTY"))
  {
    switch (IupMessageAlarm(IupGetDialog(ih_item), "Attention!", "File not saved. Save it now?", "YESNOCANCEL"))
    {
    case 1:  /* save the changes and continue */
      save_file(multitext);
      break;
    case 2:  /* ignore the changes and continue */
      break;
    case 3:  /* cancel */
      return 0;
    }
  }
  return 1;
}

static void toggle_bar_visibility(Ihandle* item, Ihandle* item_toolbar)
{
  if (IupGetInt(item, "VALUE"))
  {
    IupSetAttribute(item_toolbar, "FLOATING", "YES");
    IupSetAttribute(item_toolbar, "VISIBLE", "NO");
    IupSetAttribute(item, "VALUE", "OFF");
  }
  else
  {
    IupSetAttribute(item_toolbar, "FLOATING", "NO");
    IupSetAttribute(item_toolbar, "VISIBLE", "YES");
    IupSetAttribute(item, "VALUE", "ON");
  }

  IupRefresh(item_toolbar);  /* refresh the dialog layout */
}

static void set_find_replace_visibility(Ihandle* find_dlg, int show_replace)
{
  Ihandle* replace_txt = IupGetDialogChild(find_dlg, "REPLACE_TEXT");
  Ihandle* replace_lbl = IupGetDialogChild(find_dlg, "REPLACE_LABEL");
  Ihandle* replace_bt = IupGetDialogChild(find_dlg, "REPLACE_BUTTON");
  Ihandle* replace_all_bt = IupGetDialogChild(find_dlg, "REPLACE_ALL_BUTTON");

  if (show_replace)
  {
    IupSetAttribute(replace_txt, "VISIBLE", "Yes");
    IupSetAttribute(replace_lbl, "VISIBLE", "Yes");
    IupSetAttribute(replace_bt, "VISIBLE", "Yes");
    IupSetAttribute(replace_all_bt, "VISIBLE", "Yes");
    IupSetAttribute(replace_txt, "FLOATING", "No");
    IupSetAttribute(replace_lbl, "FLOATING", "No");
    IupSetAttribute(replace_bt, "FLOATING", "No");
    IupSetAttribute(replace_all_bt, "FLOATING", "No");

    IupSetAttribute(find_dlg, "TITLE", "Replace");
  }
  else
  {
    IupSetAttribute(replace_txt, "FLOATING", "Yes");
    IupSetAttribute(replace_lbl, "FLOATING", "Yes");
    IupSetAttribute(replace_bt, "FLOATING", "Yes");
    IupSetAttribute(replace_all_bt, "FLOATING", "Yes");
    IupSetAttribute(replace_txt, "VISIBLE", "No");
    IupSetAttribute(replace_lbl, "VISIBLE", "No");
    IupSetAttribute(replace_bt, "VISIBLE", "No");
    IupSetAttribute(replace_all_bt, "VISIBLE", "No");

    IupSetAttribute(find_dlg, "TITLE", "Find");
  }

  IupSetAttribute(find_dlg, "SIZE", NULL);  /* force a dialog resize on the IupRefresh */
  IupRefresh(find_dlg);
}


/********************************** Callbacks *****************************************/


static int dropfiles_cb(Ihandle* multitext, const char* filename)
{
  if (save_check(multitext))
    open_file(multitext, filename);

  return IUP_DEFAULT;
}

static int multitext_marginclick_cb(Ihandle* multitext, int margin, int lin, char *status)
{
  (void)status;
  toggleMarker(multitext, lin, margin);
  return IUP_DEFAULT;
}

static int multitext_valuechanged_cb(Ihandle* multitext)
{
  int dirty = IupGetInt(multitext, "DIRTY");
  if (!dirty)
  {
    char* filename = IupGetAttribute(multitext, "FILENAME");
    IupSetAttribute(multitext, "DIRTY", "YES");

    update_title(IupGetDialog(multitext), filename, 1);
  }
  return IUP_DEFAULT;
}

static int file_menu_open_cb(Ihandle* ih_menu)
{
  Ihandle* item_revert = IupGetDialogChild(ih_menu, "ITEM_REVERT");
  Ihandle* item_save = IupGetDialogChild(ih_menu, "ITEM_SAVE");
  Ihandle* multitext = IupGetDialogChild(ih_menu, "MULTITEXT");
  char* filename = IupGetAttribute(multitext, "FILENAME");
  int dirty = IupGetInt(multitext, "DIRTY");

  if (dirty)
    IupSetAttribute(item_save, "ACTIVE", "YES");
  else
    IupSetAttribute(item_save, "ACTIVE", "NO");

  if (dirty && filename)
    IupSetAttribute(item_revert, "ACTIVE", "YES");
  else
    IupSetAttribute(item_revert, "ACTIVE", "NO");
  return IUP_DEFAULT;
}

static int edit_menu_open_cb(Ihandle* ih_menu)
{
  Ihandle* find_dlg = (Ihandle*)IupGetAttribute(ih_menu, "FIND_DIALOG");

  Ihandle *item_paste = IupGetDialogChild(ih_menu, "ITEM_PASTE");
  Ihandle *item_cut = IupGetDialogChild(ih_menu, "ITEM_CUT");
  Ihandle *item_delete = IupGetDialogChild(ih_menu, "ITEM_DELETE");
  Ihandle *item_copy = IupGetDialogChild(ih_menu, "ITEM_COPY");
  Ihandle *item_find_next = IupGetDialogChild(ih_menu, "ITEM_FINDNEXT");
  Ihandle* multitext = IupGetDialogChild(ih_menu, "MULTITEXT");

  if (!IupGetInt(multitext, "CLIPBOARD"))
    IupSetAttribute(item_paste, "ACTIVE", "NO");
  else
    IupSetAttribute(item_paste, "ACTIVE", "YES");

  if (!IupGetAttribute(multitext, "SELECTEDTEXT"))
  {
    IupSetAttribute(item_cut, "ACTIVE", "NO");
    IupSetAttribute(item_delete, "ACTIVE", "NO");
    IupSetAttribute(item_copy, "ACTIVE", "NO");
  }
  else
  {
    IupSetAttribute(item_cut, "ACTIVE", "YES");
    IupSetAttribute(item_delete, "ACTIVE", "YES");
    IupSetAttribute(item_copy, "ACTIVE", "YES");
  }

  if (find_dlg)
  {
    Ihandle* txt = IupGetDialogChild(find_dlg, "FIND_TEXT");
    char* str_to_find = IupGetAttribute(txt, "VALUE");

    if (!str_to_find || str_to_find[0] == 0)
      IupSetAttribute(item_find_next, "ACTIVE", "NO");
    else
      IupSetAttribute(item_find_next, "ACTIVE", "Yes");
  }
  else
    IupSetAttribute(item_find_next, "ACTIVE", "NO");

  return IUP_DEFAULT;
}

static int config_recent_cb(Ihandle* ih_item)
{
  if (save_check(ih_item))
  {
    char* filename = IupGetAttribute(ih_item, "TITLE");
    open_file(ih_item, filename);
  }
  return IUP_DEFAULT;
}

static int multitext_caret_cb(Ihandle* multitext, int lin, int col)
{
  Ihandle *lbl_statusbar = IupGetDialogChild(multitext, "STATUSBAR");
  IupSetfAttribute(lbl_statusbar, "TITLE", "Lin %d, Col %d", lin + 1, col + 1);  /* in Scintilla lin and col start at 0 */
  return IUP_DEFAULT;
}

static int item_new_action_cb(Ihandle* item_new)
{
  if (save_check(item_new))
    new_file(item_new);

  return IUP_IGNORE;  /* to avoid garbage in Scintilla when pressing the hot key */
}

static int item_open_action_cb(Ihandle* item_open)
{
  Ihandle* ih = IupGetDialog(item_open);
  Ihandle *filedlg;
  Ihandle* config;
  const char* dir = NULL;
  char* extra_filters = IupGetAttribute(ih, "EXTRAFILTERS");

  if (!save_check(item_open))
    return IUP_DEFAULT;

  config = IupGetAttributeHandle(item_open, "CONFIG");
  if (config)
    dir = IupConfigGetVariableStr(config, "MainWindow", "LastDirectory");

  filedlg = IupFileDlg();
  IupSetAttribute(filedlg, "DIALOGTYPE", "OPEN");
  if (extra_filters)
    IupSetStrf(filedlg, "EXTFILTER", "%sText Files|*.txt|All Files|*.*|", extra_filters);
  else
    IupSetAttribute(filedlg, "EXTFILTER", "Text Files|*.txt|All Files|*.*|");
  IupSetAttributeHandle(filedlg, "PARENTDIALOG", ih);
  IupSetStrAttribute(filedlg, "DIRECTORY", dir);

  IupPopup(filedlg, IUP_CENTERPARENT, IUP_CENTERPARENT);
  if (IupGetInt(filedlg, "STATUS") != -1)
  {
    char* filename = IupGetAttribute(filedlg, "VALUE");
    open_file(item_open, filename);

    if (config)
    {
      dir = IupGetAttribute(filedlg, "DIRECTORY");
      IupConfigSetVariableStr(config, "MainWindow", "LastDirectory", dir);
    }
  }

  IupDestroy(filedlg);
  return IUP_DEFAULT;
}

static int item_saveas_action_cb(Ihandle* item_saveas)
{
  Ihandle* ih = IupGetDialog(item_saveas);
  const char* dir = NULL;
  Ihandle* multitext = IupGetDialogChild(item_saveas, "MULTITEXT");
  Ihandle* config = IupGetAttributeHandle(multitext, "CONFIG");
  Ihandle *filedlg = IupFileDlg();
  char* extra_filters = IupGetAttribute(ih, "EXTRAFILTERS");

  if (config)
    dir = IupConfigGetVariableStr(config, "MainWindow", "LastDirectory");

  IupSetAttribute(filedlg, "DIALOGTYPE", "SAVE");
  if (extra_filters)
    IupSetStrf(filedlg, "EXTFILTER", "%sText Files|*.txt|All Files|*.*|", extra_filters);
  else
    IupSetAttribute(filedlg, "EXTFILTER", "Text Files|*.txt|All Files|*.*|");
  IupSetAttributeHandle(filedlg, "PARENTDIALOG", ih);
  IupSetStrAttribute(filedlg, "FILE", IupGetAttribute(multitext, "FILENAME"));
  IupSetStrAttribute(filedlg, "DIRECTORY", dir);

  IupPopup(filedlg, IUP_CENTERPARENT, IUP_CENTERPARENT);

  if (IupGetInt(filedlg, "STATUS") != -1)
  {
    char* filename = IupGetAttribute(filedlg, "VALUE");
    saveas_file(multitext, filename);

    if (config)
    {
      dir = IupGetAttribute(filedlg, "DIRECTORY");
      IupConfigSetVariableStr(config, "MainWindow", "LastDirectory", dir);
    }
  }

  IupDestroy(filedlg);
  return IUP_DEFAULT;
}

static int item_save_action_cb(Ihandle* item_save)
{
  Ihandle* multitext = IupGetDialogChild(item_save, "MULTITEXT");
  /* test again because in can be called using the hot key */
  if (IupGetInt(multitext, "DIRTY"))
    save_file(multitext);
  return IUP_IGNORE;  /* to avoid garbage in Scintilla when pressing the hot key */
}

static int item_revert_action_cb(Ihandle* item_revert)
{
  Ihandle* multitext = IupGetDialogChild(item_revert, "MULTITEXT");
  char* filename = IupGetAttribute(multitext, "FILENAME");
  open_file(item_revert, filename);
  return IUP_DEFAULT;
}

static int find_close_action_cb(Ihandle* bt_close);

static int close_exit_action_cb(Ihandle* ih_item)
{
  Ihandle* ih = IupGetDialog(ih_item);
  Ihandle* config = IupGetAttributeHandle(ih, "CONFIG");
  Ihandle* find_dlg = (Ihandle*)IupGetAttribute(ih_item, "FIND_DIALOG");
  Icallback cb;

  if (!save_check(ih))
    return IUP_IGNORE;  /* to abort the CLOSE_CB callback */
  
  if (find_dlg)
    find_close_action_cb(find_dlg);

  if (config)
  {
    Ihandle* multitext = IupGetDialogChild(ih, "MULTITEXT");
    
    saveMarkers(config, multitext);

    cb = IupGetCallback(ih, "CONFIGSAVE_CB");
    if (cb)
      cb(ih);

    IupConfigDialogClosed(config, ih, "MainWindow");
    IupConfigSave(config);
  }

  iupAttribSet(ih, "_IUP_CLOSING", "1");
  IupHide(ih);
  iupAttribSet(ih, "_IUP_CLOSING", NULL);

  /* after hide, at the last moment */
  cb = IupGetCallback(ih, "EXIT_CB");
  if (cb)
    cb(ih);

  return IUP_DEFAULT;
}

static int show_cb(Ihandle* ih, int state)
{
  if (state == IUP_HIDE && !iupAttribGet(ih, "_IUP_CLOSING"))
    close_exit_action_cb(ih);

  return IUP_DEFAULT;
}

static int goto_ok_action_cb(Ihandle* bt_ok)
{
  int line_count = IupGetInt(bt_ok, "TEXT_LINECOUNT");
  Ihandle* txt = IupGetDialogChild(bt_ok, "LINE_TEXT");
  int line = IupGetInt(txt, "VALUE");
  if (line < 1 || line > line_count)
  {
    IupMessageError(IupGetDialog(bt_ok), "Invalid line number.");
    return IUP_DEFAULT;
  }

  IupSetAttribute(IupGetDialog(bt_ok), "STATUS", "1");
  return IUP_CLOSE;
}

static int goto_cancel_action_cb(Ihandle* bt_ok)
{
  IupSetAttribute(IupGetDialog(bt_ok), "STATUS", "0");
  return IUP_CLOSE;
}

static int item_goto_action_cb(Ihandle* item_goto)
{
  Ihandle* multitext = IupGetDialogChild(item_goto, "MULTITEXT");
  Ihandle *goto_dlg, *box, *bt_ok, *bt_cancel, *txt, *lbl;
  int line_count = IupGetInt(multitext, "LINECOUNT");

  lbl = IupLabel(NULL);
  IupSetfAttribute(lbl, "TITLE", "Line Number [1-%d]:", line_count);
  txt = IupText(NULL);
  IupSetAttribute(txt, "MASK", IUP_MASK_UINT);  /* unsigned integer numbers only */
  IupSetAttribute(txt, "NAME", "LINE_TEXT");
  IupSetAttribute(txt, "VISIBLECOLUMNS", "20");
  bt_ok = IupButton("OK", NULL);
  IupSetInt(bt_ok, "TEXT_LINECOUNT", line_count);
  IupSetAttribute(bt_ok, "PADDING", "10x2");
  IupSetCallback(bt_ok, "ACTION", (Icallback)goto_ok_action_cb);
  bt_cancel = IupButton("Cancel", NULL);
  IupSetCallback(bt_cancel, "ACTION", (Icallback)goto_cancel_action_cb);
  IupSetAttribute(bt_cancel, "PADDING", "10x2");

  box = IupVbox(
    lbl,
    txt,
    IupSetAttributes(IupHbox(
    IupFill(),
    bt_ok,
    bt_cancel,
    NULL), "NORMALIZESIZE=HORIZONTAL"),
    NULL);
  IupSetAttribute(box, "MARGIN", "10x10");
  IupSetAttribute(box, "GAP", "5");

  goto_dlg = IupDialog(box);
  IupSetAttribute(goto_dlg, "TITLE", "Go To Line");
  IupSetAttribute(goto_dlg, "DIALOGFRAME", "Yes");
  IupSetAttributeHandle(goto_dlg, "DEFAULTENTER", bt_ok);
  IupSetAttributeHandle(goto_dlg, "DEFAULTESC", bt_cancel);
  IupSetAttributeHandle(goto_dlg, "PARENTDIALOG", IupGetDialog(item_goto));

  IupPopup(goto_dlg, IUP_CENTERPARENT, IUP_CENTERPARENT);

  if (IupGetInt(goto_dlg, "STATUS") == 1)
  {
    int line = IupGetInt(txt, "VALUE");
    int pos;
    IupTextConvertLinColToPos(multitext, line - 1, 0, &pos);  /* in Scintilla lin and col start at 0 */
    IupSetAttributeId(multitext, "ENSUREVISIBLE", line-1, NULL);
    IupSetInt(multitext, "CARETPOS", pos);
    IupSetInt(multitext, "SCROLLTOPOS", pos);
  }

  IupDestroy(goto_dlg);

  return IUP_DEFAULT;
}

static int item_gotombrace_action_cb(Ihandle* ih_item)
{
  Ihandle* multitext = IupGetDialogChild(ih_item, "MULTITEXT");

  int pos = IupGetInt(multitext, "CARETPOS");

  int newpos = IupGetIntId(multitext, "BRACEMATCH", pos);

  if (newpos != -1)
  {
    IupSetStrf(multitext, "BRACEHIGHLIGHT", "%d:%d", pos, newpos);

    IupSetInt(multitext, "CARETPOS", newpos);
    IupSetInt(multitext, "SCROLLTOPOS", newpos);
  }

  return IUP_IGNORE;
}

static int item_togglemark_action_cb(Ihandle* ih_item)
{
  Ihandle* multitext = IupGetDialogChild(ih_item, "MULTITEXT");

  int pos = IupGetInt(multitext, "CARETPOS");

  int lin, col;
  IupTextConvertPosToLinCol(multitext, pos, &lin, &col);

  toggleMarker(multitext, lin, 1);

  return IUP_IGNORE;
}

static int item_nextmark_action_cb(Ihandle* ih_item)
{
  Ihandle* multitext = IupGetDialogChild(ih_item, "MULTITEXT");

  int pos = IupGetInt(multitext, "CARETPOS");

  int lin, col;
  IupTextConvertPosToLinCol(multitext, pos, &lin, &col);

  IupSetAttributeId(multitext, "MARKERNEXT", lin + 1, "1");  /* 0001 - marker=0 */

  lin = IupGetInt(multitext, "LASTMARKERFOUND");

  if (lin == -1)
    return IUP_IGNORE;

  IupTextConvertLinColToPos(multitext, lin, 0, &pos);

  IupSetInt(multitext, "CARETPOS", pos);

  return IUP_DEFAULT;
}

static int item_previousmark_action_cb(Ihandle* ih_item)
{
  Ihandle* multitext = IupGetDialogChild(ih_item, "MULTITEXT");

  int pos = IupGetInt(multitext, "CARETPOS");

  int lin, col;
  IupTextConvertPosToLinCol(multitext, pos, &lin, &col);

  IupSetAttributeId(multitext, "MARKERPREVIOUS", lin - 1, "1");  /* 0001 - marker=0 */

  lin = IupGetInt(multitext, "LASTMARKERFOUND");

  if (lin == -1)
    return IUP_IGNORE;

  IupTextConvertLinColToPos(multitext, lin, 0, &pos);

  IupSetInt(multitext, "CARETPOS", pos);

  return IUP_DEFAULT;
}

static int item_clearmarks_action_cb(Ihandle* ih_item)
{
  Ihandle* multitext = IupGetDialogChild(ih_item, "MULTITEXT");
  removeAllBookmark(multitext);
  return IUP_DEFAULT;
}

static int item_copymarked_action_cb(Ihandle* ih_item)
{
  Ihandle* multitext = IupGetDialogChild(ih_item, "MULTITEXT");
  copyMarkedLines(multitext);
  return IUP_DEFAULT;
}

static int item_cutmarked_action_cb(Ihandle* ih_item)
{
  Ihandle* multitext = IupGetDialogChild(ih_item, "MULTITEXT");
  cutMarkedLines(multitext);
  return IUP_DEFAULT;
}

static int item_pastetomarked_action_cb(Ihandle* ih_item)
{
  Ihandle* multitext = IupGetDialogChild(ih_item, "MULTITEXT");
  pasteToMarkedLines(multitext);
  return IUP_DEFAULT;
}

static int item_removemarked_action_cb(Ihandle* ih_item)
{
  Ihandle* multitext = IupGetDialogChild(ih_item, "MULTITEXT");
  removeMarkedLines(multitext);
  return IUP_DEFAULT;
}

static int item_invertmarks_action_cb(Ihandle* ih_item)
{
  Ihandle* multitext = IupGetDialogChild(ih_item, "MULTITEXT");
  invertMarkedLines(multitext);
  return IUP_DEFAULT;
}

static int item_eoltospace_action_cb(Ihandle* ih_item)
{
  Ihandle* multitext = IupGetDialogChild(ih_item, "MULTITEXT");

  IupSetAttribute(multitext, "UNDOACTION", "BEGIN");

  changeEolToSpace(multitext);

  IupSetAttribute(multitext, "UNDOACTION", "END");

  return IUP_DEFAULT;
}

static int item_fixeol_action_cb(Ihandle* ih_item)
{
  Ihandle* multitext = IupGetDialogChild(ih_item, "MULTITEXT");
  IupSetAttribute(multitext, "FIXEOL", NULL);
  return IUP_DEFAULT;
}

static int item_removespaceeol_action_cb(Ihandle* ih_item)
{
  Ihandle* multitext = IupGetDialogChild(ih_item, "MULTITEXT");

  IupSetAttribute(multitext, "UNDOACTION", "BEGIN");

  removeTrailingSpaces(multitext);

  removeLeadingSpaces(multitext);

  changeEolToSpace(multitext);

  IupSetAttribute(multitext, "UNDOACTION", "END");

  return IUP_DEFAULT;
}

static int item_trimtrailing_action_cb(Ihandle* ih_item)
{
  Ihandle* multitext = IupGetDialogChild(ih_item, "MULTITEXT");

  IupSetAttribute(multitext, "UNDOACTION", "BEGIN");

  removeTrailingSpaces(multitext);

  IupSetAttribute(multitext, "UNDOACTION", "END");

  return IUP_DEFAULT;
}

static int item_trimleading_action_cb(Ihandle* ih_item)
{
  Ihandle* multitext = IupGetDialogChild(ih_item, "MULTITEXT");

  IupSetAttribute(multitext, "UNDOACTION", "BEGIN");

  removeLeadingSpaces(multitext);

  IupSetAttribute(multitext, "UNDOACTION", "END");

  return IUP_DEFAULT;
}

static int item_trimtraillead_action_cb(Ihandle* ih_item)
{
  Ihandle* multitext = IupGetDialogChild(ih_item, "MULTITEXT");

  IupSetAttribute(multitext, "UNDOACTION", "BEGIN");

  removeTrailingSpaces(multitext);

  removeLeadingSpaces(multitext);

  IupSetAttribute(multitext, "UNDOACTION", "END");

  return IUP_DEFAULT;
}

static int item_tabtospace_action_cb(Ihandle* ih_item)
{
  Ihandle* multitext = IupGetDialogChild(ih_item, "MULTITEXT");

  IupSetAttribute(multitext, "UNDOACTION", "BEGIN");

  changeTabsToSpaces(multitext);

  IupSetAttribute(multitext, "UNDOACTION", "END");

  return IUP_DEFAULT;
}

static int item_allspacetotab_action_cb(Ihandle* ih_item)
{
  Ihandle* multitext = IupGetDialogChild(ih_item, "MULTITEXT");

  IupSetAttribute(multitext, "UNDOACTION", "BEGIN");

  changeSpacesToTabs(multitext);

  IupSetAttribute(multitext, "UNDOACTION", "END");

  return IUP_DEFAULT;
}

static int item_leadingspacetotab_action_cb(Ihandle* ih_item)
{
  Ihandle* multitext = IupGetDialogChild(ih_item, "MULTITEXT");

  IupSetAttribute(multitext, "UNDOACTION", "BEGIN");

  changeLeadingSpacesToTabs(multitext);

  IupSetAttribute(multitext, "UNDOACTION", "END");

  return IUP_DEFAULT;
}

static int find_next_action_cb(Ihandle* ih_item)
{
  /* this callback can be called from the main dialog also */
  Ihandle* find_dlg = (Ihandle*)IupGetAttribute(ih_item, "FIND_DIALOG");
  if (find_dlg)
  {
    char* str_to_find;
    Ihandle* multitext = (Ihandle*)IupGetAttribute(find_dlg, "MULTITEXT");
    Ihandle* find_txt = IupGetDialogChild(find_dlg, "FIND_TEXT");

    /* test again, because it can be called from the hot key */
    str_to_find = IupGetAttribute(find_txt, "VALUE");
    if (str_to_find && str_to_find[0] != 0)
    {
      char flags[80];
      int find_start, find_end;
      int pos_start, pos_end;

      int wrap = IupGetInt(IupGetDialogChild(find_dlg, "WRAP"), "VALUE");
      int down = IupGetInt(IupGetDialogChild(find_dlg, "DOWN"), "VALUE");
      int casesensitive = IupGetInt(IupGetDialogChild(find_dlg, "FIND_CASE"), "VALUE");
      int whole_word = IupGetInt(IupGetDialogChild(find_dlg, "WHOLE_WORD"), "VALUE");
      int regexp = IupGetInt(IupGetDialogChild(find_dlg, "REG_EXP"), "VALUE");
      int posix = IupGetInt(IupGetDialogChild(find_dlg, "POSIX"), "VALUE");

      flags[0] = 0;
      if (casesensitive)
        strcpy(flags, "MATCHCASE");
      if (whole_word)
        strcat((flags[0] != 0 ? strcat(flags, " | ") : flags), "WHOLEWORD");
      if (regexp)
        strcat((flags[0] != 0 ? strcat(flags, " | ") : flags), "REGEXP");
      if (posix)
        strcat((flags[0] != 0 ? strcat(flags, " | ") : flags), "POSIX");

      if (flags[0] != 0)
        IupSetAttribute(multitext, "SEARCHFLAGS", flags);
      else
        IupSetAttribute(multitext, "SEARCHFLAGS", NULL);

      if (!down && IupGetAttribute(multitext, "SELECTIONPOS"))
      {
        int st, ed;
        IupGetIntInt(multitext, "SELECTIONPOS", &st, &ed);
        find_start = st;
      }
      else
        find_start = IupGetInt(multitext, "CARETPOS");

      find_end = down ? IupGetInt(multitext, "COUNT") : 0;

      IupSetInt(multitext, "TARGETSTART", find_start);
      IupSetInt(multitext, "TARGETEND", find_end);

      str_to_find = IupGetAttribute(find_txt, "VALUE");
      IupSetAttribute(multitext, "SEARCHINTARGET", str_to_find);

      pos_start = IupGetInt(multitext, "TARGETSTART");
      pos_end = IupGetInt(multitext, "TARGETEND");

      if (pos_start == find_start && pos_end == find_end && wrap)
      {
        /* if not found and wrap search again in the complementary region */
        find_end = find_start;
        find_start = down ? 0 : IupGetInt(multitext, "COUNT");

        IupSetInt(multitext, "TARGETSTART", find_start);
        IupSetInt(multitext, "TARGETEND", find_end);

        str_to_find = IupGetAttribute(find_txt, "VALUE");
        IupSetAttribute(multitext, "SEARCHINTARGET", str_to_find);

        pos_start = IupGetInt(multitext, "TARGETSTART");
        pos_end = IupGetInt(multitext, "TARGETEND");
      }

      if (pos_start != find_start || pos_end != find_end)
      {
        int lin, col;

        IupSetFocus(multitext);
        IupSetfAttribute(multitext, "SELECTIONPOS", "%d:%d", pos_start, pos_end);

        /* update statusbar */
        IupTextConvertPosToLinCol(multitext, pos_end, &lin, &col);
        multitext_caret_cb(multitext, lin, col);
      }
      else
      {
        /* update statusbar */
        Ihandle *lbl_statusbar = IupGetDialogChild(multitext, "STATUSBAR");
        IupSetfAttribute(lbl_statusbar, "TITLE", "Text \"%s\" not found.", str_to_find);
      }
    }
  }

  return IUP_DEFAULT;
}

static int find_replace_action_cb(Ihandle* bt_replace)
{
  Ihandle* find_dlg = (Ihandle*)IupGetAttribute(bt_replace, "FIND_DIALOG");
  Ihandle* multitext = (Ihandle*)IupGetAttribute(find_dlg, "MULTITEXT");
  char* selectionpos = IupGetAttribute(multitext, "SELECTIONPOS");

  if (!selectionpos)
    find_next_action_cb(bt_replace);
  else
  {
    Ihandle* replace_txt = IupGetDialogChild(find_dlg, "REPLACE_TEXT");
    char* str_to_replace = IupGetAttribute(replace_txt, "VALUE");
    if (IupGetAttribute(multitext, "SELECTIONPOS"))
    {
      int init, fim;
      IupGetIntInt(multitext, "SELECTIONPOS", &init, &fim);
      IupSetInt(multitext, "TARGETSTART", init);
      IupSetInt(multitext, "TARGETEND", fim);
      IupSetAttribute(multitext, "REPLACETARGET", str_to_replace);
    }

    /* then find next */
    find_next_action_cb(bt_replace);
  }

  return IUP_DEFAULT;
}

static int find_replace_all_action_cb(Ihandle* bt_replace)
{
  Ihandle* find_dlg = (Ihandle*)IupGetAttribute(bt_replace, "FIND_DIALOG");
  if (find_dlg)
  {
    char* str_to_find;
    char* str_to_replace;
    Ihandle* multitext = (Ihandle*)IupGetAttribute(find_dlg, "MULTITEXT");
    Ihandle* replace_txt = IupGetDialogChild(find_dlg, "REPLACE_TEXT");
    Ihandle* find_txt = IupGetDialogChild(find_dlg, "FIND_TEXT");
    
    /* test again, because it can be called from the hot key */
    str_to_find = IupGetAttribute(find_txt, "VALUE");
    if (str_to_find && str_to_find[0] != 0)
    {
      char flags[80];
      int find_start, find_end;
      int pos_start, pos_end;

      int casesensitive = IupGetInt(IupGetDialogChild(find_dlg, "FIND_CASE"), "VALUE");
      int whole_word = IupGetInt(IupGetDialogChild(find_dlg, "WHOLE_WORD"), "VALUE");
      int regexp = IupGetInt(IupGetDialogChild(find_dlg, "REG_EXP"), "VALUE");
      int posix = IupGetInt(IupGetDialogChild(find_dlg, "POSIX"), "VALUE");

      flags[0] = 0;
      if (casesensitive)
        strcpy(flags, "MATCHCASE");
      if (whole_word)
        strcat((flags[0] != 0 ? strcat(flags, " | ") : flags), "WHOLEWORD");
      if (regexp)
        strcat((flags[0] != 0 ? strcat(flags, " | ") : flags), "REGEXP");
      if (posix)
        strcat((flags[0] != 0 ? strcat(flags, " | ") : flags), "POSIX");

      if (flags[0] != 0)
        IupSetAttribute(multitext, "SEARCHFLAGS", flags);
      else
        IupSetAttribute(multitext, "SEARCHFLAGS", NULL);

      find_start = 0;
      find_end = IupGetInt(multitext, "COUNT");

      IupSetInt(multitext, "TARGETSTART", find_start);
      IupSetInt(multitext, "TARGETEND", find_end);

      str_to_find = IupGetAttribute(find_txt, "VALUE");
      IupSetAttribute(multitext, "SEARCHINTARGET", str_to_find);

      pos_start = IupGetInt(multitext, "TARGETSTART");
      pos_end = IupGetInt(multitext, "TARGETEND");

      while (find_start != pos_start || find_end != pos_end)
      {
        str_to_replace = IupGetAttribute(replace_txt, "VALUE");
        IupSetAttribute(multitext, "REPLACETARGET", str_to_replace);

        find_start = IupGetInt(multitext, "TARGETEND");
        find_end = IupGetInt(multitext, "COUNT");

        IupSetInt(multitext, "TARGETSTART", find_start);
        IupSetInt(multitext, "TARGETEND", find_end);

        str_to_find = IupGetAttribute(find_txt, "VALUE");
        IupSetAttribute(multitext, "SEARCHINTARGET", str_to_find);

        pos_start = IupGetInt(multitext, "TARGETSTART");
        pos_end = IupGetInt(multitext, "TARGETEND");
      }
    }
  }

  return IUP_DEFAULT;
}

static int find_close_action_cb(Ihandle* bt_close)
{
  Ihandle* find_dlg = IupGetDialog(bt_close);
  Ihandle* multitext = (Ihandle*)IupGetAttribute(find_dlg, "MULTITEXT");
  Ihandle* config = IupGetAttributeHandle(multitext, "CONFIG");
  
  if (config)
  {
    Ihandle*  find_text = IupGetDialogChild(find_dlg, "FIND_TEXT");
    Ihandle*  replace_text = IupGetDialogChild(find_dlg, "REPLACE_TEXT");
    Ihandle*  find_case = IupGetDialogChild(find_dlg, "FIND_CASE");
    Ihandle*  whole_word = IupGetDialogChild(find_dlg, "WHOLE_WORD");
    Ihandle*  wrap = IupGetDialogChild(find_dlg, "WRAP");
    Ihandle*  searchMode = IupGetDialogChild(find_dlg, "SEARCH_RADIO");
    Ihandle*  dirMode = IupGetDialogChild(find_dlg, "DIRECTION_RADIO");
    IupConfigSetVariableStr(config, "FindDialog", "FindText", IupGetAttribute(find_text, "VALUE"));
    IupConfigSetVariableStr(config, "FindDialog", "ReplaceText", IupGetAttribute(replace_text, "VALUE"));
    IupConfigSetVariableStr(config, "FindDialog", "FindCase", IupGetAttribute(find_case, "VALUE"));
    IupConfigSetVariableStr(config, "FindDialog", "WholeWord", IupGetAttribute(whole_word, "VALUE"));
    IupConfigSetVariableStr(config, "FindDialog", "Wrap", IupGetAttribute(wrap, "VALUE"));
    IupConfigSetVariableStr(config, "FindDialog", "SearchMode", IupGetAttribute((Ihandle*)IupGetAttribute(searchMode, "VALUE_HANDLE"), "NAME"));
    IupConfigSetVariableStr(config, "FindDialog", "SearchDir", IupGetAttribute((Ihandle*)IupGetAttribute(dirMode, "VALUE_HANDLE"), "NAME"));
    
    IupConfigDialogClosed(config, find_dlg, "FindDialog");
  }

  IupHide(find_dlg);  /* do not destroy, just hide */
  return IUP_DEFAULT;
}

static Ihandle* create_find_dialog(Ihandle *multitext)
{
  Ihandle *box, *bt_next, *bt_close, *txt, *find_dlg;
  Ihandle *find_case, *whole_word, *mode, *normal, *reg_exp, *posix, *wrap, *up, *down;
  Ihandle *flags, *direction, *searchRadio, *directionRadio;
  Ihandle *txt_replace, *bt_replace, *bt_replace_all;
  Ihandle* config = IupGetAttributeHandle(multitext, "CONFIG");

  txt = IupText(NULL);
  IupSetAttribute(txt, "NAME", "FIND_TEXT");
  IupSetAttribute(txt, "EXPAND", "HORIZONTAL");
  txt_replace = IupText(NULL);
  IupSetAttribute(txt_replace, "NAME", "REPLACE_TEXT");
  IupSetAttribute(txt_replace, "EXPAND", "HORIZONTAL");
  find_case = IupToggle("Match Case", NULL);
  IupSetAttribute(find_case, "NAME", "FIND_CASE");
  whole_word = IupToggle("Match Whole Word", NULL);
  IupSetAttribute(whole_word, "NAME", "WHOLE_WORD");
  normal = IupToggle("Normal", NULL);
  IupSetAttribute(normal, "NAME", "NORMAL");
  reg_exp = IupToggle("Reg. Expression", NULL);
  IupSetAttribute(reg_exp, "NAME", "REG_EXP");
  posix = IupToggle("Posix Reg. Expr.", NULL);
  IupSetAttribute(posix, "NAME", "POSIX");
  wrap = IupToggle("Wrap Around", NULL);
  IupSetAttribute(wrap, "NAME", "WRAP");
  up = IupToggle("Up", NULL);
  IupSetAttribute(up, "NAME", "UP");
  down = IupToggle("Down", NULL);
  IupSetAttribute(down, "NAME", "DOWN");
  bt_next = IupButton("Find Next", NULL);
  IupSetAttribute(bt_next, "PADDING", "10x2");
  IupSetCallback(bt_next, "ACTION", (Icallback)find_next_action_cb);
  bt_replace = IupButton("Replace", NULL);
  IupSetAttribute(bt_replace, "PADDING", "10x2");
  IupSetCallback(bt_replace, "ACTION", (Icallback)find_replace_action_cb);
  IupSetAttribute(bt_replace, "NAME", "REPLACE_BUTTON");
  bt_replace_all = IupButton("Replace All", NULL);
  IupSetAttribute(bt_replace_all, "PADDING", "10x2");
  IupSetCallback(bt_replace_all, "ACTION", (Icallback)find_replace_all_action_cb);
  IupSetAttribute(bt_replace_all, "NAME", "REPLACE_ALL_BUTTON");
  bt_close = IupButton("Close", NULL);
  IupSetCallback(bt_close, "ACTION", (Icallback)find_close_action_cb);
  IupSetAttribute(bt_close, "PADDING", "10x2");

  flags = IupVbox(find_case,
                  whole_word,
                  wrap,
                  NULL);

  searchRadio = IupRadio(IupVbox(normal, reg_exp, posix, NULL));
  IupSetAttribute(searchRadio, "MARGIN", "10x10");
  IupSetAttribute(searchRadio, "VALUE_HANDLE", (char*)normal);
  IupSetAttribute(searchRadio, "NAME", "SEARCH_RADIO");
  mode = IupFrame(searchRadio);
  IupSetAttribute(mode, "TITLE", "Search Mode");

  directionRadio = IupRadio(IupVbox(up, down, NULL));
  IupSetAttribute(directionRadio, "MARGIN", "10x10");
  IupSetAttribute(directionRadio, "VALUE_HANDLE", (char*)down);
  IupSetAttribute(directionRadio, "NAME", "DIRECTION_RADIO");
  direction = IupFrame(directionRadio);
  IupSetAttribute(direction, "TITLE", "Direction");

  box = IupVbox(
    IupLabel("Find What:"),
    txt,
    IupSetAttributes(IupLabel("Replace with:"), "NAME=REPLACE_LABEL"),
    txt_replace,
    IupHbox(
      flags,
      direction,
      mode,
      NULL),
    IupSetAttributes(IupLabel(NULL), "SEPARATOR=HORIZONTAL"),
    IupSetAttributes(IupHbox(
      IupFill(),
      bt_next,
      bt_replace,
      bt_replace_all,
      bt_close,
    NULL), "NORMALIZESIZE=HORIZONTAL"),
    NULL);
  IupSetAttribute(box, "NMARGIN", "10x10");
  IupSetAttribute(box, "GAP", "10");

  find_dlg = IupDialog(box);
  IupSetAttribute(find_dlg, "TITLE", "Find");
  IupSetAttribute(find_dlg, "DIALOGFRAME", "Yes");
  IupSetAttributeHandle(find_dlg, "DEFAULTENTER", bt_next);
  IupSetAttributeHandle(find_dlg, "DEFAULTESC", bt_close);
  IupSetAttributeHandle(find_dlg, "PARENTDIALOG", IupGetDialog(multitext));
  IupSetCallback(find_dlg, "CLOSE_CB", (Icallback)find_close_action_cb);

  /* Save the multiline to access it from the callbacks */
  IupSetAttribute(find_dlg, "MULTITEXT", (char*)multitext);

  IupSetInt(multitext, "TARGETSTART", IupGetInt(multitext, "CARETPOS"));
  IupSetInt(multitext, "TARGETEND", IupGetInt(multitext, "COUNT") - 1);

  /* Save the dialog to reuse it */
  IupSetAttribute(find_dlg, "FIND_DIALOG", (char*)find_dlg);  /* from itself */
  IupSetAttribute(IupGetDialog(multitext), "FIND_DIALOG", (char*)find_dlg); /* from the main dialog */

  IupMap(find_dlg);

  if (config)
  {
    const char* value = IupConfigGetVariableStr(config, "FindDialog", "FindText");
    if (value)
      IupSetStrAttribute(txt, "VALUE", value);

    value = IupConfigGetVariableStr(config, "FindDialog", "ReplaceText");
    if (value)
      IupSetStrAttribute(txt_replace, "VALUE", value);

    value = IupConfigGetVariableStr(config, "FindDialog", "FindCase");
    if (value)
      IupSetStrAttribute(find_case, "VALUE", value);

    value = IupConfigGetVariableStr(config, "FindDialog", "WholeWord");
    if (value)
      IupSetStrAttribute(whole_word, "VALUE", value);

    value = IupConfigGetVariableStr(config, "FindDialog", "Wrap");
    if (value)
      IupSetStrAttribute(wrap, "VALUE", value);

    value = IupConfigGetVariableStr(config, "FindDialog", "SearchMode");
    if (value)
    {
      Ihandle* search_mode = IupGetDialogChild(find_dlg, value);
      if (search_mode)
        IupSetAttribute(searchRadio, "VALUE_HANDLE", (char*)search_mode);
    }

    value = IupConfigGetVariableStr(config, "FindDialog", "SearchDir");
    if (value)
    {
      Ihandle* search_dir = IupGetDialogChild(find_dlg, value);
      if (search_dir)
        IupSetAttribute(directionRadio, "VALUE_HANDLE", (char*)search_dir);
    }
  }

  return find_dlg;
}

static int item_find_action_cb(Ihandle* item_find)
{
  Ihandle* find_dlg = (Ihandle*)IupGetAttribute(item_find, "FIND_DIALOG");
  Ihandle* multitext = IupGetDialogChild(item_find, "MULTITEXT");
  Ihandle* config = IupGetAttributeHandle(multitext, "CONFIG");
  char* str;

  if (!find_dlg)
    find_dlg = create_find_dialog(multitext);

  set_find_replace_visibility(find_dlg, 0);

  if (config)
    IupConfigDialogShow(config, find_dlg, "FindDialog");
  else
    IupShow(find_dlg);

  str = IupGetAttribute(multitext, "SELECTEDTEXT");
  if (str && str[0] != 0)
  {
    Ihandle* txt = IupGetDialogChild(find_dlg, "FIND_TEXT");
    IupSetStrAttribute(txt, "VALUE", str);
  }

  return IUP_DEFAULT;
}

static int item_replace_action_cb(Ihandle* item_replace)
{
  Ihandle* find_dlg = (Ihandle*)IupGetAttribute(item_replace, "FIND_DIALOG");
  Ihandle* multitext = IupGetDialogChild(item_replace, "MULTITEXT");
  Ihandle* config = IupGetAttributeHandle(multitext, "CONFIG");
  char* str;

  if (!find_dlg)
    find_dlg = create_find_dialog(multitext);

  set_find_replace_visibility(find_dlg, 1);

  if (config)
    IupConfigDialogShow(config, find_dlg, "FindDialog");
  else
    IupShow(find_dlg);

  str = IupGetAttribute(multitext, "SELECTEDTEXT");
  if (str && str[0] != 0)
  {
    Ihandle* txt = IupGetDialogChild(find_dlg, "FIND_TEXT");
    IupSetStrAttribute(txt, "VALUE", str);
  }

  return IUP_IGNORE;  /* replace system processing for the hot key */
}

static int selection_find_next_action_cb(Ihandle* ih)
{
  Ihandle* multitext = IupGetDialogChild(ih, "MULTITEXT");

  char* str = IupGetAttribute(multitext, "SELECTEDTEXT");
  if (str && str[0] != 0)
  {
    Ihandle* txt;
    Ihandle* find_dlg = (Ihandle*)IupGetAttribute(ih, "FIND_DIALOG");

    if (!find_dlg)
      find_dlg = create_find_dialog(multitext);

    txt = IupGetDialogChild(find_dlg, "FIND_TEXT");
    IupSetStrAttribute(txt, "VALUE", str);

    find_next_action_cb(ih);
  }

  return IUP_DEFAULT;
}

static int item_copy_action_cb(Ihandle* item_copy)
{
  Ihandle* multitext = IupGetDialogChild(item_copy, "MULTITEXT");
  IupSetAttribute(multitext, "CLIPBOARD", "COPY");
  return IUP_DEFAULT;
}

static int item_paste_action_cb(Ihandle* item_paste)
{
  Ihandle* multitext = IupGetDialogChild(item_paste, "MULTITEXT");
  IupSetAttribute(multitext, "CLIPBOARD", "PASTE");
  return IUP_IGNORE;  /* replace system processing for the hot key, to correctly parse line feed */
}

static int item_cut_action_cb(Ihandle* item_cut)
{
  Ihandle* multitext = IupGetDialogChild(item_cut, "MULTITEXT");
  IupSetAttribute(multitext, "CLIPBOARD", "CUT");
  return IUP_DEFAULT;
}

static int item_delete_action_cb(Ihandle* item_delete)
{
  Ihandle* multitext = IupGetDialogChild(item_delete, "MULTITEXT");
  IupSetAttribute(multitext, "CLIPBOARD", "CLEAR");
  return IUP_DEFAULT;
}

static int item_select_all_action_cb(Ihandle* item_select_all)
{
  int count;
  Ihandle* multitext = IupGetDialogChild(item_select_all, "MULTITEXT");
  IupSetFocus(multitext);
  count = IupGetInt(multitext, "COUNT");
  IupSetStrf(multitext, "SELECTIONPOS", "%d:%d", 0, count - 1);
  return IUP_DEFAULT;
}

static int item_undo_action_cb(Ihandle* item)
{
  Ihandle* multitext = IupGetDialogChild(item, "MULTITEXT");
  IupSetAttribute(multitext, "UNDO", "YES");
  return IUP_DEFAULT;
}

static int item_redo_action_cb(Ihandle* item)
{
  Ihandle* multitext = IupGetDialogChild(item, "MULTITEXT");
  IupSetAttribute(multitext, "REDO", "YES");
  return IUP_DEFAULT;
}

static int item_uppercase_action_cb(Ihandle* item)
{
  char *text;
  int start, end;
  Ihandle* multitext = IupGetDialogChild(item, "MULTITEXT");
  IupGetIntInt(multitext, "SELECTIONPOS", &start, &end);
  text = IupGetAttribute(multitext, "SELECTEDTEXT");
  text = iupStrDup(text);
  iupStrUpper(text, text);
  IupSetAttribute(multitext, "SELECTEDTEXT", text);
  IupSetStrf(multitext, "SELECTIONPOS", "%d:%d", start, end);
  free(text);
  return IUP_DEFAULT;
}

static int item_lowercase_action_cb(Ihandle* item)
{
  char *text;
  int start, end;
  Ihandle* multitext = IupGetDialogChild(item, "MULTITEXT");
  IupGetIntInt(multitext, "SELECTIONPOS", &start, &end);
  text = IupGetAttribute(multitext, "SELECTEDTEXT");
  text = iupStrDup(text);
  iupStrLower(text, text);
  IupSetAttribute(multitext, "SELECTEDTEXT", text);
  IupSetStrf(multitext, "SELECTIONPOS", "%d:%d", start, end);
  free(text);
  return IUP_DEFAULT;
}

static int item_case_action_cb(Ihandle* item)
{
  char* shift = IupGetGlobal("SHIFTKEY");

  if (strcmp(shift, "ON") == 0)
    item_uppercase_action_cb(item);
  else
    item_lowercase_action_cb(item);

  return IUP_DEFAULT;
}

static int item_font_action_cb(Ihandle* item_font)
{
  Ihandle* multitext = IupGetDialogChild(item_font, "MULTITEXT");
  Ihandle* fontdlg = IupFontDlg();
  char* font = IupGetAttribute(multitext, "FONT");
  IupSetStrAttribute(fontdlg, "VALUE", font);
  IupSetAttributeHandle(fontdlg, "PARENTDIALOG", IupGetDialog(item_font));

  IupPopup(fontdlg, IUP_CENTERPARENT, IUP_CENTERPARENT);

  if (IupGetInt(fontdlg, "STATUS") == 1)
  {
    Ihandle* config = IupGetAttributeHandle(multitext, "CONFIG");

    font = IupGetAttribute(fontdlg, "VALUE");
    IupSetStrAttribute(multitext, "FONT", font);

    if (config)
      IupConfigSetVariableStr(config, "Format", "Font", font);
  }

  IupDestroy(fontdlg);
  return IUP_DEFAULT;
}

static int param_cb(Ihandle* param_dialog, int param_index, void* user_data)
{
  if (param_index == IUP_GETPARAM_MAP)
  {
    Ihandle* ih = (Ihandle*)user_data;
    IupSetAttributeHandle(param_dialog, "PARENTDIALOG", ih);
  }

  return 1;
}

static int item_tab_action_cb(Ihandle* item)
{
  Ihandle* multitext = IupGetDialogChild(item, "MULTITEXT");

  int replaceBySpace = !IupGetInt(multitext, "USETABS");
  int tabSize = IupGetInt(multitext, "TABSIZE");

  if (IupGetParam("Tab Settings", param_cb, IupGetDialog(item),
                   "Size: %i\n"
                   "Replace by Whitespace: %b\n", 
                   &tabSize, &replaceBySpace))
  {
    Ihandle* config = IupGetAttributeHandle(multitext, "CONFIG");

    IupSetInt(multitext, "TABSIZE", tabSize);
    IupSetInt(multitext, "USETABS", !replaceBySpace);

    if (config)
    {
      IupConfigSetVariableInt(config, "Format", "TabSize", tabSize);
      IupConfigSetVariableInt(config, "Format", "UseTabs", !replaceBySpace);
    }
  }

  return IUP_DEFAULT;
}

static int item_zoomin_action_cb(Ihandle* item)
{
  Ihandle* multitext = IupGetDialogChild(item, "MULTITEXT");

  IupSetAttribute(multitext, "ZOOMIN", "10");

  return IUP_DEFAULT;
}

static int item_zoomout_action_cb(Ihandle* item)
{
  Ihandle* multitext = IupGetDialogChild(item, "MULTITEXT");

  IupSetAttribute(multitext, "ZOOMOUT", "10");

  return IUP_IGNORE;  /* to avoid garbage in Scintilla when pressing the hot key */
}

static int item_restorezoom_action_cb(Ihandle* item)
{
  Ihandle* multitext = IupGetDialogChild(item, "MULTITEXT");

  IupSetAttribute(multitext, "ZOOM", "0");

  return IUP_DEFAULT;
}

static int item_wordwrap_action_cb(Ihandle* item_wordwrap)
{
  Ihandle* multitext = IupGetDialogChild(item_wordwrap, "MULTITEXT");
  Ihandle* config = IupGetAttributeHandle(multitext, "CONFIG");
  char *value = IupGetAttribute(item_wordwrap, "VALUE");

  if (iupStrBoolean(value))
    IupSetAttribute(multitext, "WORDWRAP", "WORD");
  else
    IupSetAttribute(multitext, "WORDWRAP", "NONE");

  if (config)
    IupConfigSetVariableStr(config, "View", "WordWrap", value);

  return IUP_DEFAULT;
}

static int item_showwhite_action_cb(Ihandle* item_showwhite)
{
  Ihandle* multitext = IupGetDialogChild(item_showwhite, "MULTITEXT");
  Ihandle* config = IupGetAttributeHandle(multitext, "CONFIG");
  char *value = IupGetAttribute(item_showwhite, "VALUE");

  if (iupStrBoolean(value))
    IupSetAttribute(multitext, "WHITESPACEVIEW", "VISIBLEALWAYS");
  else
    IupSetAttribute(multitext, "WHITESPACEVIEW", "INVISIBLE");

  if (config)
    IupConfigSetVariableStr(config, "View", "ShowWhite", value);

  return IUP_DEFAULT;
}

static int item_showeol_action_cb(Ihandle* item_showeol)
{
  Ihandle* multitext = IupGetDialogChild(item_showeol, "MULTITEXT");
  Ihandle* config = IupGetAttributeHandle(multitext, "CONFIG");
  char *value = IupGetAttribute(item_showeol, "VALUE");

  if (iupStrBoolean(value))
    IupSetAttribute(multitext, "EOLVISIBLE", "YES");
  else
    IupSetAttribute(multitext, "EOLVISIBLE", "NO");

  if (config)
    IupConfigSetVariableStr(config, "View", "ShowEol", value);

  return IUP_DEFAULT;
}

static int item_toolbar_action_cb(Ihandle* item_toolbar)
{
  Ihandle* multitext = IupGetDialogChild(item_toolbar, "MULTITEXT");
  Ihandle* toolbar = IupGetChild(IupGetParent(multitext), 0);
  Ihandle* config = IupGetAttributeHandle(multitext, "CONFIG");

  toggle_bar_visibility(item_toolbar, toolbar);

  if (config)
    IupConfigSetVariableStr(config, "View", "Toolbar", IupGetAttribute(item_toolbar, "VALUE"));
  return IUP_DEFAULT;
}

static int item_statusbar_action_cb(Ihandle* item_statusbar)
{
  Ihandle* multitext = IupGetDialogChild(item_statusbar, "MULTITEXT");
  Ihandle* statusbar = IupGetBrother(multitext);
  Ihandle* config = IupGetAttributeHandle(multitext, "CONFIG");

  toggle_bar_visibility(item_statusbar, statusbar);

  if (config)
    IupConfigSetVariableStr(config, "View", "Statusbar", IupGetAttribute(item_statusbar, "VALUE"));
  return IUP_DEFAULT;
}

static int item_linenumber_action_cb(Ihandle* item_linenumber)
{
  Ihandle* multitext = IupGetDialogChild(item_linenumber, "MULTITEXT");
  Ihandle* config = IupGetAttributeHandle(multitext, "CONFIG");
  char *value = IupGetAttribute(item_linenumber, "VALUE");

  if (iupStrBoolean(value))
    IupSetAttribute(multitext, "MARGINWIDTH0", LINENUMBER_MARGIN);
  else
    IupSetAttribute(multitext, "MARGINWIDTH0", "0");

  if (config)
    IupConfigSetVariableStr(config, "View", "LineNumber", value);
  return IUP_DEFAULT;
}

static int item_bookmark_action_cb(Ihandle* item_bookmark)
{
  Ihandle* multitext = IupGetDialogChild(item_bookmark, "MULTITEXT");
  Ihandle* config = IupGetAttributeHandle(multitext, "CONFIG");
  char *value = IupGetAttribute(item_bookmark, "VALUE");

  if (iupStrBoolean(value))
    IupSetAttribute(multitext, "MARGINWIDTH1", BOOKMARK_MARGIN);
  else
    IupSetAttribute(multitext, "MARGINWIDTH1", "0");

  if (config)
    IupConfigSetVariableStr(config, "View", "Bookmark", value);
  return IUP_DEFAULT;
}



/********************************** Attributes *****************************************/

static int iScintillaDlgSetConfigAttrib(Ihandle* ih, const char* value)
{
  Ihandle* config = IupGetHandle(value);
  if (config)
  {
    Icallback cb = IupGetCallback(ih, "CONFIGLOAD_CB");
    Ihandle* recent_menu = (Ihandle*)iupAttribGet(ih, "_IUP_RECENTMENU");
    Ihandle* multitext = IupGetDialogChild(ih, "MULTITEXT");

    const char* value = IupConfigGetVariableStr(config, "Format", "Font");
    if (value)
      IupSetStrAttribute(multitext, "FONT", value);

    value = IupConfigGetVariableStr(config, "Format", "TabSize");
    if (value)
      IupSetStrAttribute(multitext, "TABSIZE", value);

    value = IupConfigGetVariableStr(config, "Format", "UseTabs");
    if (value)
      IupSetStrAttribute(multitext, "USETABS", value);
    
    value = IupConfigGetVariableStr(config, "View", "WordWrap");
    if (value)
    {
      Ihandle* item_wordwrap = IupGetDialogChild(ih, "ITEM_WORDWRAP");
      IupSetAttribute(item_wordwrap, "VALUE", value);
      if (iupStrBoolean(value))
        IupSetAttribute(multitext, "WORDWRAP", "WORD");
      else
        IupSetAttribute(multitext, "WORDWRAP", "NONE");
    }

    value = IupConfigGetVariableStr(config, "View", "ShowWhite");
    if (value)
    {
      Ihandle* item_showwhite = IupGetDialogChild(ih, "ITEM_SHOWWHITE");
      IupSetAttribute(item_showwhite, "VALUE", value);
      if (iupStrBoolean(value))
        IupSetAttribute(multitext, "WHITESPACEVIEW", "VISIBLEALWAYS");
      else
        IupSetAttribute(multitext, "WHITESPACEVIEW", "INVISIBLE");
    }

    value = IupConfigGetVariableStr(config, "View", "ShowEol");
    if (value)
    {
      Ihandle* item_showeol = IupGetDialogChild(ih, "ITEM_SHOWEOL");
      IupSetAttribute(item_showeol, "VALUE", value);
      if (iupStrBoolean(value))
        IupSetAttribute(multitext, "EOLVISIBLE", "YES");
      else
        IupSetAttribute(multitext, "EOLVISIBLE", "NO");
    }

    value = IupConfigGetVariableStr(config, "View", "Toolbar");
    if (value)
    {
      Ihandle* item_toolbar = IupGetDialogChild(ih, "ITEM_TOOLBAR");
      Ihandle* toolbar = IupGetChild(IupGetParent(multitext), 0);
      toggle_bar_visibility(item_toolbar, toolbar);
    }

    value = IupConfigGetVariableStr(config, "View", "Statusbar");
    if (value)
    {
      Ihandle* item_statusbar = IupGetDialogChild(ih, "ITEM_STATUSBAR");
      Ihandle* statusbar = IupGetBrother(multitext);
      toggle_bar_visibility(item_statusbar, statusbar);
    }

    value = IupConfigGetVariableStr(config, "View", "LineNumber");
    if (value)
    {
      Ihandle* item_linenumber = IupGetDialogChild(ih, "ITEM_LINENUMBER");
      IupSetAttribute(item_linenumber, "VALUE", value);
      if (iupStrBoolean(value))
        IupSetAttribute(multitext, "MARGINWIDTH0", LINENUMBER_MARGIN);
      else
        IupSetAttribute(multitext, "MARGINWIDTH0", "0");
    }

    value = IupConfigGetVariableStr(config, "View", "Bookmark");
    if (value)
    {
      Ihandle* item_bookmark = IupGetDialogChild(ih, "ITEM_BOOKMARK");
      IupSetAttribute(item_bookmark, "VALUE", value);
      if (iupStrBoolean(value))
        IupSetAttribute(multitext, "MARGINWIDTH1", BOOKMARK_MARGIN);
      else
        IupSetAttribute(multitext, "MARGINWIDTH1", "0");
    }

    IupConfigRecentInit(config, recent_menu, config_recent_cb, 10);

    if (cb)
      cb(ih);
  }

  return 1;
}

static int iScintillaDlgSetConfigHandleAttrib(Ihandle* ih, const char* value)
{
  Ihandle* config = (Ihandle*)value;
  if (!iupObjectCheck(config))
    return 0;

  /* only valid before map */
  if (ih->handle)
    return 0;

  IupSetAttributeHandle(ih, "CONFIG", config);
  return 1;
}

static int iScintillaDlgSetOpenFileAttrib(Ihandle* ih, const char* value)
{
  if (value)
    open_file(ih, value);
  else
    new_file(ih);
  return 0;
}

static int iScintillaDlgSetSaveFileAttrib(Ihandle* ih, const char* value)
{
  Ihandle* multitext = IupGetDialogChild(ih, "MULTITEXT");
  if (value)
    saveas_file(multitext, value);
  else
    save_file(multitext);
  return 0;
}

static int iScintillaDlgSetToggleMarkerAttribId(Ihandle* ih, int id, const char* value)
{
  int margin;
  if (iupStrToInt(value, &margin))
  {
    Ihandle* multitext = IupGetDialogChild(ih, "MULTITEXT");
    toggleMarker(multitext, id, margin);
  }
  return 0;
}


/********************************** Main *****************************************/


static int iScintillaDlgCreateMethod(Ihandle* ih, void** params)
{
  Ihandle *vbox, *multitext, *menu;
  Ihandle *sub_menu_file, *file_menu, *item_exit, *item_new, *item_open, *item_save, *item_saveas, *item_revert;
  Ihandle *sub_menu_edit, *edit_menu, *item_find, *item_find_next, *item_goto, *item_gotombrace, *item_copy, *item_paste, *item_cut, *item_delete, *item_select_all;
  Ihandle *item_togglemark, *item_nextmark, *item_previousmark, *item_clearmarks, *item_cutmarked, *item_copymarked, *item_pastetomarked, *item_removemarked,
    *item_invertmarks, *item_tabtospace, *item_allspacetotab, *item_leadingspacetotab;
  Ihandle *item_trimleading, *item_trimtrailing, *item_trimtraillead, *item_eoltospace, *item_fixeol, *item_removespaceeol;
  Ihandle *item_undo, *item_redo;
  Ihandle *case_menu, *item_uppercase, *item_lowercase;
  Ihandle *btn_cut, *btn_copy, *btn_paste, *btn_find, *btn_new, *btn_open, *btn_save;
  Ihandle *sub_menu_format, *format_menu, *item_font, *item_tab, *item_replace;
  Ihandle *sub_menu_view, *view_menu, *item_toolbar, *item_statusbar, *item_linenumber, *item_bookmark;
  Ihandle *zoom_menu, *item_zoomin, *item_zoomout, *item_restorezoom;
  Ihandle *lbl_statusbar, *toolbar_hb, *recent_menu;
  Ihandle *item_wordwrap, *item_showwhite, *item_showeol;

  multitext = IupScintilla();
  IupSetAttribute(multitext, "MULTILINE", "YES");
  IupSetAttribute(multitext, "EXPAND", "YES");
  IupSetAttribute(multitext, "NAME", "MULTITEXT");
  IupSetAttribute(multitext, "DIRTY", "NO");
  IupSetCallback(multitext, "CARET_CB", (Icallback)multitext_caret_cb);
  IupSetCallback(multitext, "VALUECHANGED_CB", (Icallback)multitext_valuechanged_cb);
  IupSetCallback(multitext, "DROPFILES_CB", (Icallback)dropfiles_cb);
  IupSetCallback(multitext, "MARGINCLICK_CB", (Icallback)multitext_marginclick_cb);

  /* highlight color for BRACEHIGHLIGHT */
  IupSetAttribute(multitext, "STYLEFGCOLOR34", "255 0 0");
  /* change the default for visibility of word warps */
  IupSetAttribute(multitext, "WORDWRAPVISUALFLAGS", "MARGIN");

  /* line numbers margin=0 */
  IupSetAttribute(multitext, "MARGINWIDTH0", LINENUMBER_MARGIN);
  IupSetAttribute(multitext, "MARGINSENSITIVE0", "YES");

  /* bookmarks margin=1 */
  IupSetAttribute(multitext, "MARGINWIDTH1", BOOKMARK_MARGIN);
  IupSetAttribute(multitext, "MARGINTYPE1", "SYMBOL");
  IupSetAttribute(multitext, "MARGINSENSITIVE1", "YES");
  IupSetAttribute(multitext, "MARGINMASKFOLDERS1", "NO"); /* (disable folding) */
  IupSetAttributeId(multitext, "MARGINMASK", 1, "1");  /* 0001 - marker=0 */

  /* bookmarks marker=0 */
  IupSetAttributeId(multitext, "MARKERFGCOLOR", 0, "0 0 255");
  IupSetAttributeId(multitext, "MARKERBGCOLOR", 0, "0 0 255");
  IupSetAttributeId(multitext, "MARKERALPHA", 0, "80");
  IupSetAttributeId(multitext, "MARKERSYMBOL", 0, "BOOKMARK");

  lbl_statusbar = IupLabel("Lin 1, Col 1");
  IupSetAttribute(lbl_statusbar, "NAME", "STATUSBAR");
  IupSetAttribute(lbl_statusbar, "EXPAND", "HORIZONTAL");
  IupSetAttribute(lbl_statusbar, "PADDING", "10x5");

  item_new = IupItem("&New\tCtrl+N", NULL);
  IupSetAttribute(item_new, "IMAGE", "IUP_FileNew");
  IupSetCallback(item_new, "ACTION", (Icallback)item_new_action_cb);
  btn_new = IupButton(NULL, NULL);
  IupSetAttribute(btn_new, "IMAGE", "IUP_FileNew");
  IupSetAttribute(btn_new, "FLAT", "Yes");
  IupSetCallback(btn_new, "ACTION", (Icallback)item_new_action_cb);
  IupSetAttribute(btn_new, "TIP", "New (Ctrl+N)");
  IupSetAttribute(btn_new, "CANFOCUS", "No");

  item_open = IupItem("&Open...\tCtrl+O", NULL);
  IupSetAttribute(item_open, "IMAGE", "IUP_FileOpen");
  IupSetCallback(item_open, "ACTION", (Icallback)item_open_action_cb);
  btn_open = IupButton(NULL, NULL);
  IupSetAttribute(btn_open, "IMAGE", "IUP_FileOpen");
  IupSetAttribute(btn_open, "FLAT", "Yes");
  IupSetCallback(btn_open, "ACTION", (Icallback)item_open_action_cb);
  IupSetAttribute(btn_open, "TIP", "Open (Ctrl+O)");
  IupSetAttribute(btn_open, "CANFOCUS", "No");

  item_save = IupItem("&Save\tCtrl+S", NULL);
  IupSetAttribute(item_save, "NAME", "ITEM_SAVE");
  IupSetAttribute(item_save, "IMAGE", "IUP_FileSave");
  IupSetCallback(item_save, "ACTION", (Icallback)item_save_action_cb);
  btn_save = IupButton(NULL, NULL);
  IupSetAttribute(btn_save, "IMAGE", "IUP_FileSave");
  IupSetAttribute(btn_save, "FLAT", "Yes");
  IupSetCallback(btn_save, "ACTION", (Icallback)item_save_action_cb);
  IupSetAttribute(btn_save, "TIP", "Save (Ctrl+S)");
  IupSetAttribute(btn_save, "CANFOCUS", "No");

  item_saveas = IupItem("Save &As...", NULL);
  IupSetAttribute(item_saveas, "NAME", "ITEM_SAVEAS");
  IupSetCallback(item_saveas, "ACTION", (Icallback)item_saveas_action_cb);

  item_revert = IupItem("&Revert", NULL);
  IupSetAttribute(item_revert, "NAME", "ITEM_REVERT");
  IupSetCallback(item_revert, "ACTION", (Icallback)item_revert_action_cb);

  item_exit = IupItem("E&xit", NULL);
  IupSetCallback(item_exit, "ACTION", (Icallback)close_exit_action_cb);

  item_find = IupItem("&Find...\tCtrl+F", NULL);
  IupSetAttribute(item_find, "IMAGE", "IUP_EditFind");
  IupSetCallback(item_find, "ACTION", (Icallback)item_find_action_cb);
  btn_find = IupButton(NULL, NULL);
  IupSetAttribute(btn_find, "IMAGE", "IUP_EditFind");
  IupSetAttribute(btn_find, "FLAT", "Yes");
  IupSetCallback(btn_find, "ACTION", (Icallback)item_find_action_cb);
  IupSetAttribute(btn_find, "TIP", "Find (Ctrl+F)");
  IupSetAttribute(btn_find, "CANFOCUS", "No");

  item_find_next = IupItem("Find &Next\tF3", NULL);
  IupSetAttribute(item_find_next, "NAME", "ITEM_FINDNEXT");
  IupSetCallback(item_find_next, "ACTION", (Icallback)find_next_action_cb);

  item_replace = IupItem("&Replace...\tCtrl+H", NULL);
  IupSetCallback(item_replace, "ACTION", (Icallback)item_replace_action_cb);

  item_cut = IupItem("Cu&t\tCtrl+X", NULL);
  IupSetAttribute(item_cut, "NAME", "ITEM_CUT");
  IupSetAttribute(item_cut, "IMAGE", "IUP_EditCut");
  IupSetCallback(item_cut, "ACTION", (Icallback)item_cut_action_cb);
  btn_cut = IupButton(NULL, NULL);
  IupSetAttribute(btn_cut, "IMAGE", "IUP_EditCut");
  IupSetAttribute(btn_cut, "FLAT", "Yes");
  IupSetCallback(btn_cut, "ACTION", (Icallback)item_cut_action_cb);
  IupSetAttribute(btn_cut, "TIP", "Cut (Ctrl+X)");
  IupSetAttribute(btn_cut, "CANFOCUS", "No");

  item_copy = IupItem("&Copy\tCtrl+C", NULL);
  IupSetAttribute(item_copy, "NAME", "ITEM_COPY");
  IupSetAttribute(item_copy, "IMAGE", "IUP_EditCopy");
  IupSetCallback(item_copy, "ACTION", (Icallback)item_copy_action_cb);
  btn_copy = IupButton(NULL, NULL);
  IupSetAttribute(btn_copy, "IMAGE", "IUP_EditCopy");
  IupSetAttribute(btn_copy, "FLAT", "Yes");
  IupSetCallback(btn_copy, "ACTION", (Icallback)item_copy_action_cb);
  IupSetAttribute(btn_copy, "TIP", "Copy (Ctrl+C)");
  IupSetAttribute(btn_copy, "CANFOCUS", "No");

  item_paste = IupItem("&Paste\tCtrl+V", NULL);
  IupSetAttribute(item_paste, "NAME", "ITEM_PASTE");
  IupSetAttribute(item_paste, "IMAGE", "IUP_EditPaste");
  IupSetCallback(item_paste, "ACTION", (Icallback)item_paste_action_cb);
  btn_paste = IupButton(NULL, NULL);
  IupSetAttribute(btn_paste, "IMAGE", "IUP_EditPaste");
  IupSetAttribute(btn_paste, "FLAT", "Yes");
  IupSetCallback(btn_paste, "ACTION", (Icallback)item_paste_action_cb);
  IupSetAttribute(btn_paste, "TIP", "Paste (Ctrl+V)");
  IupSetAttribute(btn_paste, "CANFOCUS", "No");

  item_delete = IupItem("&Delete\tDel", NULL);
  IupSetAttribute(item_delete, "IMAGE", "IUP_EditErase");
  IupSetAttribute(item_delete, "NAME", "ITEM_DELETE");
  IupSetCallback(item_delete, "ACTION", (Icallback)item_delete_action_cb);

  item_select_all = IupItem("Select &All\tCtrl+A", NULL);
  IupSetCallback(item_select_all, "ACTION", (Icallback)item_select_all_action_cb);

  item_undo = IupItem("Undo\tCtrl+Z", NULL);
  IupSetCallback(item_undo, "ACTION", (Icallback)item_undo_action_cb);

  item_redo = IupItem("Redo\tCtrl+Y", NULL);
  IupSetCallback(item_redo, "ACTION", (Icallback)item_redo_action_cb);

  item_uppercase = IupItem("UPPERCASE\tCtrl+Shift+U", NULL);
  IupSetCallback(item_uppercase, "ACTION", (Icallback)item_uppercase_action_cb);

  item_lowercase = IupItem("lowercase\tCtrl+U", NULL);
  IupSetCallback(item_lowercase, "ACTION", (Icallback)item_lowercase_action_cb);

  item_goto = IupItem("&Go To...\tCtrl+G", NULL);
  IupSetCallback(item_goto, "ACTION", (Icallback)item_goto_action_cb);

  item_gotombrace = IupItem("Go To Matching Brace\tCtrl+B", NULL);
  IupSetCallback(item_gotombrace, "ACTION", (Icallback)item_gotombrace_action_cb);

  item_togglemark = IupItem("Toggle Bookmark\tCtrl+F2", NULL);
  IupSetCallback(item_togglemark, "ACTION", (Icallback)item_togglemark_action_cb);

  item_nextmark = IupItem("Next Bookmark\tF2", NULL);
  IupSetCallback(item_nextmark, "ACTION", (Icallback)item_nextmark_action_cb);

  item_previousmark = IupItem("Previous Bookmark\tShift+F2", NULL);
  IupSetCallback(item_previousmark, "ACTION", (Icallback)item_previousmark_action_cb);

  item_clearmarks = IupItem("Clear All Bookmarks", NULL);
  IupSetCallback(item_clearmarks, "ACTION", (Icallback)item_clearmarks_action_cb);

  item_copymarked = IupItem("Copy Bookmarked Lines", NULL);
  IupSetCallback(item_copymarked, "ACTION", (Icallback)item_copymarked_action_cb);

  item_cutmarked = IupItem("Cut Bookmarked Lines", NULL);
  IupSetCallback(item_cutmarked, "ACTION", (Icallback)item_cutmarked_action_cb);

  item_pastetomarked = IupItem("Paste to (Replace) Bookmarked Lines", NULL);
  IupSetCallback(item_pastetomarked, "ACTION", (Icallback)item_pastetomarked_action_cb);

  item_removemarked = IupItem("Remove Bookmarked Lines", NULL);
  IupSetCallback(item_removemarked, "ACTION", (Icallback)item_removemarked_action_cb);

  item_invertmarks = IupItem("Invert Bookmarks", NULL);
  IupSetCallback(item_invertmarks, "ACTION", (Icallback)item_invertmarks_action_cb);

  item_trimtrailing = IupItem("Trim Trailing Space", NULL);
  IupSetCallback(item_trimtrailing, "ACTION", (Icallback)item_trimtrailing_action_cb);

  item_trimtraillead = IupItem("Trim Trailing and Leading Space", NULL);
  IupSetCallback(item_trimtraillead, "ACTION", (Icallback)item_trimtraillead_action_cb);

  item_eoltospace = IupItem("End of Lines to Spaces", NULL);
  IupSetCallback(item_eoltospace, "ACTION", (Icallback)item_eoltospace_action_cb);

  item_fixeol = IupItem("Fix End of Lines", NULL);
  IupSetCallback(item_fixeol, "ACTION", (Icallback)item_fixeol_action_cb);

  item_removespaceeol = IupItem("Remove Unnecessary Blanks and EOL", NULL);
  IupSetCallback(item_removespaceeol, "ACTION", (Icallback)item_removespaceeol_action_cb);

  item_trimleading = IupItem("Trim Leading Space", NULL);
  IupSetCallback(item_trimleading, "ACTION", (Icallback)item_trimleading_action_cb);

  item_tabtospace = IupItem("TABs to Spaces", NULL);
  IupSetCallback(item_tabtospace, "ACTION", (Icallback)item_tabtospace_action_cb);

  item_allspacetotab = IupItem("Spaces to TABs (All)", NULL);
  IupSetCallback(item_allspacetotab, "ACTION", (Icallback)item_allspacetotab_action_cb);

  item_leadingspacetotab = IupItem("Spaces to TABs (Leading)", NULL);
  IupSetCallback(item_leadingspacetotab, "ACTION", (Icallback)item_leadingspacetotab_action_cb);

  item_zoomin = IupItem("Zoom In\tCtrl+'+'", NULL);
  IupSetCallback(item_zoomin, "ACTION", (Icallback)item_zoomin_action_cb);

  item_zoomout = IupItem("Zoom Out\tCtrl+'-'", NULL);
  IupSetCallback(item_zoomout, "ACTION", (Icallback)item_zoomout_action_cb);

  item_restorezoom = IupItem("Reset Zoom\tCtrl+/", NULL);
  IupSetCallback(item_restorezoom, "ACTION", (Icallback)item_restorezoom_action_cb);

  item_wordwrap = IupItem("Word Wrap", NULL);
  IupSetCallback(item_wordwrap, "ACTION", (Icallback)item_wordwrap_action_cb);
  IupSetAttribute(item_wordwrap, "AUTOTOGGLE", "YES");
  IupSetAttribute(item_wordwrap, "NAME", "ITEM_WORDWRAP");

  item_showwhite = IupItem("Show White Spaces", NULL);
  IupSetCallback(item_showwhite, "ACTION", (Icallback)item_showwhite_action_cb);
  IupSetAttribute(item_showwhite, "AUTOTOGGLE", "YES");
  IupSetAttribute(item_showwhite, "NAME", "ITEM_SHOWWHITE");

  item_showeol = IupItem("Show End of Lines", NULL);
  IupSetCallback(item_showeol, "ACTION", (Icallback)item_showeol_action_cb);
  IupSetAttribute(item_showeol, "AUTOTOGGLE", "YES");
  IupSetAttribute(item_showeol, "NAME", "ITEM_SHOWEOL");

  item_toolbar = IupItem("&Toolbar", NULL);
  IupSetCallback(item_toolbar, "ACTION", (Icallback)item_toolbar_action_cb);
  IupSetAttribute(item_toolbar, "VALUE", "ON");
  IupSetAttribute(item_toolbar, "NAME", "ITEM_TOOLBAR");

  item_statusbar = IupItem("&Statusbar", NULL);
  IupSetCallback(item_statusbar, "ACTION", (Icallback)item_statusbar_action_cb);
  IupSetAttribute(item_statusbar, "VALUE", "ON");
  IupSetAttribute(item_statusbar, "NAME", "ITEM_STATUSBAR");

  item_linenumber = IupItem("Display Line Numbers", NULL);
  IupSetCallback(item_linenumber, "ACTION", (Icallback)item_linenumber_action_cb);
  IupSetAttribute(item_linenumber, "AUTOTOGGLE", "YES");
  IupSetAttribute(item_linenumber, "VALUE", "ON");
  IupSetAttribute(item_linenumber, "NAME", "ITEM_LINENUMBER");

  item_bookmark = IupItem("Display Bookmarks", NULL);
  IupSetCallback(item_bookmark, "ACTION", (Icallback)item_bookmark_action_cb);
  IupSetAttribute(item_bookmark, "AUTOTOGGLE", "YES");
  IupSetAttribute(item_bookmark, "VALUE", "ON");
  IupSetAttribute(item_bookmark, "NAME", "ITEM_BOOKMARK");

  item_font = IupItem("&Font...", NULL);
  IupSetCallback(item_font, "ACTION", (Icallback)item_font_action_cb);

  item_tab = IupItem("Tab...", NULL);
  IupSetCallback(item_tab, "ACTION", (Icallback)item_tab_action_cb);

  recent_menu = IupMenu(NULL);
  iupAttribSet(ih, "_IUP_RECENTMENU", (char*)recent_menu);

  file_menu = IupMenu(
    item_new,
    item_open,
    item_save,
    item_saveas,
    item_revert,
    IupSeparator(),
    IupSubmenu("Recent &Files", recent_menu),
    item_exit,
    NULL);
  edit_menu = IupMenu(
    item_undo,
    item_redo,
    IupSeparator(),
    item_cut,
    item_copy,
    item_paste,
    item_delete,
    IupSeparator(),
    item_find,
    item_find_next,
    item_replace,
    item_goto,
    item_gotombrace,
    IupSeparator(),
    IupSubmenu("Bookmarks", IupMenu(item_togglemark,
      item_nextmark,
      item_previousmark,
      item_clearmarks,
      item_cutmarked,
      item_copymarked,
      item_pastetomarked,
      item_removemarked,
      item_invertmarks,
      NULL)),
    IupSubmenu("Blank Operations", IupMenu(
      item_trimtrailing,
      item_trimleading,
      item_trimtraillead,
      item_eoltospace,
      item_fixeol,
      item_removespaceeol,
      IupSeparator(),
      item_tabtospace,
      item_allspacetotab,
      item_leadingspacetotab,
      NULL)),
    IupSubmenu("Convert Case to", case_menu = IupMenu(
      item_uppercase,
      item_lowercase,
      NULL)),
    IupSeparator(),
    item_select_all,
    NULL);
  format_menu = IupMenu(
    item_font,
    item_tab,
    NULL);
  view_menu = IupMenu(
    IupSubmenu("Zoom", zoom_menu = IupMenu(
    item_zoomin,
    item_zoomout,
    item_restorezoom,
    NULL)),
    item_wordwrap,
    item_showwhite,
    item_showeol,
    IupSeparator(),
    item_toolbar,
    item_statusbar,
    item_linenumber,
    item_bookmark,
    NULL);

  IupSetCallback(file_menu, "OPEN_CB", (Icallback)file_menu_open_cb);
  IupSetCallback(edit_menu, "OPEN_CB", (Icallback)edit_menu_open_cb);

  sub_menu_file = IupSubmenu("&File", file_menu);
  sub_menu_edit = IupSubmenu("&Edit", edit_menu);
  sub_menu_format = IupSubmenu("F&ormat", format_menu);
  sub_menu_view = IupSubmenu("&View", view_menu);

  menu = IupMenu(
    sub_menu_file,
    sub_menu_edit,
    sub_menu_format,
    sub_menu_view,
    NULL);

  toolbar_hb = IupHbox(
    btn_new,
    btn_open,
    btn_save,
    IupSetAttributes(IupLabel(NULL), "SEPARATOR=VERTICAL"),
    btn_cut,
    btn_copy,
    btn_paste,
    IupSetAttributes(IupLabel(NULL), "SEPARATOR=VERTICAL"),
    btn_find,
    NULL);
  IupSetAttribute(toolbar_hb, "MARGIN", "5x5");
  IupSetAttribute(toolbar_hb, "GAP", "2");

  vbox = IupVbox(
    toolbar_hb,
    multitext,
    lbl_statusbar,
    NULL);

  /* Do not use IupAppend because we set childtype=IUP_CHILDNONE */
  iupChildTreeAppend(ih, vbox);

  IupSetAttributeHandle(ih, "MENU", menu);
  IupSetCallback(ih, "CLOSE_CB", (Icallback)close_exit_action_cb);
  IupSetCallback(ih, "SHOW_CB", (Icallback)show_cb);
  IupSetCallback(ih, "DROPFILES_CB", (Icallback)dropfiles_cb);

  IupSetCallback(ih, "K_cN", (Icallback)item_new_action_cb);
  IupSetCallback(ih, "K_cO", (Icallback)item_open_action_cb);
  IupSetCallback(ih, "K_cS", (Icallback)item_save_action_cb);
  IupSetCallback(ih, "K_cF", (Icallback)item_find_action_cb);
  IupSetCallback(ih, "K_cH", (Icallback)item_replace_action_cb);  /* replace system processing */
  IupSetCallback(ih, "K_cG", (Icallback)item_goto_action_cb);
  IupSetCallback(ih, "K_cB", (Icallback)item_gotombrace_action_cb);
  IupSetCallback(ih, "K_cF2", (Icallback)item_togglemark_action_cb);
  IupSetCallback(ih, "K_F2", (Icallback)item_nextmark_action_cb);
  IupSetCallback(ih, "K_sF2", (Icallback)item_previousmark_action_cb);
  IupSetCallback(ih, "K_F3", (Icallback)find_next_action_cb);
  IupSetCallback(ih, "K_cF3", (Icallback)selection_find_next_action_cb);
  IupSetCallback(ih, "K_c+", (Icallback)item_zoomin_action_cb);
  IupSetCallback(ih, "K_c-", (Icallback)item_zoomout_action_cb);
  IupSetCallback(ih, "K_c/", (Icallback)item_restorezoom_action_cb);
  IupSetCallback(ih, "K_cEqual", (Icallback)item_zoomin_action_cb);
  IupSetCallback(ih, "K_cMinus", (Icallback)item_zoomout_action_cb);
  IupSetCallback(ih, "K_cSlash", (Icallback)item_restorezoom_action_cb);

  /* Ctrl+C, Ctrl+X, Ctrl+A, Del, already implemented inside IupScintilla */
  IupSetCallback(multitext, "K_cV", (Icallback)item_paste_action_cb);  /* replace system processing */
  IupSetCallback(multitext, "K_cU", (Icallback)item_case_action_cb);

  (void)params;
  return IUP_NOERROR;
}

Iclass* iupScintillaDlgNewClass(void)
{
  Iclass* ic = iupClassNew(iupRegisterFindClass("dialog"));

  ic->New = iupScintillaDlgNewClass;
  ic->Create = iScintillaDlgCreateMethod;

  ic->name = "scintilladlg";
  ic->nativetype = IUP_TYPEDIALOG;
  ic->is_interactive = 1;
  ic->childtype = IUP_CHILDNONE;
  ic->has_attrib_id = 1;   /* has attributes with IDs that must be parsed */

  iupClassRegisterCallback(ic, "MARKERCHANGED_CB", "ii");
  iupClassRegisterCallback(ic, "EXIT_CB", "");
  iupClassRegisterCallback(ic, "SAVEMARKERS_CB", "");
  iupClassRegisterCallback(ic, "RESTOREMARKERS_CB", "");
  iupClassRegisterCallback(ic, "CONFIGSAVE_CB", "");
  iupClassRegisterCallback(ic, "CONFIGLOAD_CB", "");

  iupClassRegisterAttribute(ic, "SUBTITLE", NULL, NULL, IUPAF_SAMEASSYSTEM, "Notepad", IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CONFIG", NULL, iScintillaDlgSetConfigAttrib, NULL, NULL, IUPAF_IHANDLENAME | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CONFIG_HANDLE", NULL, iScintillaDlgSetConfigHandleAttrib, NULL, NULL, IUPAF_IHANDLE | IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "NEWFILE", NULL, iScintillaDlgSetOpenFileAttrib, NULL, NULL, IUPAF_WRITEONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "OPENFILE", NULL, iScintillaDlgSetOpenFileAttrib, NULL, NULL, IUPAF_WRITEONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SAVEFILE", NULL, iScintillaDlgSetSaveFileAttrib, NULL, NULL, IUPAF_WRITEONLY | IUPAF_NO_INHERIT);
  iupClassRegisterAttribute(ic, "EXTRAFILTERS", NULL, NULL, NULL, NULL, IUPAF_NOT_MAPPED | IUPAF_NO_INHERIT);
  iupClassRegisterAttributeId(ic, "TOGGLEMARKER", NULL, iScintillaDlgSetToggleMarkerAttribId, IUPAF_WRITEONLY | IUPAF_NO_INHERIT);

  return ic;
}

Ihandle *IupScintillaDlg(void)
{
  return IupCreate("scintilladlg");
}

/* TODO:
- Printing
- multi-language (portuguese)
- multiple files (IupFlatTabs)
*/
