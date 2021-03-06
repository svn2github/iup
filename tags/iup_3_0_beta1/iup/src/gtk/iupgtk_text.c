/** \file
 * \brief Text Control
 *
 * See Copyright Notice in "iup.h"
 */

#include <gtk/gtk.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>

#include "iup.h"
#include "iupcbs.h"

#include "iup_object.h"
#include "iup_layout.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_image.h"
#include "iup_mask.h"
#include "iup_drv.h"
#include "iup_drvfont.h"
#include "iup_image.h"
#include "iup_key.h"
#include "iup_array.h"
#include "iup_text.h"

#include "iupgtk_drv.h"

#ifndef PANGO_WEIGHT_SEMIBOLD
#define PANGO_WEIGHT_SEMIBOLD 600
#endif

void iupdrvTextAddSpin(int *w, int h)
{
  int spin_size = 16;
  *w += spin_size;
  (void)h;
}

void iupdrvTextAddBorders(int *x, int *y)
{
  int border_size = 2*5;
  (*x) += border_size;
  (*y) += border_size;
}

static void gtkTextParseParagraphFormat(Ihandle* formattag, GtkTextTag* tag)
{
  int val;
  char* format;

  format = iupAttribGetStr(formattag, "INDENT");
  if (format && iupStrToInt(format, &val))
    g_object_set(G_OBJECT(tag), "indent", val, NULL);

  format = iupAttribGetStr(formattag, "ALIGNMENT");
  if (format)
  {
    if (iupStrEqualNoCase(format, "JUSTIFY"))
      val = GTK_JUSTIFY_FILL;
    else if (iupStrEqualNoCase(format, "RIGHT"))
      val = GTK_JUSTIFY_RIGHT;
    else if (iupStrEqualNoCase(format, "CENTER"))
      val = GTK_JUSTIFY_CENTER;
    else /* "LEFT" */
      val = GTK_JUSTIFY_LEFT;

    g_object_set(G_OBJECT(tag), "justification", val, NULL);
  }

  format = iupAttribGetStr(formattag, "TABSARRAY");
  {
    PangoTabArray *tabs;
    int pos, i = 0;
    PangoTabAlign align;
    char* str;

    tabs = pango_tab_array_new(32, FALSE);

    while (format)
    {
      str = iupStrCopyUntil((char**)&format, ' ');
      if (!str) break;
      pos = atoi(str);
      free(str);

      str = iupStrCopyUntil((char**)&format, ' ');
      if (!str) break;

/*      if (iupStrEqualNoCase(str, "DECIMAL"))    unsupported for now
        align = PANGO_TAB_NUMERIC;
      else if (iupStrEqualNoCase(str, "RIGHT"))
        align = PANGO_TAB_RIGHT;
      else if (iupStrEqualNoCase(str, "CENTER"))
        align = PANGO_TAB_CENTER;
      else */    /* "LEFT" */
        align = PANGO_TAB_LEFT;
      free(str);

      pango_tab_array_set_tab(tabs, i, align, IUPGTK_PIXELS2PANGOUNITS(pos));
      i++;
      if (i == 32) break;
    }

    g_object_set(G_OBJECT(tag), "tabs", tabs, NULL);
    pango_tab_array_free(tabs);
  }

  format = iupAttribGetStr(formattag, "SPACEBEFORE");
  if (format && iupStrToInt(format, &val))
    g_object_set(G_OBJECT(tag), "pixels-above-lines", val, NULL);

  format = iupAttribGetStr(formattag, "SPACEAFTER");
  if (format && iupStrToInt(format, &val))
    g_object_set(G_OBJECT(tag), "pixels-below-lines", val, NULL);

  format = iupAttribGetStr(formattag, "LINESPACING");
  if (format && iupStrToInt(format, &val))
    g_object_set(G_OBJECT(tag), "pixels-inside-wrap", val, NULL);
}

static void gtkTextParseCharacterFormat(Ihandle* formattag, GtkTextTag* tag)
{
  int val;
  char* format;

  format = iupAttribGetStr(formattag, "LANGUAGE");
  if (format)
    g_object_set(G_OBJECT(tag), "language", format, NULL);

  format = iupAttribGetStr(formattag, "STRETCH");
  if (format)
  {
    if (iupStrEqualNoCase(format, "EXTRA_CONDENSED"))
      val = PANGO_STRETCH_EXTRA_CONDENSED;  
    else if (iupStrEqualNoCase(format, "CONDENSED"))
      val = PANGO_STRETCH_CONDENSED;     
    else if (iupStrEqualNoCase(format, "SEMI_CONDENSED"))
      val = PANGO_STRETCH_SEMI_CONDENSED;  
    else if (iupStrEqualNoCase(format, "SEMI_EXPANDED"))
      val = PANGO_STRETCH_SEMI_EXPANDED;      
    else if (iupStrEqualNoCase(format, "EXPANDED"))
      val = PANGO_STRETCH_EXPANDED;  
    else if (iupStrEqualNoCase(format, "EXTRA_EXPANDED"))
      val = PANGO_STRETCH_EXTRA_EXPANDED;       
    else /* "NORMAL" */
      val = PANGO_STRETCH_NORMAL;   

    g_object_set(G_OBJECT(tag), "stretch", val, NULL);
  }

  format = iupAttribGetStr(formattag, "RISE");
  if (format)
  {
    val = 0;

    if (iupStrEqualNoCase(format, "SUPERSCRIPT"))
    {
      g_object_set(G_OBJECT(tag), "scale", PANGO_SCALE_X_SMALL, NULL);
      val = 10;  /* 10 pixels up */
    }
    else if (iupStrEqualNoCase(format, "SUBSCRIPT"))
    {
      g_object_set(G_OBJECT(tag), "scale", PANGO_SCALE_X_SMALL, NULL);
      val = -10;  /* 10 pixels down */
    } 
    else 
      iupStrToInt(format, &val);

    val = IUPGTK_PIXELS2PANGOUNITS(val);
    g_object_set(G_OBJECT(tag), "rise", val, NULL);
  }

  format = iupAttribGetStr(formattag, "SMALLCAPS");
  if (format)
  {
    if (iupStrBoolean(format))
      val = PANGO_VARIANT_SMALL_CAPS;
    else
      val = PANGO_VARIANT_NORMAL;
    g_object_set(G_OBJECT(tag), "variant", val, NULL);
  }

  format = iupAttribGetStr(formattag, "ITALIC");
  if (format)
  {
    if (iupStrBoolean(format))
      val = PANGO_STYLE_ITALIC;
    else
      val = PANGO_STYLE_NORMAL;
    g_object_set(G_OBJECT(tag), "style", val, NULL);
  }

  format = iupAttribGetStr(formattag, "STRIKEOUT");
  if (format)
  {
    val = iupStrBoolean(format);
    g_object_set(G_OBJECT(tag), "strikethrough", val, NULL);
  }

  format = iupAttribGetStr(formattag, "PROTECTED");
  if (format)
  {
    val = iupStrBoolean(format);
    g_object_set(G_OBJECT(tag), "editable", val, NULL);
  }

  format = iupAttribGetStr(formattag, "FONTSIZE");
  if (format && iupStrToInt(format, &val))
  {
    if (val < 0)  /* in pixels */
    {
      val = IUPGTK_PIXELS2PANGOUNITS(-val);
      g_object_set(G_OBJECT(tag), "size", val, NULL);
    }
    else  /* in points */
      g_object_set(G_OBJECT(tag), "size-points", (double)val, NULL);
  }

  format = iupAttribGetStr(formattag, "FONTSCALE");
  if (format)
  {
    float fval = 0;
    if (iupStrEqualNoCase(format, "XX-SMALL"))
      fval = (float)PANGO_SCALE_XX_SMALL;
    else if (iupStrEqualNoCase(format, "X-SMALL"))
      fval = (float)PANGO_SCALE_X_SMALL;
    else if (iupStrEqualNoCase(format, "SMALL"))
      fval = (float)PANGO_SCALE_SMALL;
    else if (iupStrEqualNoCase(format, "MEDIUM"))
      fval = (float)PANGO_SCALE_MEDIUM;
    else if (iupStrEqualNoCase(format, "LARGE"))
      fval = (float)PANGO_SCALE_LARGE;
    else if (iupStrEqualNoCase(format, "X-LARGE"))
      fval = (float)PANGO_SCALE_X_LARGE;
    else if (iupStrEqualNoCase(format, "XX-LARGE"))
      fval = (float)PANGO_SCALE_XX_LARGE;
    else 
      iupStrToFloat(format, &fval);

    if (fval > 0)
      g_object_set(G_OBJECT(tag), "scale", (double)fval, NULL);
  }

  format = iupAttribGetStr(formattag, "FONTFACE");
  if (format)
    g_object_set(G_OBJECT(tag), "family", format, NULL);

  format = iupAttribGetStr(formattag, "FGCOLOR");
  if (format)
  {
    unsigned char r, g, b;
    if (iupStrToRGB(format, &r, &g, &b))
    {
      GdkColor color = {0L,0,0,0};

      color.red = iupCOLOR8TO16(r);
      color.green = iupCOLOR8TO16(g);
      color.blue = iupCOLOR8TO16(b);

      g_object_set(G_OBJECT(tag), "foreground-gdk", &color, NULL);
    }
  }

  format = iupAttribGetStr(formattag, "BGCOLOR");
  if (format)
  {
    unsigned char r, g, b;
    if (iupStrToRGB(format, &r, &g, &b))
    {
      GdkColor color = {0L,0,0,0};

      color.red = iupCOLOR8TO16(r);
      color.green = iupCOLOR8TO16(g);
      color.blue = iupCOLOR8TO16(b);

      g_object_set(G_OBJECT(tag), "background-gdk", &color, NULL);
    }
  }

  format = iupAttribGetStr(formattag, "UNDERLINE");
  if (format)
  {
    if (iupStrEqualNoCase(format, "SINGLE"))
      val = PANGO_UNDERLINE_SINGLE;
    else if (iupStrEqualNoCase(format, "DOUBLE"))
      val = PANGO_UNDERLINE_DOUBLE;
    else /* "NONE" */
      val = PANGO_UNDERLINE_NONE;

    g_object_set(G_OBJECT(tag), "underline", val, NULL);
  }

  format = iupAttribGetStr(formattag, "WEIGHT");
  if (format)
  {
    if (iupStrEqualNoCase(format, "EXTRALIGHT"))
      val = PANGO_WEIGHT_ULTRALIGHT;  
    else if (iupStrEqualNoCase(format, "LIGHT"))
      val = PANGO_WEIGHT_LIGHT;     
    else if (iupStrEqualNoCase(format, "SEMIBOLD"))
      val = PANGO_WEIGHT_SEMIBOLD;  
    else if (iupStrEqualNoCase(format, "BOLD"))
      val = PANGO_WEIGHT_BOLD;      
    else if (iupStrEqualNoCase(format, "EXTRABOLD"))
      val = PANGO_WEIGHT_ULTRABOLD;  
    else if (iupStrEqualNoCase(format, "HEAVY"))
      val = PANGO_WEIGHT_HEAVY;       
    else /* "NORMAL" */
      val = PANGO_WEIGHT_NORMAL;   

    g_object_set(G_OBJECT(tag), "weight", val, NULL);
  }
}

