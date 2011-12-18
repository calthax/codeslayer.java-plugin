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

static void java_page_interface_init                     (gpointer            page, 
                                                          gpointer            data);
static void java_output_class_init                       (JavaOutputClass    *klass);
static void java_output_init                             (JavaOutput         *output);
static void java_output_finalize                         (JavaOutput         *output);
static JavaPageType java_output_get_page_type            (JavaOutput         *output);                                      
static JavaConfiguration* java_output_get_configuration  (JavaOutput         *output);
static void java_output_set_configuration                (JavaOutput         *output, 
                                                          JavaConfiguration  *configuration);
static CodeSlayerDocument* java_output_get_document      (JavaOutput         *output);
static void java_output_set_document                     (JavaOutput         *output, 
                                                          CodeSlayerDocument *document);

#define JAVA_OUTPUT_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_OUTPUT_TYPE, JavaOutputPrivate))

typedef struct _JavaOutputPrivate JavaOutputPrivate;

struct _JavaOutputPrivate
{
  JavaPageType        page_type;
  JavaConfiguration  *configuration;
  CodeSlayerDocument *document;
};

G_DEFINE_TYPE_EXTENDED (JavaOutput,
                        java_output,
                        GTK_TYPE_TEXT_VIEW,
                        0,
                        G_IMPLEMENT_INTERFACE (JAVA_PAGE_TYPE ,
                                               java_page_interface_init));
      
static void
java_page_interface_init (gpointer page, 
                          gpointer data)
{
  JavaPageInterface *page_interface = (JavaPageInterface*) page;
  page_interface->get_page_type = (JavaPageType (*) (JavaPage *obj)) java_output_get_page_type;
  page_interface->get_configuration = (JavaConfiguration* (*) (JavaPage *obj)) java_output_get_configuration;
  page_interface->set_configuration = (void (*) (JavaPage *obj, JavaConfiguration*)) java_output_set_configuration;
  page_interface->get_document = (CodeSlayerDocument* (*) (JavaPage *obj)) java_output_get_document;
  page_interface->set_document = (void (*) (JavaPage *obj, CodeSlayerDocument*)) java_output_set_document;
}
      
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
java_output_new (JavaPageType page_type)
{
  GtkWidget *output;
  JavaOutputPrivate *priv;
 
  output = g_object_new (java_output_get_type (), NULL);
  priv = JAVA_OUTPUT_GET_PRIVATE (output);
  priv->page_type = page_type;
  
  return output;
}

static JavaPageType 
java_output_get_page_type (JavaOutput *output)
{
  JavaOutputPrivate *priv;
  priv = JAVA_OUTPUT_GET_PRIVATE (output);
  return priv->page_type;
}

static JavaConfiguration* 
java_output_get_configuration (JavaOutput *output)
{
  JavaOutputPrivate *priv;
  priv = JAVA_OUTPUT_GET_PRIVATE (output);
  return priv->configuration;
}

static void 
java_output_set_configuration (JavaOutput        *output, 
                               JavaConfiguration *configuration)
{
  JavaOutputPrivate *priv;
  priv = JAVA_OUTPUT_GET_PRIVATE (output);
  priv->configuration = configuration;
}                               

static CodeSlayerDocument* 
java_output_get_document (JavaOutput *output)
{
  JavaOutputPrivate *priv;
  priv = JAVA_OUTPUT_GET_PRIVATE (output);
  return priv->document;
}

static void 
java_output_set_document (JavaOutput         *output, 
                          CodeSlayerDocument *document)
{
  JavaOutputPrivate *priv;
  priv = JAVA_OUTPUT_GET_PRIVATE (output);
  priv->document = document;
}                               
