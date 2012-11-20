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

#include <stdlib.h>
#include <codeslayer/codeslayer-utils.h>
#include <codeslayer/codeslayer-linker.h>
#include "java-build-pane.h"

typedef struct
{
  gchar *file_path;
  gint   line_number;
  gint   start_offset;
  gint   end_offset;
} Link;

static void java_page_interface_init                         (gpointer             page, 
                                                              gpointer             data);
static void java_build_pane_class_init                       (JavaBuildPaneClass *klass);
static void java_build_pane_init                             (JavaBuildPane      *build_pane);
static void java_build_pane_finalize                         (JavaBuildPane      *build_pane);
static JavaPageType java_build_pane_get_page_type            (JavaBuildPane      *build_pane);                                      
static JavaConfiguration* java_build_pane_get_configuration  (JavaBuildPane      *build_pane);
static void java_build_pane_set_configuration                (JavaBuildPane      *build_pane, 
                                                              JavaConfiguration   *configuration);
static CodeSlayerDocument* java_build_pane_get_document      (JavaBuildPane      *build_pane);
static void java_build_pane_set_document                     (JavaBuildPane      *build_pane, 
                                                              CodeSlayerDocument  *document);
                                                               
static void add_text_view                                    (JavaBuildPane      *build_pane);
static void add_buttons                                      (JavaBuildPane      *build_pane);
static void clear_action                                     (GtkWidget          *text_view);
                                                               
#define JAVA_BUILD_PANE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_BUILD_PANE_TYPE, JavaBuildPanePrivate))

typedef struct _JavaBuildPanePrivate JavaBuildPanePrivate;

struct _JavaBuildPanePrivate
{
  GtkWidget          *text_view;
  GtkTextBuffer      *buffer;
  JavaPageType        page_type;
  JavaConfiguration  *configuration;
  CodeSlayerDocument *document;
  CodeSlayer         *codeslayer;
  CodeSlayerLinker   *linker;
  gint                process_id;
};

G_DEFINE_TYPE_EXTENDED (JavaBuildPane,
                        java_build_pane,
                        GTK_TYPE_HBOX,
                        0,
                        G_IMPLEMENT_INTERFACE (JAVA_PAGE_TYPE ,
                                               java_page_interface_init));
      
static void
java_page_interface_init (gpointer page, 
                          gpointer data)
{
  JavaPageInterface *page_interface = (JavaPageInterface*) page;
  page_interface->get_page_type = (JavaPageType (*) (JavaPage *obj)) java_build_pane_get_page_type;
  page_interface->get_configuration = (JavaConfiguration* (*) (JavaPage *obj)) java_build_pane_get_configuration;
  page_interface->set_configuration = (void (*) (JavaPage *obj, JavaConfiguration*)) java_build_pane_set_configuration;
  page_interface->get_document = (CodeSlayerDocument* (*) (JavaPage *obj)) java_build_pane_get_document;
  page_interface->set_document = (void (*) (JavaPage *obj, CodeSlayerDocument*)) java_build_pane_set_document;
}
      
static void 
java_build_pane_class_init (JavaBuildPaneClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) java_build_pane_finalize;
  g_type_class_add_private (klass, sizeof (JavaBuildPanePrivate));
}

static void
java_build_pane_init (JavaBuildPane *build_pane) 
{
  JavaBuildPanePrivate *priv;

  priv = JAVA_BUILD_PANE_GET_PRIVATE (build_pane);
  
  priv->text_view = gtk_text_view_new ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (priv->text_view), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (priv->text_view), GTK_WRAP_WORD);
  
  priv->buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->text_view));
}

static void
java_build_pane_finalize (JavaBuildPane *build_pane)
{
  G_OBJECT_CLASS (java_build_pane_parent_class)->finalize (G_OBJECT (build_pane));
}

GtkWidget*
java_build_pane_new (JavaPageType  page_type,
                     CodeSlayer   *codeslayer)
{
  JavaBuildPanePrivate *priv;
  GtkWidget *build_pane;
 
  build_pane = g_object_new (java_build_pane_get_type (), NULL);
  priv = JAVA_BUILD_PANE_GET_PRIVATE (build_pane);
  priv->page_type = page_type;
  priv->codeslayer = codeslayer;
  
  add_text_view (JAVA_BUILD_PANE (build_pane));
  add_buttons (JAVA_BUILD_PANE (build_pane));
  
  priv->linker = codeslayer_linker_new (codeslayer, GTK_TEXT_VIEW (priv->text_view));  
  
  return build_pane;
}

