#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <iup.h>
#include <iup_scintilla.h>
#include <iup_config.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <iuplua.h>

#include "utl_button_images.h"


lua_State *lcmd_state;

static char* read_file(const char* filename)
{
  int size;
  char* str;
  FILE* file = fopen(filename, "rb");
  if (!file)
  {
    IupMessagef("Error", "Can't open file: %s", filename);
    return NULL;
  }

  /* calculate file size */
  fseek(file, 0, SEEK_END);
  size = ftell(file);
  fseek(file, 0, SEEK_SET);

  /* allocate memory for the file contents + nul terminator */
  str = malloc(size + 1);
  /* read all data at once */
  fread(str, size, 1, file);
  /* set the nul terminator */
  str[size] = 0;

  if (ferror(file))
    IupMessagef("Error", "Fail when reading from file: %s", filename);

  fclose(file);
  return str;
}


/********************************** Utilities *****************************************/

char* getLuaKeywords(void)
{
  return "and break do else elseif end false for function if in local nil not or repeat return then true until while "
    "_VERSION assert collectgarbage dofile error gcinfo loadfile loadstring print rawget rawset require tonumber tostring type unpack "
    "_ALERT _ERRORMESSAGE _INPUT _PROMPT _OUTPUT _STDERR _STDIN _STDOUT call dostring foreach foreachi getn globals newtype sort tinsert tremove "
    "abs acos asin atan atan2 ceil cos deg exp floor format frexp gsub ldexp log log10 max min mod rad random randomseed sin sqrt strbyte strchar strfind strlen strlower strrep strsub strupper tan "
    "openfile closefile readfrom writeto appendto remove rename flush seek tmpfile tmpname read write clock date difftime execute exit getenv setlocale time "
    "_G getfenv getmetatable ipairs loadlib next pairs pcall rawequal setfenv setmetatable xpcall string table math coroutine io os debug load module select "
    "string.byte string.char string.dump string.find string.len string.lower string.rep string.sub string.upper string.format string.gfind string.gsub table.concat table.foreach table.foreachi table.getn table.sort table.insert table.remove table.setn math.abs math.acos math.asin math.atan math.atan2 math.ceil math.cos math.deg math.exp math.floor math.frexp math.ldexp math.log math.log10 math.max math.min math.mod math.pi math.pow math.rad math.random math.randomseed math.sin math.sqrt math.tan string.gmatch string.match string.reverse table.maxn math.cosh math.fmod math.modf math.sinh math.tanh math.huge "
    "coroutine.create coroutine.resume coroutine.status coroutine.wrap coroutine.yield io.close io.flush io.input io.lines io.open io.output io.read io.tmpfile io.type io.write io.stdin io.stdout io.stderr os.clock os.date os.difftime os.execute os.exit os.getenv os.remove os.rename os.setlocale os.time os.tmpname coroutine.running package.cpath package.loaded package.loadlib package.path package.preload package.seeall io.popen";
}


#define IUP_STR_EQUAL(str1, str2)      \
{                                      \
if (str1 == str2)                    \
  return 1;                          \
  \
if (!str1 || !str2)                  \
  return 0;                          \
  \
while (*str1 && *str2 &&              \
  SF(*str1) == SF(*str2))        \
{                                    \
  EXTRAINC(str1);                    \
  EXTRAINC(str2);                    \
  str1++;                            \
  str2++;                            \
}                                    \
  \
  /* check also for terminator */      \
if (*str1 == *str2) return 1;        \
}


char *getLastNonAlphaNumeric(char *text)
{
  int len = (int)strlen(text);
  char *c = text + len - 1;
  if (*c == '\n')
    c -= 1;
  for (c; c != text; c--)
  {
    if (*c < 48 || (*c > 57 && *c < 65) || (*c > 90 && *c < 97) || *c > 122)
      return c + 1;
  }
  return NULL;
}

const char* strNextValue(const char* str, int str_len, int *len, char sep)
{
  int ignore_sep = 0;

  *len = 0;

  if (!str) return NULL;

  while (*str != 0 && (*str != sep || ignore_sep) && *len < str_len)
  {
    if (*str == '\"')
    {
      if (ignore_sep)
        ignore_sep = 0;
      else
        ignore_sep = 1;
    }

    (*len)++;
    str++;
  }

  if (*str == sep)
    return str + 1;
  else
    return str;  /* no next value */
}

int strEqualPartial(const char* str1, const char* str2)
{
#define EXTRAINC(_x) (void)(_x)
#define SF(_x) (_x)
  IUP_STR_EQUAL(str1, str2);
#undef SF
#undef EXTRAINC
  if (*str2 == 0)
    return 1;  /* if second string is at terminator, then it is partially equal */
  return 0;
}

char *filterList(char *text, char *list)
{
  char *filteredList[1024];
  char *retList;
  int count = 0;

  int i, len;
  const char *lastValue = list;
  const char *nextValue = strNextValue(list, (int)strlen(list), &len, ' ');
  while (len != 0)
  {
    if ((int)strlen(text) <= len && strEqualPartial(lastValue, text))
    {
      char *value = malloc(80);

      strncpy(value, lastValue, len);
      value[len] = 0;
      filteredList[count++] = value;
    }
    lastValue = nextValue;
    nextValue = strNextValue(nextValue, (int)strlen(nextValue), &len, ' ');
  }

  retList = malloc(1024);
  retList[0] = 0;
  for (i = 0; i < count; i++)
  {
    if (i == 0)
    {
      strcpy(retList, filteredList[i]);
      strcat(retList, " ");
    }
    else
    {
      strcat(retList, filteredList[i]);
      strcat(retList, " ");
    }
  }

  for (i = 0; i < count; i++)
    free(filteredList[i]);

  return retList;
}


