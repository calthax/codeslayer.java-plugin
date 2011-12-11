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
#include "java-engine.h"
#include "java-output.h"
#include "java-project-properties.h"
#include "java-configuration.h"
#include "java-notebook.h"

static void java_engine_class_init                          (JavaEngineClass   *klass);
static void java_engine_init                                (JavaEngine        *engine);
static void java_engine_finalize                            (JavaEngine        *engine);

static JavaConfiguration* get_configuration_by_project_key  (JavaEngine        *engine, 
                                                             const gchar       *project_key);
static void project_properties_opened_action                (JavaEngine        *engine,
                                                             CodeSlayerProject *project);
static void project_properties_saved_action                 (JavaEngine        *engine,
                                                             CodeSlayerProject *project);
                                                                        
static void save_configuration_action                       (JavaEngine        *engine,
                                                             JavaConfiguration *configuration);
                                                                  
static gchar* get_configuration_file_path                   (JavaEngine        *engine);

static void compile_action                                  (JavaEngine        *engine);
/*static void project_compile_action                (JavacEngine       *engine, 
                                                   GList             *selections);

static void clean_action                          (JavacEngine       *engine);
static void project_clean_action                  (JavacEngine       *engine, 
                                                   GList             *selections);

static void execute_clean                         (JavacOutput       *output);*/

static void execute_compile                                 (JavaOutput        *output);
static void run_command                                     (JavaOutput        *output,
                                                             gchar             *command);
                                                   
#define JAVA_ENGINE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_ENGINE_TYPE, JavaEnginePrivate))

typedef struct _JavaEnginePrivate JavaEnginePrivate;

struct _JavaEnginePrivate
{
  CodeSlayer *codeslayer;
  GtkWidget  *menu;
  GtkWidget  *project_properties;
  GtkWidget  *projects_popup;
  GtkWidget  *notebook;
  GList      *configurations;
  gulong      properties_opened_id;
  gulong      properties_saved_id;
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
java_engine_init (JavaEngine *engine) 
{
  JavaEnginePrivate *priv;
  priv = JAVA_ENGINE_GET_PRIVATE (engine);
  priv->configurations = NULL;
}

static void
java_engine_finalize (JavaEngine *engine)
{
  JavaEnginePrivate *priv;
  priv = JAVA_ENGINE_GET_PRIVATE (engine);
  if (priv->configurations != NULL)
    {
      g_list_foreach (priv->configurations, (GFunc) g_object_unref, NULL);
      g_list_free (priv->configurations);
      priv->configurations = NULL;    
    }
  
  g_signal_handler_disconnect (priv->codeslayer, priv->properties_opened_id);
  g_signal_handler_disconnect (priv->codeslayer, priv->properties_saved_id);

  G_OBJECT_CLASS (java_engine_parent_class)->finalize (G_OBJECT(engine));
}

JavaEngine*
java_engine_new (CodeSlayer *codeslayer,
                 GtkWidget  *menu, 
                 GtkWidget  *project_properties,
                 GtkWidget  *projects_popup,
                 GtkWidget  *notebook)
{
  JavaEnginePrivate *priv;
  JavaEngine *engine;

  engine = JAVA_ENGINE (g_object_new (java_engine_get_type (), NULL));
  priv = JAVA_ENGINE_GET_PRIVATE (engine);

  priv->codeslayer = codeslayer;
  priv->menu = menu;
  priv->project_properties = project_properties;
  priv->projects_popup = projects_popup;
  priv->notebook = notebook;
  
  priv->properties_opened_id =  g_signal_connect_swapped (G_OBJECT (codeslayer), "project-properties-opened",
                                                          G_CALLBACK (project_properties_opened_action), engine);

  priv->properties_saved_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "project-properties-saved",
                                                        G_CALLBACK (project_properties_saved_action), engine);

  g_signal_connect_swapped (G_OBJECT (project_properties), "save-configuration",
                            G_CALLBACK (save_configuration_action), engine);
                            
  g_signal_connect_swapped (G_OBJECT (menu), "compile",
                            G_CALLBACK (compile_action), engine);

  /*g_signal_connect_swapped (G_OBJECT (projects_menu), "compile",
                            G_CALLBACK (project_compile_action), engine);

  g_signal_connect_swapped (G_OBJECT (menu), "clean",
                            G_CALLBACK (clean_action), engine);

  g_signal_connect_swapped (G_OBJECT (projects_menu), "clean",
                            G_CALLBACK (project_clean_action), engine);*/

  return engine;
}

