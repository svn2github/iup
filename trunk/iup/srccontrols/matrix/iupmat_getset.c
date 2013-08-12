/** \file
 * \brief iupmatrix setget control
 * attributes set and get
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>

#include "iup.h"
#include "iupcbs.h"

#include <cd.h>

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_drvfont.h"
#include "iup_str.h"
#include "iup_stdcontrols.h"
#include "iup_controls.h"
#include "iup_childtree.h"

#include "iupmat_def.h"
#include "iupmat_getset.h"
#include "iupmat_edit.h"
#include "iupmat_draw.h"
#include "iupmat_aux.h"


int iupMatrixCheckCellPos(Ihandle* ih, int lin, int col)
{
  /* Check if the cell exists */
  if (!iupMATRIX_CHECK_COL(ih, col) ||
      !iupMATRIX_CHECK_LIN(ih, lin))
    return 0;

  return 1;
}

static char* iMatrixSetValueNumeric(Ihandle* ih, int lin, int col, const char* value)
{
  double number;
  if (sscanf(value, "%lf", &number) != 1)   /* lf=double */
  {
    IFniid setvalue_cb;

    if (ih->data->numeric_columns[col].unit_shown!=ih->data->numeric_columns[col].unit) 
      number = ih->data->numeric_convert_func(number, ih->data->numeric_columns[col].quantity,
                                                      ih->data->numeric_columns[col].unit_shown,  /* from */
                                                      ih->data->numeric_columns[col].unit);       /* to */

    setvalue_cb = (IFniid)IupGetCallback(ih, "NUMERICSETVALUE_CB");
    if (setvalue_cb)
    {
      setvalue_cb(ih, lin, col, number);
      return NULL;
    }
    else if (ih->data->numeric_columns[col].unit_shown!=ih->data->numeric_columns[col].unit) 
    {
      /* only use the number if a conversion occurred */
      sprintf(ih->data->numeric_buffer_set, "%.18g", number);  /* maximum double precision */
      value = ih->data->numeric_buffer_set;
    }
  }

  return (char*)value;
}

void iupMatrixCellSetValue(Ihandle* ih, int lin, int col, const char* value, int edited)
{  
  /* NOTICE: this function is NOT called before map */

  if (lin != 0 && ih->data->sort_has_index)
  {
    int index = ih->data->sort_line_index[lin];
    if (index != 0) lin = index;
  }

  if (ih->data->undo_redo) iupAttribSetClassObjectId2(ih, "UNDOPUSHCELL", lin, col, value);

  if (ih->data->numeric_columns && ih->data->numeric_columns[col].flags & IMAT_IS_NUMERIC)
  {
    value = iMatrixSetValueNumeric(ih, lin, col, value);
    if (!value)
      return;
  }

  if (!ih->data->callback_mode)
  {
    if (ih->data->cells[lin][col].value)
      free(ih->data->cells[lin][col].value);

    ih->data->cells[lin][col].value = iupStrDup(value);
  }

  if (edited)
  {
    /* value_edit_cb called only when value is edited, 
       but also works in normal mode */
    IFniis value_edit_cb = (IFniis)IupGetCallback(ih, "VALUE_EDIT_CB");
    if (value_edit_cb)
      value_edit_cb(ih, lin, col, (char*)value);
  }

  ih->data->need_redraw = 1;
  if (lin==0 || col==0)
    ih->data->need_calcsize = 1;
}

double iupMatrixGetValueNumber(Ihandle* ih, int lin, int col)
{
  char* value;
  double number;

  if (lin==0)
    return 0;

  if (ih->data->sort_has_index)
  {
    int index = ih->data->sort_line_index[lin];
    if (index != 0) lin = index;
  }

  if (ih->data->callback_mode)
  {
    /* only called in callback mode */
    sIFnii value_cb = (sIFnii)IupGetCallback(ih, "VALUE_CB");
    value = value_cb(ih, lin, col);
  }
  else
    value = ih->data->cells[lin][col].value;

  if (!value)
  {
    dIFnii getvalue_cb = (dIFnii)IupGetCallback(ih, "NUMERICGETVALUE_CB");
    if (getvalue_cb)
      number = getvalue_cb(ih, lin, col);
    else
      return 0;
  }
  else
  {
    if (sscanf(value, "%lf", &number) != 1)   /* lf=double */
      return 0;
  }

  return number;
}

