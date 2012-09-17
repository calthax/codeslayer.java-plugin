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
#include <string.h>
#include "java-engine.h"
#include "java-indexer.h"
#include "java-debugger.h"
#include "java-build.h"
#include "java-page.h"
#include "java-project-properties.h"
#include "java-configuration.h"
#include "java-completion.h"
#include "java-client.h"
#include "java-notebook.h"
#include "java-usage.h"
#include "java-navigate.h"
#include "java-search.h"
#include "java-import.h"

static void java_engine_class_init                       (JavaEngineClass   *klass);
static void java_engine_init                             (JavaEngine        *engine);
static void java_engine_finalize                         (JavaEngine        *engine);
static void project_properties_opened_action             (JavaEngine        *engine,
                                                          CodeSlayerProject *project);
static void project_properties_saved_action              (JavaEngine        *engine,
                                                          CodeSlayerProject *project);
static void save_ant_build_properties                    (JavaConfiguration *configuration);
static void save_configuration_action                    (JavaEngine        *engine,
                                                          JavaConfiguration *configuration);
                          
#define JAVA_ENGINE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_ENGINE_TYPE, JavaEnginePrivate))

typedef struct _JavaEnginePrivate JavaEnginePrivate;

struct _JavaEnginePrivate
{
  CodeSlayer         *codeslayer;
  JavaClient         *client;
  JavaCompletion     *completion;
  JavaConfigurations *configurations;
  JavaBuild          *build;
  JavaDebugger       *debugger;
  JavaIndexer        *indexer;
  JavaUsage          *usage;
  JavaNavigate       *navigate;
  JavaSearch         *search;
  JavaImport         *import;
  GtkWidget          *menu;
  GtkWidget          *project_properties;
  GtkWidget          *projects_popup;
  GtkWidget          *notebook;
  gulong              properties_opened_id;
  gulong              properties_saved_id;
};

G_DEFINE_TYPE (JavaEngine, java_engine, G_TYPE_OBJECT)

static void
java_engine_class_init (JavaEngineClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) java_engine_finalize;
  g_type_class_add_private (klass, sizeof (JavaEnginePrivate));
}

static void
java_engine_init (JavaEngine *engine) {}

static void
java_engine_finalize (JavaEngine *engine)
{
  JavaEnginePrivate *priv;
  priv = JAVA_ENGINE_GET_PRIVATE (engine);
  g_signal_handler_disconnect (priv->codeslayer, priv->properties_opened_id);
  g_signal_handler_disconnect (priv->codeslayer, priv->properties_saved_id);
  g_object_unref (priv->build);
  g_object_unref (priv->debugger);
  g_object_unref (priv->configurations);
  g_object_unref (priv->indexer);
  g_object_unref (priv->completion);
  g_object_unref (priv->usage);
  g_object_unref (priv->navigate);
  g_object_unref (priv->search);
  g_object_unref (priv->import);
  g_object_unref (priv->client);
  G_OBJECT_CLASS (java_engine_parent_class)->finalize (G_OBJECT(engine));
}

JavaEngine*
java_engine_new (CodeSlayer         *codeslayer,
                 JavaConfigurations *configurations,
                 GtkWidget          *menu, 
                 GtkWidget          *project_properties,
                 GtkWidget          *projects_popup,
                 GtkWidget          *notebook)
{
  JavaEnginePrivate *priv;
  JavaEngine *engine;

  engine = JAVA_ENGINE (g_object_new (java_engine_get_type (), NULL));
  priv = JAVA_ENGINE_GET_PRIVATE (engine);

  priv->codeslayer = codeslayer;
  priv->configurations = configurations;
  priv->menu = menu;
  priv->project_properties = project_properties;
  priv->projects_popup = projects_popup;
  priv->notebook = notebook;
  
  priv->client = java_client_new (codeslayer);
  priv->build = java_build_new (codeslayer, configurations, menu, projects_popup, notebook);
  priv->debugger = java_debugger_new (codeslayer, configurations, menu, notebook);
  priv->indexer = java_indexer_new (codeslayer, menu, configurations, priv->client);
  priv->completion = java_completion_new  (codeslayer, priv->client);
  priv->usage = java_usage_new (codeslayer, menu, notebook, configurations, priv->client);
  priv->navigate = java_navigate_new (codeslayer, menu, configurations, priv->client);
  priv->search = java_search_new (codeslayer, menu, priv->client);
  priv->import = java_import_new (codeslayer, menu, priv->client);
  
  priv->properties_opened_id =  g_signal_connect_swapped (G_OBJECT (codeslayer), "project-properties-opened",
                                                          G_CALLBACK (project_properties_opened_action), engine);

  priv->properties_saved_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "project-properties-saved",
                                                        G_CALLBACK (project_properties_saved_action), engine);

  g_signal_connect_swapped (G_OBJECT (project_properties), "save-configuration",
                            G_CALLBACK (save_configuration_action), engine);
                            
  return engine;
}