void 
java_engine_load_configurations (JavaEngine *engine)
{
  JavaEnginePrivate *priv;
  GList *configurations;
  gchar *file_path;

  priv = JAVA_ENGINE_GET_PRIVATE (engine);
  
  file_path = get_configuration_file_path (engine);
  configurations = codeslayer_utils_get_gobjects (JAVA_CONFIGURATION_TYPE,
                                                  FALSE,
                                                  file_path, 
                                                  "java",
                                                  "project_key", G_TYPE_STRING,
                                                  "ant_file", G_TYPE_STRING, 
                                                  "build_folder", G_TYPE_STRING, 
                                                  "lib_folder", G_TYPE_STRING, 
                                                  "source_folder", G_TYPE_STRING, 
                                                  "test_folder", G_TYPE_STRING, 
                                                  NULL);
  priv->configurations = configurations;
  g_free (file_path);
}

static JavaConfiguration*
get_configuration_by_project_key (JavaEngine *engine, 
                                  const gchar     *project_key)
{
  JavaEnginePrivate *priv;
  GList *list;

  priv = JAVA_ENGINE_GET_PRIVATE (engine);

  list = priv->configurations;
  while (list != NULL)
    {
      JavaConfiguration *configuration = list->data;
      const gchar *key;
      
      key = java_configuration_get_project_key (configuration);
      
      if (g_strcmp0 (project_key, key) == 0)
        return configuration;

      list = g_list_next (list);
    }

  return NULL;
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
  configuration = get_configuration_by_project_key (engine, project_key);
  
  java_project_properties_opened (JAVA_PROJECT_PROPERTIES (priv->project_properties),
                                  configuration, project);
}

static void
project_properties_saved_action (JavaEngine   *engine,
                                 CodeSlayerProject *project)
{
  JavaEnginePrivate *priv;
  const gchar *project_key;
  JavaConfiguration *configuration;
  
  priv = JAVA_ENGINE_GET_PRIVATE (engine);

  project_key = codeslayer_project_get_key (project);
  configuration = get_configuration_by_project_key (engine, project_key);
  
  java_project_properties_saved (JAVA_PROJECT_PROPERTIES (priv->project_properties),
                                      configuration, project);
}

static void
save_configuration_action (JavaEngine        *engine,
                           JavaConfiguration *configuration)
{
  JavaEnginePrivate *priv;
  GList *list;
  GList *tmp;
  gchar *file_path;
    
  priv = JAVA_ENGINE_GET_PRIVATE (engine);
  
  if (configuration)
    priv->configurations = g_list_prepend (priv->configurations, configuration);

  list = g_list_copy (priv->configurations);
  tmp = list;
  
  while (tmp != NULL)
    {
      JavaConfiguration *configuration = tmp->data;
      const gchar *ant_file;
      const gchar *build_folder;

      ant_file = java_configuration_get_ant_file (configuration);
      build_folder = java_configuration_get_build_folder (configuration);
      
      if (g_utf8_strlen (ant_file, -1) == 0 &&
          g_utf8_strlen (build_folder, -1) == 0)
        priv->configurations = g_list_remove (priv->configurations, configuration);
      tmp = g_list_next (tmp);
    }
    
  g_list_free (list);    
  
  file_path = get_configuration_file_path (engine);  
  codeslayer_utils_save_gobjects (priv->configurations,
                                  file_path, 
                                  "java",
                                  "project_key", G_TYPE_STRING,
                                  "ant_file", G_TYPE_STRING, 
                                  "build_folder", G_TYPE_STRING, 
                                  "lib_folder", G_TYPE_STRING, 
                                  "source_folder", G_TYPE_STRING, 
                                  "test_folder", G_TYPE_STRING, 
                                  NULL);  
  g_free (file_path);
}

static gchar*
get_configuration_file_path (JavaEngine *engine)
{
  JavaEnginePrivate *priv;
  gchar *folder_path;
  gchar *file_path;
  
  priv = JAVA_ENGINE_GET_PRIVATE (engine);

  folder_path = codeslayer_get_active_group_folder_path (priv->codeslayer);  
  file_path = g_build_filename (folder_path, "java.xml", NULL);
  g_free (folder_path);
  
  return file_path;
}

static void
compile_action (JavaEngine *engine)
{
  JavaEnginePrivate *priv;
  GtkWidget *output;  

  priv = JAVA_ENGINE_GET_PRIVATE (engine);

  output = java_output_new (JAVA_NOTEBOOK_PAGE_TYPE_COMPILER);
  if (output)
    {
      /*codeslayer_show_bottom_pane (priv->codeslayer, priv->notebook);*/
      /*javac_notebook_select_page_by_output (JAVAC_NOTEBOOK (priv->notebook), 
                                                GTK_WIDGET (output));*/
      java_notebook_add_page (JAVA_NOTEBOOK (priv->notebook), output, "Compile");
      g_thread_create ((GThreadFunc) execute_compile, JAVA_OUTPUT (output), FALSE, NULL);
    }
}

