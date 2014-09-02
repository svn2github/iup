/** \file
 * \brief GL Drawing functions.
 *
 * See Copyright Notice in "iup.h"
 */

#ifdef WIN32
#include <windows.h>
#endif

#if defined (__APPLE__) || defined (OSX)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "iup.h"

#include "iup_assert.h"
#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_array.h"
#include "iup_image.h"

#include "iup_glcontrols.h"
#include "iup_glfont.h"
#include "iup_gldraw.h"
#include "iup_glicon.h"
#include "iup_glimage.h"


void iupGLDrawLine(Ihandle* ih, int x1, int y1, int x2, int y2, float linewidth, const char* color, int active)
{
  unsigned char r = 0, g = 0, b = 0, a = 255;

  if (linewidth == 0)
    return;

  iupStrToRGBA(color, &r, &g, &b, &a);
  if (color && !active)
    iupGLColorMakeInactive(&r, &g, &b);

  glColor4ub(r, g, b, a);

  glLineWidth(linewidth);

  /* y is oriented top to bottom in IUP */
  y1 = ih->currentheight - 1 - y1;
  y2 = ih->currentheight - 1 - y2;

  glBegin(GL_LINES);
  glVertex2i(x1, y1);
  glVertex2i(x2, y2);
  glEnd();
}

void iupGLDrawFrameRect(Ihandle* ih, int xmin, int xmax, int ymin, int ymax, float linewidth, const char* color, int active, int title_x, int title_width, int title_height)
{
  unsigned char r = 0, g = 0, b = 0, a = 255;
  int d = 2;

  if (linewidth == 0 || xmin == xmax || ymin == ymax)
    return;

  if (xmin > xmax) { int _t = xmin; xmin = xmax; xmax = _t; }
  if (ymin > ymax) { int _t = ymin; ymin = ymax; ymax = _t; }

  iupStrToRGBA(color, &r, &g, &b, &a);
  if (color && !active)
    iupGLColorMakeInactive(&r, &g, &b);

  glColor4ub(r, g, b, a);

  glLineWidth(linewidth);

  /* y is oriented top to bottom in IUP */
  ymin = ih->currentheight - 1 - ymin;
  ymax = ih->currentheight - 1 - ymax;

  /* position frame title at left-center */
  ymin -= title_height / 2;

  glBegin(GL_LINE_STRIP);

  glVertex2i(xmin + title_x + title_width, ymin);

  glVertex2i(xmax - d, ymin);
  glVertex2i(xmax, ymin - d);

  glVertex2i(xmax, ymax + d);
  glVertex2i(xmax - d, ymax);

  glVertex2i(xmin + d, ymax);
  glVertex2i(xmin, ymax + d);

  glVertex2i(xmin, ymin - d);
  glVertex2i(xmin + d, ymin);

  glVertex2i(xmin + title_x, ymin);

  glEnd();
}

void iupGLDrawRect(Ihandle* ih, int xmin, int xmax, int ymin, int ymax, float linewidth, const char* color, int active, int round)
{
  unsigned char r = 0, g = 0, b = 0, a = 255;

  if (linewidth == 0 || xmin == xmax || ymin == ymax)
    return;

  if (xmin > xmax) { int _t = xmin; xmin = xmax; xmax = _t; }
  if (ymin > ymax) { int _t = ymin; ymin = ymax; ymax = _t; }

  iupStrToRGBA(color, &r, &g, &b, &a);
  if (color && !active)
    iupGLColorMakeInactive(&r, &g, &b);

  glColor4ub(r, g, b, a);

  glLineWidth(linewidth);

  /* y is oriented top to bottom in IUP */
  ymin = ih->currentheight - 1 - ymin;
  ymax = ih->currentheight - 1 - ymax;

  glBegin(GL_LINE_LOOP);

  if (round)
  {
    int r = 2;

    glVertex2i(xmin, ymin-r);
    glVertex2i(xmin+r, ymin);

    glVertex2i(xmax-r, ymin);
    glVertex2i(xmax, ymin-r);

    glVertex2i(xmax, ymax+r);
    glVertex2i(xmax-r, ymax);

    glVertex2i(xmin+r, ymax);
    glVertex2i(xmin, ymax+r);
  }
  else
  {
    glVertex2i(xmin, ymin);
    glVertex2i(xmax, ymin);
    glVertex2i(xmax, ymax);
    glVertex2i(xmin, ymax);
  }

  glEnd();
}

