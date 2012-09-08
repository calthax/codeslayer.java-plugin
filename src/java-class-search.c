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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <codeslayer/codeslayer-utils.h>
#include "java-indexer-index.h"
#include "java-class-search.h"
#include "java-utils.h"

static void java_class_search_class_init    (JavaClassSearchClass *klass);
static void java_class_search_init          (JavaClassSearch      *search);
static void java_class_search_finalize      (JavaClassSearch      *search);

static void class_search_action             (JavaClassSearch      *search);
static void run_dialog                      (JavaClassSearch      *search);
static gboolean key_release_action          (JavaClassSearch      *search,
                                             GdkEventKey          *event);
static void row_activated_action            (JavaClassSearch      *search,
                                             GtkTreePath          *path,
                                             GtkTreeViewColumn    *column);
static gchar* get_input                     (JavaClassSearch      *search, 
                                             const gchar          *text);
static void render_output                   (JavaClassSearch      *search, 
                                             gchar                *output);
static void render_line                     (JavaClassSearch      *search, 
                                             gchar                *line);

#define JAVA_CLASS_SEARCH_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_CLASS_SEARCH_TYPE, JavaClassSearchPrivate))

typedef struct _JavaClassSearchPrivate JavaClassSearchPrivate;

struct _JavaClassSearchPrivate
{
  CodeSlayer   *codeslayer;
  JavaClient   *client;
  GtkWidget    *dialog;
  GtkWidget    *entry;
  GtkWidget    *tree;
  GtkListStore *store;
};

enum
{
  SIMPLE_CLASS_NAME = 0,
  CLASS_NAME,
  FILE_PATH,
  COLUMNS
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
  JavaClassSearchPrivate *priv;
  priv = JAVA_CLASS_SEARCH_GET_PRIVATE (search);
  priv->dialog = NULL;
}

static void
java_class_search_finalize (JavaClassSearch *search)
{
  JavaClassSearchPrivate *priv;
  priv = JAVA_CLASS_SEARCH_GET_PRIVATE (search);
  
  if (priv->dialog != NULL)
    gtk_widget_destroy (priv->dialog);
  
  G_OBJECT_CLASS (java_class_search_parent_class)-> finalize (G_OBJECT (search));
}

JavaClassSearch*
java_class_search_new (CodeSlayer *codeslayer,
                       GtkWidget  *menu, 
                       JavaClient *client)
{
  JavaClassSearchPrivate *priv;
  JavaClassSearch *search;

  search = JAVA_CLASS_SEARCH (g_object_new (java_class_search_get_type (), NULL));
  priv = JAVA_CLASS_SEARCH_GET_PRIVATE (search);
  priv->codeslayer = codeslayer;
  priv->client = client;

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
  JavaClassSearchPrivate *priv;
  priv = JAVA_CLASS_SEARCH_GET_PRIVATE (search);
  
  if (priv->dialog == NULL)
    {
      GtkWidget *content_area;
      GtkWidget *vbox;
      GtkWidget *hbox;
      GtkWidget *label;
      GtkWidget *scrolled_window;
      GtkTreeViewColumn *column;
      GtkCellRenderer *renderer;

      priv->dialog = gtk_dialog_new_with_buttons ("Class Search", 
                                            NULL,
                                            GTK_DIALOG_MODAL,
                                            GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
                                            NULL);
      gtk_window_set_skip_taskbar_hint (GTK_WINDOW (priv->dialog), TRUE);
      gtk_window_set_skip_pager_hint (GTK_WINDOW (priv->dialog), TRUE);

      content_area = gtk_dialog_get_content_area (GTK_DIALOG (priv->dialog));
      
      vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 1);
      gtk_box_set_homogeneous (GTK_BOX (vbox), FALSE);
      
      /* the completion box */
      
      hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
      gtk_box_set_homogeneous (GTK_BOX (hbox), FALSE);
      
      label = gtk_label_new ("Class: ");
      priv->entry = gtk_entry_new ();
      gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);
      gtk_box_pack_start (GTK_BOX (hbox), priv->entry, TRUE, TRUE, 2);
      
      /* the tree view */   
         
      priv->store = gtk_list_store_new (COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
      priv->tree =  gtk_tree_view_new ();
      gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (priv->tree), FALSE);
      gtk_tree_view_set_enable_search (GTK_TREE_VIEW (priv->tree), FALSE);
      gtk_tree_view_set_model (GTK_TREE_VIEW (priv->tree), GTK_TREE_MODEL (priv->store));
      g_object_unref (priv->store);
      
      column = gtk_tree_view_column_new ();
      gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
      renderer = gtk_cell_renderer_text_new ();
      gtk_tree_view_column_pack_start (column, renderer, FALSE);
      gtk_tree_view_column_add_attribute (column, renderer, "text", SIMPLE_CLASS_NAME);
      gtk_tree_view_append_column (GTK_TREE_VIEW (priv->tree), column);
      
      column = gtk_tree_view_column_new ();
      gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
      renderer = gtk_cell_renderer_text_new ();
      gtk_tree_view_column_pack_start (column, renderer, FALSE);
      gtk_tree_view_column_add_attribute (column, renderer, "text", CLASS_NAME);
      gtk_tree_view_append_column (GTK_TREE_VIEW (priv->tree), column);
      
      scrolled_window = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                      GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
      gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (priv->tree));
      
      /* hook up the signals */
      
      g_signal_connect_swapped (G_OBJECT (priv->entry), "key-release-event",
                                G_CALLBACK (key_release_action), search);
                                
      g_signal_connect_swapped (G_OBJECT (priv->tree), "row-activated",
                                G_CALLBACK (row_activated_action), search);
                                
      
      /* render everything */
      
      gtk_widget_set_size_request (content_area, 600, 400);
      
      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
      gtk_box_pack_start (GTK_BOX (vbox), scrolled_window, TRUE, TRUE, 0);
      gtk_box_pack_start (GTK_BOX (content_area), vbox, TRUE, TRUE, 0);

      gtk_widget_show_all (content_area);
    }
    
  gtk_widget_grab_focus (priv->entry);
  gtk_dialog_run (GTK_DIALOG (priv->dialog));
  gtk_widget_hide (priv->dialog);
}

