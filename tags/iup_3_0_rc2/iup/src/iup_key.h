/** \file
 * \brief Manage keys encoding and decoding.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUP_KEY_H 
#define __IUP_KEY_H


/** \defgroup key Key Coding and Key Callbacks
 * \par
 * See \ref iup_key.h
 * \ingroup cpi */


/** Returns the key name from its code. 
 * Returns NULL if code not found.
 * \ingroup key */
char *iupKeyCodeToName(int code);

/** Returns the key code from its name. 
 * Returns 0 if name not found.
 * \ingroup key */
int iupKeyNameToCode(const char *name);

/** Returns true if the key code can be changed by CAPSLOCK.
 * \ingroup key */
int iupKeyCanCaps(int code);

/** Calls a function for each defined key. 
 * \ingroup key */
void iupKeyForEach(void (*func)(const char *name, int code, void* user_data), void* user_data);

/** Calls the K_ANY or K_* callbacks. Should be called when a keyboard event occoured.
 * \ingroup key */
int iupKeyCallKeyCb(Ihandle *ih, int c);

/** Calls the KEYPRESS_CB callback. Should be called when a keyboard event occoured.
 * \ingroup key */
int iupKeyCallKeyPressCb(Ihandle *ih, int code, int press);

/** Process DEFAULTENTER and DEFAULTESC in key press events.
 * \ingroup key */
void iupKeyCallDefaultButtons(Ihandle* ih, int key);
                             
                             
/* Called only from IupOpen. */
void iupKeyInit(void);
                        
                        
#define IUPKEY_STATUS_SIZE 11 /* 10 chars + null */
#define IUPKEY_STATUS_INIT "          "  /* 10 spaces */
#define iupKEYSETSHIFT(_s)    (_s[0]='S')
#define iupKEYSETCONTROL(_s)  (_s[1]='C')
#define iupKEYSETBUTTON1(_s)  (_s[2]='1')
#define iupKEYSETBUTTON2(_s)  (_s[3]='2')
#define iupKEYSETBUTTON3(_s)  (_s[4]='3')
#define iupKEYSETDOUBLE(_s)   (_s[5]='D')
#define iupKEYSETALT(_s)      (_s[6]='A')
#define iupKEYSETSYS(_s)      (_s[7]='Y')
#define iupKEYSETBUTTON4(_s)  (_s[8]='4')
#define iupKEYSETBUTTON5(_s)  (_s[9]='5')


#ifdef __cplusplus
}
#endif

#endif