static void gtkTextMoveIterToLinCol(GtkTextBuffer *buffer, GtkTextIter *iter, int lin, int col)
{
  int line_count, line_length;

  lin--; /* IUP starts at 1 */
  col--;

  line_count = gtk_text_buffer_get_line_count(buffer);
  if (lin < 0) lin = 0;
  if (lin >= line_count)
    lin = line_count-1;

  gtk_text_buffer_get_iter_at_line(buffer, iter, lin);
  line_length = gtk_text_iter_get_chars_in_line(iter);

  if (col < 0) col = 0;
  if (col > line_length)
    col = line_length;  /* after the last character */

  gtk_text_iter_set_line_offset(iter, col);
}

static void gtkTextGetLinColFromPosition(const GtkTextIter *iter, int *lin, int *col)
{
  *lin = gtk_text_iter_get_line(iter);
  *col = gtk_text_iter_get_line_offset(iter);

  (*lin)++; /* IUP starts at 1 */
  (*col)++;
}

static int gtkTextGetCharSize(Ihandle* ih)
{
  int charwidth;
  PangoFontMetrics* metrics;
  PangoContext* context;
  PangoFontDescription* fontdesc = (PangoFontDescription*)iupgtkGetPangoFontDescAttrib(ih);
  if (!fontdesc)
    return 0;

  context = gdk_pango_context_get();
  metrics = pango_context_get_metrics(context, fontdesc, pango_context_get_language(context));
  charwidth = pango_font_metrics_get_approximate_char_width(metrics);
  pango_font_metrics_unref(metrics);
  return charwidth;
}

void iupdrvTextConvertXYToChar(Ihandle* ih, int x, int y, int *lin, int *col, int *pos)
{
  if (ih->data->is_multiline)
  {
    GtkTextIter iter;
    gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(ih->handle), GTK_TEXT_WINDOW_WIDGET, x, y, &x, &y);
    gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(ih->handle), &iter, x, y);
    gtkTextGetLinColFromPosition(&iter, lin, col);
    *pos = gtk_text_iter_get_offset(&iter);
  }
  else
  {
    int trailing, off_x, off_y;

    /* transform to Layout coordinates */
    gtk_entry_get_layout_offsets(GTK_ENTRY(ih->handle), &off_x, &off_y);
    x = IUPGTK_PIXELS2PANGOUNITS(x - off_x); 
    y = IUPGTK_PIXELS2PANGOUNITS(y - off_y);

    pango_layout_xy_to_index(gtk_entry_get_layout(GTK_ENTRY(ih->handle)), x, y, pos, &trailing);
    *col = (*pos) + 1; /* IUP starts at 1 */
    *lin = 1;
  }
}


/*******************************************************************************************/


static int gtkTextSetSelectionAttrib(Ihandle* ih, const char* value)
{
  if (!value)
    return 0;

  if (!value || iupStrEqualNoCase(value, "NONE"))
  {
    if (ih->data->is_multiline)
    {
      GtkTextIter start_iter;
      GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));
      gtk_text_buffer_get_start_iter(buffer, &start_iter);
      gtk_text_buffer_select_range(buffer, &start_iter, &start_iter);
    }
    else
      gtk_editable_select_region(GTK_EDITABLE(ih->handle), 0, 0);
    return 0;
  }

  if (iupStrEqualNoCase(value, "ALL"))
  {
    if (ih->data->is_multiline)
    {
      GtkTextIter start_iter;
      GtkTextIter end_iter;
      GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));
      gtk_text_buffer_get_start_iter(buffer, &start_iter);
      gtk_text_buffer_get_end_iter(buffer, &end_iter);
      gtk_text_buffer_select_range(buffer, &start_iter, &end_iter);
    }
    else
      gtk_editable_select_region(GTK_EDITABLE(ih->handle), 0, -1);
    return 0;
  }

  if (ih->data->is_multiline)
  {
    int lin_start=1, col_start=1, lin_end=1, col_end=1;
    GtkTextIter start_iter, end_iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));

    if (sscanf(value, "%d,%d:%d,%d", &lin_start, &col_start, &lin_end, &col_end)!=4) return 0;
    if (lin_start<1 || col_start<1 || lin_end<1 || col_end<1) return 0;

    gtkTextMoveIterToLinCol(buffer, &start_iter, lin_start, col_start);
    gtkTextMoveIterToLinCol(buffer, &end_iter, lin_end, col_end);

    gtk_text_buffer_select_range(buffer, &start_iter, &end_iter);
  }
  else
  {
    int start=1, end=1;
    if (iupStrToIntInt(value, &start, &end, ':')!=2) 
      return 0;

    if(start<1 || end<1) 
      return 0;

    start--; /* IUP starts at 1 */
    end--;

    gtk_editable_select_region(GTK_EDITABLE(ih->handle), start, end);
  }

  return 0;
}

