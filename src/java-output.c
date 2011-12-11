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

#include "java-output.h"

static void java_output_class_init  (JavaOutputClass *klass);
static void java_output_init        (JavaOutput      *output);
static void java_output_finalize    (JavaOutput      *output);

#define JAVA_OUTPUT_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_OUTPUT_TYPE, JavaOutputPrivate))

typedef struct _JavaOutputPrivate JavaOutputPrivate;

struct _JavaOutputPrivate
{
  JavaNotebookPageType page_type;
};

G_DEFINE_TYPE (JavaOutput, java_output, GTK_TYPE_TEXT_VIEW)
      
static void 
java_output_class_init (JavaOutputClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) java_output_finalize;
  g_type_class_add_private (klass, sizeof (JavaOutputPrivate));
}

static void
java_output_init (JavaOutput *output) 
{
  gtk_text_view_set_editable (GTK_TEXT_VIEW (output), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (output), GTK_WRAP_WORD);
}

static void
java_output_finalize (JavaOutput *output)
{
  G_OBJECT_CLASS (java_output_parent_class)->finalize (G_OBJECT (output));
}

GtkWidget*
java_output_new (JavaNotebookPageType page_type)
{
  GtkWidget *output;
  JavaOutputPrivate *priv;
 
  output = g_object_new (java_output_get_type (), NULL);
  priv = JAVA_OUTPUT_GET_PRIVATE (output);
  priv->page_type = page_type;
  
  return output;
}
