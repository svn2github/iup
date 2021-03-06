/** \file
* \brief Draw Functions for DirectD2 using WinDrawLib
*
* See Copyright Notice in "iup.h"
*/

#include <windows.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>

#include "iup.h"

#include "iup_attrib.h"
#include "iup_class.h"
#include "iup_image.h"
#include "iup_drvdraw.h"
#include "iup_draw.h"
#include "iup_str.h"

#include "iupwin_drv.h"
#include "iupwin_info.h"
#include "iupwin_draw.h"
#include "iupwin_str.h"

#include "wdl.h"

/* From iupwin_image_wdl.c - can be used only here */
void iupwinWdlImageInit(void);
WD_HIMAGE iupwinWdlImageGetImage(const char* name, Ihandle* ih_parent, int make_inactive, const char* bgcolor);

/* From iupwin_draw_gdi.c */
IdrawCanvas* iupdrvDrawCreateCanvasGDI(Ihandle* ih);
void iupdrvDrawKillCanvasGDI(IdrawCanvas* dc);
void iupdrvDrawFlushGDI(IdrawCanvas* dc);
void iupdrvDrawUpdateSizeGDI(IdrawCanvas* dc);
void iupdrvDrawGetSizeGDI(IdrawCanvas* dc, int *w, int *h);
void iupdrvDrawLineGDI(IdrawCanvas* dc, int x1, int y1, int x2, int y2, long color, int style, int line_width);
void iupdrvDrawRectangleGDI(IdrawCanvas* dc, int x1, int y1, int x2, int y2, long color, int style, int line_width);
void iupdrvDrawArcGDI(IdrawCanvas* dc, int x1, int y1, int x2, int y2, double a1, double a2, long color, int style, int line_width);
void iupdrvDrawPolygonGDI(IdrawCanvas* dc, int* points, int count, long color, int style, int line_width);
void iupdrvDrawTextGDI(IdrawCanvas* dc, const char* text, int len, int x, int y, int w, int h, long color, const char* font, int flags, double text_orientation);
void iupdrvDrawImageGDI(IdrawCanvas* dc, const char* name, int make_inactive, const char* bgcolor, int x, int y, int w, int h);
void iupdrvDrawSetClipRectGDI(IdrawCanvas* dc, int x1, int y1, int x2, int y2);
void iupdrvDrawResetClipGDI(IdrawCanvas* dc);
void iupdrvDrawGetClipRectGDI(IdrawCanvas* dc, int *x1, int *y1, int *x2, int *y2);
void iupdrvDrawSelectRectGDI(IdrawCanvas* dc, int x1, int y1, int x2, int y2);
void iupdrvDrawFocusRectGDI(IdrawCanvas* dc, int x1, int y1, int x2, int y2);



struct _IdrawCanvas{
  Ihandle* ih;
  int w, h;

  HWND hWnd;
  WD_HCANVAS hCanvas;
  HDC hDC;

  IdrawCanvas* gdi_dc;

  int clip_x1, clip_y1, clip_x2, clip_y2;
};

/* must be the same in wdInitialize and wdTerminate */
const DWORD wdl_flags = WD_INIT_COREAPI | WD_INIT_IMAGEAPI | WD_INIT_STRINGAPI;

void iupwinDrawInit(void)
{
  iupwinDrawThemeInit();

#if 0
  wdPreInitialize(NULL, NULL, WD_DISABLE_D2D);  /* to force GDI+ */
#endif

  wdInitialize(wdl_flags);

  iupwinWdlImageInit();
}

void iupwinDrawFinish(void)
{
  wdTerminate(wdl_flags);
}

