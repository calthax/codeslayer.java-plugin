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
#include "java-utils.h"
#include "java-configuration.h"

static void java_indexer_class_init   (JavaIndexerClass *klass);
static void java_indexer_init         (JavaIndexer      *indexer);
static void java_indexer_finalize     (JavaIndexer      *indexer);

static GList* get_indexes             (gchar            *group_folder_path,
                                       gchar            *text,
                                       gchar            *context_path);
static void editor_saved_action       (JavaIndexer      *indexer,
                                       CodeSlayerEditor *editor);
static void execute_create_indexes    (JavaIndexer      *indexer);
static gboolean start_create_indexes  (JavaIndexer      *indexer);
static void finish_create_indexes     (JavaIndexer      *indexer);
static void find_symbol_action        (JavaIndexer      *indexer);
static void find_method               (JavaIndexer      *indexer, 
                                       CodeSlayerEditor *editor,
                                       GtkTextIter       iter, 
                                       gchar            *text);
static void find_class                (JavaIndexer      *indexer, 
                                       CodeSlayerEditor *editor,
                                       GtkTextIter       iter, 
                                       gchar            *class_name);
static void select_editor             (CodeSlayer       *codeslayer, 
                                       JavaIndexerIndex *index, 
                                       gboolean          has_line_number);
static void verify_dir_exists         (CodeSlayer       *codeslayer);

#define JAVA_INDEXER_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_INDEXER_TYPE, JavaIndexerPrivate))

typedef struct _JavaIndexerPrivate JavaIndexerPrivate;

struct _JavaIndexerPrivate
{
  CodeSlayer         *codeslayer;
  JavaConfigurations *configurations;
  gulong              saved_handler_id;
  guint               event_source_id;
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
                                                     
  verify_dir_exists (codeslayer);

  return indexer;
}

GList* 
java_indexer_get_indexes (JavaIndexer      *indexer, 
                          CodeSlayerEditor *editor,
                          GtkTextIter       iter)
{
  JavaIndexerPrivate *priv;
  GList *indexes = NULL;
  gchar *context_path;
  gchar *text;   

  priv = JAVA_INDEXER_GET_PRIVATE (indexer);
  
  text = java_utils_get_text_to_search (GTK_TEXT_VIEW (editor), iter);
  
  if (text == NULL)
    return NULL;
  
  context_path = java_indexer_utils_get_context_path (text);
  if (context_path != NULL)
    {
      gchar *group_folder_path;
      group_folder_path = codeslayer_get_active_group_folder_path (priv->codeslayer);

      g_print ("context path: %s\n", context_path);

      indexes = get_indexes (group_folder_path, text, context_path);

      g_free (group_folder_path);
      g_free (context_path);
    }
  
  g_free (text);
    
  return indexes; 
}

/*
 * Take the context path, start at the beginning, and resolve the 
 * actual classes and methods types.
 *
 * For example after the title figure out that this starts as a Column 
 * object and that the title() method also returns a Column object.
 *
 * new Column().title().
 *
 */
static GList*
get_indexes (gchar *group_folder_path,
             gchar *text,
             gchar *context_path)
{
  gchar **split;
  gchar **array;
  GList *indexes = NULL;
  
  split = g_strsplit (context_path, ".", -1);
  array = split;
  
  if (codeslayer_utils_has_text (*array))
    {
      gchar *class_name;
      class_name = java_indexer_utils_get_class_name (text, *array);
      if (class_name != NULL)
        {
          gchar *package_name;
          
          g_print ("class name: %s\n", class_name);
          
          package_name = java_indexer_utils_get_package_name (group_folder_path, text, class_name);
          if (package_name != NULL)
            {
              indexes = java_indexer_utils_get_indexes (group_folder_path, package_name);
              g_free (package_name);
            }
          
          g_free (class_name);
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
  index_file_name = g_build_filename (group_folder_path, "indexes", NULL);
  
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

  string = g_string_append (string, " -indexes ");
  string = g_string_append (string, index_file_name);
  string = g_string_append (string, " -type projects ");

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
  
  if (g_ascii_isupper (*text))
    find_class (indexer, editor, start, text);
  else
    find_method (indexer, editor, start, text);
  
  if (text != NULL)
    g_free (text);
}

static void
find_class (JavaIndexer      *indexer, 
            CodeSlayerEditor *editor,
            GtkTextIter       iter, 
            gchar            *class_name)
{
  JavaIndexerPrivate *priv;
  gchar *text;
  gchar *package_name;
  gchar *group_folder_path;
  GList *indexes;
  GList *list = NULL;
  
  priv = JAVA_INDEXER_GET_PRIVATE (indexer);
  
  text = java_utils_get_text_to_search (GTK_TEXT_VIEW (editor), iter);
  group_folder_path = codeslayer_get_active_group_folder_path (priv->codeslayer);
  package_name = java_indexer_utils_get_package_name (group_folder_path, text, class_name);
  
  g_print ("package_name %s\n", package_name);
  
  indexes = java_indexer_utils_get_indexes (group_folder_path, package_name);
  g_free (group_folder_path);
  
  list = indexes;
  
  if (indexes != NULL)
    {
      JavaIndexerIndex *index = indexes->data;
      select_editor (priv->codeslayer, index, FALSE);
    }

  if (list != NULL)
    {
      g_list_foreach (list, (GFunc) g_object_unref, NULL);
      g_list_free (list);
    }
}
static void
find_method (JavaIndexer      *indexer, 
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
          select_editor (priv->codeslayer, index, TRUE);
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
               JavaIndexerIndex *index, 
               gboolean          has_line_number)
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
  if (has_line_number)
    codeslayer_document_set_line_number (document, line_number);
  codeslayer_document_set_project (document, project);

  codeslayer_select_editor (codeslayer, document);
  
  g_object_unref (document);
}

static void
verify_dir_exists (CodeSlayer *codeslayer)
{
  gchar *group_folder_path;
  gchar *file_name;
  GFile *file;
  
  group_folder_path = codeslayer_get_active_group_folder_path (codeslayer);  
  file_name = g_build_filename (group_folder_path, "indexes", NULL);
  file = g_file_new_for_path (file_name);

  if (!g_file_query_exists (file, NULL)) 
    g_file_make_directory (file, NULL, NULL);

  g_free (group_folder_path);    
  g_free (file_name);
  g_object_unref (file);
}