char* iupMatrixGetValueText(Ihandle* ih, int lin, int col)
{  
  char* value;

  if (lin==0)
    return NULL;

  if (ih->data->sort_has_index)
  {
    int index = ih->data->sort_line_index[lin];
    if (index != 0) lin = index;
  }

  if (ih->data->callback_mode)
  {
    /* only called in callback mode */
    sIFnii value_cb = (sIFnii)IupGetCallback(ih, "VALUE_CB");
    value = iupStrDup(value_cb(ih, lin, col));  /* must duplicate here */
  }
  else
    value = ih->data->cells[lin][col].value;

  return value;
}

static char* iMatrixGetValueNumeric(Ihandle* ih, int lin, int col, const char* value)
{
  char *format=NULL;
  double number;

  if (lin==0)
  {
    if (ih->data->numeric_columns[col].flags & IMAT_HAS_FORMATTITLE)
      format = iupAttribGetId(ih, "NUMERICFORMATTITLE", col);

    if (format)
    {
      char* unit_symbol = iupAttribGetId(ih, "NUMERICUNITSHOWNSYMBOL", col);
      if (unit_symbol)
      {
        if (value)
          sprintf(ih->data->numeric_buffer_get, format, value, unit_symbol);
        else
          sprintf(ih->data->numeric_buffer_get, format, unit_symbol);
        return ih->data->numeric_buffer_get;
      }
    }

    return (char*)value;
  }

  /* from here lin!=0 */

  if (ih->data->numeric_columns[col].flags & IMAT_HAS_FORMAT)
    format = iupAttribGetId(ih, "NUMERICFORMAT", col);

  if ((ih->data->numeric_columns[col].unit_shown==ih->data->numeric_columns[col].unit) &&
      (format==NULL))
    return (char*)value;

  if (format==NULL)
    format = iupAttribGetStr(ih, "NUMERICFORMATDEF");

  if (!value)
  {
    dIFnii getvalue_cb = (dIFnii)IupGetCallback(ih, "NUMERICGETVALUE_CB");
    if (getvalue_cb)
      number = getvalue_cb(ih, lin, col);
    else
      return NULL;
  }
  else
  {
    if (sscanf(value, "%lf", &number) != 1)   /* lf=double */
      return (char*)value;
  }

  if (ih->data->numeric_columns[col].unit_shown!=ih->data->numeric_columns[col].unit) 
    number = ih->data->numeric_convert_func(number, ih->data->numeric_columns[col].quantity,
                                                    ih->data->numeric_columns[col].unit, /* from */
                                                    ih->data->numeric_columns[col].unit_shown);  /* to */

  sprintf(ih->data->numeric_buffer_get, format, number);
  return ih->data->numeric_buffer_get;
}

char* iupMatrixCellGetValue (Ihandle* ih, int lin, int col)
{  
  /* NOTICE: this function is CAN BE called before map */
  if (!ih->handle)
    return iupAttribGetId2(ih, "", lin, col);
  else
  {
    char* value;

    if (lin != 0 && ih->data->sort_has_index)
    {
      int index = ih->data->sort_line_index[lin];
      if (index != 0) lin = index;
    }

    if (ih->data->callback_mode)
    {
      /* only called in callback mode */
      sIFnii value_cb = (sIFnii)IupGetCallback(ih, "VALUE_CB");
      value = value_cb(ih, lin, col);
    }
    else
      value = ih->data->cells[lin][col].value;

    if (ih->data->numeric_columns && ih->data->numeric_columns[col].flags & IMAT_IS_NUMERIC)
      return iMatrixGetValueNumeric(ih, lin, col, value);
    else
      return value;
  }
}