/********************************** Callbacks *****************************************/


int marker_changed_cb(Ihandle *ih, int lin, int margin, int value)
{
  if (margin == 2)
  {
    lua_getfield(lcmd_state, LUA_GLOBALSINDEX, "toggleBreakpoint");
    lua_pushinteger(lcmd_state, lin + 1);
    lua_pushinteger(lcmd_state, margin - 1);
    lua_pushinteger(lcmd_state, value);
    lua_call(lcmd_state, 3, 0);
  }

  return IUP_DEFAULT;
}

int k_any(Ihandle *ih, int c)
{
  Ihandle* multitext = IupGetDialogChild(ih, "MULTITEXT");

  if (!IupGetInt(multitext, "AUTOCOMPLETION"))
    return IUP_CONTINUE;

  if (c == K_ESC && IupGetInt(multitext, "AUTOCACTIVE"))
    IupSetAttribute(multitext, "AUTOCCANCEL", "YES");

  return IUP_CONTINUE;
}

int multitext_valuechanged_cb(Ihandle* multitext)
{
  IupSetAttribute(multitext, "DIRTY", "YES");

  if (IupGetInt(multitext, "AUTOCOMPLETION"))
  {
    char *t;
    int pos = IupGetInt(multitext, "CARETPOS");
    char *text = IupGetAttribute(multitext, "VALUE");
    text[pos + 1] = '\0';
    t = getLastNonAlphaNumeric(text);
    if (t != NULL && *t != '\n' && *t != 0)
    {
      char *fList = filterList(t, getLuaKeywords());
      if (strlen(fList) > 0)
        IupSetAttributeId(multitext, "AUTOCSHOW", (int)strlen(t) - 1, fList);
      free(fList);
    }

    return IUP_DEFAULT;
  }

  return IUP_CONTINUE;
}

int item_autocomplete_action_cb(Ihandle* ih)
{
  Ihandle* multitext = IupGetDialogChild(ih, "MULTITEXT");

  if (IupGetInt(ih, "VALUE"))
  {
    IupSetAttribute(ih, "VALUE", "OFF");
    IupSetAttribute(multitext, "AUTOCOMPLETION", "OFF");
  }
  else
  {
    IupSetAttribute(ih, "VALUE", "ON");
    IupSetAttribute(multitext, "AUTOCOMPLETION", "ON");
  }

  return IUP_DEFAULT;
}

void debug_set_state(lua_State *l, const char* state)
{
  int st;

  lua_getglobal(l, state);
  st = (int)lua_tointeger(l, -1);

  lua_getfield(l, LUA_GLOBALSINDEX, "debug_set_state");
  lua_pushinteger(l, st);
  lua_call(l, 1, 0);
}

int item_debug_action_cb(Ihandle *item)
{
  Ihandle* multitext = IupGetDialogChild(item, "MULTITEXT");

  char* filename = IupGetAttribute(multitext, "FILENAME");

  if (filename == NULL || strcmp(filename, "") == 0)
    return IUP_DEFAULT;

  debug_set_state(lcmd_state, "DEBUG_ACTIVE");

  lua_getfield(lcmd_state, LUA_GLOBALSINDEX, "startDebug");
  lua_pushstring(lcmd_state, filename);
  lua_call(lcmd_state, 1, 0);

  return IUP_DEFAULT;
}

int item_run_action_cb(Ihandle *item)
{
  Ihandle* multitext = IupGetDialogChild(item, "MULTITEXT");

  char* filename = IupGetAttribute(multitext, "FILENAME");

  if (filename == NULL || strcmp(filename, "") == 0)
    return IUP_DEFAULT;

  debug_set_state(lcmd_state, "DEBUG_ACTIVE");

  lua_getfield(lcmd_state, LUA_GLOBALSINDEX, "startDebug");
  lua_pushstring(lcmd_state, filename);
  lua_pushinteger(lcmd_state, 1);
  lua_call(lcmd_state, 2, 0);

  return IUP_DEFAULT;
}

int item_stop_action_cb(Ihandle *item)
{
  debug_set_state(lcmd_state, "DEBUG_STOPPED");

  return IUP_DEFAULT;
}

int item_pause_action_cb(Ihandle *item)
{
  debug_set_state(lcmd_state, "DEBUG_PAUSED");

  return IUP_DEFAULT;
}

int pause_action_cb(Ihandle* item)
{
  char* mod = IupGetGlobal("MODKEYSTATE");

  if (mod[1] == 'C' && mod[2] == 'A')
    item_pause_action_cb(item);

  return IUP_DEFAULT;
}

int item_continue_action_cb(Ihandle *item)
{
  debug_set_state(lcmd_state, "DEBUG_ACTIVE");

  return IUP_DEFAULT;
}

int item_stepinto_action_cb(Ihandle *item)
{
  debug_set_state(lcmd_state, "DEBUG_STEP_INTO");

  return IUP_DEFAULT;
}

int item_stepover_action_cb(Ihandle *item)
{
  debug_set_state(lcmd_state, "DEBUG_STEP_OVER");

  return IUP_DEFAULT;
}

int item_stepout_action_cb(Ihandle *item)
{
  debug_set_state(lcmd_state, "DEBUG_STEP_OUT");

  return IUP_DEFAULT;
}

