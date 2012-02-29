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

#include "java-class-search.h"

static void java_class_search_class_init    (JavaClassSearchClass *klass);
static void java_class_search_init          (JavaClassSearch      *search);
static void java_class_search_finalize      (JavaClassSearch      *search);

static void class_search_action             (JavaClassSearch      *search);
static void run_dialog                      (JavaClassSearch      *search);

#define JAVA_CLASS_SEARCH_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_CLASS_SEARCH_TYPE, JavaClassSearchPrivate))

typedef struct _JavaClassSearchPrivate JavaClassSearchPrivate;

struct _JavaClassSearchPrivate
{
  GtkWindow *parent;
};

G_DEFINE_TYPE (JavaClassSearch, java_class_search, G_TYPE_OBJECT)

static void 
java_class_search_class_init (JavaClassSearchClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) java_class_search_finalize;
  g_type_class_add_private (klass, sizeof (JavaClassSearchPrivate));
}

static void
java_class_search_init (JavaClassSearch *search)
{
  /*GtkWidget *vbox;
  gtk_window_set_title (GTK_WINDOW (search), "Class Search");  
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (search), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (search), TRUE);
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (search), vbox);*/
}

static void
java_class_search_finalize (JavaClassSearch *search)
{
  G_OBJECT_CLASS (java_class_search_parent_class)-> finalize (G_OBJECT (search));
}

JavaClassSearch*
java_class_search_new (GtkWidget *menu)
{
  /*JavaClassSearchPrivate *priv;*/
  /*priv = JAVA_CLASS_SEARCH_GET_PRIVATE (search);*/

  JavaClassSearch *search;

  search = JAVA_CLASS_SEARCH (g_object_new (java_class_search_get_type (), NULL));

  g_signal_connect_swapped (G_OBJECT (menu), "class-search",
                            G_CALLBACK (class_search_action), search);

  return search;
}

static void
class_search_action (JavaClassSearch *search)
{
  run_dialog (search);
}

static void
run_dialog (JavaClassSearch *search)
{
  /*JavaClassSearchPrivate *priv;*/
  GtkWidget *dialog;
  GtkWidget *content_area;
  /*GtkWidget *plugins_selector;*/
  
  /*priv = JAVA_CLASS_SEARCH_GET_PRIVATE (search);*/

  dialog = gtk_dialog_new_with_buttons ("Class Search", 
                                        NULL,
                                        GTK_DIALOG_MODAL,
                                        GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
                                        NULL);
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (dialog), TRUE);

  content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

  gtk_widget_set_size_request (content_area, 350, 400);

  /*plugins_selector = codeslayer_plugins_selector_new (plugins, group);
  gtk_box_pack_start (GTK_BOX (content_area), plugins_selector, TRUE, TRUE, 0);*/

  gtk_widget_show_all (content_area);

  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}