void iupMatrixCellSetFlag(Ihandle* ih, int lin, int col, unsigned char attr, int set)
{
  if (!ih->handle)
    return;

  if (lin==IUP_INVALID_ID)
  {
    if (!iupMATRIX_CHECK_COL(ih, col))
      return;

    if (set)
      ih->data->columns.dt[col].flags |= attr;
    else
      ih->data->columns.dt[col].flags &= ~attr;
  }
  else if (col==IUP_INVALID_ID)
  {
    if (!iupMATRIX_CHECK_LIN(ih, lin))
      return;

    if (set)
      ih->data->lines.dt[lin].flags |= attr;
    else
      ih->data->lines.dt[lin].flags &= ~attr;
  }
  else
  {
    if (!ih->data->callback_mode)
    {
      if (!iupMATRIX_CHECK_COL(ih, col) ||
          !iupMATRIX_CHECK_LIN(ih, lin))
        return;

      if (set)
        ih->data->cells[lin][col].flags |= attr;
      else
        ih->data->cells[lin][col].flags &= ~attr;
    }
  }
}

void iupMatrixCellUpdateValue(Ihandle* ih)
{
  char *value = iupMatrixEditGetValue(ih);
  if (ih->data->undo_redo) iupAttribSetClassObject(ih, "UNDOPUSHBEGIN", "EDITCELL");
  iupMatrixCellSetValue(ih, ih->data->lines.focus_cell, ih->data->columns.focus_cell, value, 1);
  if (ih->data->undo_redo) iupAttribSetClassObject(ih, "UNDOPUSHEND", NULL);
  iupMatrixPrepareDrawData(ih);
  iupMatrixDrawCells(ih, ih->data->lines.focus_cell, ih->data->columns.focus_cell, ih->data->lines.focus_cell, ih->data->columns.focus_cell);
}

void iupMatrixPrepareDrawData(Ihandle* ih)
{
  /* FGCOLOR, BGCOLOR and FONT */
  iupAttribSetStr(ih, "_IUPMAT_BGCOLOR_PARENT", iupControlBaseGetParentBgColor(ih));
  iupAttribSetStr(ih, "_IUPMAT_BGCOLOR", IupGetAttribute(ih, "BGCOLOR"));
  iupAttribSetStr(ih, "_IUPMAT_FGCOLOR", IupGetAttribute(ih, "FGCOLOR"));
  iupAttribSetStr(ih, "_IUPMAT_FONT", IupGetAttribute(ih, "FONT"));

  ih->data->bgcolor_parent = iupAttribGet(ih, "_IUPMAT_BGCOLOR_PARENT");
  ih->data->bgcolor = iupAttribGet(ih, "_IUPMAT_BGCOLOR");
  ih->data->fgcolor = iupAttribGet(ih, "_IUPMAT_FGCOLOR");
  ih->data->font = iupAttribGet(ih, "_IUPMAT_FONT");

  ih->data->font_cb = (sIFnii)IupGetCallback(ih, "FONT_CB");
  ih->data->fgcolor_cb = (IFniiIII)IupGetCallback(ih, "FGCOLOR_CB");
  ih->data->bgcolor_cb = (IFniiIII)IupGetCallback(ih, "BGCOLOR_CB");
}

static char* iMatrixGetCellAttrib(Ihandle* ih, unsigned char attr, int lin, int col, int native_parent)
{
  char* value = NULL;
  const char* attrib = NULL;
  char* attrib_global = NULL; 

  if (attr == IMAT_HAS_FONT)
  {
    attrib = "FONT";
    attrib_global = ih->data->font;
  }
  else if (attr == IMAT_HAS_BGCOLOR)
  {
    attrib = "BGCOLOR";
    attrib_global = ih->data->bgcolor;
  }
  else if (attr == IMAT_HAS_FGCOLOR)
  {
    attrib = "FGCOLOR";
    attrib_global = ih->data->fgcolor;
  }

  /* 1 -  check for this cell */
  if (ih->data->callback_mode || ih->data->cells[lin][col].flags & attr)
    value = iupAttribGetId2(ih, attrib, lin, col);
  if (!value)
  {
    /* 2 - check for this line, if not title col */
    if (col != 0)
    {
      if (ih->data->lines.dt[lin].flags & attr)
        value = iupAttribGetId(ih, attrib, lin);
    }

    if (!value)
    {
      /* 3 - check for this column, if not title line */
      if (lin != 0)
      {
        if (ih->data->columns.dt[col].flags & attr)
          value = iupAttribGetId(ih, attrib, col);
      }

      if (!value)
      {
        /* 4 - check for the matrix or native parent */
        if (native_parent)
          value = ih->data->bgcolor_parent;
        else
          value = attrib_global;
      }
    }
  }

  return value;
}