static gboolean
key_release_action (JavaClassSearch *search,
                    GdkEventKey     *event)
{
  JavaClassSearchPrivate *priv;
  const gchar *text;
  gchar *input;
  gchar *output;
  
  priv = JAVA_CLASS_SEARCH_GET_PRIVATE (search);
  
  text = gtk_entry_get_text (GTK_ENTRY (priv->entry));
  
  if (priv->store != NULL)
    gtk_list_store_clear (priv->store);
    
  if (!codeslayer_utils_has_text (text))
    return FALSE;
  
  input = get_input (search, text);
  
  g_print ("input: %s\n", input);
  
  output = java_client_send (priv->client, input);
  
  if (output != NULL)
    {
      g_print ("output: %s\n", output);
      render_output (search, output);
      g_free (output);
    }

  g_free (input);

  return FALSE;
}

static gchar* 
get_input (JavaClassSearch *search, 
           const gchar     *text)
{
  JavaClassSearchPrivate *priv;

  gchar *indexes_folder;
  gchar *result;
  
  priv = JAVA_CLASS_SEARCH_GET_PRIVATE (search);
  
  indexes_folder = java_utils_get_indexes_folder (priv->codeslayer);
  
  result = g_strconcat ("-program search", 
                        " -name ", text,
                        indexes_folder, 
                        NULL);
  
  g_free (indexes_folder);

  return result;
}

static void
render_output (JavaClassSearch *search, 
               gchar           *output)
{
  gchar **split;
  gchar **tmp;
  
  if (!codeslayer_utils_has_text (output))
    return;
  
  split = g_strsplit (output, "\n", -1);

  if (split != NULL)
    {
      tmp = split;
      while (*tmp != NULL)
        {
          render_line (search, *tmp);
          tmp++;
        }
      g_strfreev (split);
    }
}

static void
render_line (JavaClassSearch *search, 
             gchar           *line)
{
  JavaClassSearchPrivate *priv;
  GtkTreeIter iter;
  gchar **split;
  gchar **tmp;
  
  priv = JAVA_CLASS_SEARCH_GET_PRIVATE (search);
  
  if (!codeslayer_utils_has_text (line))
    return;
  
  split = g_strsplit (line, "\t", -1);
  if (split != NULL)
    {
      gchar *simple_class_name;  
      gchar *class_name;  
      gchar *file_path;
      
      tmp = split;

      simple_class_name = *tmp;
      class_name = *++tmp;
      file_path = *++tmp;
      
      if (simple_class_name != NULL && 
          class_name != NULL && 
          file_path != NULL)
        {
          gtk_list_store_append (priv->store, &iter);
          gtk_list_store_set (priv->store, &iter, 
                              SIMPLE_CLASS_NAME, simple_class_name, 
                              CLASS_NAME, class_name, 
                              FILE_PATH, file_path, 
                              -1);
        }

      g_strfreev (split);
    }
}

static void
row_activated_action (JavaClassSearch   *search,
                      GtkTreePath       *path,
                      GtkTreeViewColumn *column)
{
  JavaClassSearchPrivate *priv;
  GtkTreeSelection *tree_selection;
  GtkTreeModel *tree_model;
  GList *selected_rows = NULL;
  GList *tmp = NULL;  
  
  priv = JAVA_CLASS_SEARCH_GET_PRIVATE (search);

  tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->tree));
  tree_model = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->tree));
  selected_rows = gtk_tree_selection_get_selected_rows (tree_selection, &tree_model);

  tmp = selected_rows;
  
  if (tmp != NULL)
    {
      GtkTreeIter treeiter;
      CodeSlayerProject *project;
      CodeSlayerDocument *document;
      const gchar *file_path; 
      GtkTreePath *tree_path = tmp->data;
      
      gtk_tree_model_get_iter (tree_model, &treeiter, tree_path);
      gtk_tree_model_get (GTK_TREE_MODEL (priv->store), &treeiter, FILE_PATH, &file_path, -1);
      
      document = codeslayer_document_new ();
      project = codeslayer_get_project_by_file_path (priv->codeslayer, file_path);
      codeslayer_document_set_file_path (document, file_path);
      codeslayer_document_set_project (document, project);
      
      codeslayer_select_editor (priv->codeslayer, document);
      gtk_widget_hide (priv->dialog);
      
      g_object_unref (document);
      gtk_tree_path_free (tree_path);
    }

  g_list_free (selected_rows);
}                     
