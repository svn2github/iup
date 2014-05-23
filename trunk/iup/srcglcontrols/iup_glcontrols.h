/** \file
 * \brief GL Controls Class Initialization functions.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUP_GLCONTROLS_H 
#define __IUP_GLCONTROLS_H


#ifdef __cplusplus
extern "C" {
#endif


Iclass* iupGLCanvasBoxNewClass(void);
Iclass* iupGLSubCanvasNewClass(void);
Iclass* iupGLLabelNewClass(void);
Iclass* iupGLSeparatorNewClass(void);
Iclass* iupGLButtonNewClass(void);

void iupGLSubCanvasSetTransform(Ihandle* ih, Ihandle* gl_parent);
void iupGLSubCanvasSaveState(void);
void iupGLSubCanvasRestoreState(Ihandle* gl_parent);
void iupGLSubCanvasRedrawFront(Ihandle* ih);
void iupGLSubCanvasUpdateSizeFromFont(Ihandle* ih);
int iupGLSubCanvasRedraw(Ihandle* ih);

void iupGLImageGetInfo(const char* name, int *w, int *h, int *bpp);
unsigned char* iupGLImageGetData(Ihandle* ih, int active);
void iupGLColorMakeInactive(unsigned char *r, unsigned char *g, unsigned char *b);

void iupGLFontGetMultiLineStringSize(Ihandle* ih, const char* str, int *w, int *h);
int iupGLSetStandardFontAttrib(Ihandle* ih, const char* value);
void iupGLFontGetCharSize(Ihandle* ih, int *charwidth, int *charheight);
void iupGLFontRenderString(Ihandle* ih, const char* str, int len);
void iupGLFontInit(void);
void iupGLFontFinish(void);
void iupGLFontRelease(Ihandle* gl_parent);
void iupGLFontGetDim(Ihandle* ih, int *maxwidth, int *height, int *ascent, int *descent);

void iupGLDrawText(Ihandle* ih, int x, int y, const char* str, const char* color, int active);
void iupGLDrawImage(Ihandle* ih, int x, int y, const char* name, int active);
void iupGLDrawLine(Ihandle* ih, int x1, int y1, int x2, int y2, float width, const char* color, int active);
void iupGLDrawRect(Ihandle* ih, int xmin, int xmax, int ymin, int ymax, float width, const char* color, int active);
void iupGLDrawBox(Ihandle* ih, int xmin, int xmax, int ymin, int ymax, const char* color);

void iupGLIconRegisterAttrib(Iclass* ic);
void iupGLIconDraw(Ihandle* ih, int icon_width, int icon_height, const char *image, const char* title, const char* fgcolor, int active);
void iupGLIconGetNaturalSize(Ihandle* ih, const char* image, const char* title, int *w, int *h);
Ihandle* iupGLIconGetImageHandle(Ihandle* ih, const char* name, int active);



#ifdef __cplusplus
}
#endif

#endif
