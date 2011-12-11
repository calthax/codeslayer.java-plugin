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

#include "java-notebook-page.h"

static void java_notebook_page_class_init  (JavaNotebookPageClass *klass);
static void java_notebook_page_init        (JavaNotebookPage      *notebook_page);
static void java_notebook_page_finalize    (JavaNotebookPage      *notebook_page);

static void add_widget                     (JavaNotebookPage      *notebook_page, 
                                            GtkWidget             *widget);
static void add_buttons                    (JavaNotebookPage      *notebook_page, 
                                            GtkWidget             *widget);
static void clear_action                   (GtkWidget             *widget);

#define JAVA_NOTEBOOK_PAGE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_NOTEBOOK_PAGE_TYPE, JavaNotebookPagePrivate))

typedef struct _JavaNotebookPagePrivate JavaNotebookPagePrivate;

struct _JavaNotebookPagePrivate
{
  GtkWidget *widget;
};

G_DEFINE_TYPE (JavaNotebookPage, java_notebook_page, GTK_TYPE_HBOX)

static void
java_notebook_page_class_init (JavaNotebookPageClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) java_notebook_page_finalize;
  g_type_class_add_private (klass, sizeof (JavaNotebookPagePrivate));
}

static void
java_notebook_page_init (JavaNotebookPage *notebook_page) {}

static void
java_notebook_page_finalize (JavaNotebookPage *notebook_page)
{
  G_OBJECT_CLASS (java_notebook_page_parent_class)->finalize (G_OBJECT(notebook_page));
}

GtkWidget*
java_notebook_page_new (GtkWidget *widget)
{
  JavaNotebookPagePrivate *priv;
  GtkWidget *notebook_page;
  
  notebook_page = g_object_new (java_notebook_page_get_type (), NULL);
  priv = JAVA_NOTEBOOK_PAGE_GET_PRIVATE (notebook_page);
  priv->widget = widget;
  
  add_widget (JAVA_NOTEBOOK_PAGE (notebook_page), widget);
  add_buttons (JAVA_NOTEBOOK_PAGE (notebook_page), widget);

  return notebook_page;
}

static void 
add_buttons (JavaNotebookPage *notebook_page, 
             GtkWidget             *widget)
{
  GtkWidget *table;
  GtkWidget *clear_button;
  GtkWidget *clear_image;
  
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

  gtk_box_pack_start (GTK_BOX (notebook_page), table, FALSE, FALSE, 2);
  
  g_signal_connect_swapped (G_OBJECT (clear_button), "clicked",
                            G_CALLBACK (clear_action), widget);
}

static void 
add_widget (JavaNotebookPage *notebook_page, 
            GtkWidget        *widget)
{
  GtkWidget *scrolled_window;

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (widget));

  gtk_box_pack_start (GTK_BOX (notebook_page), scrolled_window, TRUE, TRUE, 0);
}

static void
clear_action (GtkWidget *widget)
{
  GtkTextBuffer *buffer;
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget));
  gtk_text_buffer_set_text (buffer, "", -1);
}

GtkWidget*
java_notebook_page_get_widget (JavaNotebookPage *notebook_page)
{
  return JAVA_NOTEBOOK_PAGE_GET_PRIVATE (notebook_page)->widget;
}
