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

#include <codeslayer/codeslayer-utils.h>
#include "java-indexer.h"
#include "java-indexer-index.h"
#include "java-indexer-utils.h"
#include "java-configuration.h"

static void java_indexer_class_init              (JavaIndexerClass *klass);
static void java_indexer_init                    (JavaIndexer      *indexer);
static void java_indexer_finalize                (JavaIndexer      *indexer);

static GList* get_indexes                        (JavaIndexer      *indexer,
                                                  CodeSlayerEditor *editor,
                                                  gchar            *text, 
                                                  gchar            *path);
static gint sort_indexes                         (JavaIndexerIndex *index1, 
                                                  JavaIndexerIndex *index2);
static void editor_saved_action                  (JavaIndexer      *indexer,
                                                  CodeSlayerEditor *editor);
static void execute_create_indexes               (JavaIndexer      *indexer);
static gboolean start_create_indexes             (JavaIndexer      *indexer);
static void finish_create_indexes                (JavaIndexer      *indexer);
static void find_symbol_action                   (JavaIndexer      *indexer);
static void find_symbol                          (JavaIndexer      *indexer, 
                                                  CodeSlayerEditor *editor,
                                                  GtkTextIter       iter, 
                                                  gchar            *text);
static void select_editor                        (CodeSlayer       *codeslayer, 
                                                  JavaIndexerIndex *index);

#define JAVA_INDEXER_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_INDEXER_TYPE, JavaIndexerPrivate))

typedef struct _JavaIndexerPrivate JavaIndexerPrivate;

struct _JavaIndexerPrivate
{
  CodeSlayer         *codeslayer;
  JavaConfigurations *configurations;
  gulong      saved_handler_id;
  guint       event_source_id;
};

G_DEFINE_TYPE (JavaIndexer, java_indexer, G_TYPE_OBJECT)
     
static void 
java_indexer_class_init (JavaIndexerClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) java_indexer_finalize;
  g_type_class_add_private (klass, sizeof (JavaIndexerPrivate));
}

static void
java_indexer_init (JavaIndexer *indexer){}

static void
java_indexer_finalize (JavaIndexer *indexer)
{
  JavaIndexerPrivate *priv;
  priv = JAVA_INDEXER_GET_PRIVATE (indexer);
  g_signal_handler_disconnect (priv->codeslayer, priv->saved_handler_id);
  G_OBJECT_CLASS (java_indexer_parent_class)->finalize (G_OBJECT (indexer));
}

JavaIndexer*
java_indexer_new (CodeSlayer         *codeslayer,
                  GtkWidget          *menu,
                  JavaConfigurations *configurations)
{
  JavaIndexerPrivate *priv;
  JavaIndexer *indexer;

  indexer = JAVA_INDEXER (g_object_new (java_indexer_get_type (), NULL));
  priv = JAVA_INDEXER_GET_PRIVATE (indexer);
  priv->codeslayer = codeslayer;
  priv->configurations = configurations;

  g_signal_connect_swapped (G_OBJECT (menu), "find-symbol",
                            G_CALLBACK (find_symbol_action), indexer);

  priv->saved_handler_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "editor-saved", 
                                                     G_CALLBACK (editor_saved_action), indexer);

  return indexer;
}

GList* 
java_indexer_get_indexes  (JavaIndexer      *indexer, 
                           CodeSlayerEditor *editor,
                           GtkTextIter       iter)
{
  GList *indexes = NULL;
  gchar *path;
  gchar *text;   

  text = java_indexer_utils_get_text_to_search (editor, iter);
  path = java_indexer_utils_find_path (text);

  if (codeslayer_utils_has_text (path))
    {
      gchar *comments_stripped;
      gchar *parameters_stripped;
      
      comments_stripped = java_indexer_utils_strip_path_comments (path);
      parameters_stripped = java_indexer_utils_strip_path_parameters (comments_stripped);
      parameters_stripped = g_strreverse (parameters_stripped);
      
      g_print ("path %s\n", parameters_stripped);

      indexes = get_indexes (indexer, editor, text, parameters_stripped);
      
      if (indexes != NULL)
        indexes = g_list_sort (indexes, (GCompareFunc) sort_indexes);
      
      g_free (path);
      g_free (comments_stripped);
      g_free (parameters_stripped);
    }
    
  g_free (text);
    
  return indexes; 
}