static char* gtkTextGetSelectionAttrib(Ihandle* ih)
{
  char *str;

  if (ih->data->is_multiline)
  {
    int start_col, start_lin, end_col, end_lin;

    GtkTextIter start_iter, end_iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));
    if (gtk_text_buffer_get_selection_bounds(buffer, &start_iter, &end_iter))
    {
      gtkTextGetLinColFromPosition(&start_iter, &start_lin, &start_col);
      gtkTextGetLinColFromPosition(&end_iter,   &end_lin,   &end_col);

      str = iupStrGetMemory(100);
      sprintf(str,"%d,%d:%d,%d", start_lin, start_col, end_lin, end_col);
      return str;
    }
  }
  else
  {
    int start, end;
    if (gtk_editable_get_selection_bounds(GTK_EDITABLE(ih->handle), &start, &end))
    {
      start++; /* IUP starts at 1 */
      end++;
      str = iupStrGetMemory(100);
      sprintf(str, "%d:%d", (int)start, (int)end);
      return str;
    }
  }

  return NULL;
}

static int gtkTextSetSelectionPosAttrib(Ihandle* ih, const char* value)
{
  int start=0, end=0;

  if (!value)
    return 0;

  if (!value || iupStrEqualNoCase(value, "NONE"))
  {
    if (ih->data->is_multiline)
    {
      GtkTextIter start_iter;
      GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));
      gtk_text_buffer_get_start_iter(buffer, &start_iter);
      gtk_text_buffer_select_range(buffer, &start_iter, &start_iter);
    }
    else
      gtk_editable_select_region(GTK_EDITABLE(ih->handle), 0, 0);
    return 0;
  }

  if (iupStrEqualNoCase(value, "ALL"))
  {
    if (ih->data->is_multiline)
    {
      GtkTextIter start_iter;
      GtkTextIter end_iter;
      GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));
      gtk_text_buffer_get_start_iter(buffer, &start_iter);
      gtk_text_buffer_get_end_iter(buffer, &end_iter);
      gtk_text_buffer_select_range(buffer, &start_iter, &end_iter);
    }
    else
      gtk_editable_select_region(GTK_EDITABLE(ih->handle), 0, -1);
    return 0;
  }

  if (iupStrToIntInt(value, &start, &end, ':')!=2) 
    return 0;

  if(start<0 || end<0) 
    return 0;

  if (ih->data->is_multiline)
  {
    GtkTextIter start_iter, end_iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));

    gtk_text_buffer_get_iter_at_offset(buffer, &start_iter, start);
    gtk_text_buffer_get_iter_at_offset(buffer, &end_iter, end);

    gtk_text_buffer_select_range(buffer, &start_iter, &end_iter);
  }
  else
    gtk_editable_select_region(GTK_EDITABLE(ih->handle), start, end);

  return 0;
}

static char* gtkTextGetSelectionPosAttrib(Ihandle* ih)
{
  int start, end;
  char *str;

  if (ih->data->is_multiline)
  {
    GtkTextIter start_iter, end_iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));
    if (gtk_text_buffer_get_selection_bounds(buffer, &start_iter, &end_iter))
    {
      start = gtk_text_iter_get_offset(&start_iter);
      end = gtk_text_iter_get_offset(&end_iter);

      str = iupStrGetMemory(100);
      sprintf(str, "%d:%d", (int)start, (int)end);
      return str;
    }
  }
  else
  {
    if (gtk_editable_get_selection_bounds(GTK_EDITABLE(ih->handle), &start, &end))
    {
      str = iupStrGetMemory(100);
      sprintf(str, "%d:%d", (int)start, (int)end);
      return str;
    }
  }

  return NULL;
}

static int gtkTextSetSelectedTextAttrib(Ihandle* ih, const char* value)
{
  if (!value)
    return 0;

  if (ih->data->is_multiline)
  {
    GtkTextIter start_iter, end_iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));
    if (gtk_text_buffer_get_selection_bounds(buffer, &start_iter, &end_iter))
    {
      /* disable callbacks */
      iupAttribSetStr(ih, "_IUPGTK_DISABLE_TEXT_CB", "1");
      gtk_text_buffer_delete(buffer, &start_iter, &end_iter);
      gtk_text_buffer_insert(buffer, &start_iter, iupgtkStrConvertToUTF8(value), -1);
      iupAttribSetStr(ih, "_IUPGTK_DISABLE_TEXT_CB", NULL);
    }
  }
  else
  {
    int start, end;
    if (gtk_editable_get_selection_bounds(GTK_EDITABLE(ih->handle), &start, &end))
    {
      /* disable callbacks */
      iupAttribSetStr(ih, "_IUPGTK_DISABLE_TEXT_CB", "1");
      gtk_editable_delete_selection(GTK_EDITABLE(ih->handle));
      gtk_editable_insert_text(GTK_EDITABLE(ih->handle), iupgtkStrConvertToUTF8(value), -1, &start);
      iupAttribSetStr(ih, "_IUPGTK_DISABLE_TEXT_CB", NULL);
    }
  }

  return 0;
}

static char* gtkTextGetSelectedTextAttrib(Ihandle* ih)
{
  if (ih->data->is_multiline)
  {
    GtkTextIter start_iter, end_iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));
    if (gtk_text_buffer_get_selection_bounds(buffer, &start_iter, &end_iter))
      return iupStrGetMemoryCopy(iupgtkStrConvertFromUTF8(gtk_text_buffer_get_text(buffer, &start_iter, &end_iter, TRUE)));
  }
  else
  {
    int start, end;
    if (gtk_editable_get_selection_bounds(GTK_EDITABLE(ih->handle), &start, &end))
    {
      char* selectedtext = gtk_editable_get_chars(GTK_EDITABLE(ih->handle), start, end);
      char* str = iupStrGetMemoryCopy(iupgtkStrConvertFromUTF8(selectedtext));
      g_free(selectedtext);
      return str;
    }
  }

  return NULL;
}

static int gtkTextSetCaretAttrib(Ihandle* ih, const char* value)
{
  int pos = 1;

  if (!value)
    return 0;

  if (ih->data->is_multiline)
  {
    int lin = 1, col = 1;
    GtkTextIter iter;

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));

    iupStrToIntInt(value, &lin, &col, ',');

    gtkTextMoveIterToLinCol(buffer, &iter, lin, col);

    gtk_text_buffer_place_cursor(buffer, &iter);
    gtk_text_view_place_cursor_onscreen(GTK_TEXT_VIEW(ih->handle));
  }
  else
  {
    sscanf(value,"%i",&pos);
    pos--; /* IUP starts at 1 */
    if (pos < 0) pos = 0;

    gtk_editable_set_position(GTK_EDITABLE(ih->handle), pos);
  }

  return 0;
}

