/***************************************************************************
 * define.h is part of Math Graphic Library
 * Copyright (C) 2007-2014 Alexey Balakin <mathgl.abalakin@gmail.ru>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef _MGL_DEFINE_H_
#define _MGL_DEFINE_H_
//-----------------------------------------------------------------------------
#include "mgl2/config.h"
#ifndef SWIG

#include "mgl2/dllexport.h"
#ifdef MGL_SRC
#if MGL_HAVE_OMP
#include <omp.h>
#endif
#endif

#endif

#define MGL_VER2 	2.2	// minor version of MathGL 2.* (like 1.3 for v.2.1.3)
//-----------------------------------------------------------------------------
#ifdef WIN32 //_MSC_VER needs this before math.h
#define	_USE_MATH_DEFINES
#endif

#ifdef MGL_SRC
#if MGL_HAVE_ZLIB
#include <zlib.h>
#ifndef Z_BEST_COMPRESSION
#define Z_BEST_COMPRESSION 9
#endif
#else
#define gzFile	FILE*
#define gzread(fp,buf,size)	fread(buf,1,size,fp)
#define gzopen	fopen
#define gzclose	fclose
#define gzprintf	fprintf
#define gzgets(fp,str,size)	fgets(str,size,fp)
#define gzgetc	fgetc
#endif
#endif

#if (defined(_MSC_VER) && (_MSC_VER<1600)) || defined(__BORLANDC__)
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed long int32_t;
typedef signed long long int64_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef unsigned long long uint64_t;
#else
#include <stdint.h>
#endif
#if defined(__BORLANDC__)
typedef unsigned long uintptr_t;
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#if (defined(_MSC_VER) && _MSC_VER < 1800) || defined(__BORLANDC__)
#define fmin(a,b)	((a)<(b))?(a):(b)
#define fmax(a,b)	((a)>(b))?(a):(b)
#endif

#if defined(_MSC_VER)
#define collapse(a)	// MSVS don't support OpenMP 3.*
#define strtoull _strtoui64
#define getcwd	_getcwd
#define chdir	_chdir // BORLAND has chdir
#define snprintf _snprintf
#if (_MSC_VER < 1800)
#define hypot	_hypot
#endif
#endif

#if defined(_MSC_VER) || defined(__BORLANDC__)
#include <float.h>
#include <math.h>

#ifdef WIN32
const unsigned long long mgl_nan[2] = {0x7fffffffffffffff, 0x7fffffff};
const unsigned long long mgl_inf[2] = {0x7ff0000000000000, 0x7f800000};
#else
const unsigned long mgl_nan[2] = {0x7fffffffffffffff, 0x7fffffff};
const unsigned long mgl_inf[2] = {0x7ff0000000000000, 0x7f800000};
#endif

#define NANd    (*(double*)mgl_nan)
#define NANf    (*(float*)(mgl_nan+1))
#define INFd    (*(double*)mgl_inf)
#define INFf    (*(float*)(mgl_inf+1))

#if !defined(NAN)
#if MGL_USE_DOUBLE
#define NAN		NANd
#else
#define NAN		NANf
#endif
#endif

#if !defined(INFINITY)
#if MGL_USE_DOUBLE
#define INFINITY	INFd
#else
#define INFINITY	INFf
#endif
#endif

#endif

#ifndef M_PI
#define M_PI	3.14159265358979323846  /* pi */
#endif
//-----------------------------------------------------------------------------
#ifdef WIN32
#define mglprintf    _snwprintf
#else
#define mglprintf    swprintf
#endif
//#define FLT_EPS	1.1920928955078125e-07
//-----------------------------------------------------------------------------
#if MGL_USE_DOUBLE
typedef double mreal;
#define MGL_EPSILON	(1.+1e-10)
#else
typedef float mreal;
#define MGL_EPSILON	(1.+1e-5)
#endif
#define MGL_FEPSILON	(1.+1e-5)
//-----------------------------------------------------------------------------
#ifndef MGL_CMAP_COLOR
#define MGL_CMAP_COLOR	32
#endif
//-----------------------------------------------------------------------------
#ifndef MGL_DEF_VIEWER
#define MGL_DEF_VIEWER "evince"
#endif
//-----------------------------------------------------------------------------
#define mgl_min(a,b)	(((a)>(b)) ? (b) : (a))
#define mgl_max(a,b)	(((a)>(b)) ? (a) : (b))
#define mgl_isnan(a)	((a)!=(a))
//#define mgl_isnum(a)	((a)==(a) && 2*(a)!=(a))
#define mgl_isnum(a)	((a)==(a))
#define mgl_isfin(a)	((a)-(a)==0.)
#define mgl_isbad(a)	((a)-(a)!=0.)
//-----------------------------------------------------------------------------
#define SMOOTH_NONE		0
#define SMOOTH_LINE_3	1
#define SMOOTH_LINE_5	2
#define SMOOTH_QUAD_5	3
//-----------------------------------------------------------------------------
#define MGL_HIST_IN		0
#define MGL_HIST_SUM	1
#define MGL_HIST_UP		2
#define MGL_HIST_DOWN	3
//-----------------------------------------------------------------------------
enum{	// types of predefined curvelinear coordinate systems
	mglCartesian = 0,	// no transformation
	mglPolar,
	mglSpherical,
	mglParabolic,
	mglParaboloidal,
	mglOblate,
	mglProlate,
	mglElliptic,
	mglToroidal,
	mglBispherical,
	mglBipolar,
	mglLogLog,
	mglLogX,
	mglLogY
};
//-----------------------------------------------------------------------------
// types of drawing
#define MGL_DRAW_WIRE	0	// fastest, no faces
#define MGL_DRAW_FAST	1	// fast, no color interpolation
#define MGL_DRAW_NORM	2	// high quality, slower
#define MGL_DRAW_LMEM	4	// low memory usage (direct to pixel)
#define MGL_DRAW_DOTS	8	// draw dots instead of primitives
#define MGL_DRAW_NONE	9	// no ouput (for testing only)
//-----------------------------------------------------------------------------
enum{	// Codes for warnings/messages
	mglWarnNone = 0,// Everything OK
	mglWarnDim,		// Data dimension(s) is incompatible
	mglWarnLow, 		// Data dimension(s) is too small
	mglWarnNeg,	 	// Minimal data value is negative
	mglWarnFile, 	// No file or wrong data dimensions
	mglWarnMem,		// Not enough memory
	mglWarnZero, 	// Data values are zero
	mglWarnLeg,		// No legend entries
	mglWarnSlc,		// Slice value is out of range
	mglWarnCnt,		// Number of contours is zero or negative
	mglWarnOpen, 	// Couldn't open file
	mglWarnLId,		// Light: ID is out of range
	mglWarnSize, 	// Setsize: size(s) is zero or negative
	mglWarnFmt,		// Format is not supported for that build
	mglWarnTern, 	// Axis ranges are incompatible
	mglWarnNull, 	// Pointer is NULL
	mglWarnSpc,		// Not enough space for plot
	mglScrArg,		// Wrong argument(s) in MGL script
	mglScrCmd,		// Wrong command in MGL script
	mglScrLong,		// Too long line in MGL script
	mglScrStr,		// Unbalanced ' in MGL script
	mglWarnEnd		// Maximal number of warnings (must be last)
};
//-----------------------------------------------------------------------------
#define MGL_DEF_PAL	"bgrcmyhlnqeupH"	// default palette
#define MGL_DEF_SCH	"BbcyrR"	// default palette
#define MGL_COLORS	"kwrgbcymhWRGBCYMHlenpquLENPQU"
//-----------------------------------------------------------------------------
#define MGL_TRANSP_NORM		0x000000
#define MGL_TRANSP_GLASS 	0x000001
#define MGL_TRANSP_LAMP		0x000002
#define MGL_ENABLE_CUT		0x000004 	///< Flag which determines how points outside bounding box are drown.
#define MGL_ENABLE_RTEXT 	0x000008 	///< Use text rotation along axis
#define MGL_AUTO_FACTOR		0x000010 	///< Enable autochange PlotFactor
#define MGL_ENABLE_ALPHA 	0x000020 	///< Flag that Alpha is used
#define MGL_ENABLE_LIGHT 	0x000040 	///< Flag of using lightning
#define MGL_TICKS_ROTATE 	0x000080 	///< Allow ticks rotation
#define MGL_TICKS_SKIP		0x000100 	///< Allow ticks rotation
// flags for internal use only
#define MGL_DISABLE_SCALE	0x000200 	///< Temporary flag for disable scaling (used for axis)
#define MGL_FINISHED 		0x000400 	///< Flag that final picture (i.e. mglCanvas::G) is ready
#define MGL_USE_GMTIME		0x000800 	///< Use gmtime instead of localtime
#define MGL_SHOW_POS		0x001000 	///< Switch to show or not mouse click position
#define MGL_CLF_ON_UPD		0x002000 	///< Clear plot before Update()
#define MGL_NOSUBTICKS		0x004000 	///< Disable subticks drawing (for bounding box)
//#define MGL_DIFFUSIVE		0x008000 	///< Use diffusive light instead of specular
#define MGL_VECT_FRAME		0x010000 	///< Use DrwDat to remember all data of frames
#define MGL_REDUCEACC		0x020000 	///< Reduce accuracy of points (to reduc size of output files)
#define MGL_PREFERVC 		0x040000 	///< Prefer vertex color instead of texture if output format supports
#define MGL_ONESIDED 		0x080000 	///< Render only front side of surfaces if output format supports (for debugging)
#define MGL_NO_ORIGIN 		0x100000 	///< Don't draw tick labels at axis origin
//-----------------------------------------------------------------------------
#ifdef __cplusplus
//-----------------------------------------------------------------------------
extern float mgl_cos[360];	///< contain cosine with step 1 degree
//-----------------------------------------------------------------------------
#include <complex>
typedef std::complex<mreal> dual;
//-----------------------------------------------------------------------------
extern "C" {
#else
#include <complex.h>
#if MGL_USE_DOUBLE
typedef double _Complex dual;
#else
typedef float _Complex dual;
#endif
#endif
/// Find length of wchar_t string (bypass standard wcslen bug)
double MGL_EXPORT mgl_hypot(double x, double y);
/// Find length of wchar_t string (bypass standard wcslen bug)
size_t MGL_EXPORT mgl_wcslen(const wchar_t *str);
/// Get RGB values for given color id or fill by -1 if no one found
void MGL_EXPORT mgl_chrrgb(char id, float rgb[3]);
/// Check if string contain color id and return its number
long MGL_EXPORT mgl_have_color(const char *stl);
/// Find symbol in string excluding {} and return its position or NULL
const char *mglchr(const char *str, char ch);
/// Find any symbol from chr in string excluding {} and return its position or NULL
const char *mglchrs(const char *str, const char *chr);
/// Set number of thread for plotting and data handling (for pthread version only)
void MGL_EXPORT mgl_set_num_thr(int n);
void MGL_EXPORT mgl_set_num_thr_(int *n);
void MGL_EXPORT mgl_test_txt(const char *str, ...);
void MGL_EXPORT mgl_set_test_mode(int enable);
/// Remove spaces at begining and at the end of the string
void MGL_EXPORT mgl_strtrim(char *str);
void MGL_EXPORT mgl_wcstrim(wchar_t *str);
/** Change register to lowercase (only for ANSI symbols) */
void MGL_EXPORT mgl_strlwr(char *str);
void MGL_EXPORT mgl_wcslwr(wchar_t *str);
/// Convert wchar_t* string into char* one
void MGL_EXPORT mgl_wcstombs(char *dst, const wchar_t *src, int size);
/// Clear internal data for speeding up FFT and Hankel transforms
void MGL_EXPORT mgl_clear_fft();
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