void
java_build_pane_clear_text (JavaBuildPane *build_pane)
{
  JavaBuildPanePrivate *priv;
  priv = JAVA_BUILD_PANE_GET_PRIVATE (build_pane);
  gtk_text_buffer_set_text (priv->buffer, "", -1);
}

void
java_build_pane_append_text (JavaBuildPane *build_pane, 
                             gchar         *text)
{
  JavaBuildPanePrivate *priv;
  GtkTextIter iter;
  GtkTextMark *text_mark;

  priv = JAVA_BUILD_PANE_GET_PRIVATE (build_pane);

  gtk_text_buffer_get_end_iter (priv->buffer, &iter);
  gtk_text_buffer_insert (priv->buffer, &iter, text, -1);
  text_mark = gtk_text_buffer_create_mark (priv->buffer, NULL, &iter, TRUE);
  gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (priv->text_view), 
                                text_mark, 0.0, FALSE, 0, 0);
}

void
java_build_pane_start_process (JavaBuildPane *build_pane, 
                               gchar         *text)
{
  JavaBuildPanePrivate *priv;
  priv = JAVA_BUILD_PANE_GET_PRIVATE (build_pane);  
  priv->process_id = codeslayer_add_to_processes (priv->codeslayer, text, NULL, NULL);
}

void 
java_build_pane_stop_process (JavaBuildPane *build_pane)
{
  JavaBuildPanePrivate *priv;
  priv = JAVA_BUILD_PANE_GET_PRIVATE (build_pane);  
  codeslayer_remove_from_processes (priv->codeslayer, priv->process_id);
}

static void 
add_text_view (JavaBuildPane *build_pane)
{
  JavaBuildPanePrivate *priv;
  GtkWidget *scrolled_window;

  priv = JAVA_BUILD_PANE_GET_PRIVATE (build_pane);
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrolled_window), priv->text_view);

  gtk_box_pack_start (GTK_BOX (build_pane), scrolled_window, TRUE, TRUE, 0);
}

static void 
add_buttons (JavaBuildPane *build_pane)
{
  JavaBuildPanePrivate *priv;
  GtkWidget *grid;
  GtkWidget *clear_button;
  GtkWidget *clear_image;
  
  priv = JAVA_BUILD_PANE_GET_PRIVATE (build_pane);

  grid = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (grid), 2);
  
  clear_button = gtk_button_new ();
  gtk_widget_set_tooltip_text (clear_button, "Clear");

  gtk_button_set_relief (GTK_BUTTON (clear_button), GTK_RELIEF_NONE);
  gtk_button_set_focus_on_click (GTK_BUTTON (clear_button), FALSE);
  clear_image = gtk_image_new_from_stock (GTK_STOCK_CLEAR, GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (clear_button), clear_image);
  gtk_widget_set_can_focus (clear_button, FALSE);
  
  gtk_grid_attach (GTK_GRID (grid), clear_button, 0, 0, 1, 1);

  gtk_box_pack_start (GTK_BOX (build_pane), grid, FALSE, FALSE, 2);
  
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
java_build_pane_get_page_type (JavaBuildPane *build_pane)
{
  JavaBuildPanePrivate *priv;
  priv = JAVA_BUILD_PANE_GET_PRIVATE (build_pane);
  return priv->page_type;
}

static JavaConfiguration* 
java_build_pane_get_configuration (JavaBuildPane *build_pane)
{
  JavaBuildPanePrivate *priv;
  priv = JAVA_BUILD_PANE_GET_PRIVATE (build_pane);
  return priv->configuration;
}

static void 
java_build_pane_set_configuration (JavaBuildPane    *build_pane, 
                                   JavaConfiguration *configuration)
{
  JavaBuildPanePrivate *priv;
  priv = JAVA_BUILD_PANE_GET_PRIVATE (build_pane);
  priv->configuration = configuration;
}                               

static CodeSlayerDocument* 
java_build_pane_get_document (JavaBuildPane *build_pane)
{
  JavaBuildPanePrivate *priv;
  priv = JAVA_BUILD_PANE_GET_PRIVATE (build_pane);
  return priv->document;
}

static void 
java_build_pane_set_document (JavaBuildPane     *build_pane, 
                              CodeSlayerDocument *document)
{
  JavaBuildPanePrivate *priv;
  priv = JAVA_BUILD_PANE_GET_PRIVATE (build_pane);
  priv->document = document;
}