static char* gtkTextGetCaretAttrib(Ihandle* ih)
{
  char* str = iupStrGetMemory(50);

  if (ih->data->is_multiline)
  {
    int col, lin;
    GtkTextIter iter;

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));
    gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
    gtkTextGetLinColFromPosition(&iter, &lin, &col);

    sprintf(str, "%d,%d", lin, col);
  }
  else
  {
    int pos = gtk_editable_get_position(GTK_EDITABLE(ih->handle));
    pos++; /* IUP starts at 1 */
    sprintf(str, "%d", (int)pos);
  }

  return str;
}

static int gtkTextSetCaretPosAttrib(Ihandle* ih, const char* value)
{
  int pos = 0;

  if (!value)
    return 0;

  sscanf(value,"%i",&pos);
  if (pos < 0) pos = 0;

  if (ih->data->is_multiline)
  {
    GtkTextIter iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));
    gtk_text_buffer_get_iter_at_offset(buffer, &iter, pos);
    gtk_text_buffer_place_cursor(buffer, &iter);
    gtk_text_view_place_cursor_onscreen(GTK_TEXT_VIEW(ih->handle));
  }
  else
    gtk_editable_set_position(GTK_EDITABLE(ih->handle), pos);

  return 0;
}

static char* gtkTextGetCaretPosAttrib(Ihandle* ih)
{
  int pos;
  char* str = iupStrGetMemory(50);

  if (ih->data->is_multiline)
  {
    GtkTextIter iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));
    gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
    pos = gtk_text_iter_get_offset(&iter);
  }
  else
    pos = gtk_editable_get_position(GTK_EDITABLE(ih->handle));

  sprintf(str, "%d", (int)pos);
  return str;
}

static int gtkTextSetScrollToAttrib(Ihandle* ih, const char* value)
{
  if (!value)
    return 0;

  if (ih->data->is_multiline)
  {
    int lin = 1, col = 1;
    GtkTextIter iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));

    iupStrToIntInt(value, &lin, &col, ',');
    if (lin < 1) lin = 1;
    if (col < 1) col = 1;

    gtkTextMoveIterToLinCol(buffer, &iter, lin, col);
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(ih->handle), &iter, 0, FALSE, 0, 0);
  }
  else
  {
    int pos = 1;
    sscanf(value,"%i",&pos);
    if (pos < 1) pos = 1;
    pos--;  /* return to Motif referece */
    gtk_editable_set_position(GTK_EDITABLE(ih->handle), pos);
  }

  return 0;
}

static int gtkTextSetScrollToPosAttrib(Ihandle* ih, const char* value)
{
  int pos = 0;

  if (!value)
    return 0;

  sscanf(value,"%i",&pos);
  if (pos < 0) pos = 0;

  if (ih->data->is_multiline)
  {
    GtkTextIter iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));
    gtk_text_buffer_get_iter_at_offset(buffer, &iter, pos);
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(ih->handle), &iter, 0, FALSE, 0, 0);
  }
  else
    gtk_editable_set_position(GTK_EDITABLE(ih->handle), pos);

  return 0;
}

static int gtkTextSetValueAttrib(Ihandle* ih, const char* value)
{
  if (!value) value = "";
  /* disable callbacks */
  iupAttribSetStr(ih, "_IUPGTK_DISABLE_TEXT_CB", "1");
  if (ih->data->is_multiline)
  {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));
    gtk_text_buffer_set_text(buffer, iupgtkStrConvertToUTF8(value), -1);
  }
  else
    gtk_entry_set_text(GTK_ENTRY(ih->handle), iupgtkStrConvertToUTF8(value));
  iupAttribSetStr(ih, "_IUPGTK_DISABLE_TEXT_CB", NULL);
  return 0;
}

static char* gtkTextGetValueAttrib(Ihandle* ih)
{
  if (ih->data->is_multiline)
  {
    GtkTextIter start_iter;
    GtkTextIter end_iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));
    gtk_text_buffer_get_start_iter(buffer, &start_iter);
    gtk_text_buffer_get_end_iter(buffer, &end_iter);
    return iupStrGetMemoryCopy(iupgtkStrConvertFromUTF8(gtk_text_buffer_get_text(buffer, &start_iter, &end_iter, TRUE)));
  }
  else
    return iupStrGetMemoryCopy(iupgtkStrConvertFromUTF8(gtk_entry_get_text(GTK_ENTRY(ih->handle))));
}
                       
static int gtkTextSetInsertAttrib(Ihandle* ih, const char* value)
{
  if (!value)
    return 0;

  /* disable callbacks */
  iupAttribSetStr(ih, "_IUPGTK_DISABLE_TEXT_CB", "1");
  if (ih->data->is_multiline)
  {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));
    gtk_text_buffer_insert_at_cursor(buffer, iupgtkStrConvertToUTF8(value), -1);
  }
  else
  {
    gint pos = gtk_editable_get_position(GTK_EDITABLE(ih->handle));
    gtk_editable_insert_text(GTK_EDITABLE(ih->handle), iupgtkStrConvertToUTF8(value), -1, &pos);
  }
  iupAttribSetStr(ih, "_IUPGTK_DISABLE_TEXT_CB", NULL);

  return 0;
}

static int gtkTextSetAppendAttrib(Ihandle* ih, const char* value)
{
  /* disable callbacks */
  iupAttribSetStr(ih, "_IUPGTK_DISABLE_TEXT_CB", "1");
  if (ih->data->is_multiline)
  {
    GtkTextIter iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));
    gtk_text_buffer_get_end_iter(buffer, &iter);
    if (ih->data->append_newline)
      gtk_text_buffer_insert(buffer, &iter, "\n", 1);
    gtk_text_buffer_insert(buffer, &iter, iupgtkStrConvertToUTF8(value), -1);
  }
  else
  {
    gint pos = strlen(gtk_entry_get_text(GTK_ENTRY(ih->handle)))+1;
    gtk_editable_insert_text(GTK_EDITABLE(ih->handle), iupgtkStrConvertToUTF8(value), -1, &pos);
  }
  iupAttribSetStr(ih, "_IUPGTK_DISABLE_TEXT_CB", NULL);
  return 0;
}

static int gtkTextSetAlignmentAttrib(Ihandle* ih, const char* value)
{
  float xalign;
  GtkJustification justification;

  if (iupStrEqualNoCase(value, "ARIGHT"))
  {
    xalign = 1.0f;
    justification = GTK_JUSTIFY_RIGHT;
  }
  else if (iupStrEqualNoCase(value, "ACENTER"))
  {
    xalign = 0.5f;
    justification = GTK_JUSTIFY_CENTER;
  }
  else /* "ALEFT" */
  {
    xalign = 0;
    justification = GTK_JUSTIFY_LEFT;
  }

  if (ih->data->is_multiline)
    gtk_text_view_set_justification(GTK_TEXT_VIEW(ih->handle), justification);
  else
    gtk_entry_set_alignment(GTK_ENTRY(ih->handle), xalign);

  return 1;
}

static int gtkTextSetPaddingAttrib(Ihandle* ih, const char* value)
{
  iupStrToIntInt(value, &ih->data->horiz_padding, &ih->data->vert_padding, 'x');
  if (ih->handle)
  {
    if (ih->data->is_multiline)
    {
      gtk_text_view_set_left_margin(GTK_TEXT_VIEW(ih->handle), ih->data->horiz_padding);
      gtk_text_view_set_right_margin(GTK_TEXT_VIEW(ih->handle), ih->data->horiz_padding);
      ih->data->vert_padding = 0;
    }
    else
    {
      GtkBorder border;
      border.bottom = border.top = ih->data->vert_padding;
      border.left = border.right = ih->data->horiz_padding;
#if GTK_CHECK_VERSION(2, 10, 0)
      gtk_entry_set_inner_border(GTK_ENTRY(ih->handle), &border);
#endif
    }
  }
  return 0;
}

