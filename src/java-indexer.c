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
#include "java-utils.h"
#include "java-configuration.h"
#include "java-client.h"

static void java_indexer_class_init    (JavaIndexerClass *klass);
static void java_indexer_init          (JavaIndexer      *indexer);
static void java_indexer_finalize      (JavaIndexer      *indexer);

static void editors_all_saved_action   (JavaIndexer      *indexer,
                                        GList            *editors);
static void create_projects_indexes    (JavaIndexer      *indexer);
static void create_libs_indexes        (JavaIndexer      *indexer);
static void verify_dir_exists          (CodeSlayer       *codeslayer);
static void index_projects_action      (JavaIndexer      *indexer);
static void index_libs_action          (JavaIndexer      *indexer);

#define JAVA_INDEXER_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_INDEXER_TYPE, JavaIndexerPrivate))

typedef struct _JavaIndexerPrivate JavaIndexerPrivate;

struct _JavaIndexerPrivate
{
  CodeSlayer          *codeslayer;
  JavaClient          *client;
  JavaToolsProperties *tools_properties;
  JavaConfigurations  *configurations;
  gulong               saved_handler_id;
  guint                event_source_id;
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
  
  if (priv->client)
    g_object_unref (priv->client);
  
  g_signal_handler_disconnect (priv->codeslayer, priv->saved_handler_id);
  G_OBJECT_CLASS (java_indexer_parent_class)->finalize (G_OBJECT (indexer));
}

JavaIndexer*
java_indexer_new (CodeSlayer          *codeslayer,
                  GtkWidget           *menu,
                  JavaToolsProperties *tools_properties,
                  JavaConfigurations  *configurations)
{
  JavaIndexerPrivate *priv;
  JavaIndexer *indexer;

  indexer = JAVA_INDEXER (g_object_new (java_indexer_get_type (), NULL));
  priv = JAVA_INDEXER_GET_PRIVATE (indexer);
  priv->codeslayer = codeslayer;
  priv->tools_properties = tools_properties;
  priv->configurations = configurations;
  
  priv->client = java_client_new (codeslayer);

  priv->saved_handler_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "editors-all-saved", 
                                                     G_CALLBACK (editors_all_saved_action), indexer);
                                                     
  verify_dir_exists (codeslayer);

  g_signal_connect_swapped (G_OBJECT (menu), "index-projects",
                            G_CALLBACK (index_projects_action), indexer);

  g_signal_connect_swapped (G_OBJECT (menu), "index-libs",
                            G_CALLBACK (index_libs_action), indexer);

  return indexer;
}

static void 
editors_all_saved_action (JavaIndexer *indexer,
                          GList      *editors)
{
  gboolean found;
  found = FALSE;
  
  while (editors != NULL)
    {
      CodeSlayerEditor *editor = editors->data;
      CodeSlayerDocument *document;
      const gchar *file_path;
      
      document = codeslayer_editor_get_document (editor);
      file_path = codeslayer_document_get_file_path (document);
      if (g_str_has_suffix (file_path, ".java"))
        found = TRUE;
        
      editors = g_list_next (editors);
    }

  if (found)
    g_thread_create ((GThreadFunc) create_projects_indexes, indexer, FALSE, NULL);
}

static void
index_projects_action (JavaIndexer *indexer)
{
  g_thread_create ((GThreadFunc) create_projects_indexes, indexer, FALSE, NULL);
}

static void
index_libs_action (JavaIndexer *indexer)
{
  g_thread_create ((GThreadFunc) create_libs_indexes, indexer, FALSE, NULL);
}

static void
create_projects_indexes (JavaIndexer *indexer)
{
  JavaIndexerPrivate *priv;

  gchar *source_indexes_folders;
  gchar *input;
  gchar *output;
  
  priv = JAVA_INDEXER_GET_PRIVATE (indexer);
  
  source_indexes_folders = get_source_indexes_folders (priv->codeslayer, priv->configurations);
  
  input = g_strconcat ("-program indexer -type projects", source_indexes_folders, NULL);
  
  g_print ("input %s\n", input);

  output = java_client_send (priv->client, input);
  
  g_free (source_indexes_folders);
  g_free (input);
  
  if (output != NULL)
    {
      g_print ("output %s\n", output);    
      g_free (output);
    }
}

static void
create_libs_indexes (JavaIndexer *indexer)
{
  JavaIndexerPrivate *priv;

  GString *string;

  gchar *lib_indexes_folders;

  gchar *group_folder_path;
  gchar *tmp_folder_path;
  const gchar *jdk_folder;
  const gchar *suppressions_file;

  gchar *input;
  gchar *output;
  
  priv = JAVA_INDEXER_GET_PRIVATE (indexer);
  
  lib_indexes_folders = get_lib_indexes_folders (priv->codeslayer, priv->configurations);
  jdk_folder = java_tools_properties_get_jdk_folder (priv->tools_properties);
  suppressions_file = java_tools_properties_get_suppressions_file (priv->tools_properties);
  
  group_folder_path = codeslayer_get_active_group_folder_path (priv->codeslayer);
  tmp_folder_path = g_build_filename (group_folder_path, "indexes", "tmp", NULL);
  
  string = g_string_new ("");
  string = g_string_append (string, "-program indexer -type libs");
  string = g_string_append (string, lib_indexes_folders);
  
  if (codeslayer_utils_has_text (suppressions_file))
    {
      string = g_string_append (string, " -suppressionsfile ");
      string = g_string_append (string, suppressions_file);
    }

  if (codeslayer_utils_has_text (jdk_folder))
    {
      string = g_string_append (string, " -zipfile ");
      string = g_string_append (string, jdk_folder);
      string = g_string_append (string, G_DIR_SEPARATOR_S);
      string = g_string_append (string, "src.zip");
      string = g_string_append (string, " -tmpfolder ");
      string = g_string_append (string, tmp_folder_path);
    }
    
  input = g_string_free (string, FALSE);    
  
  g_print ("input %s\n", input);

  output = java_client_send (priv->client, input);
  
  g_free (lib_indexes_folders);
  g_free (input);
  
  if (output != NULL)
    {
      g_print ("output %s\n", output);    
      g_free (output);
    }
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
