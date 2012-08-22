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

#include "java-usage-pane.h"

static void java_page_interface_init                         (gpointer             page, 
                                                              gpointer             data);
static void java_usage_pane_class_init                       (JavaUsagePaneClass *klass);
static void java_usage_pane_init                             (JavaUsagePane      *build_pane);
static void java_usage_pane_finalize                         (JavaUsagePane      *build_pane);
static JavaPageType java_usage_pane_get_page_type            (JavaUsagePane      *build_pane);                                      
static JavaConfiguration* java_usage_pane_get_configuration  (JavaUsagePane      *build_pane);
static void java_usage_pane_set_configuration                (JavaUsagePane      *build_pane, 
                                                              JavaConfiguration   *configuration);
static CodeSlayerDocument* java_usage_pane_get_document      (JavaUsagePane      *build_pane);
static void java_usage_pane_set_document                     (JavaUsagePane      *build_pane, 
                                                              CodeSlayerDocument  *document);
                                                               
static void add_text_view                                    (JavaUsagePane      *build_pane);
static void add_buttons                                      (JavaUsagePane      *build_pane);
static void clear_action                                     (GtkWidget           *text_view);
                                                               

#define JAVA_USAGE_PANE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_USAGE_PANE_TYPE, JavaUsagePanePrivate))

typedef struct _JavaUsagePanePrivate JavaUsagePanePrivate;

struct _JavaUsagePanePrivate
{
  GtkWidget          *text_view;
  JavaPageType        page_type;
  JavaConfiguration  *configuration;
  CodeSlayerDocument *document;
};

G_DEFINE_TYPE_EXTENDED (JavaUsagePane,
                        java_usage_pane,
                        GTK_TYPE_HBOX,
                        0,
                        G_IMPLEMENT_INTERFACE (JAVA_PAGE_TYPE ,
                                               java_page_interface_init));
      
static void
java_page_interface_init (gpointer page, 
                          gpointer data)
{
  JavaPageInterface *page_interface = (JavaPageInterface*) page;
  page_interface->get_page_type = (JavaPageType (*) (JavaPage *obj)) java_usage_pane_get_page_type;
  page_interface->get_configuration = (JavaConfiguration* (*) (JavaPage *obj)) java_usage_pane_get_configuration;
  page_interface->set_configuration = (void (*) (JavaPage *obj, JavaConfiguration*)) java_usage_pane_set_configuration;
  page_interface->get_document = (CodeSlayerDocument* (*) (JavaPage *obj)) java_usage_pane_get_document;
  page_interface->set_document = (void (*) (JavaPage *obj, CodeSlayerDocument*)) java_usage_pane_set_document;
}
      
static void 
java_usage_pane_class_init (JavaUsagePaneClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) java_usage_pane_finalize;
  g_type_class_add_private (klass, sizeof (JavaUsagePanePrivate));
}

static void
java_usage_pane_init (JavaUsagePane *build_pane) 
{
  JavaUsagePanePrivate *priv;
  priv = JAVA_USAGE_PANE_GET_PRIVATE (build_pane);
  priv->text_view = gtk_text_view_new ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (priv->text_view), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (priv->text_view), GTK_WRAP_WORD);
}

static void
java_usage_pane_finalize (JavaUsagePane *build_pane)
{
  G_OBJECT_CLASS (java_usage_pane_parent_class)->finalize (G_OBJECT (build_pane));
}

GtkWidget*
java_usage_pane_new (JavaPageType page_type)
{
  JavaUsagePanePrivate *priv;
  GtkWidget *build_pane;
 
  build_pane = g_object_new (java_usage_pane_get_type (), NULL);
  priv = JAVA_USAGE_PANE_GET_PRIVATE (build_pane);
  priv->page_type = page_type;
  
  add_text_view (JAVA_USAGE_PANE (build_pane));
  add_buttons (JAVA_USAGE_PANE (build_pane));
  
  return build_pane;
}

GtkTextView*
java_usage_pane_get_text_view (JavaUsagePane *build_pane)
{
  JavaUsagePanePrivate *priv;
  priv = JAVA_USAGE_PANE_GET_PRIVATE (build_pane);
  return GTK_TEXT_VIEW (priv->text_view);
}

static void 
add_text_view (JavaUsagePane *build_pane)
{
  JavaUsagePanePrivate *priv;
  GtkWidget *scrolled_window;

  priv = JAVA_USAGE_PANE_GET_PRIVATE (build_pane);
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrolled_window), priv->text_view);

  gtk_box_pack_start (GTK_BOX (build_pane), scrolled_window, TRUE, TRUE, 0);
}

static void 
add_buttons (JavaUsagePane *build_pane)
{
  JavaUsagePanePrivate *priv;
  GtkWidget *table;
  GtkWidget *clear_button;
  GtkWidget *clear_image;
  
  priv = JAVA_USAGE_PANE_GET_PRIVATE (build_pane);

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

  gtk_box_pack_start (GTK_BOX (build_pane), table, FALSE, FALSE, 2);
  
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
java_usage_pane_get_page_type (JavaUsagePane *build_pane)
{
  JavaUsagePanePrivate *priv;
  priv = JAVA_USAGE_PANE_GET_PRIVATE (build_pane);
  return priv->page_type;
}

static JavaConfiguration* 
java_usage_pane_get_configuration (JavaUsagePane *build_pane)
{
  JavaUsagePanePrivate *priv;
  priv = JAVA_USAGE_PANE_GET_PRIVATE (build_pane);
  return priv->configuration;
}

static void 
java_usage_pane_set_configuration (JavaUsagePane    *build_pane, 
                                   JavaConfiguration *configuration)
{
  JavaUsagePanePrivate *priv;
  priv = JAVA_USAGE_PANE_GET_PRIVATE (build_pane);
  priv->configuration = configuration;
}                               

static CodeSlayerDocument* 
java_usage_pane_get_document (JavaUsagePane *build_pane)
{
  JavaUsagePanePrivate *priv;
  priv = JAVA_USAGE_PANE_GET_PRIVATE (build_pane);
  return priv->document;
}

static void 
java_usage_pane_set_document (JavaUsagePane     *build_pane, 
                              CodeSlayerDocument *document)
{
  JavaUsagePanePrivate *priv;
  priv = JAVA_USAGE_PANE_GET_PRIVATE (build_pane);
  priv->document = document;
}                         