static int gtkTextSetNCAttrib(Ihandle* ih, const char* value)
{
  if (!iupStrToInt(value, &ih->data->nc))
    ih->data->nc = INT_MAX;

  if (!ih->data->is_multiline && ih->handle)
    gtk_entry_set_max_length(GTK_ENTRY(ih->handle), ih->data->nc);

  return 0;
}

static int gtkTextSetClipboardAttrib(Ihandle *ih, const char *value)
{
  /* disable callbacks */
  iupAttribSetStr(ih, "_IUPGTK_DISABLE_TEXT_CB", "1");
  if (iupStrEqualNoCase(value, "COPY"))
  {
    if (ih->data->is_multiline)
    {
      GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));
      GtkClipboard *clipboard = gtk_clipboard_get(gdk_atom_intern("CLIPBOARD", FALSE));
      gtk_text_buffer_copy_clipboard(buffer, clipboard);
    }
    else
      gtk_editable_copy_clipboard(GTK_EDITABLE(ih->handle));
  }
  else if (iupStrEqualNoCase(value, "CUT"))
  {
    if (ih->data->is_multiline)
    {
      GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));
      GtkClipboard *clipboard = gtk_clipboard_get(gdk_atom_intern("CLIPBOARD", FALSE));
      gtk_text_buffer_cut_clipboard(buffer, clipboard, TRUE);
    }
    else
      gtk_editable_cut_clipboard(GTK_EDITABLE(ih->handle));
  }
  else if (iupStrEqualNoCase(value, "PASTE"))
  {
    if (ih->data->is_multiline)
    {
      GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));
      GtkClipboard *clipboard = gtk_clipboard_get(gdk_atom_intern("CLIPBOARD", FALSE));
      gtk_text_buffer_paste_clipboard(buffer, clipboard, NULL, TRUE);
    }
    else
      gtk_editable_paste_clipboard(GTK_EDITABLE(ih->handle));
  }
  else if (iupStrEqualNoCase(value, "CLEAR"))
  {
    if (ih->data->is_multiline)
    {
      GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));
      gtk_text_buffer_delete_selection(buffer, FALSE, TRUE);
    }
    else
      gtk_editable_delete_selection(GTK_EDITABLE(ih->handle));
  }
  iupAttribSetStr(ih, "_IUPGTK_DISABLE_TEXT_CB", NULL);
  return 0;
}

static int gtkTextSetReadOnlyAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->is_multiline)
    gtk_text_view_set_editable(GTK_TEXT_VIEW(ih->handle), !iupStrBoolean(value));
  else
    gtk_editable_set_editable(GTK_EDITABLE(ih->handle), !iupStrBoolean(value));
  return 0;
}

static char* gtkTextGetReadOnlyAttrib(Ihandle* ih)
{
  int editable;
  if (ih->data->is_multiline)
    editable = gtk_text_view_get_editable(GTK_TEXT_VIEW(ih->handle));
  else
    editable = gtk_editable_get_editable(GTK_EDITABLE(ih->handle));
  if (!editable)
    return "YES";
  else
    return "NO";
}

static char* gtkTextGetPangoLayoutAttrib(Ihandle* ih)
{
  if (ih->data->is_multiline)
    return NULL;
  else
    return (char*)gtk_entry_get_layout(GTK_ENTRY(ih->handle));
}

static int gtkTextSetBgColorAttrib(Ihandle* ih, const char* value)
{
  if (ih->data->is_multiline)
  {
    GtkScrolledWindow* scrolled_window = (GtkScrolledWindow*)iupAttribGetStr(ih, "_IUP_EXTRAPARENT");
    unsigned char r, g, b;

    /* ignore given value, must use only from parent for the scrollbars */
    char* parent_value = iupAttribGetStrNativeParent(ih, "BGCOLOR");
    if (!parent_value) parent_value = IupGetGlobal("DLGBGCOLOR");

    if (iupStrToRGB(parent_value, &r, &g, &b))
    {
      GtkWidget* sb;

      iupgtkBaseSetBgColor((GtkWidget*)scrolled_window, r, g, b);

#if GTK_CHECK_VERSION(2, 8, 0)
      sb = gtk_scrolled_window_get_hscrollbar(scrolled_window);
      if (sb) iupgtkBaseSetBgColor(sb, r, g, b);

      sb = gtk_scrolled_window_get_vscrollbar(scrolled_window);
      if (sb) iupgtkBaseSetBgColor(sb, r, g, b);
#endif
    }
  }

  return iupdrvBaseSetBgColorAttrib(ih, value);
}

static int gtkTextSetTabSizeAttrib(Ihandle* ih, const char* value)
{
  PangoTabArray *tabs;
  int tabsize, charwidth;
  if (!ih->data->is_multiline)
    return 0;

  iupStrToInt(value, &tabsize);
  charwidth = gtkTextGetCharSize(ih);
  tabsize *= charwidth;
  tabs = pango_tab_array_new_with_positions(1, FALSE, PANGO_TAB_LEFT, tabsize);
  gtk_text_view_set_tabs(GTK_TEXT_VIEW(ih->handle), tabs);
  pango_tab_array_free(tabs);
  return 1;
}

static char* gtkTextGetFormattingAttrib(Ihandle* ih)
{
  if (ih->data->has_formatting)
    return "YES";
  else
    return "NO";
}

static int gtkTextSetOverwriteAttrib(Ihandle* ih, const char* value)
{
  if (!ih->data->is_multiline)
    return 0;
  gtk_text_view_set_overwrite(GTK_TEXT_VIEW(ih->handle), iupStrBoolean(value));
  return 0;
}

static char* gtkTextGetOverwriteAttrib(Ihandle* ih)
{
  if (!ih->data->is_multiline)
    return "NO";
  if (gtk_text_view_get_overwrite(GTK_TEXT_VIEW(ih->handle)))
    return "YES";
  else
    return "NO";
}

void iupdrvTextAddFormatTag(Ihandle* ih, Ihandle* formattag)
{
  GtkTextBuffer *buffer;
  GtkTextIter start_iter, end_iter;
  GtkTextTag* tag;
  char *selection;

  if (!ih->data->is_multiline)
    return;

  selection = iupAttribGetStr(formattag, "SELECTION");
  if (selection)
  {
    /* simulate Windows behavior and change the current selection */
    gtkTextSetSelectionAttrib(ih, selection);
    iupAttribSetStr(ih, "SELECTION", NULL);
  }
  else
  {
    char* selectionpos = iupAttribGetStr(formattag, "SELECTIONPOS");
    if (selectionpos)
    {
      /* simulate Windows behavior and change the current selection */
      gtkTextSetSelectionPosAttrib(ih, selectionpos);
      iupAttribSetStr(ih, "SELECTIONPOS", NULL);
    }
  }

  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));
  if (!gtk_text_buffer_get_selection_bounds(buffer, &start_iter, &end_iter))
  {
    GtkTextMark* mark = gtk_text_buffer_get_insert(buffer);
    gtk_text_buffer_get_iter_at_mark(buffer, &start_iter, mark);
    gtk_text_buffer_get_iter_at_mark(buffer, &end_iter, mark);
  }

  tag = gtk_text_buffer_create_tag(buffer, NULL, NULL);
  gtkTextParseParagraphFormat(formattag, tag);
  gtkTextParseCharacterFormat(formattag, tag);
  gtk_text_buffer_apply_tag(buffer, tag, &start_iter, &end_iter);

  /* reset the selection */
  gtkTextSetSelectionAttrib(ih, NULL);
}