IdrawCanvas* iupdrvDrawCreateCanvas(Ihandle* ih)
{
  IdrawCanvas* dc = calloc(1, sizeof(IdrawCanvas));
  PAINTSTRUCT ps;
  RECT rect;
  int x, y, w, h;
  char *rcPaint;

  dc->ih = ih;

  if (iupAttribGetBoolean(ih, "DRAWUSEGDI"))
  {
    dc->gdi_dc = iupdrvDrawCreateCanvasGDI(ih);
    return dc;
  }

  dc->hWnd = (HWND)IupGetAttribute(ih, "HWND");  /* Use the attribute, so it can work with FileDlg preview area */

  GetClientRect(dc->hWnd, &rect);
  dc->w = rect.right - rect.left;
  dc->h = rect.bottom - rect.top;

  /* valid only inside the ACTION callback of an IupCanvas */
  rcPaint = iupAttribGetStr(ih, "CLIPRECT");
  if (rcPaint)
  {
    ps.hdc = (HDC)iupAttribGet(ih, "HDC_WMPAINT");
    dc->hDC = NULL;
    sscanf(rcPaint, "%d %d %d %d", &x, &y, &w, &h);
    ps.rcPaint.left = x;
    ps.rcPaint.top = y;
    ps.rcPaint.right = x + w;
    ps.rcPaint.bottom = y + h;
  }
  else
  {
    ps.hdc = GetDC(dc->hWnd);
    dc->hDC = ps.hdc;
    ps.rcPaint.left = 0;
    ps.rcPaint.top = 0;
    ps.rcPaint.right = dc->w;
    ps.rcPaint.bottom = dc->h;
  }

  dc->hCanvas = wdCreateCanvasWithPaintStruct(dc->hWnd, &ps, WD_CANVAS_DOUBLEBUFFER | WD_CANVAS_NOGDICOMPAT);

  if (wdBackend() == WD_BACKEND_D2D)
    iupAttribSet(ih, "DRAWDRIVER", "D2D");
  else
    iupAttribSet(ih, "DRAWDRIVER", "GDI+");

  wdBeginPaint(dc->hCanvas);

  return dc;
}

void iupdrvDrawKillCanvas(IdrawCanvas* dc)
{
  if (dc->gdi_dc)
  {
    iupdrvDrawKillCanvasGDI(dc->gdi_dc);

    memset(dc, 0, sizeof(IdrawCanvas));
    free(dc);
    return;
  }

  wdSetClip(dc->hCanvas, NULL, NULL); /* must reset clip before destroy */
  wdDestroyCanvas(dc->hCanvas);

  if (dc->hDC)
    ReleaseDC(dc->hWnd, dc->hDC);  /* to match GetDC */

  memset(dc, 0, sizeof(IdrawCanvas));
  free(dc);
}

void iupdrvDrawUpdateSize(IdrawCanvas* dc)
{
  int w, h;
  RECT rect;

  if (dc->gdi_dc)
  {
    iupdrvDrawUpdateSizeGDI(dc->gdi_dc);
    return;
  }

  GetClientRect(dc->hWnd, &rect);
  w = rect.right - rect.left;
  h = rect.bottom - rect.top;

  if (w != dc->w || h != dc->h)
  {
    dc->w = w;
    dc->h = h;

    wdResizeCanvas(dc->hCanvas, w, h);
  }
}

void iupdrvDrawFlush(IdrawCanvas* dc)
{
  if (dc->gdi_dc)
  {
    iupdrvDrawFlushGDI(dc->gdi_dc);
    return;
  }

  wdEndPaint(dc->hCanvas);
}

void iupdrvDrawGetSize(IdrawCanvas* dc, int *w, int *h)
{
  if (dc->gdi_dc)
  {
    iupdrvDrawGetSizeGDI(dc->gdi_dc, w, h);
    return;
  }

  if (w) *w = dc->w;
  if (h) *h = dc->h;
}

