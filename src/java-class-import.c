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
#include "java-class-import.h"

static void java_class_import_class_init      (JavaClassImportClass *klass);
static void java_class_import_init            (JavaClassImport      *import);
static void java_class_import_finalize        (JavaClassImport      *import);

static void class_import_action               (JavaClassImport      *import);
static void insert_import_line                (JavaClassImport      *import, 
                                               const gchar          *package_name);
static void run_dialog                        (JavaClassImport      *import,
                                               GList                *indexes);
static GList* get_class_indexes               (JavaClassImport      *import, 
                                               const gchar          *class_name);
static GList* get_class_indexes_by_file_name  (gchar                *group_folder_path, 
                                               const gchar          *index_file_name,
                                               const gchar          *class_name);
static gchar* get_local_package_name          (GtkTextBuffer        *buffer);                                               
static gint sort_indexes                      (JavaIndexerIndex     *index1, 
                                               JavaIndexerIndex     *index2);
static void row_activated_action              (JavaClassImport      *import,
                                               GtkTreePath          *path,
                                               GtkTreeViewColumn    *column);

#define JAVA_CLASS_IMPORT_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_CLASS_IMPORT_TYPE, JavaClassImportPrivate))

typedef struct _JavaClassImportPrivate JavaClassImportPrivate;

struct _JavaClassImportPrivate
{
  CodeSlayer   *codeslayer;
  GtkWidget    *dialog;
  GtkWidget    *tree;
  GtkListStore *store;
};

enum
{
  PACKAGENAME = 0,
  INDEX,
  COLUMNS
};

G_DEFINE_TYPE (JavaClassImport, java_class_import, G_TYPE_OBJECT)

static void 
java_class_import_class_init (JavaClassImportClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) java_class_import_finalize;
  g_type_class_add_private (klass, sizeof (JavaClassImportPrivate));
}

static void
java_class_import_init (JavaClassImport *import)
{
  JavaClassImportPrivate *priv;
  priv = JAVA_CLASS_IMPORT_GET_PRIVATE (import);
  priv->dialog = NULL;
}

static void
java_class_import_finalize (JavaClassImport *import)
{
  JavaClassImportPrivate *priv;
  priv = JAVA_CLASS_IMPORT_GET_PRIVATE (import);
  if (priv->dialog != NULL)
    gtk_widget_destroy (priv->dialog);
  G_OBJECT_CLASS (java_class_import_parent_class)-> finalize (G_OBJECT (import));
}

JavaClassImport*
java_class_import_new (CodeSlayer *codeslayer,
                       GtkWidget  *menu)
{
  JavaClassImportPrivate *priv;
  JavaClassImport *import;

  import = JAVA_CLASS_IMPORT (g_object_new (java_class_import_get_type (), NULL));
  priv = JAVA_CLASS_IMPORT_GET_PRIVATE (import);
  priv->codeslayer = codeslayer;

  g_signal_connect_swapped (G_OBJECT (menu), "class-import",
                            G_CALLBACK (class_import_action), import);

  return import;
}

static void
class_import_action (JavaClassImport *import)
{
  JavaClassImportPrivate *priv;
  CodeSlayerEditor *editor;
  GtkTextBuffer *buffer;
  GtkTextIter start;
  GtkTextIter end;

  priv = JAVA_CLASS_IMPORT_GET_PRIVATE (import);

  editor = codeslayer_get_active_editor (priv->codeslayer);  
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));
  
  if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
    {
      GList *indexes;
      gchar *text;
      text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
      indexes = get_class_indexes (import, text);
      
      if (indexes == NULL)
        {
          g_free (text);
          return;        
        }
      
      if (g_list_length (indexes) > 1)
        {
          run_dialog (import, indexes);
        }
      else
        {
          JavaIndexerIndex *index = indexes->data;
          const gchar *package_name;
          package_name = java_indexer_index_get_package_name (index);
          insert_import_line (import, package_name);
        }
        
      g_free (text);
      
      if (indexes != NULL)
        {
          g_list_foreach (indexes, (GFunc) g_object_unref, NULL);
          g_list_free (indexes);
        }
    }
}

