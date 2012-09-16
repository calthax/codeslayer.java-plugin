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
#include "java-import.h"
#include "java-utils.h"

static void java_import_class_init  (JavaImportClass   *klass);
static void java_import_init        (JavaImport        *import);
static void java_import_finalize    (JavaImport        *import);

static void import_action           (JavaImport        *import);
static void run_dialog              (JavaImport        *import);
static gchar* get_input             (JavaImport        *import, 
                                     const gchar       *text);
static void render_output           (JavaImport        *import, 
                                     gchar             *output);
static void render_line             (JavaImport        *import, 
                                     gchar             *line);
static void row_activated_action    (JavaImport        *import,
                                     GtkTreePath       *path,
                                     GtkTreeViewColumn *column);
static void insert_import           (JavaImport        *import, 
                                     const gchar       *text);
static gchar* get_package_name      (GtkTextBuffer     *buffer);

#define JAVA_IMPORT_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_IMPORT_TYPE, JavaImportPrivate))

typedef struct _JavaImportPrivate JavaImportPrivate;

struct _JavaImportPrivate
{
  CodeSlayer   *codeslayer;
  JavaClient   *client;
  GtkWidget    *dialog;
  GtkWidget    *tree;
  GtkListStore *store;
};

enum
{
  CLASS_NAME = 0,
  COLUMNS
};

G_DEFINE_TYPE (JavaImport, java_import, G_TYPE_OBJECT)

static void 
java_import_class_init (JavaImportClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) java_import_finalize;
  g_type_class_add_private (klass, sizeof (JavaImportPrivate));
}

static void
java_import_init (JavaImport *import)
{
  JavaImportPrivate *priv;
  priv = JAVA_IMPORT_GET_PRIVATE (import);
  priv->dialog = NULL;
}

static void
java_import_finalize (JavaImport *import)
{
  JavaImportPrivate *priv;
  priv = JAVA_IMPORT_GET_PRIVATE (import);
  
  if (priv->dialog != NULL)
    gtk_widget_destroy (priv->dialog);
  
  G_OBJECT_CLASS (java_import_parent_class)-> finalize (G_OBJECT (import));
}

JavaImport*
java_import_new (CodeSlayer *codeslayer,
                 GtkWidget  *menu, 
                 JavaClient *client)
{
  JavaImportPrivate *priv;
  JavaImport *import;

  import = JAVA_IMPORT (g_object_new (java_import_get_type (), NULL));
  priv = JAVA_IMPORT_GET_PRIVATE (import);
  priv->codeslayer = codeslayer;
  priv->client = client;

  g_signal_connect_swapped (G_OBJECT (menu), "import",
                            G_CALLBACK (import_action), import);

  return import;
}

static void
import_action (JavaImport *import)
{
  JavaImportPrivate *priv;
  CodeSlayerEditor *editor;
  const gchar *file_path;
  GtkTextBuffer *buffer;
  gchar *text;
  gchar *input;
  gchar *output;

  GtkTextIter start, end;

  priv = JAVA_IMPORT_GET_PRIVATE (import);

  editor = codeslayer_get_active_editor (priv->codeslayer);
  
  if (editor == NULL)
    return;
    
  file_path = codeslayer_editor_get_file_path (editor);
  if (!g_str_has_suffix (file_path, ".java"))
    return;
  
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));
  
  if (!gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
    return;  
    
  run_dialog (import);
  gtk_list_store_clear (priv->store);      
    
  text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
  
  input = get_input (import, text);

  g_print ("input: %s\n", input);
  
  output = java_client_send (priv->client, input);
  
  if (output != NULL)
    {
      g_print ("output: %s\n", output);
      render_output (import, output);
      g_free (output);
    }

  gtk_dialog_run (GTK_DIALOG (priv->dialog));
  gtk_widget_hide (priv->dialog);
  
  g_free (input);
}

static void
run_dialog (JavaImport *import)
{
  JavaImportPrivate *priv;
  priv = JAVA_IMPORT_GET_PRIVATE (import);
  
  if (priv->dialog == NULL)
    {
      GtkWidget *content_area;
      GtkWidget *vbox;
      GtkWidget *scrolled_window;
      GtkTreeViewColumn *column;
      GtkCellRenderer *renderer;

      priv->dialog = gtk_dialog_new_with_buttons ("Import", 
                                            NULL,
                                            GTK_DIALOG_MODAL,
                                            GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
                                            NULL);
      gtk_window_set_skip_taskbar_hint (GTK_WINDOW (priv->dialog), TRUE);
      gtk_window_set_skip_pager_hint (GTK_WINDOW (priv->dialog), TRUE);

      content_area = gtk_dialog_get_content_area (GTK_DIALOG (priv->dialog));
      
      vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 1);
      gtk_box_set_homogeneous (GTK_BOX (vbox), FALSE);
      
      /* the tree view */   
         
      priv->store = gtk_list_store_new (COLUMNS, G_TYPE_STRING);
      priv->tree =  gtk_tree_view_new ();
      gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (priv->tree), FALSE);
      gtk_tree_view_set_enable_search (GTK_TREE_VIEW (priv->tree), FALSE);
      
      gtk_tree_view_set_model (GTK_TREE_VIEW (priv->tree), GTK_TREE_MODEL (priv->store));
      g_object_unref (priv->store);
      
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
      
      g_signal_connect_swapped (G_OBJECT (priv->tree), "row-activated",
                                G_CALLBACK (row_activated_action), import);
                                
      
      /* render everything */
      
      gtk_widget_set_size_request (content_area, 600, 400);
      
      gtk_box_pack_start (GTK_BOX (vbox), scrolled_window, TRUE, TRUE, 0);
      gtk_box_pack_start (GTK_BOX (content_area), vbox, TRUE, TRUE, 0);

      gtk_widget_show_all (content_area);
    }
}

