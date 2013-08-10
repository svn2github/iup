/** \file
 * \brief Windows Driver keyboard mapping
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>

#include <windows.h>

#include "iup.h"
#include "iupkey.h"

#include "iup_object.h"
#include "iup_key.h"
#include "iup_str.h"

#include "iup_drv.h"
#include "iup_focus.h"
#include "iup_attrib.h"
#include "iupwin_drv.h"
                   
                   
typedef struct _Iwin2iupkey
{
  int iupcode;
  int shift_iupcode;   /* base code when shift or caps are pressed */
} Iwin2iupkey;

static void winKeyInitXKey(Iwin2iupkey* map)
{
  map[VK_ESCAPE].iupcode =    K_ESC;
  map[VK_PAUSE].iupcode =     K_PAUSE;
  map[VK_SNAPSHOT].iupcode =  K_Print;
  map[VK_APPS].iupcode =      K_Menu;

  map[VK_CAPITAL].iupcode =   K_CAPS;
  map[VK_NUMLOCK].iupcode =   K_NUM;
  map[VK_SCROLL].iupcode =    K_SCROLL;

  map[VK_SHIFT].iupcode =   K_LSHIFT;
  map[VK_CONTROL].iupcode = K_LCTRL;
  map[VK_MENU].iupcode =    K_LALT;
                         
  map[VK_HOME].iupcode =      K_HOME;
  map[VK_UP].iupcode =        K_UP;
  map[VK_PRIOR].iupcode =     K_PGUP;
  map[VK_LEFT].iupcode =      K_LEFT;
  map[VK_CLEAR].iupcode =     K_MIDDLE;
  map[VK_RIGHT].iupcode =     K_RIGHT;
  map[VK_END].iupcode =       K_END;
  map[VK_DOWN].iupcode =      K_DOWN;
  map[VK_NEXT].iupcode =      K_PGDN;
  map[VK_INSERT].iupcode =    K_INS;
  map[VK_DELETE].iupcode =    K_DEL;
  map[VK_SPACE].iupcode =     K_SP;
  map[VK_TAB].iupcode =       K_TAB;
  map[VK_RETURN].iupcode =    K_CR;
  map[VK_BACK].iupcode =      K_BS;

  map[VK_F1].iupcode =  K_F1;
  map[VK_F2].iupcode =  K_F2;
  map[VK_F3].iupcode =  K_F3;
  map[VK_F4].iupcode =  K_F4;
  map[VK_F5].iupcode =  K_F5;
  map[VK_F6].iupcode =  K_F6;
  map[VK_F7].iupcode =  K_F7;
  map[VK_F8].iupcode =  K_F8;
  map[VK_F9].iupcode =  K_F9;
  map[VK_F10].iupcode = K_F10;
  map[VK_F11].iupcode = K_F11;
  map[VK_F12].iupcode = K_F12;

  map[VK_OEM_PLUS].iupcode =   K_plus;
  map[VK_OEM_COMMA].iupcode =  K_comma;
  map[VK_OEM_MINUS].iupcode =  K_minus;
  map[VK_OEM_PERIOD].iupcode = K_period;

  map[VK_NUMPAD0].iupcode =   K_0;
  map[VK_NUMPAD1].iupcode =   K_1;
  map[VK_NUMPAD2].iupcode =   K_2;
  map[VK_NUMPAD3].iupcode =   K_3;
  map[VK_NUMPAD4].iupcode =   K_4;
  map[VK_NUMPAD5].iupcode =   K_5;
  map[VK_NUMPAD6].iupcode =   K_6;
  map[VK_NUMPAD7].iupcode =   K_7;
  map[VK_NUMPAD8].iupcode =   K_8;
  map[VK_NUMPAD9].iupcode =   K_9;
  map[VK_MULTIPLY].iupcode =  K_asterisk;
  map[VK_ADD].iupcode =       K_plus;
  map[VK_SUBTRACT].iupcode =  K_minus;
  map[VK_DIVIDE].iupcode =    K_slash;

  map[VK_DECIMAL].iupcode =   MapVirtualKeyA(VK_DECIMAL, MAPVK_VK_TO_CHAR);
  map[VK_SEPARATOR].iupcode = MapVirtualKeyA(VK_SEPARATOR, MAPVK_VK_TO_CHAR);

  /* TODO: how to get the shift code? */
  map[VK_OEM_PLUS].iupcode = MapVirtualKeyA(VK_OEM_PLUS, MAPVK_VK_TO_CHAR);
  map[VK_OEM_COMMA].iupcode = MapVirtualKeyA(VK_OEM_COMMA, MAPVK_VK_TO_CHAR);
  map[VK_OEM_MINUS].iupcode = MapVirtualKeyA(VK_OEM_MINUS, MAPVK_VK_TO_CHAR);
  map[VK_OEM_PERIOD].iupcode = MapVirtualKeyA(VK_OEM_PERIOD, MAPVK_VK_TO_CHAR);

  if (!map[VK_OEM_1].iupcode) map[VK_OEM_1].iupcode = MapVirtualKeyA(VK_OEM_1, MAPVK_VK_TO_CHAR);
  if (!map[VK_OEM_2].iupcode) map[VK_OEM_2].iupcode = MapVirtualKeyA(VK_OEM_2, MAPVK_VK_TO_CHAR);
  if (!map[VK_OEM_3].iupcode) map[VK_OEM_3].iupcode = MapVirtualKeyA(VK_OEM_3, MAPVK_VK_TO_CHAR);
  if (!map[VK_OEM_4].iupcode) map[VK_OEM_4].iupcode = MapVirtualKeyA(VK_OEM_4, MAPVK_VK_TO_CHAR);
  if (!map[VK_OEM_5].iupcode) map[VK_OEM_5].iupcode = MapVirtualKeyA(VK_OEM_5, MAPVK_VK_TO_CHAR);
  if (!map[VK_OEM_6].iupcode) map[VK_OEM_6].iupcode = MapVirtualKeyA(VK_OEM_6, MAPVK_VK_TO_CHAR);
  if (!map[VK_OEM_7].iupcode) map[VK_OEM_7].iupcode = MapVirtualKeyA(VK_OEM_7, MAPVK_VK_TO_CHAR);
  if (!map[VK_OEM_8].iupcode) map[VK_OEM_8].iupcode = MapVirtualKeyA(VK_OEM_8, MAPVK_VK_TO_CHAR);

  map[VK_OEM_102].iupcode = MapVirtualKeyA(VK_OEM_102, MAPVK_VK_TO_CHAR);

  /* ABNT extra definitions */
  map[0xC2].iupcode = MapVirtualKeyA(0xC2, MAPVK_VK_TO_CHAR);
  map[0xC1].iupcode = MapVirtualKeyA(0xC1, MAPVK_VK_TO_CHAR);
  map[0xC1].shift_iupcode = '?';
}