static int iMatrixCallColorCB(Ihandle* ih, IFniiIII cb, int lin, int col, unsigned char *r, unsigned char *g, unsigned char *b)
{
  int ir, ig, ib, ret;
  ret = cb(ih, lin, col, &ir, &ig, &ib);
  *r = iupBYTECROP(ir);
  *g = iupBYTECROP(ig);
  *b = iupBYTECROP(ib);
  return ret;
}

char* iupMatrixGetFgColor(Ihandle* ih, int lin, int col)
{
  unsigned char r = 0, g = 0, b = 0;
  /* called from Edit only */
  if (!ih->data->fgcolor_cb || (iMatrixCallColorCB(ih, ih->data->fgcolor_cb, lin, col, &r, &g, &b) == IUP_IGNORE))
  {
    char* fgcolor = iMatrixGetCellAttrib(ih, IMAT_HAS_FGCOLOR, lin, col, 0);
    if (!fgcolor) 
    {
      if (lin ==0 || col == 0)
        fgcolor = IupGetGlobal("DLGFGCOLOR");
      else
        fgcolor = IupGetGlobal("TXTFGCOLOR");
    }
    return fgcolor;
  }
  else
    return iupStrReturnRGB(r, g, b);
}

void iupMatrixGetFgRGB(Ihandle* ih, int lin, int col, unsigned char *r, unsigned char *g, unsigned char *b)
{
  /* called from Draw only */
  if (!ih->data->fgcolor_cb || (iMatrixCallColorCB(ih, ih->data->fgcolor_cb, lin, col, r, g, b) == IUP_IGNORE))
  {
    char* fgcolor = iMatrixGetCellAttrib(ih, IMAT_HAS_FGCOLOR, lin, col, 0);
    if (!fgcolor) 
    {
      if (lin ==0 || col == 0)
        fgcolor = IupGetGlobal("DLGFGCOLOR");
      else
        fgcolor = IupGetGlobal("TXTFGCOLOR");
    }
    iupStrToRGB(fgcolor, r, g, b);
  }
}

char* iupMatrixGetBgColor(Ihandle* ih, int lin, int col)
{
  unsigned char r = 0, g = 0, b = 0;
  /* called from Edit only */
  if (!ih->data->bgcolor_cb || (iMatrixCallColorCB(ih, ih->data->bgcolor_cb, lin, col, &r, &g, &b) == IUP_IGNORE))
  {
    int native_parent = 0;
    if (lin == 0 || col == 0)
      native_parent = 1;
    return iMatrixGetCellAttrib(ih, IMAT_HAS_BGCOLOR, lin, col, native_parent);
  }
  else
    return iupStrReturnRGB(r, g, b);
}

#define IMAT_DARKER(_x)    (((_x)*9)/10)

void iupMatrixGetBgRGB(Ihandle* ih, int lin, int col, unsigned char *r, unsigned char *g, unsigned char *b)
{
  /* called from Draw only */
  if (!ih->data->bgcolor_cb || (iMatrixCallColorCB(ih, ih->data->bgcolor_cb, lin, col, r, g, b) == IUP_IGNORE))
  {
    int native_parent = 0;
    if (lin == 0 || col == 0)
      native_parent = 1;
    iupStrToRGB(iMatrixGetCellAttrib(ih, IMAT_HAS_BGCOLOR, lin, col, native_parent), r, g, b);
    if (native_parent)
    {
      int ir = IMAT_DARKER(*r), ig=IMAT_DARKER(*g), ib=IMAT_DARKER(*b); /* use a darker version of the background by 10% */
      *r = iupBYTECROP(ir);
      *g = iupBYTECROP(ig);
      *b = iupBYTECROP(ib);
    }
  }
}

char* iupMatrixGetFont(Ihandle* ih, int lin, int col)
{
  char* font = NULL;
  /* called from Draw and Edit only */
  if (ih->data->font_cb)
    font = ih->data->font_cb(ih, lin, col);
  if (!font)
    font = iMatrixGetCellAttrib(ih, IMAT_HAS_FONT, lin, col, 0);
  return font;
}

