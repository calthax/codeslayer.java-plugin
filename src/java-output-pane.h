/*
 * Copyright (C) 2010 - Jeff Johnston
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __JAVA_OUTPUT_PANE_H__
#define	__JAVA_OUTPUT_PANE_H__

#include <gtk/gtk.h>
#include "java-page.h"

G_BEGIN_DECLS

#define JAVA_OUTPUT_PANE_TYPE            (java_output_pane_get_type ())
#define JAVA_OUTPUT_PANE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_OUTPUT_PANE_TYPE, JavaOutputPane))
#define JAVA_OUTPUT_PANE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_OUTPUT_PANE_TYPE, JavaOutputPaneClass))
#define IS_JAVA_OUTPUT_PANE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_OUTPUT_PANE_TYPE))
#define IS_JAVA_OUTPUT_PANE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_OUTPUT_PANE_TYPE))

typedef struct _JavaOutputPane JavaOutputPane;
typedef struct _JavaOutputPaneClass JavaOutputPaneClass;

struct _JavaOutputPane
{
  GtkHBox parent_instance;
};

struct _JavaOutputPaneClass
{
  GtkHBoxClass parent_class;
};

GType java_output_pane_get_type (void) G_GNUC_CONST;

GtkWidget*    java_output_pane_new            (JavaPageType    page_type);

GtkTextView*  java_output_pane_get_text_view  (JavaOutputPane *output_pane);

G_END_DECLS

#endif /* __JAVA_OUTPUT_PANE_H__ */