static void winKeySetCharMap(Iwin2iupkey* winkey_map, char c)
{
  SHORT ret = VkKeyScanA(c);
  int wincode = LOBYTE(ret);
  int state = HIBYTE(ret);
  if (wincode != -1)
  {
    if (state & 1) /* Shift */
      winkey_map[wincode].shift_iupcode = (BYTE)c;
    else
      winkey_map[wincode].iupcode = (BYTE)c;
  }
}

static Iwin2iupkey winkey_map[256];

static int winKeyMap(int wincode)
{
  static int init = 0;
  if (init==0)
  {
    int i;

    memset(winkey_map, 0, sizeof(Iwin2iupkey)*256);

    for (i=32; i<127; i++)
      winKeySetCharMap(winkey_map, (char)i);

    winKeySetCharMap(winkey_map, K_ccedilla);
    winKeySetCharMap(winkey_map, K_Ccedilla);
    winKeySetCharMap(winkey_map, K_acute);
    winKeySetCharMap(winkey_map, K_diaeresis);

    winKeyInitXKey(winkey_map);

    init = 1;
  }

  /* now do the mapping */
  if (wincode == VK_SHIFT && (GetKeyState(VK_RSHIFT) & 0x8000))
    return K_RSHIFT;
  else if (wincode == VK_CONTROL && (GetKeyState(VK_RCONTROL) & 0x8000))
    return K_RCTRL;
  else if (wincode == VK_MENU && (GetKeyState(VK_RMENU) & 0x8000))
    return K_RALT;
  else
  {
    int has_caps = GetKeyState(VK_CAPITAL) & 0x01;
    int has_shift = GetKeyState(VK_SHIFT) & 0x8000;
    if (( has_caps && !has_shift) ||  /* CapsLock or Shift, but not both */
        (!has_caps &&  has_shift))
    {
      if (winkey_map[wincode].shift_iupcode)
        return winkey_map[wincode].shift_iupcode;
      else
        return winkey_map[wincode].iupcode;
    }
    else
      return winkey_map[wincode].iupcode;
  }
}

void iupdrvKeyEncode(int code, unsigned int *wincode, unsigned int *state)
{
  int i, iupcode = iup_XkeyBase(code);

  /* Must un-remap always */
  for (i = 0; i < 256; i++)
  {
    if (winkey_map[i].iupcode == iupcode)
    {
      *wincode = i;
      break;
    }
    if (winkey_map[i].shift_iupcode == iupcode)
    {
      *wincode = i;
      *state = VK_SHIFT;
      break;
    }
  }

  *state = 0;
  if (iup_isShiftXkey(code))
    *state = VK_SHIFT;
  else if (iup_isCtrlXkey(code))
    *state = VK_CONTROL;
  else if (iup_isAltXkey(code))
    *state = VK_MENU;
  else if (iup_isSysXkey(code))
    *state = VK_LWIN;
}