int item_resetluastate_action_cb(Ihandle *item)
{
  return IUP_DEFAULT;
}

int item_help_action_cb(void)
{
  IupHelp("http://www.tecgraf.puc-rio.br/iup");
  return IUP_DEFAULT;
}

int item_about_action_cb(void)
{
  IupMessage("About", "   IupLua Scripter\n\nAutors:\n   Camilo Freire\n   Antonio Scuri");
  return IUP_DEFAULT;
}

int item_curline_action_cb(Ihandle* ih)
{
  int lin, col;
  Ihandle* multitext = IupGetDialogChild(ih, "MULTITEXT");
  int pos = IupGetInt(multitext, "CARETPOS");
  IupTextConvertPosToLinCol(multitext, pos, &lin, &col);
  lua_pushinteger(lcmd_state, lin + 1);
  lua_setglobal(lcmd_state, "breakAtCaret");

  debug_set_state(lcmd_state, "DEBUG_ACTIVE");

  return IUP_DEFAULT;
}

int item_curlinebreak_action_cb(void)
{
  return IUP_DEFAULT;
}

int txt_cmdline_cb(Ihandle *ih, int c)
{
  switch (c)
  {
    case K_CR:
      lua_getfield(lcmd_state, LUA_GLOBALSINDEX, "consoleEnterCommand");
      lua_call(lcmd_state, 0, 0);
      break;
    case K_ESC:
      IupSetAttribute(ih, IUP_VALUE, "");
      break;
    case K_UP:
      lua_getfield(lcmd_state, LUA_GLOBALSINDEX, "consoleKeyUpCommand");
      lua_call(lcmd_state, 0, 0);
      break;
    case K_DOWN:
      lua_getfield(lcmd_state, LUA_GLOBALSINDEX, "consoleKeyDownCommand");
      lua_call(lcmd_state, 0, 0);
      break;
  }
  return IUP_CONTINUE;
}

int btn_listfunc_cb(Ihandle *ih)
{
  lua_getfield(lcmd_state, LUA_GLOBALSINDEX, "consoleListFuncAction");
  lua_call(lcmd_state, 0, 0);

  return IUP_DEFAULT;
}

int btn_listvar_cb(Ihandle *ih)
{
  lua_getfield(lcmd_state, LUA_GLOBALSINDEX, "consoleListVarAction");
  lua_call(lcmd_state, 0, 0);

  return IUP_DEFAULT;
}

int btn_clear_cb(Ihandle *ih)
{
  lua_getfield(lcmd_state, LUA_GLOBALSINDEX, "consoleClearAction");
  lua_call(lcmd_state, 0, 0);
  return IUP_DEFAULT;
}

int btn_buffsize_cb(Ihandle *ih)
{
  return IUP_DEFAULT;
}

int but_printlocal_cb(Ihandle *ih)
{
  return IUP_DEFAULT;
}

int but_setlocal_cb(Ihandle *ih)
{
  int value = IupGetInt(IupGetDialogChild(ih, "LIST_LOCAL"), "VALUE");

  if (value == 0)
    return IUP_DEFAULT;

  lua_getfield(lcmd_state, LUA_GLOBALSINDEX, "SetLocal");
  lua_pushinteger(lcmd_state, value);
  lua_call(lcmd_state, 1, 0);

  return IUP_DEFAULT;
}

int lst_stack_cb(Ihandle *ih, char *t, int i, int v)
{
  if (v == 0)
    return IUP_DEFAULT;

  lua_getfield(lcmd_state, LUA_GLOBALSINDEX, "listStackAction");
  lua_pushinteger(lcmd_state, i);
  lua_call(lcmd_state, 1, 0);

  return IUP_DEFAULT;
}

int but_printlevel_cb(Ihandle *ih)
{
  return IUP_DEFAULT;
}

int but_printstack_cb(Ihandle *ih)
{
  return IUP_DEFAULT;
}

int but_addbreak_cb(Ihandle *ih)
{
  int lin, col;
  Ihandle* multitext = IupGetDialogChild(ih, "MULTITEXT");
  int pos = IupGetInt(multitext, "CARETPOS");
  IupTextConvertPosToLinCol(multitext, pos, &lin, &col);

  IupSetAttributeId(IupGetDialog(ih), "TOGGLEMARKER", lin, "2");

  return IUP_DEFAULT;
}

int but_removebreak_cb(Ihandle *ih)
{
  Ihandle* listBreak = IupGetDialogChild(ih, "LIST_BREAK");
  int value = IupGetInt(listBreak, "VALUE");

  if (value == 0)
    return IUP_DEFAULT;

  lua_getfield(lcmd_state, LUA_GLOBALSINDEX, "removeBreakpointFromList");
  lua_pushinteger(lcmd_state, value);
  lua_call(lcmd_state, 1, 0);

  return IUP_DEFAULT;
}

int but_removeallbreaks_cb(Ihandle *ih)
{
  lua_getfield(lcmd_state, LUA_GLOBALSINDEX, "removeAllBreakpoints");
  lua_call(lcmd_state, 0, 0);

  return IUP_DEFAULT;
}

/********************************** Main *****************************************/