static void iGLDrawBuildSmallCircle(int cx, int cy, int rd)
{
  /* Reference: http://slabode.exofire.net/circle_draw.shtml
  Copyright SiegeLord's Abode */
  int i, num_segments = 16;
  double theta = 2 * 3.1415926 / (double)num_segments;
  double c = cos(theta);  /* precalculate the sine and cosine */
  double s = sin(theta);
  double t, x, y;

  x = rd;  /* we start at angle = 0 */
  y = 0;

  for (i = 0; i < num_segments; i++)
  {
    glVertex2d(x + cx, y + cy);

    /* apply the rotation matrix */
    t = x;
    x = c * x - s * y;
    y = s * t + c * y;
  }
}

void iupGLDrawSmallCircle(Ihandle* ih, int cx, int cy, int rd, float linewidth, const char* color, int active)
{
  unsigned char r = 0, g = 0, b = 0, a = 255;

  if (linewidth == 0 || rd == 0)
    return;

  iupStrToRGBA(color, &r, &g, &b, &a);
  if (color && !active)
    iupGLColorMakeInactive(&r, &g, &b);

  glColor4ub(r, g, b, a);

  glLineWidth(linewidth);

  /* y is oriented top to bottom in IUP */
  cy = ih->currentheight - 1 - cy;

  glBegin(GL_LINE_LOOP);

  iGLDrawBuildSmallCircle(cx, cy, rd);

  glEnd();
}

void iupGLDrawSmallDisc(Ihandle* ih, int cx, int cy, int rd, const char* color, int active)
{
  unsigned char r = 0, g = 0, b = 0, a = 255;

  if (rd == 0)
    return;

  iupStrToRGBA(color, &r, &g, &b, &a);
  if (color && !active)
    iupGLColorMakeInactive(&r, &g, &b);

  glColor4ub(r, g, b, a);

  /* y is oriented top to bottom in IUP */
  cy = ih->currentheight - 1 - cy;

  glBegin(GL_POLYGON);

  iGLDrawBuildSmallCircle(cx, cy, rd);

  glEnd();
}

void iupGLDrawBox(Ihandle* ih, int xmin, int xmax, int ymin, int ymax, const char* color, int active)
{
  unsigned char r = 0, g = 0, b = 0, a = 255;

  if (!color || xmin == xmax || ymin == ymax)
    return;

  iupStrToRGBA(color, &r, &g, &b, &a);
  if (color && !active)
    iupGLColorMakeInactive(&r, &g, &b);

  glColor4ub(r, g, b, a);

  /* y is oriented top to bottom in IUP */
  ymin = ih->currentheight - 1 - ymin;
  ymax = ih->currentheight - 1 - ymax;

  glRecti(xmin, ymax, xmax + 1, ymin + 1);
}

void iupGLDrawPolygon(Ihandle* ih, const int* points, int count, const char* color, int active)
{
  unsigned char r = 0, g = 0, b = 0, a = 255;
  int i, x, y;

  if (!color || count < 3)
    return;

  iupStrToRGBA(color, &r, &g, &b, &a);
  if (color && !active)
    iupGLColorMakeInactive(&r, &g, &b);

  glColor4ub(r, g, b, a);

  glBegin(GL_POLYGON);

  for (i = 0; i < count; i++)
  {
    x = points[2 * i + 0];
    y = points[2 * i + 1];

    /* y is oriented top to bottom in IUP */
    y = ih->currentheight - 1 - y;

    glVertex2i(x, y);
  }

  glEnd();
}