static WD_HSTROKESTYLE iDrawSetLineStyle(int style)
{
  if (style == IUP_DRAW_STROKE_DASH)
  {
    float dashes[2] = { 9.0f, 3.0f };
    return wdCreateStrokeStyleCustom(dashes, 2, WD_LINECAP_FLAT, WD_LINEJOIN_MITER);
  }
  else if (style == IUP_DRAW_STROKE_DOT)
  {
    float dashes[2] = { 1.0f, 2.0f };
    return wdCreateStrokeStyleCustom(dashes, 2, WD_LINECAP_FLAT, WD_LINEJOIN_MITER);
  }
  else if (style == IUP_DRAW_STROKE_DASH_DOT)
  {
    float dashes[4] = { 7.0f, 3.0f, 1.0f, 3.0f };
    return wdCreateStrokeStyleCustom(dashes, 4, WD_LINECAP_FLAT, WD_LINEJOIN_MITER);
  }
  else if (style == IUP_DRAW_STROKE_DASH_DOT_DOT)
  {
    float dashes[6] = { 7.0f, 3.0f, 1.0f, 3.0f, 1.0f, 3.0f };
    return wdCreateStrokeStyleCustom(dashes, 6, WD_LINECAP_FLAT, WD_LINEJOIN_MITER);
  }
  else
    return NULL;
}

static void iDrawRelaseStyle(WD_HSTROKESTYLE stroke_style)
{
  if (stroke_style)
    wdDestroyStrokeStyle(stroke_style);
}

#define iupInt2Float(_x) ((float)_x)
#define iupInt2FloatW(_x) ((float)_x)
#define iupColor2ARGB(_c) WD_ARGB(iupDrawAlpha(_c), iupDrawRed(_c), iupDrawGreen(_c), iupDrawBlue(_c))

void iupdrvDrawRectangle(IdrawCanvas* dc, int x1, int y1, int x2, int y2, long color, int style, int line_width)
{
  WD_HBRUSH brush;

  if (dc->gdi_dc)
  {
    iupdrvDrawRectangleGDI(dc->gdi_dc, x1, y1, x2, y2, color, style, line_width);
    return;
  }

  brush = wdCreateSolidBrush(dc->hCanvas, iupColor2ARGB(color));

  iupDrawCheckSwapCoord(x1, x2);
  iupDrawCheckSwapCoord(y1, y2);

  if (style == IUP_DRAW_FILL)
    wdFillRect(dc->hCanvas, brush, iupInt2Float(x1 - 0.5f), iupInt2Float(y1 - 0.5f), iupInt2Float(x2 + 0.5f), iupInt2Float(y2 + 0.5f));
  else
  {
    WD_HSTROKESTYLE stroke_style = iDrawSetLineStyle(style);
    wdDrawRectStyled(dc->hCanvas, brush, iupInt2Float(x1), iupInt2Float(y1), iupInt2Float(x2), iupInt2Float(y2), iupInt2FloatW(line_width), stroke_style);
    iDrawRelaseStyle(stroke_style);
  }

  wdDestroyBrush(brush);
}

void iupdrvDrawLine(IdrawCanvas* dc, int x1, int y1, int x2, int y2, long color, int style, int line_width)
{
  WD_HBRUSH brush;
  WD_HSTROKESTYLE stroke_style;

  if (dc->gdi_dc)
  {
    iupdrvDrawLineGDI(dc->gdi_dc, x1, y1, x2, y2, color, style, line_width);
    return;
  }

  brush = wdCreateSolidBrush(dc->hCanvas, iupColor2ARGB(color));
  stroke_style = iDrawSetLineStyle(style);
  if (wdBackend() == WD_BACKEND_D2D && line_width == 1)
  {
    /* compensate Direct2D horizontal and vertical lines when line_width == 1 */
    if (x1 == x2)
      wdDrawLineStyled(dc->hCanvas, brush, iupInt2Float(x1), iupInt2Float(y1 - 0.5f), iupInt2Float(x2), iupInt2Float(y2 + 0.5f), iupInt2FloatW(line_width), stroke_style);
    else if (y1 == y2)
      wdDrawLineStyled(dc->hCanvas, brush, iupInt2Float(x1 - 0.5f), iupInt2Float(y1), iupInt2Float(x2 + 0.5f), iupInt2Float(y2), iupInt2FloatW(line_width), stroke_style);
    else
      wdDrawLineStyled(dc->hCanvas, brush, iupInt2Float(x1), iupInt2Float(y1), iupInt2Float(x2), iupInt2Float(y2), iupInt2FloatW(line_width), stroke_style);
  }
  else
    wdDrawLineStyled(dc->hCanvas, brush, iupInt2Float(x1), iupInt2Float(y1), iupInt2Float(x2), iupInt2Float(y2), iupInt2FloatW(line_width), stroke_style);
  iDrawRelaseStyle(stroke_style);
  wdDestroyBrush(brush);
}