static int gtkTextSetRemoveFormattingAttrib(Ihandle* ih, const char* value)
{
  GtkTextBuffer *buffer;
  GtkTextIter start_iter, end_iter;

  if (!ih->data->is_multiline)
    return 0;

  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));
  if (gtk_text_buffer_get_selection_bounds(buffer, &start_iter, &end_iter))
    gtk_text_buffer_remove_all_tags(buffer, &start_iter, &end_iter);

  (void)value;
  return 0;
}


/************************************************************************************************/

static gboolean gtkTextSpinOutput(GtkSpinButton *spin, Ihandle* ih)
{
  if (iupAttribGetStr(ih, "_IUPGTK_SPIN_NOAUTO"))
  {
    iupAttribSetInt(ih, "_IUPGTK_SPIN_VALUE", (int)spin->adjustment->value);
    return TRUE; /* disable output update */
  }
  else
  {
    iupAttribSetStr(ih, "_IUPGTK_DISABLE_TEXT_CB", "1");
    return FALSE;
  }
}

static gint gtkTextSpinInput(GtkSpinButton *spin, gdouble *val, Ihandle* ih)
{
  (void)spin;
  *val = (double)iupAttribGetInt(ih, "_IUPGTK_SPIN_VALUE");
  /* called only when SPINAUTO=NO */
  return TRUE; /* disable input update */
}

static void gtkTextSpinValueChanged(GtkSpinButton* spin, Ihandle* ih)
{
  IFni cb;

  iupAttribSetStr(ih, "_IUPGTK_DISABLE_TEXT_CB", NULL);

  cb = (IFni)IupGetCallback(ih, "SPIN_CB");
  if (cb) 
  {
    int pos;
    if (iupAttribGetStr(ih, "_IUPGTK_SPIN_NOAUTO"))
      pos = iupAttribGetInt(ih, "_IUPGTK_SPIN_VALUE");
    else
      pos = gtk_spin_button_get_value_as_int((GtkSpinButton*)ih->handle);

    cb(ih, pos);
  }

  (void)spin;
}

static int gtkTextSetSpinMinAttrib(Ihandle* ih, const char* value)
{
  if (GTK_IS_SPIN_BUTTON(ih->handle))
  {
    int min;
    if (iupStrToInt(value, &min))
    {
      int max = iupAttribGetIntDefault(ih, "SPINMAX");

      iupAttribSetStr(ih, "_IUPGTK_DISABLE_TEXT_CB", "1");
      g_signal_handlers_block_by_func(G_OBJECT(ih->handle), G_CALLBACK(gtkTextSpinValueChanged), ih);

      gtk_spin_button_set_range((GtkSpinButton*)ih->handle, (double)min, (double)max);

      g_signal_handlers_unblock_by_func(G_OBJECT(ih->handle), G_CALLBACK(gtkTextSpinValueChanged), ih);
      iupAttribSetStr(ih, "_IUPGTK_DISABLE_TEXT_CB", NULL);
    }
  }
  return 1;
}

static int gtkTextSetSpinMaxAttrib(Ihandle* ih, const char* value)
{
  if (GTK_IS_SPIN_BUTTON(ih->handle))
  {
    int max;
    if (iupStrToInt(value, &max))
    {
      int min = iupAttribGetIntDefault(ih, "SPINMIN");
      iupAttribSetStr(ih, "_IUPGTK_DISABLE_TEXT_CB", "1");
      g_signal_handlers_block_by_func(G_OBJECT(ih->handle), G_CALLBACK(gtkTextSpinValueChanged), ih);

      gtk_spin_button_set_range((GtkSpinButton*)ih->handle, (double)min, (double)max);

      g_signal_handlers_unblock_by_func(G_OBJECT(ih->handle), G_CALLBACK(gtkTextSpinValueChanged), ih);
      iupAttribSetStr(ih, "_IUPGTK_DISABLE_TEXT_CB", NULL);
    }
  }
  return 1;
}

static int gtkTextSetSpinIncAttrib(Ihandle* ih, const char* value)
{
  if (GTK_IS_SPIN_BUTTON(ih->handle))
  {
    int inc;
    if (iupStrToInt(value, &inc))
      gtk_spin_button_set_increments((GtkSpinButton*)ih->handle, (double)inc, (double)(inc*10));
  }
  return 1;
}

static int gtkTextSetSpinValueAttrib(Ihandle* ih, const char* value)
{
  if (GTK_IS_SPIN_BUTTON(ih->handle))
  {
    int pos;
    if (iupStrToInt(value, &pos))
    {
      iupAttribSetStr(ih, "_IUPGTK_DISABLE_TEXT_CB", "1");
      g_signal_handlers_block_by_func(G_OBJECT(ih->handle), G_CALLBACK(gtkTextSpinValueChanged), ih);

      gtk_spin_button_set_value((GtkSpinButton*)ih->handle, (double)pos);

      g_signal_handlers_unblock_by_func(G_OBJECT(ih->handle), G_CALLBACK(gtkTextSpinValueChanged), ih);
      iupAttribSetStr(ih, "_IUPGTK_DISABLE_TEXT_CB", NULL);

      if (iupAttribGetStr(ih, "_IUPGTK_SPIN_NOAUTO"))
        iupAttribSetInt(ih, "_IUPGTK_SPIN_VALUE", pos);
    }
  }
  return 1;
}

static char* gtkTextGetSpinValueAttrib(Ihandle* ih)
{
  if (GTK_IS_SPIN_BUTTON(ih->handle))
  {
    int pos;
    char *str = iupStrGetMemory(50);

    if (iupAttribGetStr(ih, "_IUPGTK_SPIN_NOAUTO"))
      pos = iupAttribGetInt(ih, "_IUPGTK_SPIN_VALUE");
    else
      pos = gtk_spin_button_get_value_as_int((GtkSpinButton*)ih->handle);

    sprintf(str, "%d", pos);
    return str;
  }
  return NULL;
}


/**********************************************************************************************************/


static void gtkTextMoveCursor(GtkWidget *w, GtkMovementStep step, gint count, gboolean extend_selection, Ihandle* ih)
{
  int col, lin, pos;

  IFniii cb = (IFniii)IupGetCallback(ih, "CARET_CB");
  if (!cb) return;

  if (ih->data->is_multiline)
  {
    GtkTextIter iter;

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));
    gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
    gtkTextGetLinColFromPosition(&iter, &lin, &col);
    pos = gtk_text_iter_get_offset(&iter);
  }
  else
  {
    pos = gtk_editable_get_position(GTK_EDITABLE(ih->handle));
    col = pos;
    col++; /* IUP starts at 1 */
    lin = 1;
  }

  if (pos != ih->data->last_caret_pos)
  {
    ih->data->last_caret_pos = pos;

    cb(ih, lin, col, pos);
  }

  (void)w;
  (void)step;
  (void)count;
  (void)extend_selection;
}

static gboolean gtkTextKeyReleaseEvent(GtkWidget *widget, GdkEventKey *evt, Ihandle *ih)
{
  gtkTextMoveCursor(NULL, 0, 0, 0, ih);
  (void)widget;
  (void)evt;
  return FALSE;
}

static gboolean gtkTextButtonEvent(GtkWidget *widget, GdkEventButton *evt, Ihandle *ih)
{
  gtkTextMoveCursor(NULL, 0, 0, 0, ih);
  return iupgtkButtonEvent(widget, evt, ih);
}