void iupGLDrawPolyline(Ihandle* ih, const int* points, int count, float linewidth, const char* color, int active, int loop)
{
  unsigned char r = 0, g = 0, b = 0, a = 255;
  int i, x, y;

  if (!color || count < 3)
    return;

  iupStrToRGBA(color, &r, &g, &b, &a);
  if (color && !active)
    iupGLColorMakeInactive(&r, &g, &b);

  glColor4ub(r, g, b, a);

  glLineWidth(linewidth);

  if (loop)
    glBegin(GL_LINE_LOOP);
  else
    glBegin(GL_LINE_STRIP);

  for (i = 0; i < count; i++)
  {
    x = points[2 * i + 0];
    y = points[2 * i + 1];

    /* y is oriented top to bottom in IUP */
    y = ih->currentheight - 1 - y;

    glVertex2i(x, y);
  }

  glEnd();
}

void iupGLDrawText(Ihandle* ih, int x, int y, const char* str, const char* color, int active)
{
  unsigned char r = 0, g = 0, b = 0, a = 255;

  if (!str)
    return;

  iupStrToRGBA(color, &r, &g, &b, &a);
  if (color && !active)
    iupGLColorMakeInactive(&r, &g, &b);

  glColor4ub(r, g, b, a);

  if (str[0])
  {
    int len, lineheight, ascent, baseline;
    const char *nextstr;
    const char *curstr = str;
    int underline = iupAttribGetInt(ih, "UNDERLINE");

    iupGLFontGetDim(ih, NULL, &lineheight, &ascent, NULL);
    baseline = lineheight - ascent;

    /* y is at text baseline and oriented bottom to top in OpenGL */
    y = y + lineheight - baseline;  /* move to baseline */

    /* y is oriented top to bottom in IUP */
    y = ih->currentheight - 1 - y;

    if (underline)
      glLineWidth(1.0f);

    glPushMatrix();
    glTranslated((double)x, (double)y, 0.0);

    do
    {
      nextstr = iupStrNextLine(curstr, &len);
      if (len)
      {
        iupGLFontRenderString(ih, curstr, len);

        if (underline)
        {
          int width = iupGLFontGetStringWidth(ih, curstr, len);
          glBegin(GL_LINES);
          glVertex2i(0, -2);
          glVertex2i(width-1, -2);
          glEnd();
        }
      }

      glTranslated(0.0, (double)-lineheight, 0.0);

      curstr = nextstr;
    } while (*nextstr);

    glPopMatrix();
  }
}

void iupGLDrawImage(Ihandle* ih, int x, int y, const char* name, int active)
{
  Ihandle* image = iupGLIconGetImageHandle(ih, name, active);
  if (image)
  {
    unsigned char* gldata = iupGLImageGetData(image, active);
    int depth = iupAttribGetInt(image, "GL_DEPTH");
    int format = GL_RGB;
    if (depth == 4)
      format = GL_RGBA;

    /* y is at image bottom and oriented bottom to top in OpenGL */
    y = y + image->currentheight - 1;  /* move to bottom */

    /* y is oriented top to bottom in IUP */
    y = ih->currentheight - 1 - y;

    glRasterPos2i(x, y);
    glDrawPixels(image->currentwidth, image->currentheight, format, GL_UNSIGNED_BYTE, gldata);
  }
}

void iupGLDrawArrow(Ihandle *ih, int x, int y, int size, const char* color, int active, int dir)
{
  int points[6];

  int off1 = iupRound((double)size * 0.13);
  int off2   = iupRound((double)size * 0.87);
  int half = size / 2;

  switch (dir)
  {
  case IUPGL_ARROW_LEFT:  /* arrow points left */
    points[0] = x + off2;
    points[1] = y;
    points[2] = x + off2;
    points[3] = y + size;
    points[4] = x + off1;
    points[5] = y + half;
    break;
  case IUPGL_ARROW_TOP:    /* arrow points top */
    points[0] = x;
    points[1] = y + off2;
    points[2] = x + size;
    points[3] = y + off2;
    points[4] = x + half;
    points[5] = y + off1;
    break;
  case IUPGL_ARROW_RIGHT:  /* arrow points right */
    points[0] = x + off1;
    points[1] = y;
    points[2] = x + off1;
    points[3] = y + size;
    points[4] = x + size - off1;
    points[5] = y + half;
    break;
  case IUPGL_ARROW_BOTTOM:  /* arrow points bottom */
    points[0] = x;
    points[1] = y + off1;
    points[2] = x + size;
    points[3] = y + off1;
    points[4] = x + half;
    points[5] = y + size - off1;
    break;
  }

  iupGLDrawPolygon(ih, points, 3, color, active);
  iupGLDrawPolyline(ih, points, 3, 1, color, active, 1);
}