/*static void
project_compile_action (JavacEngine *engine, 
                        GList       *selections)
{
  JavacEnginePrivate *priv;
  JavacOutput *output;
  CodeSlayerProject *project;

  priv = JAVAC_ENGINE_GET_PRIVATE (engine);
  
  project = get_selections_project (selections);
  output = get_output_by_project (engine, project);
  if (output)
    {
      codeslayer_show_bottom_pane (priv->codeslayer, priv->notebook);
      javac_notebook_select_page_by_output (JAVAC_NOTEBOOK (priv->notebook), 
                                            GTK_WIDGET (output));
      g_thread_create ((GThreadFunc) execute_compile, output, FALSE, NULL);    
    }
}

static void
clean_action (JavacEngine *engine)
{
  JavacEnginePrivate *priv;
  JavacOutput *output;  

  priv = JAVAC_ENGINE_GET_PRIVATE (engine);

  output =  get_output_by_active_editor (engine);
  if (output)
    {
      codeslayer_show_bottom_pane (priv->codeslayer, priv->notebook);
      javac_notebook_select_page_by_output (JAVAC_NOTEBOOK (priv->notebook), 
                                                GTK_WIDGET (output));
      g_thread_create ((GThreadFunc) execute_clean, output, FALSE, NULL);
    }
}

static void
project_clean_action (JavacEngine *engine, 
                      GList       *selections)
{
  JavacEnginePrivate *priv;
  JavacOutput *output;  
  CodeSlayerProject *project;

  priv = JAVAC_ENGINE_GET_PRIVATE (engine);
  
  project = get_selections_project (selections);
  output =  get_output_by_project (engine, project);
  if (output)
    {
      codeslayer_show_bottom_pane (priv->codeslayer, priv->notebook);
      javac_notebook_select_page_by_output (JAVAC_NOTEBOOK (priv->notebook), 
                                                GTK_WIDGET (output));
      g_thread_create ((GThreadFunc) execute_clean, output, FALSE, NULL);
    }
}

static CodeSlayerProject*
get_selections_project (GList *selections)
{
  if (selections != NULL)
    {
      CodeSlayerProjectsSelection *selection = selections->data;
      return codeslayer_projects_selection_get_project (CODESLAYER_PROJECTS_SELECTION (selection));
    }
  return NULL;  
}
*/

static void
execute_compile (JavaOutput *output)
{
  /*CodeSlayerProject *project;*/
  /*const gchar *build_file_path;*/
  gchar *command;
  gchar *dirname = "/home/jeff/workspace/jmesa";
  
  /*project = java_output_get_project (output);
  build_file_path = codeslayer_project_get_build_file_path (project);
  dirname = g_path_get_dirname (build_file_path);*/
  
  command = g_strconcat ("cd ", dirname, "; ant compile 2>&1", NULL);
  run_command (output, command);
  g_free (command);
  /*g_free (dirname);*/
}

/*static void
execute_clean (JavacOutput *output)
{
  CodeSlayerProject *project;
  const gchar *build_file_path;
  gchar *command;
  gchar *dirname;
  
  project = javac_output_get_project (output);
  build_file_path = codeslayer_project_get_build_file_path (project);
  dirname = g_path_get_dirname (build_file_path);
  
  command = g_strconcat ("cd ", dirname, "; ant clean 2>&1", NULL);
  run_command (output, command);
  g_free (command);
  g_free (dirname);
}*/

static void
run_command (JavaOutput *output,
             gchar      *command)
{
  GtkTextBuffer *buffer;
  GtkTextIter iter;
  GtkTextMark *text_mark;
  char out[BUFSIZ];
  FILE *file;
  
  gdk_threads_enter ();
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (output));
  gtk_text_buffer_set_text (buffer, "", -1);
  gdk_threads_leave ();
  
  file = popen (command, "r");
  if (file != NULL)
    {
      while (fgets (out, BUFSIZ, file))
        {
          gdk_threads_enter ();
          gtk_text_buffer_get_end_iter (buffer, &iter);
          gtk_text_buffer_insert (buffer, &iter, out, -1);
          text_mark = gtk_text_buffer_create_mark (buffer, NULL, &iter, TRUE);
          gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (output), text_mark, 0.0, FALSE, 0, 0);
          gdk_threads_leave ();
        }
      pclose (file);
    }
}