static void
insert_import_line (JavaClassImport *import, 
                    const gchar     *package_name)
{
  JavaClassImportPrivate *priv;
  CodeSlayerEditor *editor;
  GtkTextBuffer *buffer;
  GtkTextIter iter;
  GtkTextIter match_start;
  GtkTextIter match_end;
  gchar *local_package_name;

  priv = JAVA_CLASS_IMPORT_GET_PRIVATE (import);

  editor = codeslayer_get_active_editor (priv->codeslayer);  
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));
  
  gtk_text_buffer_get_start_iter (buffer, &iter);
  local_package_name = get_local_package_name (buffer);

  if (gtk_text_iter_forward_search (&iter, local_package_name,
                                    GTK_TEXT_SEARCH_TEXT_ONLY,
                                    &match_start, &match_end, NULL))
    {
      gchar *concat;
      concat = g_strconcat ("\n", "import ", package_name, ";", NULL);
      gtk_text_buffer_begin_user_action (buffer);
      gtk_text_buffer_insert (buffer, &match_end, concat, -1);
      gtk_text_buffer_end_user_action (buffer);
      g_free (concat);
    }
  
  if (local_package_name != NULL)
    g_free (local_package_name);
}

static gchar*
get_local_package_name (GtkTextBuffer *buffer)
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

static void
run_dialog (JavaClassImport *import,
            GList           *indexes)
{
  JavaClassImportPrivate *priv;
  GtkTreeIter tree_iter;
  
  priv = JAVA_CLASS_IMPORT_GET_PRIVATE (import);
  
  if (priv->dialog == NULL)
    {
      GtkWidget *content_area;
      GtkWidget *scrolled_window;
      GtkTreeViewColumn *column;
      GtkCellRenderer *renderer;

      priv->dialog = gtk_dialog_new_with_buttons ("Class Import", 
                                            NULL,
                                            GTK_DIALOG_MODAL,
                                            GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
                                            NULL);
      gtk_window_set_skip_taskbar_hint (GTK_WINDOW (priv->dialog), TRUE);
      gtk_window_set_skip_pager_hint (GTK_WINDOW (priv->dialog), TRUE);

      content_area = gtk_dialog_get_content_area (GTK_DIALOG (priv->dialog));
      
      /* the tree view */   
         
      priv->store = gtk_list_store_new (COLUMNS, G_TYPE_STRING, G_TYPE_POINTER);
      priv->tree =  gtk_tree_view_new ();
      gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (priv->tree), FALSE);
      gtk_tree_view_set_enable_search (GTK_TREE_VIEW (priv->tree), FALSE);
      gtk_tree_view_set_model (GTK_TREE_VIEW (priv->tree), GTK_TREE_MODEL (priv->store));
      g_object_unref (priv->store);
      
      column = gtk_tree_view_column_new ();
      gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
      renderer = gtk_cell_renderer_text_new ();
      gtk_tree_view_column_pack_start (column, renderer, FALSE);
      gtk_tree_view_column_add_attribute (column, renderer, "text", PACKAGENAME);
      gtk_tree_view_append_column (GTK_TREE_VIEW (priv->tree), column);
      
      scrolled_window = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                      GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
      gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (priv->tree));
      
      /* hook up the signals */
      
      g_signal_connect_swapped (G_OBJECT (priv->tree), "row-activated",
                                G_CALLBACK (row_activated_action), import);
                                
      
      /* render everything */
      
      gtk_widget_set_size_request (content_area, 350, 200);
      
      gtk_box_pack_start (GTK_BOX (content_area), scrolled_window, TRUE, TRUE, 0);

      gtk_widget_show_all (content_area);
    }
    
    if (priv->store != NULL)
      gtk_list_store_clear (priv->store);

    while (indexes != NULL)
      {
        JavaIndexerIndex *index = indexes->data;
        const gchar *package_name;
        package_name = java_indexer_index_get_package_name (index);
        gtk_list_store_append (priv->store, &tree_iter);
        gtk_list_store_set (priv->store, &tree_iter, 
                            PACKAGENAME, package_name, 
                            INDEX, index, 
                            -1);
        indexes = g_list_next (indexes);
      }
      
    gtk_dialog_run (GTK_DIALOG (priv->dialog));
    gtk_widget_hide (priv->dialog);
  }

