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

#include "java-build-pane.h"

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
                                                               
static GList* find_matches                                   (gchar              *text);
static void mark_matches                                     (GtkTextBuffer      *buffer, 
                                                              GList              *matches);
static gboolean select_link_action                           (JavaBuildPane      *build_pane, 
                                                              GdkEventButton     *event);

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
  gtk_text_buffer_create_tag (priv->buffer, "underline", "underline", 
                              PANGO_UNDERLINE_SINGLE, NULL);
                              
  g_signal_connect_swapped (G_OBJECT (priv->text_view), "button-press-event",
                            G_CALLBACK (select_link_action), build_pane);
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
  GtkWidget *table;
  GtkWidget *clear_button;
  GtkWidget *clear_image;
  
  priv = JAVA_BUILD_PANE_GET_PRIVATE (build_pane);

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

void
java_build_pane_create_links (JavaBuildPane *build_pane)
{
  JavaBuildPanePrivate *priv;
  GtkTextIter start;
  GtkTextIter end;
  gchar *text;
  GList *matches;

  priv = JAVA_BUILD_PANE_GET_PRIVATE (build_pane);

  gtk_text_buffer_get_bounds (priv->buffer, &start, &end);
  
  text = gtk_text_buffer_get_text (priv->buffer, &start, &end, FALSE);
  
  matches = find_matches (text);
  mark_matches (priv->buffer, matches);
}

static GList*
find_matches (gchar *text)
{
  GList *results = NULL;
  GRegex *regex;
  GMatchInfo *match_info;
  GError *error = NULL;
  
  regex = g_regex_new ("\\s(\\/.*?\\.java:\\d+)", 0, 0, NULL);
  
  g_regex_match_full (regex, text, -1, 0, 0, &match_info, &error);
  
  while (g_match_info_matches (match_info))
    {
      gchar *match_text = NULL;
      match_text = g_match_info_fetch (match_info, 1);
      
      g_print ("match %s\n", match_text);
      results = g_list_prepend (results, g_strdup (match_text));
        
      g_free (match_text);
      g_match_info_next (match_info, &error);
    }
  
  g_match_info_free (match_info);
  g_regex_unref (regex);
  
  if (error != NULL)
    {
      g_printerr ("search text for completion word error: %s\n", error->message);
      g_error_free (error);
    }

  return results;    
}

static void
mark_matches (GtkTextBuffer *buffer, 
              GList         *matches)
{
  while (matches != NULL)
    {
      gchar *match_text = matches->data;
      GtkTextIter start, begin, end;
      gboolean success;

      gdk_threads_enter ();
      
      gtk_text_buffer_get_start_iter (buffer, &start);
      
      success = gtk_text_iter_forward_search (&start, match_text, 
                                              GTK_TEXT_SEARCH_TEXT_ONLY, 
                                              &begin, &end, NULL);
                                                    
      do
        {
          gtk_text_buffer_apply_tag_by_name (buffer, "underline", &begin, &end);
          gtk_text_iter_forward_char (&start);
          success = gtk_text_iter_forward_search (&start, match_text, 
                                                  GTK_TEXT_SEARCH_TEXT_ONLY, 
                                                  &begin, &end, NULL);
          start = begin;                                                  
        }
      while (success);
      
      gdk_threads_leave ();                                              
      
      matches = g_list_next (matches);
    }
}

static gboolean
select_link_action (JavaBuildPane  *build_pane, 
                    GdkEventButton *event)
{
  JavaBuildPanePrivate *priv;
  priv = JAVA_BUILD_PANE_GET_PRIVATE (build_pane);

  if ((event->button == 1) && (event->type == GDK_BUTTON_PRESS))
    {
      GdkWindow *window;
      GtkTextIter iter;
      gint offset, x, y;

      window = gtk_text_view_get_window (GTK_TEXT_VIEW (priv->text_view),
                                         GTK_TEXT_WINDOW_TEXT);
                                                                                                                
      gdk_window_get_device_position (window, event->device, &x, &y, NULL);      
      
      gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (priv->text_view),
                                          &iter, x, y);
      
      offset = gtk_text_iter_get_offset (&iter);
    
      g_print ("select_link_action %d\n", offset);
    }

  return FALSE;
}
