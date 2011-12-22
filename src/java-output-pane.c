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

#include "java-output-pane.h"

static void java_page_interface_init                          (gpointer             page, 
                                                               gpointer             data);
static void java_output_pane_class_init                       (JavaOutputPaneClass *klass);
static void java_output_pane_init                             (JavaOutputPane      *output_pane);
static void java_output_pane_finalize                         (JavaOutputPane      *output_pane);
static JavaPageType java_output_pane_get_page_type            (JavaOutputPane      *output_pane);                                      
static JavaConfiguration* java_output_pane_get_configuration  (JavaOutputPane      *output_pane);
static void java_output_pane_set_configuration                (JavaOutputPane      *output_pane, 
                                                               JavaConfiguration   *configuration);
static CodeSlayerDocument* java_output_pane_get_document      (JavaOutputPane      *output_pane);
static void java_output_pane_set_document                     (JavaOutputPane      *output_pane, 
                                                               CodeSlayerDocument  *document);
                                                               
static void add_text_view                                     (JavaOutputPane      *output_pane);
static void add_buttons                                       (JavaOutputPane      *output_pane);
static void clear_action                                      (GtkWidget           *text_view);
                                                               

#define JAVA_OUTPUT_PANE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_OUTPUT_PANE_TYPE, JavaOutputPanePrivate))

typedef struct _JavaOutputPanePrivate JavaOutputPanePrivate;

struct _JavaOutputPanePrivate
{
  GtkWidget          *text_view;
  JavaPageType        page_type;
  JavaConfiguration  *configuration;
  CodeSlayerDocument *document;
};

G_DEFINE_TYPE_EXTENDED (JavaOutputPane,
                        java_output_pane,
                        GTK_TYPE_HBOX,
                        0,
                        G_IMPLEMENT_INTERFACE (JAVA_PAGE_TYPE ,
                                               java_page_interface_init));
      
static void
java_page_interface_init (gpointer page, 
                          gpointer data)
{
  JavaPageInterface *page_interface = (JavaPageInterface*) page;
  page_interface->get_page_type = (JavaPageType (*) (JavaPage *obj)) java_output_pane_get_page_type;
  page_interface->get_configuration = (JavaConfiguration* (*) (JavaPage *obj)) java_output_pane_get_configuration;
  page_interface->set_configuration = (void (*) (JavaPage *obj, JavaConfiguration*)) java_output_pane_set_configuration;
  page_interface->get_document = (CodeSlayerDocument* (*) (JavaPage *obj)) java_output_pane_get_document;
  page_interface->set_document = (void (*) (JavaPage *obj, CodeSlayerDocument*)) java_output_pane_set_document;
}
      
static void 
java_output_pane_class_init (JavaOutputPaneClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) java_output_pane_finalize;
  g_type_class_add_private (klass, sizeof (JavaOutputPanePrivate));
}

static void
java_output_pane_init (JavaOutputPane *output_pane) 
{
  JavaOutputPanePrivate *priv;
  priv = JAVA_OUTPUT_PANE_GET_PRIVATE (output_pane);
  priv->text_view = gtk_text_view_new ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (priv->text_view), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (priv->text_view), GTK_WRAP_WORD);
}

static void
java_output_pane_finalize (JavaOutputPane *output_pane)
{
  G_OBJECT_CLASS (java_output_pane_parent_class)->finalize (G_OBJECT (output_pane));
}

GtkWidget*
java_output_pane_new (JavaPageType page_type)
{
  JavaOutputPanePrivate *priv;
  GtkWidget *output_pane;
 
  output_pane = g_object_new (java_output_pane_get_type (), NULL);
  priv = JAVA_OUTPUT_PANE_GET_PRIVATE (output_pane);
  priv->page_type = page_type;
  
  add_text_view (JAVA_OUTPUT_PANE (output_pane));
  add_buttons (JAVA_OUTPUT_PANE (output_pane));
  
  return output_pane;
}

GtkTextView*
java_output_pane_get_text_view (JavaOutputPane *output_pane)
{
  JavaOutputPanePrivate *priv;
  priv = JAVA_OUTPUT_PANE_GET_PRIVATE (output_pane);
  return GTK_TEXT_VIEW (priv->text_view);
}

static void 
add_text_view (JavaOutputPane *output_pane)
{
  JavaOutputPanePrivate *priv;
  GtkWidget *scrolled_window;

  priv = JAVA_OUTPUT_PANE_GET_PRIVATE (output_pane);
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrolled_window), priv->text_view);

  gtk_box_pack_start (GTK_BOX (output_pane), scrolled_window, TRUE, TRUE, 0);
}

static void 
add_buttons (JavaOutputPane *output_pane)
{
  JavaOutputPanePrivate *priv;
  GtkWidget *table;
  GtkWidget *clear_button;
  GtkWidget *clear_image;
  
  priv = JAVA_OUTPUT_PANE_GET_PRIVATE (output_pane);

  table = gtk_table_new (1, 1, FALSE);
  
  clear_button = gtk_button_new ();
  gtk_widget_set_tooltip_text (clear_button, "Clear");

  gtk_button_set_relief (GTK_BUTTON (clear_button), GTK_RELIEF_NONE);
  gtk_button_set_focus_on_click (GTK_BUTTON (clear_button), FALSE);
  clear_image = gtk_image_new_from_stock (GTK_STOCK_CLEAR, GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (clear_button), clear_image);
  gtk_widget_set_can_focus (clear_button, FALSE);
  
  gtk_table_attach (GTK_TABLE (table), clear_button, 
                    0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);                                                         

  gtk_box_pack_start (GTK_BOX (output_pane), table, FALSE, FALSE, 2);
  
  g_signal_connect_swapped (G_OBJECT (clear_button), "clicked",
                            G_CALLBACK (clear_action), priv->text_view);
}

static void
clear_action (GtkWidget *text_view)
{
  GtkTextBuffer *buffer;
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
  gtk_text_buffer_set_text (buffer, "", -1);
}

static JavaPageType 
java_output_pane_get_page_type (JavaOutputPane *output_pane)
{
  JavaOutputPanePrivate *priv;
  priv = JAVA_OUTPUT_PANE_GET_PRIVATE (output_pane);
  return priv->page_type;
}

static JavaConfiguration* 
java_output_pane_get_configuration (JavaOutputPane *output_pane)
{
  JavaOutputPanePrivate *priv;
  priv = JAVA_OUTPUT_PANE_GET_PRIVATE (output_pane);
  return priv->configuration;
}

static void 
java_output_pane_set_configuration (JavaOutputPane    *output_pane, 
                                    JavaConfiguration *configuration)
{
  JavaOutputPanePrivate *priv;
  priv = JAVA_OUTPUT_PANE_GET_PRIVATE (output_pane);
  priv->configuration = configuration;
}                               

static CodeSlayerDocument* 
java_output_pane_get_document (JavaOutputPane *output_pane)
{
  JavaOutputPanePrivate *priv;
  priv = JAVA_OUTPUT_PANE_GET_PRIVATE (output_pane);
  return priv->document;
}

static void 
java_output_pane_set_document (JavaOutputPane     *output_pane, 
                               CodeSlayerDocument *document)
{
  JavaOutputPanePrivate *priv;
  priv = JAVA_OUTPUT_PANE_GET_PRIVATE (output_pane);
  priv->document = document;
}                         