Ihandle *buildTabOutput(void)
{
  Ihandle *txt_cmdLine, *btn_listfunc, *btn_listvar, *btn_clear, *btn_buffsize, *console_bts;
  Ihandle *frm_consolebts, *ml_output, *output;

  txt_cmdLine = IupText(NULL);
  IupSetAttribute(txt_cmdLine, "EXPAND", "HORIZONTAL");
  IupSetAttribute(txt_cmdLine, "NAME", "TXT_CMDLINE");
  btn_listfunc = IupButton("List Funcs", NULL);
  IupSetAttribute(btn_listfunc, "EXPAND", "NO");
  IupSetAttribute(btn_listfunc, "NAME", "BTN_LISTFUNC");
  btn_listvar = IupButton("List Vars", NULL);
  IupSetAttribute(btn_listvar, "EXPAND", "NO");
  IupSetAttribute(btn_listvar, "NAME", "BTN_LISTVAR");
  btn_clear = IupButton("Clear", NULL);
  IupSetAttribute(btn_clear, "EXPAND", "NO");
  IupSetAttribute(btn_clear, "NAME", "BTN_CLEAR");
  btn_buffsize = IupButton("Buffer Size...", NULL);
  IupSetAttribute(btn_buffsize, "EXPAND", "NO");
  IupSetAttribute(btn_buffsize, "NAME", "BTN_BUFSIZE");

  console_bts = IupHbox(txt_cmdLine, btn_listfunc, btn_listvar, btn_clear, btn_buffsize, NULL);
  IupSetAttribute(console_bts, "EXPAND", "HORIZONTAL");
  IupSetAttribute(console_bts, "MARGIN", "3x3");
  IupSetAttribute(console_bts, "GAP", "2");

  frm_consolebts = IupFrame(console_bts);
  IupSetAttribute(frm_consolebts, "EXPAND", "HORIZONTAL");
  IupSetAttribute(frm_consolebts, "TITLE", "Command Line:");

  ml_output = IupMultiLine(NULL);
  IupSetAttribute(ml_output, "NAME", "MTL_OUTPUT");
  IupSetAttribute(ml_output, "EXPAND", "YES");
  IupSetAttribute(ml_output, "RASTERSIZE", "x250");
  IupSetAttribute(ml_output, "READONLY", "YES");
  IupSetAttribute(ml_output, "BGCOLOR", "224 224 2254");

  output = IupVbox(frm_consolebts, ml_output, NULL);
  IupSetAttribute(output, "EXPAND", "YES");
  IupSetAttribute(output, "MARGIN", "0x0");
  IupSetAttribute(output, "GAP", "4");
  IupSetAttribute(output, "TABTITLE", "Output");
  IupSetAttribute(output, "NAME", "OUTPUT");

  IupSetHandle("tabOutput", output);

  IupSetCallback(txt_cmdLine, IUP_K_ANY, (Icallback)txt_cmdline_cb);
  IupSetCallback(btn_listfunc, "ACTION", (Icallback)btn_listfunc_cb);
  IupSetCallback(btn_listvar, "ACTION", (Icallback)btn_listvar_cb);
  IupSetCallback(btn_clear, "ACTION", (Icallback)btn_clear_cb);
  IupSetCallback(btn_buffsize, "ACTION", (Icallback)btn_buffsize_cb);

  return output;
}

Ihandle *buildTabLocals(void)
{
  Ihandle *list_local, *button_printLocal, *button_setLocal, *vbox_local, *frame_local;
  Ihandle *list_stack, *button_printLevel, *button_printStack, *vbox_stack, *frame_stack, *locals;

  list_local = IupList(NULL);
  IupSetAttribute(list_local, "EXPAND", "YES");
  IupSetAttribute(list_local, "NAME", "LIST_LOCAL");

  button_printLocal = IupButton("Print", NULL);
  IupSetAttribute(button_printLocal, "ACTIVE", "NO");
  IupSetAttribute(button_printLocal, "SIZE", "35x");
  IupSetAttribute(button_printLocal, "NAME", "PRINT_LOCAL");

  button_setLocal = IupButton("Set...", NULL);
  IupSetAttribute(button_setLocal, "ACTIVE", "YES");
  IupSetAttribute(button_setLocal, "SIZE", "35x");
  IupSetAttribute(button_setLocal, "NAME", "SET_LOCAL");

  vbox_local = IupVbox(button_printLocal, button_setLocal, NULL);
  IupSetAttribute(vbox_local, "EXPAND", "NO");
  IupSetAttribute(vbox_local, "MARGIN", "0x0");
  IupSetAttribute(vbox_local, "GAP", "4");

  frame_local = IupFrame(IupHbox(list_local, vbox_local, NULL));
  IupSetAttribute(frame_local, "EXPAND", "YES");
  IupSetAttribute(frame_local, "MARGIN", "4x4");
  IupSetAttribute(frame_local, "GAP", "4");
  IupSetAttribute(frame_local, "TITLE", "Locals");

  list_stack = IupList(NULL);
  IupSetAttribute(list_stack, "EXPAND", "YES");
  IupSetAttribute(list_stack, "NAME", "LIST_STACK");

  button_printLevel = IupButton("Print", NULL);
  IupSetAttribute(button_printLevel, "TIP", "Prints information about the selected call stack level.");
  IupSetAttribute(button_printLevel, "ACTIVE", "NO");
  IupSetAttribute(button_printLevel, "SIZE", "35x");
  IupSetAttribute(button_printLevel, "NAME", "PRINT_LEVEL");

  button_printStack = IupButton("Print ALL", NULL);
  IupSetAttribute(button_printStack, "TIP", "Prints information about all the stack levels.");
  IupSetAttribute(button_printStack, "ACTIVE", "NO");
  IupSetAttribute(button_printStack, "SIZE", "35x");
  IupSetAttribute(button_printStack, "NAME", "PRINT_STACK");

  vbox_stack = IupVbox(button_printLevel, button_printStack, NULL);
  IupSetAttribute(vbox_stack, "EXPAND", "NO");
  IupSetAttribute(vbox_stack, "MARGIN", "0x0");
  IupSetAttribute(vbox_stack, "GAP", "4");

  frame_stack = IupFrame(IupHbox(list_stack, vbox_stack, NULL));
  IupSetAttribute(frame_stack, "EXPAND", "YES");
  IupSetAttribute(frame_stack, "MARGIN", "4x4");
  IupSetAttribute(frame_stack, "GAP", "4");
  IupSetAttribute(frame_stack, "TITLE", "Call Stack");

  locals = IupHbox(frame_local, frame_stack, NULL);
  IupSetAttribute(locals, "EXPAND", "YES");
  IupSetAttribute(locals, "MARGIN", "0x0");
  IupSetAttribute(locals, "GAP", "4");
  IupSetAttribute(locals, "TABTITLE", "Debug");

  IupSetHandle("tabLocals", locals);

  IupSetCallback(button_printLocal, "ACTION", (Icallback)but_printlocal_cb);
  IupSetCallback(button_setLocal, "ACTION", (Icallback)but_setlocal_cb);
  IupSetCallback(list_stack, "ACTION", (Icallback)lst_stack_cb);
  IupSetCallback(button_printLevel, "ACTION", (Icallback)but_printlevel_cb);
  IupSetCallback(button_printStack, "ACTION", (Icallback)but_printstack_cb);

  return locals;
}

