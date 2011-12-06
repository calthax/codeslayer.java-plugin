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

#include "autotools-output.h"

static void autotools_output_class_init      (AutotoolsOutputClass *klass);
static void autotools_output_init            (AutotoolsOutput      *output);
static void autotools_output_finalize        (AutotoolsOutput      *output);

#define AUTOTOOLS_OUTPUT_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), AUTOTOOLS_OUTPUT_TYPE, AutotoolsOutputPrivate))

typedef struct _AutotoolsOutputPrivate AutotoolsOutputPrivate;

struct _AutotoolsOutputPrivate
{
  AutotoolsConfiguration *configuration;
};

G_DEFINE_TYPE (AutotoolsOutput, autotools_output, GTK_TYPE_TEXT_VIEW)
      
static void 
autotools_output_class_init (AutotoolsOutputClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) autotools_output_finalize;
  g_type_class_add_private (klass, sizeof (AutotoolsOutputPrivate));
}

static void
autotools_output_init (AutotoolsOutput *output) 
{
  gtk_text_view_set_editable (GTK_TEXT_VIEW (output), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (output), GTK_WRAP_WORD);
}

static void
autotools_output_finalize (AutotoolsOutput *output)
{
  G_OBJECT_CLASS (autotools_output_parent_class)->finalize (G_OBJECT (output));
}

GtkWidget*
autotools_output_new (AutotoolsConfiguration *configuration)
{
  GtkWidget *output;
  AutotoolsOutputPrivate *priv;
 
  output = g_object_new (autotools_output_get_type (), NULL);
  priv = AUTOTOOLS_OUTPUT_GET_PRIVATE (output);
  priv->configuration = configuration;
  
  return output;
}

AutotoolsConfiguration* 
autotools_output_get_configuration (AutotoolsOutput *output)
{
  AutotoolsOutputPrivate *priv;
  priv = AUTOTOOLS_OUTPUT_GET_PRIVATE (output);
  return priv->configuration;
}