void iupdrvDrawArc(IdrawCanvas* dc, int x1, int y1, int x2, int y2, double a1, double a2, long color, int style, int line_width)
{
  float xc, yc, rx, ry;
  float baseAngle, sweepAngle;
  WD_HBRUSH brush;

  if (dc->gdi_dc)
  {
    iupdrvDrawArcGDI(dc->gdi_dc, x1, y1, x2, y2, a1, a2, color, style, line_width);
    return;
  }

  brush = wdCreateSolidBrush(dc->hCanvas, 0);
  wdSetSolidBrushColor(brush, iupColor2ARGB(color));

  iupDrawCheckSwapCoord(x1, x2);
  iupDrawCheckSwapCoord(y1, y2);

  rx = (x2 - x1) / 2.0f;
  ry = (y2 - y1) / 2.0f;

  xc = iupInt2Float(x1) + rx;
  yc = iupInt2Float(y1) + ry;

  baseAngle = (float)(360.0 - a2);
  sweepAngle = (float)(a2 - a1);

  if (style == IUP_DRAW_FILL)
  {
    if (sweepAngle == 360.0f)
      wdFillEllipse(dc->hCanvas, brush, xc, yc, rx, ry);
    else
      wdFillEllipsePie(dc->hCanvas, brush, xc, yc, rx, ry, baseAngle, sweepAngle);
  }
  else
  {
    WD_HSTROKESTYLE stroke_style = iDrawSetLineStyle(style);
    if (sweepAngle == 360.0f)
      wdDrawEllipseStyled(dc->hCanvas, brush, xc, yc, rx, ry, iupInt2FloatW(line_width), stroke_style);
    else
      wdDrawEllipseArcStyled(dc->hCanvas, brush, xc, yc, rx, ry, baseAngle, sweepAngle, iupInt2FloatW(line_width), stroke_style);
    iDrawRelaseStyle(stroke_style);
  }

  wdDestroyBrush(brush);
}

void iupdrvDrawPolygon(IdrawCanvas* dc, int* points, int count, long color, int style, int line_width)
{
  WD_HBRUSH brush;
  WD_HPATH path;
  WD_PATHSINK sink;
  int i;

  if (dc->gdi_dc)
  {
    iupdrvDrawPolygonGDI(dc->gdi_dc, points, count, color, style, line_width);
    return;
  }

  brush = wdCreateSolidBrush(dc->hCanvas, iupColor2ARGB(color));

  path = wdCreatePath(dc->hCanvas);
  wdOpenPathSink(&sink, path);
  wdBeginFigure(&sink, iupInt2Float(points[0]), iupInt2Float(points[1]));

  for (i = 2; i < count * 2; i = i + 2)
    wdAddLine(&sink, iupInt2Float(points[i]), iupInt2Float(points[i + 1]));

  wdEndFigure(&sink, FALSE);
  wdClosePathSink(&sink);

  if (style == IUP_DRAW_FILL)
    wdFillPath(dc->hCanvas, brush, path);
  else
  {
    WD_HSTROKESTYLE stroke_style = iDrawSetLineStyle(style);
    wdDrawPathStyled(dc->hCanvas, brush, path, iupInt2FloatW(line_width), stroke_style);
    iDrawRelaseStyle(stroke_style);
  }

  wdDestroyPath(path);
  wdDestroyBrush(brush);
}

void iupdrvDrawGetClipRect(IdrawCanvas* dc, int *x1, int *y1, int *x2, int *y2)
{
  if (dc->gdi_dc)
  {
    iupdrvDrawGetClipRectGDI(dc->gdi_dc, x1, y1, x2, y2);
    return;
  }

  if (x1) *x1 = dc->clip_x1;
  if (y1) *y1 = dc->clip_y1;
  if (x2) *x2 = dc->clip_x2;
  if (y2) *y2 = dc->clip_y2;
}