static int winKeyMap2Iup(int wincode)
{
  int code = wincode;
  int has_caps = GetKeyState(VK_CAPITAL) & 0x01;
  int has_shift = GetKeyState(VK_SHIFT) & 0x8000;
  int has_ctrl = GetKeyState(VK_CONTROL) & 0x8000;
  int has_alt = GetKeyState(VK_MENU) & 0x8000;
  int has_sys = (GetKeyState(VK_LWIN) & 0x8000) || (GetKeyState(VK_RWIN) & 0x8000);

  if (( has_caps && !has_shift) ||  /* CapsLock or Shift, but not both */
      (!has_caps &&  has_shift))
  {
    if ((wincode < K_exclam || wincode > K_tilde) ||
        (has_ctrl || has_alt || has_sys))
      code |= iup_XkeyShift(code);  /* only add Shift modifiers for non-ASCii codes, except for K_SP and bellow, 
                                       and except when other modifiers are used */
  }
  else if (has_ctrl || has_alt || has_sys)
  {
    /* If not shift, but has some of the other modifiers then use upper case version */
    if (wincode >= K_a && wincode <= K_z)
      code = iup_toupper(wincode);
    else if (wincode==K_ccedilla)
      code = K_Ccedilla;
  }

  if (has_ctrl)   /* Ctrl */
    code |= iup_XkeyCtrl(code);

  if (has_alt)    /* Alt */
    code |= iup_XkeyAlt(code);

  if (has_sys)    /* Apple/Win */
    code |= iup_XkeySys(code);

  return code;
}

int iupwinKeyDecode(int wincode)
{
  int iupcode = winKeyMap(wincode);
  if (!iupcode)
    iupcode=wincode;

  return winKeyMap2Iup(iupcode);
}

int iupwinKeyEvent(Ihandle* ih, int wincode, int press)
{
  int result, code;

  if (!ih->iclass->is_interactive)
    return 1;

  code = iupwinKeyDecode(wincode);
  if (code == 0)
    return 1;

  if (press)
  {
    result = iupKeyCallKeyCb(ih, code);
    if (result == IUP_CLOSE)
    {
      IupExitLoop();
      return 1;
    }
    else if (result == IUP_IGNORE)
      return 0;

    /* in the previous callback the dialog could be destroyed */
    if (iupObjectCheck(ih))
    {
      /* this is called only for canvas */
      if (ih->iclass->nativetype == IUP_TYPECANVAS) 
      {
        result = iupKeyCallKeyPressCb(ih, code, 1);
        if (result == IUP_CLOSE)
        {
          IupExitLoop();
          return 1;
        }
        else if (result == IUP_IGNORE)
          return 0;
      }
    }

    if ((GetKeyState(VK_MENU) & 0x8000) && wincode < 128) /* Alt + mnemonic */
    {
      if (iupKeyProcessMnemonic(ih, wincode))
        return 0;
    }

    if (iupKeyProcessNavigation(ih, code, (GetKeyState(VK_SHIFT) & 0x8000)))
      return 0;
  }
  else
  {
    /* this is called only for canvas */
    if (ih->iclass->nativetype == IUP_TYPECANVAS)
    {
      result = iupKeyCallKeyPressCb(ih, code, 0);
      if (result == IUP_CLOSE)
      {
        IupExitLoop();
        return 1;
      }
      else if (result == IUP_IGNORE)
        return 0;
    }
  }

  return 1;
}

void iupwinButtonKeySetStatus(WORD keys, char* status, int doubleclick)
{
  if (keys & MK_SHIFT)
    iupKEY_SETSHIFT(status);

  if (keys & MK_CONTROL)
    iupKEY_SETCONTROL(status); 

  if (keys & MK_LBUTTON)
    iupKEY_SETBUTTON1(status);

  if (keys & MK_MBUTTON)
    iupKEY_SETBUTTON2(status);

  if (keys & MK_RBUTTON)
    iupKEY_SETBUTTON3(status);

  if (doubleclick)
    iupKEY_SETDOUBLE(status);

  if (GetKeyState(VK_MENU) & 0x8000)
    iupKEY_SETALT(status);

  if ((GetKeyState(VK_LWIN) & 0x8000) || (GetKeyState(VK_RWIN) & 0x8000))
    iupKEY_SETSYS(status);

  if (keys & MK_XBUTTON1)
    iupKEY_SETBUTTON4(status);

  if (keys & MK_XBUTTON2)
    iupKEY_SETBUTTON5(status);
}
