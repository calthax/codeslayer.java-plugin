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
#include <stdio.h>
#include <stdlib.h>
#include "java-indexer.h"
#include "java-indexer-index.h"
#include "java-configuration.h"
#include "java-utils.h"

static void java_indexer_class_init   (JavaIndexerClass *klass);
static void java_indexer_init         (JavaIndexer      *indexer);
static void java_indexer_finalize     (JavaIndexer      *indexer);

static GList* get_package_indexes     (JavaIndexer      *indexer, 
                                       gchar            *group_folder_path,
                                       gchar            *index_file_name,
                                       gchar            *package_name);
static void editor_saved_action       (JavaIndexer      *indexer,
                                       CodeSlayerEditor *editor);
static void execute_create_indexes    (JavaIndexer      *indexer);
static gboolean start_create_indexes  (JavaIndexer      *indexer);
static void finish_create_indexes     (JavaIndexer      *indexer);

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
                  JavaConfigurations *configurations)
{
  JavaIndexerPrivate *priv;
  JavaIndexer *indexer;

  indexer = JAVA_INDEXER (g_object_new (java_indexer_get_type (), NULL));
  priv = JAVA_INDEXER_GET_PRIVATE (indexer);
  priv->codeslayer = codeslayer;
  priv->configurations = configurations;

  priv->saved_handler_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "editor-saved", 
                                                     G_CALLBACK (editor_saved_action), indexer);

  return indexer;
}

GList*
java_indexer_get_package_indexes (JavaIndexer *indexer, 
                                  gchar       *package_name)
{
  JavaIndexerPrivate *priv;
  GList *indexes = NULL;
  gchar *group_folder_path;
  
  priv = JAVA_INDEXER_GET_PRIVATE (indexer);
  
  group_folder_path = codeslayer_get_active_group_folder_path (priv->codeslayer);

  indexes = get_package_indexes (indexer, group_folder_path, "projects.indexes", package_name);
  if (indexes == NULL)
    indexes = get_package_indexes (indexer, group_folder_path, "libs.indexes", package_name);
    
  g_free (group_folder_path);

  return indexes;
}

static GList*
get_package_indexes (JavaIndexer *indexer,
                     gchar       *group_folder_path,
                     gchar       *index_file_name,
                     gchar       *package_name)
{
  JavaIndexerPrivate *priv;
  GList *indexes = NULL;
  gchar *file_name;
  GIOChannel *channel;
  gchar *text;
  GError *error = NULL;
  
  priv = JAVA_INDEXER_GET_PRIVATE (indexer);
  
  group_folder_path = codeslayer_get_active_group_folder_path (priv->codeslayer);
  file_name = g_build_filename (group_folder_path, "indexes", index_file_name, NULL);
  
  if (!g_file_test (file_name, G_FILE_TEST_EXISTS))
    {
      g_warning ("There is no projects indexes.");
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

      if (g_strcmp0 (package_name, *array) == 0)
        {
          JavaIndexerIndex *index;
          index = java_indexer_index_new ();
          java_indexer_index_set_package_name (index, *array);
          java_indexer_index_set_class_name (index, *++array);
          java_indexer_index_set_method_modifier (index, *++array);
          java_indexer_index_set_method_name (index, *++array);
          java_indexer_index_set_method_parameters (index, *++array);
          java_indexer_index_set_method_completion (index, *++array);
          java_indexer_index_set_method_return_type (index, *++array);
          
          if (*++array != NULL)
            {
              java_indexer_index_set_file_path (index, *array);
              java_indexer_index_set_line_number (index, atoi (*++array));
            }
            
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
  CodeSlayerGroup *group;
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
  
  group = codeslayer_get_active_group (priv->codeslayer);
  list = codeslayer_group_get_projects (group);
  while (list != NULL)
    {
      CodeSlayerProject *project = list->data;
      const gchar *project_folder_path;
      project_folder_path = codeslayer_project_get_folder_path (project);
      if (codeslayer_utils_has_text (project_folder_path))
        {
          string = g_string_append (string, project_folder_path);
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