void iupdrvDrawSetClipRect(IdrawCanvas* dc, int x1, int y1, int x2, int y2)
{
  WD_RECT rect;

  if (dc->gdi_dc)
  {
    iupdrvDrawSetClipRectGDI(dc->gdi_dc, x1, y1, x2, y2);
    return;
  }

  if (x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0)
  {
    iupdrvDrawResetClip(dc);
    return;
  }

  /* make it an empty region */
  if (x1 >= x2) x1 = x2;
  if (y1 >= y2) y1 = y2;

  if (x1 == x2 || y1 == y2)
  {
    rect.x0 = iupInt2Float(x1);
    rect.y0 = iupInt2Float(y1);
    rect.x1 = iupInt2Float(x2);
    rect.y1 = iupInt2Float(y2);
  }
  else
  {
    rect.x0 = iupInt2Float(x1 - 0.5f);
    rect.y0 = iupInt2Float(y1 - 0.5f);
    rect.x1 = iupInt2Float(x2 + 0.5f);
    rect.y1 = iupInt2Float(y2 + 0.5f);
  }

  wdSetClip(dc->hCanvas, &rect, NULL);

  dc->clip_x1 = x1;
  dc->clip_y1 = y1;
  dc->clip_x2 = x2;
  dc->clip_y2 = y2;
}

void iupdrvDrawResetClip(IdrawCanvas* dc)
{
  if (dc->gdi_dc)
  {
    iupdrvDrawResetClipGDI(dc->gdi_dc);
    return;
  }

  wdSetClip(dc->hCanvas, NULL, NULL);

  dc->clip_x1 = 0;
  dc->clip_y1 = 0;
  dc->clip_x2 = 0;
  dc->clip_y2 = 0;
}

static int iCompensatePosX(float font_height)
{
  return iupRound(font_height / 7.);  /* 15% */
}

void iupdrvDrawText(IdrawCanvas* dc, const char* text, int len, int x, int y, int w, int h, long color, const char* font, int flags, double text_orientation)
{
  if (dc->gdi_dc)
    iupdrvDrawTextGDI(dc->gdi_dc, text, len, x, y, w, h, color, font, flags, text_orientation);
  else
  {
    WD_RECT rect;
    DWORD dwFlags = WD_STR_TOPALIGN;
    WCHAR *wtext = iupwinStrToSystemLen(text, &len);
    WD_HBRUSH brush = wdCreateSolidBrush(dc->hCanvas, iupColor2ARGB(color));
    int layout_center = flags & IUP_DRAW_LAYOUTCENTER;

    HFONT hFont = (HFONT)iupwinGetHFont(font);
    WD_HFONT wdFont = wdCreateFontWithGdiHandle(hFont);
    int layout_w = w, layout_h = h;

    if (text_orientation && layout_center)
      iupDrawGetTextInnerBounds(w, h, text_orientation, &layout_w, &layout_h);

    rect.x0 = iupInt2Float(x);
    rect.x1 = iupInt2Float(x + layout_w);
    rect.y0 = iupInt2Float(y);
    rect.y1 = iupInt2Float(y + layout_h);

    dwFlags |= WD_STR_LEFTALIGN;
    if (flags & IUP_DRAW_RIGHT)
      dwFlags |= WD_STR_RIGHTALIGN;
    else if (flags & IUP_DRAW_CENTER)
      dwFlags |= WD_STR_CENTERALIGN;

    if (!(flags & IUP_DRAW_WRAP))
    {
      dwFlags |= WD_STR_NOWRAP;

      if (flags & IUP_DRAW_ELLIPSIS)
        dwFlags |= WD_STR_ENDELLIPSIS;
    }

    if (wdBackend() == WD_BACKEND_GDIPLUS)
    {
      /* compensate GDI+ internal padding */
      WD_FONTMETRICS metrics;
      wdFontMetrics(wdFont, &metrics);
      rect.x0 -= iCompensatePosX(metrics.fLeading);
    }

    if (text_orientation)
    {
      if (layout_center)
      {
        wdRotateWorld(dc->hCanvas, iupInt2Float(x + layout_w / 2), iupInt2Float(y + layout_h / 2), (float)-text_orientation);  /* counterclockwise */
        wdTranslateWorld(dc->hCanvas, iupInt2Float(w - layout_w) / 2, iupInt2Float(h - layout_h) / 2);  /* append the transform */
      }
      else
        wdRotateWorld(dc->hCanvas, iupInt2Float(x), iupInt2Float(y), (float)-text_orientation);  /* counterclockwise */
    }

    if (!(flags & IUP_DRAW_CLIP))
      dwFlags |= WD_STR_NOCLIP;

    wdDrawString(dc->hCanvas, wdFont, &rect, wtext, len, brush, dwFlags);

    wdDestroyBrush(brush);
    wdDestroyFont(wdFont);

    /* restore settings */
    if (text_orientation)
      wdResetWorld(dc->hCanvas);
  }
}