/* isPowerOfTwo By Rick Regan
   http://www.exploringbinary.com/ten-ways-to-check-if-an-integer-is-a-power-of-two-in-c/
*/
static int isPowerOfTwo(unsigned int x)
{
  while (((x & 1) == 0) && x > 1) /* While x is even and > 1 */
    x >>= 1;
  return (x == 1);
}

static int iGLDestroyTexture_CB(Ihandle* image)
{
  GLuint texture = (GLuint)iupAttribGetInt(image, "GL_TEXTURE");
  if (texture)
    glDeleteTextures(1, &texture);

  texture = (GLuint)iupAttribGetInt(image, "GL_TEXTURE_INACTIVE");
  if (texture)
    glDeleteTextures(1, &texture);

  return IUP_DEFAULT;
}

static GLuint iGLDrawGenTexture(Ihandle* ih, const char* name, int active)
{
  Ihandle* image = iupGLIconGetImageHandle(ih, name, active);
  if (image)
  {
    GLuint texture;
    const char *texture_name = "GL_TEXTURE";
    int make_inactive = !active && iupAttribGetInt(ih, "MAKEINACTIVE");
    if (make_inactive)
      texture_name = "GL_TEXTURE_INACTIVE";

    texture = (GLuint)iupAttribGetInt(image, texture_name);
    if (texture)
      return texture;

    if (isPowerOfTwo(image->currentwidth) && isPowerOfTwo(image->currentheight))
    {
      unsigned char* gldata = iupGLImageGetData(image, 1);
      int depth = iupAttribGetInt(image, "GL_DEPTH");
      int format = GL_RGB;
      if (depth == 4)
        format = GL_RGBA;

      glGenTextures(1, &texture);

      glBindTexture(GL_TEXTURE_2D, texture);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

      /* width and height must be 2^n */
      glTexImage2D(GL_TEXTURE_2D, 0, format, image->currentwidth, image->currentheight, 0, format, GL_UNSIGNED_BYTE, gldata);

      iupAttribSetInt(image, texture_name, (int)texture);
      IupSetCallback(image, "DESTROY_CB", iGLDestroyTexture_CB);

      return texture;
    }
  }

  return 0;
}

void iupGLDrawImageTexture(Ihandle *ih, int xmin, int xmax, int ymin, int ymax, const char* name, const char* color, int active)
{
  GLuint texture;
  unsigned char r = 0, g = 0, b = 0, a = 255;

  if (!color || xmin == xmax || ymin == ymax)
    return;

  iupStrToRGBA(color, &r, &g, &b, &a);
  if (color && !active)
    iupGLColorMakeInactive(&r, &g, &b);

  /* y is oriented top to bottom in IUP */
  ymin = ih->currentheight - 1 - ymin;
  ymax = ih->currentheight - 1 - ymax;

  texture = iGLDrawGenTexture(ih, name, active);
  if (texture)
  {
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glColor4f(r, g, b, a);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2i(xmin, ymin);
    glTexCoord2f(1.0f, 0.0f); glVertex2i(xmax, ymin);
    glTexCoord2f(1.0f, 1.0f); glVertex2i(xmax, ymax);
    glTexCoord2f(0.0f, 1.0f); glVertex2i(xmin, ymax);
    glEnd();

    glDisable(GL_TEXTURE_2D);
  }
}