int iupMatrixGetColumnWidth(Ihandle* ih, int col, int use_value)
{
  int width = 0, pixels = 0;
  char* value;

  /* can be called for invalid columns (col>numcol) */

  value = iupAttribGetId(ih, "WIDTH", col);
  if (!value)
  {
    value = iupAttribGetId(ih, "RASTERWIDTH", col);
    if (value)
      pixels = 1;
  }

  if (use_value && !value)
  {
    /* Use the titles to define the size */
    if (col == 0)
    {
      if (!ih->data->callback_mode || ih->data->use_title_size)
      {
        /* find the largest title */
        int lin, max_width = 0;
        for(lin = 0; lin < ih->data->lines.num; lin++)
        {
          char* title_value = iupMatrixCellGetValue(ih, lin, 0);
          if (title_value)
          {
            iupdrvFontGetMultiLineStringSize(ih, title_value, &width, NULL);
            if (width > max_width)
              max_width = width;
          }
        }
        width = max_width;
      }
    }
    else if (ih->data->use_title_size && (col>0 && col<ih->data->columns.num))
    {
      char* title_value = iupMatrixCellGetValue(ih, 0, col);
      if (title_value)
        iupdrvFontGetMultiLineStringSize(ih, title_value, &width, NULL);
    }
    if (width)
      return width + IMAT_PADDING_W + IMAT_FRAME_W;

    if (col != 0)
      value = iupAttribGetStr(ih, "WIDTHDEF");
  }

  if (iupStrToInt(value, &width))
  {
    if (width <= 0)
      return 0;
    else
    {
      if (pixels)
        return width + IMAT_PADDING_W + IMAT_FRAME_W;
      else
      {
        int charwidth;
        iupdrvFontGetCharSize(ih, &charwidth, NULL);
        return iupWIDTH2RASTER(width, charwidth) + IMAT_PADDING_W + IMAT_FRAME_W;
      }
    }
  }
  return 0;
}

int iupMatrixGetLineHeight(Ihandle* ih, int lin, int use_value)
{
  int height = 0, pixels = 0;
  char* value;

  /* can be called for invalid lines (lin>numlin) */

  value = iupAttribGetId(ih, "HEIGHT", lin);
  if(!value)
  {
    value = iupAttribGetId(ih, "RASTERHEIGHT", lin);
    if(value)
      pixels = 1;
  }

  if (use_value && !value)
  {
    /* Use the titles to define the size */
    if (lin == 0)
    {
      if (!ih->data->callback_mode || ih->data->use_title_size)
      {
        /* find the highest title */
        int col, max_height = 0;
        for(col = 0; col < ih->data->columns.num; col++)
        {
          char* title_value = iupMatrixCellGetValue(ih, 0, col);
          if (title_value && title_value[0])
          {
            iupdrvFontGetMultiLineStringSize(ih, title_value, NULL, &height);
            if (height > max_height)
              max_height = height;
          }
        }
        height = max_height;
      }
    }
    else if (ih->data->use_title_size && (lin>0 && lin<ih->data->lines.num))
    {
      char* title_value = iupMatrixCellGetValue(ih, lin, 0);
      if (title_value && title_value[0])
        iupdrvFontGetMultiLineStringSize(ih, title_value, NULL, &height);
    }
    if (height)
      return height + IMAT_PADDING_H + IMAT_FRAME_H;

    if (lin != 0)
      value = iupAttribGetStr(ih, "HEIGHTDEF");
  }
  
  if (iupStrToInt(value, &height))
  {
    if (height <= 0)
      return 0;
    else
    {
      if (pixels)
        return height + IMAT_PADDING_H + IMAT_FRAME_H;
      else
      {
        int charheight;
        iupdrvFontGetCharSize(ih, NULL, &charheight);
        return iupHEIGHT2RASTER(height, charheight) + IMAT_PADDING_H + IMAT_FRAME_H;
      }
    }
  }
  return 0;
}