static gchar* 
get_input (JavaImport  *import, 
           const gchar *text)
{
  JavaImportPrivate *priv;

  gchar *indexes_folder;
  gchar *result;
  
  priv = JAVA_IMPORT_GET_PRIVATE (import);
  
  indexes_folder = java_utils_get_indexes_folder (priv->codeslayer);
  
  result = g_strconcat ("-program import", 
                        " -name ", text,
                        indexes_folder, 
                        NULL);
  
  g_free (indexes_folder);

  return result;
}

static void
render_output (JavaImport *import, 
               gchar      *output)
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
          render_line (import, *tmp);
          tmp++;
        }
      g_strfreev (split);
    }
}

static void
render_line (JavaImport *import, 
             gchar      *line)
{
  JavaImportPrivate *priv;
  GtkTreeIter iter;
  gchar **split;
  gchar **tmp;
  
  priv = JAVA_IMPORT_GET_PRIVATE (import);
  
  if (!codeslayer_utils_has_text (line))
    return;
  
  split = g_strsplit (line, "\t", -1);
  
  if (g_strcmp0 (*split, "NO_RESULTS_FOUND") == 0)
    return;
  
  if (split != NULL)
    {
      gchar *class_name;  
      
      tmp = split;

      class_name = *tmp;
      
      if (class_name != NULL)
        {
          gtk_list_store_append (priv->store, &iter);
          gtk_list_store_set (priv->store, &iter, 
                              CLASS_NAME, class_name, 
                              -1);
        }

      g_strfreev (split);
    }
}

static void
row_activated_action (JavaImport        *import,
                      GtkTreePath       *path,
                      GtkTreeViewColumn *column)
{
  JavaImportPrivate *priv;
  GtkTreeSelection *tree_selection;
  GtkTreeModel *tree_model;
  GList *selected_rows = NULL;
  GList *tmp = NULL;  
  
  priv = JAVA_IMPORT_GET_PRIVATE (import);

  tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->tree));
  tree_model = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->tree));
  selected_rows = gtk_tree_selection_get_selected_rows (tree_selection, &tree_model);

  tmp = selected_rows;
  
  if (tmp != NULL)
    {
      GtkTreeIter treeiter;
      const gchar *class_name; 
      GtkTreePath *tree_path = tmp->data;
      
      gtk_tree_model_get_iter (tree_model, &treeiter, tree_path);
      gtk_tree_model_get (GTK_TREE_MODEL (priv->store), &treeiter, CLASS_NAME, &class_name, -1);
      
      g_print ("class_name %s\n", class_name);
      
      insert_import (import, class_name);
      
      gtk_widget_hide (priv->dialog);
      
      gtk_tree_path_free (tree_path);
    }

  g_list_free (selected_rows);
}

static void
insert_import (JavaImport  *import, 
               const gchar *class_name)
{
  JavaImportPrivate *priv;
  CodeSlayerEditor *editor;
  GtkTextBuffer *buffer;
  GtkTextIter iter;
  GtkTextIter match_start;
  GtkTextIter match_end;
  gchar *package_name;

  priv = JAVA_IMPORT_GET_PRIVATE (import);

  editor = codeslayer_get_active_editor (priv->codeslayer);  
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));
  
  gtk_text_buffer_get_start_iter (buffer, &iter);
  package_name = get_package_name (buffer);

  if (gtk_text_iter_forward_search (&iter, package_name,
                                    GTK_TEXT_SEARCH_TEXT_ONLY,
                                    &match_start, &match_end, NULL))
    {
      gchar *concat;
      concat = g_strconcat ("\n", "import ", class_name, ";", NULL);
      gtk_text_buffer_begin_user_action (buffer);
      gtk_text_buffer_insert (buffer, &match_end, concat, -1);
      gtk_text_buffer_end_user_action (buffer);
      g_free (concat);
    }
  
  if (package_name != NULL)
    g_free (package_name);
}

static gchar*
get_package_name (GtkTextBuffer *buffer)
{
  gchar* result = NULL;
  GRegex *regex;
  GMatchInfo *match_info;
  GError *error = NULL;
  GtkTextIter start;
  GtkTextIter end;
  gchar   *text;
  
  gtk_text_buffer_get_bounds (buffer, &start, &end);
  
  text = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
  
  regex = g_regex_new ("package\\s(.*?);", 0, 0, NULL);
  
  g_regex_match_full (regex, text, -1, 0, 0, &match_info, &error);
  
  if (result == NULL && g_match_info_matches (match_info))
    result = g_match_info_fetch (match_info, 0);
  
  g_match_info_free (match_info);
  g_regex_unref (regex);
  
  if (error != NULL)
    {
      g_printerr ("search text for package error: %s\n", error->message);
      g_error_free (error);
    }
    
  g_free (text);
    
  return result;   
}