/*
 * Take the path, start at the beginning, and resolve the 
 * actual classes and methods types.
 *
 * For example after the title figure out that this starts as a Column 
 * object and that the title() method also returns a Column object.
 *
 * new Column().title().
 *
 */
static GList*
get_indexes (JavaIndexer      *indexer,
             CodeSlayerEditor *editor,
             gchar            *text,
             gchar            *path)
{
  JavaIndexerPrivate *priv;
  gchar **split;
  gchar **array;
  GList *indexes = NULL;
  
  priv = JAVA_INDEXER_GET_PRIVATE (indexer);
  
  split = g_strsplit (path, ".", -1);
  array = split;
  
  if (codeslayer_utils_has_text (*array))
    {
      gchar *class_symbol;
      class_symbol = java_indexer_utils_search_text_for_class_symbol (text, *array);
      if (class_symbol != NULL)
        {
          gchar *group_folder_path;
          gchar *import;
          g_print ("class_symbol %s\n", class_symbol);
          group_folder_path = codeslayer_get_active_group_folder_path (priv->codeslayer);
          import = java_indexer_utils_search_text_for_import (group_folder_path, text, class_symbol);
          if (import != NULL)
            {
              indexes = java_indexer_utils_get_package_indexes (group_folder_path, import);
              g_free (import);
            }
          g_free (class_symbol);
          g_free (group_folder_path);
        }      
      array++;
    }

  while (codeslayer_utils_has_text (*array))
    {
      g_print ("other %s\n", *array);
      array++;
    }
  
  if (split != NULL)
    g_strfreev (split);
  
  return indexes;
}
                           
static gint                
sort_indexes (JavaIndexerIndex *index1, 
              JavaIndexerIndex *index2)
{
  const gchar *name1;
  const gchar *name2;
  name1 = java_indexer_index_get_method_name (index1);
  name2 = java_indexer_index_get_method_name (index2);  
  return g_strcmp0 (name2, name1);
}


static void 
editor_saved_action (JavaIndexer      *indexer, 
                     CodeSlayerEditor *editor) 
{
  CodeSlayerDocument *document;
  const gchar *file_path;
  
  document = codeslayer_editor_get_document (editor);
  file_path = codeslayer_document_get_file_path (document);
  if (!g_str_has_suffix (file_path, ".java"))
    return;
  
  execute_create_indexes (indexer);
}

static void
execute_create_indexes (JavaIndexer *indexer) 
{
  JavaIndexerPrivate *priv;
  priv = JAVA_INDEXER_GET_PRIVATE (indexer);

  if (priv->event_source_id == 0)
    {
      priv->event_source_id = g_timeout_add_seconds_full (G_PRIORITY_DEFAULT, 2,
                                                          (GSourceFunc ) start_create_indexes,
                                                          indexer,
                                                          (GDestroyNotify) finish_create_indexes);
    }
}

