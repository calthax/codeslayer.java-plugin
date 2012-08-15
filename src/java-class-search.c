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

static void java_class_search_class_init    (JavaClassSearchClass *klass);
static void java_class_search_init          (JavaClassSearch      *search);
static void java_class_search_finalize      (JavaClassSearch      *search);

static void class_search_action             (JavaClassSearch      *search);
static void run_dialog                      (JavaClassSearch      *search);
static gboolean key_release_action          (JavaClassSearch      *search,
                                             GdkEventKey          *event);
static GList* get_class_indexes             (JavaClassSearch      *search, 
                                             const gchar          *package_name);
static gint sort_indexes                    (JavaIndexerIndex     *index1, 
                                             JavaIndexerIndex     *index2);
static void row_activated_action            (JavaClassSearch      *search,
                                             GtkTreePath          *path,
                                             GtkTreeViewColumn    *column);

#define JAVA_CLASS_SEARCH_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_CLASS_SEARCH_TYPE, JavaClassSearchPrivate))

typedef struct _JavaClassSearchPrivate JavaClassSearchPrivate;

struct _JavaClassSearchPrivate
{
  CodeSlayer   *codeslayer;
  GtkWidget    *dialog;
  GtkWidget    *entry;
  GtkWidget    *tree;
  GtkListStore *store;
  GList        *indexes;
};

enum
{
  PACKAGENAME = 0,
  INDEX,
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
  priv->indexes = NULL;
}

static void
java_class_search_finalize (JavaClassSearch *search)
{
  JavaClassSearchPrivate *priv;
  priv = JAVA_CLASS_SEARCH_GET_PRIVATE (search);
  
  if (priv->dialog != NULL)
    gtk_widget_destroy (priv->dialog);
  
  if (priv->indexes != NULL)
    {
      g_list_foreach (priv->indexes, (GFunc) g_object_unref, NULL);
      g_list_free (priv->indexes);
      priv->indexes = NULL;
    }
  
  G_OBJECT_CLASS (java_class_search_parent_class)-> finalize (G_OBJECT (search));
}

JavaClassSearch*
java_class_search_new (CodeSlayer *codeslayer,
                       GtkWidget  *menu)
{
  JavaClassSearchPrivate *priv;
  JavaClassSearch *search;

  search = JAVA_CLASS_SEARCH (g_object_new (java_class_search_get_type (), NULL));
  priv = JAVA_CLASS_SEARCH_GET_PRIVATE (search);
  priv->codeslayer = codeslayer;

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
  GtkTreeIter tree_iter;
  GList *list; 
  const gchar *text;
  gchar *find_globbing;
  GPatternSpec *find_pattern = NULL;
  
  priv = JAVA_CLASS_SEARCH_GET_PRIVATE (search);
  
  text = gtk_entry_get_text (GTK_ENTRY (priv->entry));
  
  if (priv->store != NULL)
    gtk_list_store_clear (priv->store);
  
  if (!codeslayer_utils_has_text (text))
    {
      if (priv->indexes != NULL)
        {
          g_list_foreach (priv->indexes, (GFunc) g_object_unref, NULL);
          g_list_free (priv->indexes);
          priv->indexes = NULL;
        }
      return FALSE;
    }
  
  if (priv->indexes == NULL)
    priv->indexes = get_class_indexes (search, text);

  find_globbing = g_strconcat (text, "*", NULL);
  find_pattern = g_pattern_spec_new (find_globbing);      

  list = priv->indexes;  
  while (list != NULL)
    {
      JavaIndexerIndex *index = list->data;
      const gchar *class_name;
      const gchar *package_name;

      class_name = java_indexer_index_get_class_name (index);
      package_name = java_indexer_index_get_package_name (index);
      
      if (g_pattern_match_string (find_pattern, class_name))
        {
          gtk_list_store_append (priv->store, &tree_iter);
          gtk_list_store_set (priv->store, &tree_iter, 
                              PACKAGENAME, package_name, 
                              INDEX, index, 
                              -1);
        }      
      list = g_list_next (list);
    }
    
  if (find_pattern != NULL)
    g_pattern_spec_free (find_pattern);
    
  g_free (find_globbing);    
    
  return FALSE;
}

static GList*
get_class_indexes (JavaClassSearch *search, 
                   const gchar     *class_name)
{
  JavaClassSearchPrivate *priv;
  GList *indexes = NULL;
  gchar *group_folder_path;
  gchar *file_name;
  GIOChannel *channel;
  gchar *text;
  GError *error = NULL;
  
  priv = JAVA_CLASS_SEARCH_GET_PRIVATE (search);

  group_folder_path = codeslayer_get_active_group_folder_path (priv->codeslayer);

  file_name = g_build_filename (group_folder_path, "indexes", "projects.classes", NULL);

  if (!g_file_test (file_name, G_FILE_TEST_EXISTS))
    {
      g_warning ("There is no projects.classes file.");
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
      
      if (g_str_has_prefix (*array, class_name))
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
  g_free (group_folder_path);
  
  indexes = g_list_sort (indexes, (GCompareFunc) sort_indexes);

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
      JavaIndexerIndex *index;
      CodeSlayerProject *project;
      CodeSlayerDocument *document;
      const gchar *file_path; 
      GtkTreePath *tree_path = tmp->data;
      
      gtk_tree_model_get_iter (tree_model, &treeiter, tree_path);
      gtk_tree_model_get (GTK_TREE_MODEL (priv->store), &treeiter, INDEX, &index, -1);
      
      document = codeslayer_document_new ();
      file_path = java_indexer_index_get_file_path (index);
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