static int gtkTextCallActionCb(Ihandle* ih, const char* insert_value, int len, int start, int end)
{
  char *new_value, *value;
  int ret = -1, key = 0;

  IFnis cb = (IFnis)IupGetCallback(ih, "ACTION");
  if (!cb && !ih->data->mask)
    return -1; /* continue */

  value = gtkTextGetValueAttrib(ih);  /* new_value is the internal buffer */

  if (!insert_value)
  {
    new_value = iupStrDup(value);
    if (end<0) end = strlen(value)+1;
    iupStrRemove(new_value, start, end, 1);
  }
  else
  {
    if (!value)
      new_value = iupStrDup(insert_value);
    else
    {
      if (len < end-start)
      {
        new_value = iupStrDup(value);
        new_value = iupStrInsert(new_value, insert_value, start, end);
      }
      else
        new_value = iupStrInsert(value, insert_value, start, end);
    }
  }

  if (insert_value && insert_value[0]!=0 && insert_value[1]==0)
    key = insert_value[0];

  if (!new_value)
    return -1; /* continue */

  if (ih->data->nc && (int)strlen(new_value) > ih->data->nc)
  {
    if (new_value != value) free(new_value);
    return 0; /* abort */
  }

  if (ih->data->mask && iupMaskCheck(ih->data->mask, new_value)==0)
  {
    if (new_value != value) free(new_value);
    return 0; /* abort */
  }

  if (cb)
  {
    int cb_ret = cb(ih, key, (char*)new_value);
    if (cb_ret==IUP_IGNORE)
      ret = 0; /* abort */
    else if (cb_ret==IUP_CLOSE)
    {
      IupExitLoop();
      ret = 0; /* abort */
    }
    else if (cb_ret!=0 && key!=0 && 
             cb_ret != IUP_DEFAULT && cb_ret != IUP_CONTINUE)  
      ret = cb_ret; /* abort and replace */
  }

  if (new_value != value) free(new_value);
  return ret; /* continue */
}

static void gtkTextEntryDeleteText(GtkEditable *editable, int start, int end, Ihandle* ih)
{
  if (iupAttribGetStr(ih, "_IUPGTK_DISABLE_TEXT_CB"))
    return;

  if (gtkTextCallActionCb(ih, NULL, 0, start, end)==0)
    g_signal_stop_emission_by_name (editable, "delete_text");
}

static void gtkTextEntryInsertText(GtkEditable *editable, char *insert_value, int len, int *pos, Ihandle* ih)
{
  int ret;

  if (iupAttribGetStr(ih, "_IUPGTK_DISABLE_TEXT_CB"))
    return;

  ret = gtkTextCallActionCb(ih, iupStrGetMemoryCopy(iupgtkStrConvertFromUTF8(insert_value)), len, *pos, *pos);
  if (ret == 0)
    g_signal_stop_emission_by_name(editable, "insert_text");
  else if (ret != -1)
  {
    insert_value[0] = (char)ret;  /* replace key */

    /* disable callbacks */
    iupAttribSetStr(ih, "_IUPGTK_DISABLE_TEXT_CB", "1");
    gtk_editable_insert_text(editable, insert_value, 1, pos);
    iupAttribSetStr(ih, "_IUPGTK_DISABLE_TEXT_CB", NULL);

    g_signal_stop_emission_by_name(editable, "insert_text"); 
  }
}

static void gtkTextBufferDeleteRange(GtkTextBuffer *textbuffer, GtkTextIter *start_iter, GtkTextIter *end_iter, Ihandle* ih)
{
  int start, end;
  if (iupAttribGetStr(ih, "_IUPGTK_DISABLE_TEXT_CB"))
    return;

  start = gtk_text_iter_get_offset(start_iter);
  end = gtk_text_iter_get_offset(end_iter);

  if (gtkTextCallActionCb(ih, NULL, 0, start, end)==0)
    g_signal_stop_emission_by_name (textbuffer, "delete_range");
}

static void gtkTextBufferInsertText(GtkTextBuffer *textbuffer, GtkTextIter *pos_iter, gchar *insert_value, gint len, Ihandle* ih)
{
  int ret, pos;

  if (iupAttribGetStr(ih, "_IUPGTK_DISABLE_TEXT_CB"))
    return;

  pos = gtk_text_iter_get_offset(pos_iter);

  ret = gtkTextCallActionCb(ih, iupStrGetMemoryCopy(iupgtkStrConvertFromUTF8(insert_value)), len, pos, pos);
  if (ret == 0)
    g_signal_stop_emission_by_name(textbuffer, "insert_text");
  else if (ret != -1)
  {
    insert_value[0] = (char)ret;  /* replace key */

    /* disable callbacks */
    iupAttribSetStr(ih, "_IUPGTK_DISABLE_TEXT_CB", "1");
    gtk_text_buffer_insert(textbuffer, pos_iter, insert_value, 1);
    iupAttribSetStr(ih, "_IUPGTK_DISABLE_TEXT_CB", NULL);

    g_signal_stop_emission_by_name(textbuffer, "insert_text"); 
  }
}


/**********************************************************************************************************/