void iupdrvDrawImage(IdrawCanvas* dc, const char* name, int make_inactive, const char* bgcolor, int x, int y, int w, int h)
{
  WD_HIMAGE hImage;

  if (dc->gdi_dc)
  {
    iupdrvDrawImageGDI(dc->gdi_dc, name, make_inactive, bgcolor, x, y, w, h);
    return;
  }

  hImage = iupwinWdlImageGetImage(name, dc->ih, make_inactive, bgcolor);
  if (hImage)
  {
    UINT img_w, img_h;
    WD_RECT rect;

    wdGetImageSize(hImage, &img_w, &img_h);

    if (w == -1 || w == 0) w = img_w;
    if (h == -1 || h == 0) h = img_h;

    rect.x0 = iupInt2Float(x);
    rect.y0 = iupInt2Float(y);
    rect.x1 = iupInt2Float(x + w);
    rect.y1 = iupInt2Float(y + h);

    wdBitBltImage(dc->hCanvas, hImage, &rect, NULL);
  }
}

void iupdrvDrawSelectRect(IdrawCanvas* dc, int x1, int y1, int x2, int y2)
{
  WD_HBRUSH brush;

  if (dc->gdi_dc)
  {
    iupdrvDrawSelectRectGDI(dc->gdi_dc, x1, y1, x2, y2);
    return;
  }

  iupDrawCheckSwapCoord(x1, x2);
  iupDrawCheckSwapCoord(y1, y2);

  brush = wdCreateSolidBrush(dc->hCanvas, WD_ARGB(153, 0, 0, 255));
  wdFillRect(dc->hCanvas, brush, iupInt2Float(x1 - 0.5f), iupInt2Float(y1 - 0.5f), iupInt2Float(x2 + 0.5f), iupInt2Float(y2 + 0.5f));
  wdDestroyBrush(brush);
}

void iupdrvDrawFocusRect(IdrawCanvas* dc, int x1, int y1, int x2, int y2)
{
#if 0
  RECT rect;
  HDC hDC;

  iupDrawCheckSwapCoord(x1, x2);
  iupDrawCheckSwapCoord(y1, y2);

  rect.left = x1;
  rect.right = x2;
  rect.top = y1;
  rect.bottom = y2;

  hDC = wdStartGdi(dc->hCanvas, TRUE);  /* TODO wdStartGdi is crashing in D2D */
  DrawFocusRect(hDC, &rect);
  wdEndGdi(dc->hCanvas, hDC);
#else
  if (dc->gdi_dc)
  {
    iupdrvDrawFocusRectGDI(dc->gdi_dc, x1, y1, x2, y2);
    return;
  }

  iupdrvDrawRectangle(dc, x1, y1, x2, y2, iupDrawColor(0, 0, 0, 224), IUP_DRAW_STROKE_DOT, 1);
#endif
}