Ihandle *buildTabBreaks(void)
{
  Ihandle *button_addbreak, *button_removebreak, *button_removeallbreaks, *hbox, *list, *vbox;

  button_addbreak = IupButton("Toggle Breakpoint", NULL);
  IupSetAttribute(button_addbreak, "TIP", "Toggle breakpoint at the current line. Ctrl+B");

  button_removebreak = IupButton("Remove", NULL);
  IupSetAttribute(button_removebreak, "TIP", "Removes the selected breakpoint at the list.");

  button_removeallbreaks = IupButton("Remove All", NULL);
  IupSetAttribute(button_removeallbreaks, "TIP", "Removes all breakpoints.");

  hbox = IupHbox(button_addbreak, button_removebreak, button_removeallbreaks, NULL);
  IupSetAttribute(hbox, "EXPAND", "NO");
  IupSetAttribute(hbox, "MARGIN", "0x0");
  IupSetAttribute(hbox, "GAP", "4");

  list = IupList(NULL);
  IupSetAttribute(list, "EXPAND", "YES");
  IupSetAttribute(list, "NAME", "LIST_BREAK");

  vbox = IupVbox(hbox, list, NULL);
  IupSetAttribute(vbox, "EXPAND", "YES");
  IupSetAttribute(vbox, "MARGIN", "0x0");
  IupSetAttribute(vbox, "GAP", "4");
  IupSetAttribute(vbox, "TABTITLE", "Breakpoints");

  IupSetCallback(button_addbreak, "ACTION", (Icallback)but_addbreak_cb);
  IupSetCallback(button_removebreak, "ACTION", (Icallback)but_removebreak_cb);
  IupSetCallback(button_removeallbreaks, "ACTION", (Icallback)but_removeallbreaks_cb);

  return vbox;
}

void set_attribs(Ihandle *multitext)
{
  IupSetAttribute(multitext, "CLEARALL", "");
  IupSetAttribute(multitext, "LEXERLANGUAGE", "lua");

  IupSetAttribute(multitext, "KEYWORDS0", getLuaKeywords());

  IupSetAttribute(multitext, "STYLEFONT32", "Consolas");
  IupSetAttribute(multitext, "STYLEFONTSIZE32", "11");
  IupSetAttribute(multitext, "STYLECLEARALL", "Yes");  /* sets all styles to have the same attributes as 32 */

  IupSetAttribute(multitext, "STYLEFGCOLOR1", "0 128 0");    /* 1-Lua comment�*/
  IupSetAttribute(multitext, "STYLEFGCOLOR2", "0 128 0");    /* 2-Lua comment line��*/
  IupSetAttribute(multitext, "STYLEFGCOLOR4", "255 128 0");  /* 4-Number��*/
  IupSetAttribute(multitext, "STYLEFGCOLOR5", "0 0 255");    /* 5-Keyword��*/
  IupSetAttribute(multitext, "STYLEFGCOLOR6", "171 0 149");  /* 6-String��*/
  IupSetAttribute(multitext, "STYLEFGCOLOR7", "171 0 149");  /* 7-Character��*/
  IupSetAttribute(multitext, "STYLEFGCOLOR9", "0 0 255");    /* 9-Preprocessor block��*/
  IupSetAttribute(multitext, "STYLEFGCOLOR10", "0 0 0");     /* 10-Operator��*/
  IupSetAttribute(multitext, "STYLEBOLD10", "YES");
}

void showVersionInfo()
{
  lua_getfield(lcmd_state, LUA_GLOBALSINDEX, "showVersionInfo");
  lua_call(lcmd_state, 0, 0);
}