static int gtkTextMapMethod(Ihandle* ih)
{
  GtkScrolledWindow* scrolled_window = NULL;

  if (ih->data->is_multiline)
  {
    GtkPolicyType hscrollbar_policy, vscrollbar_policy;
    int wordwrap = 0;

    ih->handle = gtk_text_view_new();
    if (!ih->handle)
      return IUP_ERROR;

    scrolled_window = (GtkScrolledWindow*)gtk_scrolled_window_new(NULL, NULL);
    if (!scrolled_window)
      return IUP_ERROR;

    gtk_container_add((GtkContainer*)scrolled_window, ih->handle);
    gtk_widget_show((GtkWidget*)scrolled_window);

    iupAttribSetStr(ih, "_IUP_EXTRAPARENT", (char*)scrolled_window);

    /* formatting is always supported when MULTILINE=YES */
    ih->data->has_formatting = 1;

    if (iupStrBoolean(iupAttribGetStr(ih, "WORDWRAP")))
    {
      wordwrap = 1;
      ih->data->sb &= ~IUP_SB_HORIZ;  /* must remove the horizontal scroolbar */
    }

    if (IupGetInt(ih, "BORDER"))              /* Use IupGetInt for inheritance */
      gtk_scrolled_window_set_shadow_type(scrolled_window, GTK_SHADOW_IN); 
    else
      gtk_scrolled_window_set_shadow_type(scrolled_window, GTK_SHADOW_NONE);

    if (ih->data->sb & IUP_SB_HORIZ)
    {
      if (iupStrBoolean(iupAttribGetStr(ih, "AUTOHIDE")))
        hscrollbar_policy = GTK_POLICY_AUTOMATIC;
      else
        hscrollbar_policy = GTK_POLICY_ALWAYS;
    }
    else
      hscrollbar_policy = GTK_POLICY_NEVER;

    if (ih->data->sb & IUP_SB_VERT)
    {
      if (iupStrBoolean(iupAttribGetStr(ih, "AUTOHIDE")))
        vscrollbar_policy = GTK_POLICY_AUTOMATIC;
      else
        vscrollbar_policy = GTK_POLICY_ALWAYS;
    }
    else
      vscrollbar_policy = GTK_POLICY_NEVER;

    gtk_scrolled_window_set_policy(scrolled_window, hscrollbar_policy, vscrollbar_policy);

    if (wordwrap)
      gtk_text_view_set_wrap_mode((GtkTextView*)ih->handle, GTK_WRAP_WORD);

    gtk_widget_add_events(ih->handle, GDK_ENTER_NOTIFY_MASK|GDK_LEAVE_NOTIFY_MASK);
  }
  else
  {
    if (iupAttribGetInt(ih, "SPIN"))
      ih->handle = gtk_spin_button_new_with_range(0, 100, 1);
    else
      ih->handle = gtk_entry_new();

    if (!ih->handle)
      return IUP_ERROR;
    
    gtk_entry_set_has_frame((GtkEntry*)ih->handle, IupGetInt(ih, "BORDER"));

    if (iupStrBoolean(iupAttribGetStr(ih, "PASSWORD")))
      gtk_entry_set_visibility((GtkEntry*)ih->handle, FALSE);

    if (GTK_IS_SPIN_BUTTON(ih->handle))
    {
      gtk_spin_button_set_numeric((GtkSpinButton*)ih->handle, FALSE);
      gtk_spin_button_set_digits((GtkSpinButton*)ih->handle, 0);

      gtk_spin_button_set_wrap((GtkSpinButton*)ih->handle, iupStrBoolean(iupAttribGetStr(ih, "SPINWRAP")));

      g_signal_connect(G_OBJECT(ih->handle), "value-changed", G_CALLBACK(gtkTextSpinValueChanged), ih);
      g_signal_connect(G_OBJECT(ih->handle), "output", G_CALLBACK(gtkTextSpinOutput), ih);

      if (!iupStrBoolean(iupAttribGetStrDefault(ih, "SPINAUTO")))
      {
        g_signal_connect(G_OBJECT(ih->handle), "input", G_CALLBACK(gtkTextSpinInput), ih);
        iupAttribSetStr(ih, "_IUPGTK_SPIN_NOAUTO", "1");
      }
    }
  }

  /* add to the parent, all GTK controls must call this. */
  iupgtkBaseAddToParent(ih);

  if (!iupStrBoolean(iupAttribGetStrDefault(ih, "CANFOCUS")))
    GTK_WIDGET_FLAGS(ih->handle) &= ~GTK_CAN_FOCUS;

  g_signal_connect(G_OBJECT(ih->handle), "enter-notify-event", G_CALLBACK(iupgtkEnterLeaveEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "leave-notify-event", G_CALLBACK(iupgtkEnterLeaveEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "focus-in-event",     G_CALLBACK(iupgtkFocusInOutEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "focus-out-event",    G_CALLBACK(iupgtkFocusInOutEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "key-press-event",    G_CALLBACK(iupgtkKeyPressEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "show-help",          G_CALLBACK(iupgtkShowHelp), ih);

  g_signal_connect_after(G_OBJECT(ih->handle), "move-cursor", G_CALLBACK(gtkTextMoveCursor), ih);  /* only report some caret movements */
  g_signal_connect_after(G_OBJECT(ih->handle), "key-release-event", G_CALLBACK(gtkTextKeyReleaseEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "button-press-event", G_CALLBACK(gtkTextButtonEvent), ih);  /* if connected "after" then it is ignored */
  g_signal_connect(G_OBJECT(ih->handle), "button-release-event",G_CALLBACK(gtkTextButtonEvent), ih);
  g_signal_connect(G_OBJECT(ih->handle), "motion-notify-event",G_CALLBACK(iupgtkMotionNotifyEvent), ih);

  if (ih->data->is_multiline)
  {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ih->handle));
    g_signal_connect(G_OBJECT(buffer), "delete-range", G_CALLBACK(gtkTextBufferDeleteRange), ih);
    g_signal_connect(G_OBJECT(buffer), "insert-text", G_CALLBACK(gtkTextBufferInsertText), ih);
  }
  else
  {
    g_signal_connect(G_OBJECT(ih->handle), "delete-text", G_CALLBACK(gtkTextEntryDeleteText), ih);
    g_signal_connect(G_OBJECT(ih->handle), "insert-text", G_CALLBACK(gtkTextEntryInsertText), ih);
  }

  if (scrolled_window)
    gtk_widget_realize((GtkWidget*)scrolled_window);
  gtk_widget_realize(ih->handle);

  /* ensure the default values, that are different from the native ones */
  gtkTextSetTabSizeAttrib(ih, iupAttribGetStrDefault(ih, "TABSIZE"));

  /* configure for DRAG&DROP */
  if (IupGetCallback(ih, "DROPFILES_CB"))
    iupAttribSetStr(ih, "DRAGDROP", "YES");

  /* update a mnemonic in a label if necessary */
  iupgtkUpdateMnemonic(ih);

  if (ih->data->formattags)
    iupTextUpdateFormatTags(ih);

  return IUP_NOERROR;
}

void iupdrvTextInitClass(Iclass* ic)
{
  /* Driver Dependent Class functions */
  ic->Map = gtkTextMapMethod;

  /* Driver Dependent Attribute functions */

  /* Common GTK only (when text is in a secondary element) */
  iupClassRegisterAttribute(ic, "PANGOLAYOUT", gtkTextGetPangoLayoutAttrib, NULL, NULL, IUP_MAPPED, IUP_NO_INHERIT);

  /* Visual */
  iupClassRegisterAttribute(ic, "BGCOLOR", NULL, gtkTextSetBgColorAttrib, "TXTBGCOLOR", IUP_MAPPED, IUP_INHERIT);

  /* Special */
  iupClassRegisterAttribute(ic, "FGCOLOR", NULL, iupdrvBaseSetFgColorAttrib, "TXTFGCOLOR", IUP_MAPPED, IUP_INHERIT);

  /* IupText only */
  iupClassRegisterAttribute(ic, "ALIGNMENT", NULL, gtkTextSetAlignmentAttrib, "ALEFT", IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "PADDING", iupTextGetPaddingAttrib, gtkTextSetPaddingAttrib, "0x0", IUP_NOT_MAPPED, IUP_INHERIT);
  iupClassRegisterAttribute(ic, "VALUE", gtkTextGetValueAttrib, gtkTextSetValueAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SELECTEDTEXT", gtkTextGetSelectedTextAttrib, gtkTextSetSelectedTextAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SELECTION", gtkTextGetSelectionAttrib, gtkTextSetSelectionAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SELECTIONPOS", gtkTextGetSelectionPosAttrib, gtkTextSetSelectionPosAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CARET", gtkTextGetCaretAttrib, gtkTextSetCaretAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "CARETPOS", gtkTextGetCaretPosAttrib, gtkTextSetCaretPosAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "INSERT", NULL, gtkTextSetInsertAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "APPEND", NULL, gtkTextSetAppendAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "READONLY", gtkTextGetReadOnlyAttrib, gtkTextSetReadOnlyAttrib, NULL, IUP_MAPPED, IUP_INHERIT);
  iupClassRegisterAttribute(ic, "NC", iupTextGetNCAttrib, gtkTextSetNCAttrib, NULL, IUP_NOT_MAPPED, IUP_INHERIT);
  iupClassRegisterAttribute(ic, "CLIPBOARD", NULL, gtkTextSetClipboardAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "TABSIZE", NULL, gtkTextSetTabSizeAttrib, "8", IUP_MAPPED, IUP_INHERIT);
  iupClassRegisterAttribute(ic, "FORMATTING", gtkTextGetFormattingAttrib, iupBaseNoSetAttrib, NULL, IUP_NOT_MAPPED, IUP_NO_INHERIT);  /* can not set FORMATTING, it depends on MULTILINE */
  iupClassRegisterAttribute(ic, "REMOVEFORMATTING", NULL, gtkTextSetRemoveFormattingAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "OVERWRITE", gtkTextGetOverwriteAttrib, gtkTextSetOverwriteAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "DRAGDROP", NULL, iupgtkSetDragDropAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SCROLLTO", NULL, gtkTextSetScrollToAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SCROLLTOPOS", NULL, gtkTextSetScrollToPosAttrib, NULL, IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SPINMIN", NULL, gtkTextSetSpinMinAttrib, "0", IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SPINMAX", NULL, gtkTextSetSpinMaxAttrib, "100", IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SPININC", NULL, gtkTextSetSpinIncAttrib, "1", IUP_MAPPED, IUP_NO_INHERIT);
  iupClassRegisterAttribute(ic, "SPINVALUE", gtkTextGetSpinValueAttrib, gtkTextSetSpinValueAttrib, "0", IUP_MAPPED, IUP_NO_INHERIT);
}