static gboolean
start_create_indexes (JavaIndexer *indexer)
{
  JavaIndexerPrivate *priv;
  gchar *group_folder_path;
  gchar *index_file_name;
  FILE *file;
  GList *list;
  GString *string;
  gchar *command;

  priv = JAVA_INDEXER_GET_PRIVATE (indexer);
  
  group_folder_path = codeslayer_get_active_group_folder_path (priv->codeslayer);
  index_file_name = g_build_filename (group_folder_path, "indexes", "projects.indexes", NULL);
  
  string = g_string_new ("codeslayer-jindexer -sourcefolder ");
  
  list = java_configurations_get_list (priv->configurations);
  while (list != NULL)
    {
      JavaConfiguration *configuration = list->data;
      const gchar *source_folder;
      source_folder = java_configuration_get_source_folder (configuration);
      if (codeslayer_utils_has_text (source_folder))
        {
          string = g_string_append (string, source_folder);
          string = g_string_append (string, ":");        
        }
      list = g_list_next (list);
    }

  string = g_string_append (string, " -index ");
  string = g_string_append (string, index_file_name);

  command = g_string_free (string, FALSE);
  
  file = popen (command, "r");
  
  if (file != NULL)
    pclose (file);
  
  g_free (command);
  g_free (group_folder_path);
  g_free (index_file_name);
  
  return FALSE;  
}

static void
finish_create_indexes (JavaIndexer *indexer)
{
  JavaIndexerPrivate *priv;
  priv = JAVA_INDEXER_GET_PRIVATE (indexer);
  priv->event_source_id = 0;
}

static void 
find_symbol_action (JavaIndexer *indexer)
{
  JavaIndexerPrivate *priv;
  CodeSlayerEditor *editor;
  CodeSlayerDocument *document;
  const gchar *file_path;
  GtkTextBuffer *buffer;

  GtkTextMark *insert_mark;
  GtkTextMark *selection_mark;
  gchar *text;

  GtkTextIter start, end;

  priv = JAVA_INDEXER_GET_PRIVATE (indexer);

  editor = codeslayer_get_active_editor (priv->codeslayer);
  
  if (editor == NULL)
    return;
    
  document = codeslayer_editor_get_document (editor);
  file_path = codeslayer_document_get_file_path (document);
  if (!g_str_has_suffix (file_path, ".java"))
    return;
  
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));

  insert_mark = gtk_text_buffer_get_insert (buffer);    
  selection_mark = gtk_text_buffer_get_selection_bound (buffer);

  gtk_text_buffer_get_iter_at_mark (buffer, &start, insert_mark);
  gtk_text_buffer_get_iter_at_mark (buffer, &end, selection_mark);

  text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
  
  if (text != NULL)
    g_strstrip (text);
  
  g_print ("select %s\n", text);
  
  find_symbol (indexer, editor, start, text);
    
  if (text != NULL)
    g_free (text);
}

static void
find_symbol (JavaIndexer      *indexer, 
             CodeSlayerEditor *editor,
             GtkTextIter       iter, 
             gchar            *text)
{
  JavaIndexerPrivate *priv;
  GList *indexes;
  GList *list = NULL;
  
  priv = JAVA_INDEXER_GET_PRIVATE (indexer);
  
  indexes = java_indexer_get_indexes (indexer, editor, iter);
  
  list = indexes;
  
  while (indexes != NULL)
    {
      JavaIndexerIndex *index = indexes->data;
      const gchar *method_name;
      method_name = java_indexer_index_get_method_name (index);

      if (g_strcmp0 (method_name, text) == 0)
        {
          select_editor (priv->codeslayer, index);
          break;
        }
        
      indexes = g_list_next (indexes);
    }

  if (list != NULL)
    {
      g_list_foreach (list, (GFunc) g_object_unref, NULL);
      g_list_free (list);
    }
}

static void
select_editor (CodeSlayer       *codeslayer, 
               JavaIndexerIndex *index)
{
  const gchar *file_path;
  gint line_number;
  CodeSlayerDocument *document;
  CodeSlayerProject *project;
  
  file_path = java_indexer_index_get_file_path (index);
  line_number = java_indexer_index_get_line_number (index);

  document = codeslayer_document_new ();
  project = codeslayer_get_project_by_file_path (codeslayer, file_path);

  codeslayer_document_set_file_path (document, file_path);
  codeslayer_document_set_line_number (document, line_number);
  codeslayer_document_set_project (document, project);

  codeslayer_select_editor (codeslayer, document);
  
  g_object_unref (document);
}