void appendDebugButtons(Ihandle *dialog)
{
  Ihandle *toolbar, *btn_debug, *btn_run, *btn_stop, *btn_pause, *btn_continue, *btn_curlinebreak;
  Ihandle *zbox_debug_continue, *btn_stepinto, *btn_stepover, *btn_stepout, *btn_curline;

  toolbar = IupGetChild(IupGetChild(dialog, 0), 0);

  btn_debug = IupButton(NULL, NULL);
  IupSetAttribute(btn_debug, "NAME", "BTN_DEBUG");
  IupSetHandle("BTN_DEBUG", btn_debug);
  IupSetAttribute(btn_debug, "IMAGE", "utlDebugButton");
  IupSetAttribute(btn_debug, "FLAT", "Yes");
  IupSetCallback(btn_debug, "ACTION", (Icallback)item_debug_action_cb);
  IupSetAttribute(btn_debug, "TIP", "Debug (F5)");
  IupSetAttribute(btn_debug, "CANFOCUS", "No");

  btn_run = IupButton(NULL, NULL);
  IupSetAttribute(btn_run, "NAME", "BTN_RUN");
  IupSetAttribute(btn_run, "IMAGE", "utlRunButton");
  IupSetAttribute(btn_run, "FLAT", "Yes");
  IupSetCallback(btn_run, "ACTION", (Icallback)item_run_action_cb);
  IupSetAttribute(btn_run, "TIP", "Run (Ctrl+F5)");
  IupSetAttribute(btn_run, "CANFOCUS", "No");

  btn_stop = IupButton(NULL, NULL);
  IupSetAttribute(btn_stop, "NAME", "BTN_STOP");
  IupSetAttribute(btn_stop, "ACTIVE", "NO");
  IupSetAttribute(btn_stop, "IMAGE", "utlStopButton");
  IupSetAttribute(btn_stop, "FLAT", "Yes");
  IupSetCallback(btn_stop, "ACTION", (Icallback)item_stop_action_cb);
  IupSetAttribute(btn_stop, "TIP", "Stop (Shift+F5)");
  IupSetAttribute(btn_stop, "CANFOCUS", "No");

  btn_pause = IupButton(NULL, NULL);
  IupSetAttribute(btn_pause, "NAME", "BTN_PAUSE");
  IupSetAttribute(btn_pause, "ACTIVE", "NO");
  IupSetAttribute(btn_pause, "IMAGE", "utlPauseButton");
  IupSetAttribute(btn_pause, "FLAT", "Yes");
  IupSetCallback(btn_pause, "ACTION", (Icallback)item_pause_action_cb);
  IupSetAttribute(btn_pause, "TIP", "Debug (F5)");
  IupSetAttribute(btn_pause, "CANFOCUS", "No");

  btn_continue = IupButton(NULL, NULL);
  IupSetAttribute(btn_continue, "NAME", "BTN_CONTINUE");
  IupSetHandle("BTN_CONTINUE", btn_continue);
  IupSetAttribute(btn_continue, "ACTIVE", "NO");
  IupSetAttribute(btn_continue, "IMAGE", "utlDebugButton");
  IupSetAttribute(btn_continue, "FLAT", "Yes");
  IupSetCallback(btn_continue, "ACTION", (Icallback)item_continue_action_cb);
  IupSetAttribute(btn_continue, "TIP", "Continue (F5)");
  IupSetAttribute(btn_continue, "CANFOCUS", "No");

  zbox_debug_continue = IupZbox(btn_debug, btn_continue, NULL);
  IupSetAttribute(zbox_debug_continue, "NAME", "ZBOX_DEBUG_CONTINUE");

  btn_stepinto = IupButton(NULL, NULL);
  IupSetAttribute(btn_stepinto, "NAME", "BTN_STEPINTO");
  IupSetAttribute(btn_stepinto, "ACTIVE", "NO");
  IupSetAttribute(btn_stepinto, "IMAGE", "utlStepIntoButton");
  IupSetAttribute(btn_stepinto, "FLAT", "Yes");
  IupSetCallback(btn_stepinto, "ACTION", (Icallback)item_stepinto_action_cb);
  IupSetAttribute(btn_stepinto, "TIP", "Executes one step into the execution.");
  IupSetAttribute(btn_stepinto, "CANFOCUS", "No");

  btn_stepover = IupButton(NULL, NULL);
  IupSetAttribute(btn_stepover, "NAME", "BTN_STEPOVER");
  IupSetAttribute(btn_stepover, "ACTIVE", "NO");
  IupSetAttribute(btn_stepover, "IMAGE", "utlStepOverButton");
  IupSetAttribute(btn_stepover, "FLAT", "Yes");
  IupSetCallback(btn_stepover, "ACTION", (Icallback)item_stepover_action_cb);
  IupSetAttribute(btn_stepover, "TIP", "Executes one step over the execution.");
  IupSetAttribute(btn_stepover, "CANFOCUS", "No");

  btn_stepout = IupButton(NULL, NULL);
  IupSetAttribute(btn_stepout, "NAME", "BTN_STEPOUT");
  IupSetAttribute(btn_stepout, "ACTIVE", "NO");
  IupSetAttribute(btn_stepout, "IMAGE", "utlStepOutButton");
  IupSetAttribute(btn_stepout, "FLAT", "Yes");
  IupSetCallback(btn_stepout, "ACTION", (Icallback)item_stepout_action_cb);
  IupSetAttribute(btn_stepout, "TIP", "Executes one step out of the execution.");
  IupSetAttribute(btn_stepout, "CANFOCUS", "No");

  btn_curline = IupButton("->", NULL);
  IupSetAttribute(btn_curline, "NAME", "BTN_CURLINE");
  IupSetAttribute(btn_curline, "ACTIVE", "NO");
  IupSetAttribute(btn_curline, "FLAT", "Yes");
  IupSetCallback(btn_curline, "ACTION", (Icallback)item_curline_action_cb);
  IupSetAttribute(btn_curline, "TIP", "Shows the current line (Ctrl+L)");
  IupSetAttribute(btn_curline, "CANFOCUS", "No");

  btn_curlinebreak = IupButton("B", NULL);
  IupSetAttribute(btn_curlinebreak, "NAME", "BTN_CURLINEBREAK");
  IupSetAttribute(btn_curlinebreak, "ACTIVE", "NO");
  IupSetAttribute(btn_curlinebreak, "FLAT", "Yes");
  IupSetCallback(btn_curlinebreak, "ACTION", (Icallback)item_curlinebreak_action_cb);
  IupSetAttribute(btn_curlinebreak, "TIP", "Toggle breakpoint at the current line (Ctrl+B)");
  IupSetAttribute(btn_curlinebreak, "CANFOCUS", "No");

  IupAppend(toolbar, IupSetAttributes(IupLabel(NULL), "SEPARATOR=VERTICAL"));
  IupAppend(toolbar, btn_run);
  IupAppend(toolbar, zbox_debug_continue);
  IupAppend(toolbar, btn_stop);
  IupAppend(toolbar, btn_pause);
  IupAppend(toolbar, btn_stepinto);
  IupAppend(toolbar, btn_stepover);
  IupAppend(toolbar, btn_stepout);
  IupAppend(toolbar, IupSetAttributes(IupFill(), "EXPAND=NO, SIZE=10"));
  IupAppend(toolbar, btn_curline);
  IupAppend(toolbar, btn_curlinebreak);
  IupAppend(toolbar, IupSetAttributes(IupLabel(NULL), "SEPARATOR=VERTICAL"));
}