char *iupMatrixGetSize(Ihandle* ih, int index, int m, int pixels_unit)
{
  int size;
  ImatLinColData *p;

  if(m == IMAT_PROCESS_LIN)
    p = &(ih->data->lines);
  else
    p = &(ih->data->columns);

  if (index < 0 || index > p->num-1)
    return NULL;

  if (m == IMAT_PROCESS_LIN)
    size = iupMatrixGetLineHeight(ih, index, 1);
  else
    size = iupMatrixGetColumnWidth(ih, index, 1);

  if (size)
  {
    /* remove the decoration added in iupMatrixAuxGet */
    if (m == IMAT_PROCESS_COL)
      size -= IMAT_PADDING_W + IMAT_FRAME_W;
    else
      size -= IMAT_PADDING_H + IMAT_FRAME_H;

    if (!pixels_unit)
    {
      int charwidth, charheight;
      iupdrvFontGetCharSize(ih, &charwidth, &charheight);
      if (m == IMAT_PROCESS_COL)
        size = iupRASTER2WIDTH(size, charwidth);
      else
        size = iupRASTER2HEIGHT(size, charheight);
    }
  }

  return iupStrReturnInt(size);
}

static int iMatrixGetOffset(int index, int *offset, ImatLinColData *p)
{
  int i;

  *offset = 0;

  /* check if the cell is not empty */
  if (!p->dt[index].size)
    return 0;

  if (index < p->num_noscroll)
  {
    for(i = 0; i < index; i++)
      *offset += p->dt[i].size;
  }
  else
  {
    if (index < p->first ||
        index > p->last)
      return 0;

    for(i = 0; i < p->num_noscroll; i++)
      *offset += p->dt[i].size;

    /* Find the initial position */
    *offset -= p->first_offset;  /* index is always greater or equal to first */
    for(i = p->first; i < index; i++)
      *offset += p->dt[i].size;
  }

  return 1;
}

int iupMatrixGetCellOffset(Ihandle* ih, int lin, int col, int *x, int *y)
{
  if (!iMatrixGetOffset(col, x, &(ih->data->columns)))
    return 0;

  if (!iMatrixGetOffset(lin, y, &(ih->data->lines)))
    return 0;

  return 1;
}

static int iMatrixGetIndexFromOffset(int pos, ImatLinColData *p)
{
  int offset = 0, i;

  if (pos < 0)
    return -1;  /* invalid */

  for(i = 0; i < p->num_noscroll; i++)  /* for all non scrollable cells */
  {
    offset += p->dt[i].size;

    if (pos < offset)
      break;
  }

  if (pos >= offset)
  {
    for(i = p->first; i <= p->last; i++)  /* for all visible cells */
    {
      offset += p->dt[i].size;
      if (i == p->first)
        offset -= p->first_offset;

      if (pos < offset)
        break;
    }

    if (i > p->last)
      i = -1;    /* invisible */
  }

  return i;
}

int iupMatrixGetCellFromOffset(Ihandle* ih, int x, int y, int* l, int* c)
{
  int col = iMatrixGetIndexFromOffset(x, &(ih->data->columns));
  int lin = iMatrixGetIndexFromOffset(y, &(ih->data->lines));

  if (col < 0 || lin < 0)
    return 0;

  *l = lin;
  *c = col;
  return 1;
}

static void iMatrixGetCellDim(int index, int* offset, int* size, ImatLinColData *p)
{
  int i;

  *offset = 0;
  if (index < p->num_noscroll)
  {
    for(i = 0; i < index; i++)
      *offset += p->dt[i].size;
  }
  else
  {
    for(i = 0; i < p->num_noscroll; i++)
      *offset += p->dt[i].size;

    for(i = p->first; i < index; i++)
    {
      *offset += p->dt[i].size;
      if (i == p->first)
        *offset -= p->first_offset;  /* add only when index greater than first */
    }
  }

  *size = p->dt[index].size - 1;
  if (index == p->first)
    *size -= p->first_offset;
}

void iupMatrixGetVisibleCellDim(Ihandle* ih, int lin, int col, int* x, int* y, int* w, int* h)
{
  iMatrixGetCellDim(col, x, w, &(ih->data->columns));
  iMatrixGetCellDim(lin, y, h, &(ih->data->lines));
}