static GList*
get_class_indexes (JavaClassImport *import, 
                   const gchar     *class_name)
{
  JavaClassImportPrivate *priv;
  GList *indexes = NULL;
  GList *projects_indexes = NULL;
  GList *libs_indexes = NULL;
  gchar *group_folder_path;
  
  priv = JAVA_CLASS_IMPORT_GET_PRIVATE (import);

  group_folder_path = codeslayer_get_active_group_folder_path (priv->codeslayer);
  
  projects_indexes = get_class_indexes_by_file_name (group_folder_path, "projects.classes", class_name);
  libs_indexes = get_class_indexes_by_file_name (group_folder_path, "libs.classes", class_name);
  
  indexes = g_list_concat (projects_indexes, libs_indexes);
  indexes = g_list_sort (indexes, (GCompareFunc) sort_indexes);
    
  g_free (group_folder_path);

  return indexes;
}

static GList*
get_class_indexes_by_file_name (gchar       *group_folder_path,
                                const gchar *index_file_name,
                                const gchar *class_name)
{
  GList *indexes = NULL;  
  gchar *file_name;
  GIOChannel *channel;
  gchar *text;
  GError *error = NULL;
  
  file_name = g_build_filename (group_folder_path, "indexes", index_file_name, NULL);

  if (!g_file_test (file_name, G_FILE_TEST_EXISTS))
    {
      g_warning ("There is no %s file.", index_file_name);
      g_free (file_name);
      return indexes;
    }
  
  channel = g_io_channel_new_file (file_name, "r", &error);
  
  while (g_io_channel_read_line (channel, &text, NULL, NULL, NULL) == G_IO_STATUS_NORMAL)
    {
      gchar **split;
      gchar **array;
    
      split = g_strsplit (text, "\t", -1);
      array = split;
      
      if (g_strcmp0 (*array, class_name) == 0)
        {
          JavaIndexerIndex *index;
          index = java_indexer_index_new ();
          java_indexer_index_set_class_name (index, *array);
          java_indexer_index_set_package_name (index, *++array);
          java_indexer_index_set_file_path (index, g_strstrip(*++array));
          indexes = g_list_prepend (indexes, index);
        }
        
      g_strfreev (split);
      g_free (text);
    }
    
  g_io_channel_shutdown(channel, FALSE, NULL);
  g_io_channel_unref (channel);  
  g_free (file_name);

  return indexes;
}

static gint                
sort_indexes (JavaIndexerIndex *index1, 
              JavaIndexerIndex *index2)
{
  const gchar *class_name1;
  const gchar *class_name2;
  class_name1 = java_indexer_index_get_class_name (index1);
  class_name2 = java_indexer_index_get_class_name (index2);  
  return g_strcmp0 (class_name1, class_name2);
}

static void
row_activated_action (JavaClassImport   *import,
                      GtkTreePath       *path,
                      GtkTreeViewColumn *column)
{
  JavaClassImportPrivate *priv;
  GtkTreeSelection *tree_selection;
  GtkTreeModel *tree_model;
  GList *selected_rows = NULL;
  GList *tmp = NULL;  
  
  priv = JAVA_CLASS_IMPORT_GET_PRIVATE (import);

  tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->tree));
  tree_model = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->tree));
  selected_rows = gtk_tree_selection_get_selected_rows (tree_selection, &tree_model);

  tmp = selected_rows;
  
  if (tmp != NULL)
    {
      GtkTreeIter treeiter;
      const gchar *package_name; 
      GtkTreePath *tree_path = tmp->data;
      
      gtk_tree_model_get_iter (tree_model, &treeiter, tree_path);
      gtk_tree_model_get (GTK_TREE_MODEL (priv->store), &treeiter, PACKAGENAME, &package_name, -1);
      
      insert_import_line (import, package_name);

      gtk_widget_hide (priv->dialog);
      
      gtk_tree_path_free (tree_path);
    }

  g_list_free (selected_rows);
}                     