void appendDebugMenuItens(Ihandle *menu)
{
  Ihandle *item_debug, *item_run, *item_stop, *item_pause, *item_continue, *item_stepinto,
          *item_stepover, *item_stepout, *item_resetluastate, *debugMneu, *subMenuDebug;

  item_debug = IupItem("&Debug\tF5", NULL);
  IupSetAttribute(item_debug, "NAME", "ITM_DEBUG");
  IupSetCallback(item_debug, "ACTION", (Icallback)item_debug_action_cb);

  item_run = IupItem("&Run\tCtrl+F5", NULL);
  IupSetAttribute(item_run, "NAME", "ITM_RUN");
  IupSetCallback(item_run, "ACTION", (Icallback)item_run_action_cb);

  item_stop = IupItem("&Stop\tShift+F5", NULL);
  IupSetAttribute(item_stop, "NAME", "ITM_STOP");
  IupSetCallback(item_stop, "ACTION", (Icallback)item_stop_action_cb);
  IupSetAttribute(item_stop, "ACTIVE", "NO");

  item_pause = IupItem("&Pause\tCtrl+Alt+Break", NULL);
  IupSetAttribute(item_pause, "NAME", "ITM_PAUSE");
  IupSetCallback(item_pause, "ACTION", (Icallback)item_pause_action_cb);
  IupSetAttribute(item_pause, "ACTIVE", "NO");

  item_continue = IupItem("&Continue\tF5", NULL);
  IupSetAttribute(item_continue, "NAME", "ITM_CONTINUE");
  IupSetCallback(item_continue, "ACTION", (Icallback)item_continue_action_cb);
  IupSetAttribute(item_continue, "ACTIVE", "NO");

  item_stepinto = IupItem("Step &Into\tF11", NULL);
  IupSetAttribute(item_stepinto, "NAME", "ITM_STEPINTO");
  IupSetCallback(item_stepinto, "ACTION", (Icallback)item_stepinto_action_cb);
  IupSetAttribute(item_stepinto, "ACTIVE", "NO");

  item_stepover = IupItem("Step &Over\tF10", NULL);
  IupSetAttribute(item_stepover, "NAME", "ITM_STEPOVER");
  IupSetCallback(item_stepover, "ACTION", (Icallback)item_stepover_action_cb);
  IupSetAttribute(item_stepover, "ACTIVE", "NO");

  item_stepout = IupItem("Step Ou&t\tShift+F11", NULL);
  IupSetAttribute(item_stepout, "NAME", "ITM_STEPOUT");
  IupSetCallback(item_stepout, "ACTION", (Icallback)item_stepout_action_cb);
  IupSetAttribute(item_stepout, "ACTIVE", "NO");

  item_resetluastate = IupItem("&Reset Lua State", NULL);
  IupSetCallback(item_resetluastate, "ACTION", (Icallback)item_resetluastate_action_cb);

  debugMneu = IupMenu(
    item_debug,
    item_run,
    item_stop,
    item_pause,
    item_continue,
    item_stepover,
    item_stepinto,
    item_stepout,
    item_resetluastate,
    NULL);

  subMenuDebug = IupSubmenu("&Lua", debugMneu);

  IupAppend(menu, subMenuDebug);
}