static void
project_properties_opened_action (JavaEngine        *engine,
                                  CodeSlayerProject *project)
{
  JavaEnginePrivate *priv;
  const gchar *project_key;
  JavaConfiguration *configuration;
  
  priv = JAVA_ENGINE_GET_PRIVATE (engine);

  project_key = codeslayer_project_get_key (project);
  configuration = java_configurations_find_configuration (priv->configurations, project_key);
  
  java_project_properties_opened (JAVA_PROJECT_PROPERTIES (priv->project_properties),
                                  configuration, project);
}

static void
project_properties_saved_action (JavaEngine        *engine,
                                 CodeSlayerProject *project)
{
  JavaEnginePrivate *priv;
  const gchar *project_key;
  JavaConfiguration *configuration;
  
  priv = JAVA_ENGINE_GET_PRIVATE (engine);

  project_key = codeslayer_project_get_key (project);
  configuration = java_configurations_find_configuration (priv->configurations, project_key);
  
  if (java_project_properties_saved (JAVA_PROJECT_PROPERTIES (priv->project_properties), 
                                     configuration, project))
    {
      if (configuration == NULL)
        configuration = java_configurations_find_configuration (priv->configurations, project_key);
      save_ant_build_properties (configuration);
    }
}

static void
save_ant_build_properties (JavaConfiguration *configuration)
{
  GString *string;
  const gchar *ant_file;
  gchar *dirname;
  gchar *file_path;
  GFile *file;
  gchar *contents;

  ant_file = java_configuration_get_ant_file (configuration);
  dirname = g_path_get_dirname (ant_file);
  file_path = g_build_filename (dirname, "build.properties", NULL);

  string = g_string_new ("");
  string = g_string_append (string, "\nsrc=");
  string = g_string_append (string, java_configuration_get_source_folder (configuration));
  string = g_string_append (string, "\ntest=");
  string = g_string_append (string, java_configuration_get_test_folder (configuration));
  string = g_string_append (string, "\nlib=");
  string = g_string_append (string, java_configuration_get_lib_folder (configuration));
  string = g_string_append (string, "\nbuild=");
  string = g_string_append (string, java_configuration_get_build_folder (configuration));
  
  file = g_file_new_for_path (file_path);
  if (!g_file_query_exists (file, NULL))
    {
      GFileIOStream *stream;           
      stream = g_file_create_readwrite (file, G_FILE_CREATE_NONE, NULL, NULL);
      if (g_io_stream_close (G_IO_STREAM (stream), NULL, NULL))
        g_object_unref (stream);
    }

  contents = g_string_free (string, FALSE);

  g_file_set_contents (file_path, contents, -1, NULL);
  g_object_unref (file);
  g_free (contents);
  g_free (file_path);
  g_free (dirname);
}

static void
save_configuration_action (JavaEngine        *engine,
                           JavaConfiguration *configuration)
{
  JavaEnginePrivate *priv;
  priv = JAVA_ENGINE_GET_PRIVATE (engine);  
  java_configurations_save (priv->configurations, configuration);
}