int main(int argc, char **argv)
{
  Ihandle *config;
  lua_State *L;
  Ihandle *multitext, *menu, *stabs, *box, *statusBar;
  Ihandle *tabConsole, *tabLocals, *tabBreaks, *debugTabs;
  Ihandle *main_dialog;

  IupOpen(&argc, &argv);
  IupImageLibOpen();
  utlCreateButtonImages();

  IupScintillaOpen();

  L = lcmd_state = lua_open();
  luaL_openlibs(lcmd_state);

  iuplua_open(lcmd_state);

  config = IupConfig();
  IupSetAttribute(config, "APP_NAME", "iupluascripter");
  IupConfigLoad(config);

  main_dialog = IupScintillaDlg();

  multitext = IupGetDialogChild(main_dialog, "MULTITEXT");

  appendDebugButtons(main_dialog);

  /* breakpoints */
  IupSetInt(multitext, "MARGINWIDTH2", 15);
  IupSetAttribute(multitext, "MARGINTYPE2", "SYMBOL");
  IupSetAttribute(multitext, "MARGINSENSITIVE2", "YES");
  IupSetAttribute(multitext, "MARGINMASKFOLDERS2", "NO");

  IupSetIntId(multitext, "MARGINMASK", 1, 0x000005);
  IupSetAttributeId(multitext, "MARKERFGCOLOR", 1, "255 0 0");
  IupSetAttributeId(multitext, "MARKERBGCOLOR", 1, "255 0 0");
  IupSetAttributeId(multitext, "MARKERALPHA", 1, "80");
  IupSetAttributeId(multitext, "MARKERSYMBOL", 1, "CIRCLE");

  IupSetIntId(multitext, "MARGINMASK", 2, 0x000002);
  IupSetAttributeId(multitext, "MARKERBGCOLOR", 2, "0 255 0");
  IupSetAttributeId(multitext, "MARKERALPHA", 2, "80");
  IupSetAttributeId(multitext, "MARKERSYMBOL", 2, "BACKGROUND");

  IupSetCallback(main_dialog, "K_F5", (Icallback)item_debug_action_cb);
  IupSetCallback(main_dialog, "K_cF5", (Icallback)item_run_action_cb);
  IupSetCallback(main_dialog, "K_sF5", (Icallback)item_stop_action_cb);
  IupSetCallback(main_dialog, "K_PAUSE", (Icallback)pause_action_cb);
  IupSetCallback(main_dialog, "K_F5", (Icallback)item_continue_action_cb);
  IupSetCallback(main_dialog, "K_F11", (Icallback)item_stepinto_action_cb);
  IupSetCallback(main_dialog, "K_F10", (Icallback)item_stepover_action_cb);
  IupSetCallback(main_dialog, "K_sF11", (Icallback)item_stepout_action_cb);

  IupSetCallback(main_dialog, "MARKERCHANGED_CB", (Icallback)marker_changed_cb);

  menu = IupGetAttributeHandle(main_dialog, "MENU");

  appendDebugMenuItens(menu);

  IupAppend(menu, IupSubmenu("&Help", IupMenu(
    IupSetCallbacks(IupItem("&Help...", NULL), "ACTION", (Icallback)item_help_action_cb, NULL),
    IupSetCallbacks(IupItem("&About...", NULL), "ACTION", (Icallback)item_about_action_cb, NULL),
    NULL)));

  tabConsole = buildTabOutput();

  tabLocals = buildTabLocals();

  tabBreaks = buildTabBreaks();

  debugTabs = IupTabs(tabConsole, tabLocals, tabBreaks, NULL);
  IupSetAttribute(debugTabs, "EXPAND", "YES");
  IupSetAttribute(debugTabs, "MARGIN", "0x0");
  IupSetAttribute(debugTabs, "GAP", "4");
  IupSetAttribute(debugTabs, "TABTYPE", "BOTTOM");
  IupSetAttribute(debugTabs, "NAME", "TBS_DEBUG");

  stabs = IupSbox(debugTabs);
  IupSetAttribute(stabs, "EXPAND", "YES");
  IupSetAttribute(stabs, "DIRECTION", "NORTH");
  IupSetAttribute(stabs, "MARGIN", "0x0");
  IupSetAttribute(stabs, "GAP", "4");

  box = IupGetChild(main_dialog, 0);
  statusBar = IupGetDialogChild(main_dialog, "STATUSBAR");
  IupInsert(box, statusBar, stabs);

  IupSetAttributeHandle(main_dialog, "CONFIG", config);
  IupSetAttribute(main_dialog, "SUBTITLE", "IupLuaScriter");
  IupSetAttribute(main_dialog, "EXTRAFILTERS", "Lua Files|*.lua|");

  iuplua_pushihandle(lcmd_state, main_dialog);
  lua_setglobal(lcmd_state, "main_dialog");

#ifdef IUPLUA_USELOH
#include "debugger.loh"
#include "console.loh"
#else
#ifdef IUPLUA_USELH
#include "debugger.lh"
#include "console.lh"
#else
  iuplua_dofile(L, "console.lua");
  iuplua_dofile(L, "debugger.lua");
#endif
#endif

  /* show the dialog at the last position, with the last size */
  IupConfigDialogShow(config, main_dialog, "MainWindow");

  /* initialize the current file */
  IupSetAttribute(main_dialog, "NEWFILE", NULL);

  /* open a file from the command line (allow file association in Windows) */
  if (argc > 1 && argv[1])
  {
    const char* filename = argv[1];
    IupSetStrAttribute(main_dialog, "OPENFILE", filename);
  }

  /* show the dialog at the last position, with the last size */
  IupConfigDialogShow(config, main_dialog, "MainWindow");

  showVersionInfo();

  set_attribs(IupGetDialogChild(main_dialog, "MULTITEXT"));

  IupMainLoop();

  IupClose();
  return EXIT_SUCCESS;
}
