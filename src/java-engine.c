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
#include "autotools-engine.h"
#include "autotools-project-properties.h"
#include "autotools-configuration.h"
#include "autotools-notebook.h"
#include "autotools-output.h"

static void autotools_engine_class_init                          (AutotoolsEngineClass   *klass);
static void autotools_engine_init                                (AutotoolsEngine        *engine);
static void autotools_engine_finalize                            (AutotoolsEngine        *engine);

static void make_action                                          (AutotoolsEngine        *engine);
static void project_make_action                                  (AutotoolsEngine        *engine, 
                                                                  GList                  *selections);

static void make_install_action                                  (AutotoolsEngine        *engine);
static void project_make_install_action                          (AutotoolsEngine        *engine, 
                                                                  GList                  *selections);

static void make_clean_action                                    (AutotoolsEngine        *engine);
static void project_make_clean_action                            (AutotoolsEngine        *engine, 
                                                                  GList                  *selections);

static void project_configure_action                             (AutotoolsEngine        *engine, 
                                                                  GList                  *selections);

static void project_autoreconf_action                            (AutotoolsEngine        *engine, 
                                                                  GList                  *selections);

static void execute_make                                         (AutotoolsOutput        *output);
static void execute_make_clean                                   (AutotoolsOutput        *output);
static void execute_make_install                                 (AutotoolsOutput        *output);
static void execute_configure                                    (AutotoolsOutput        *output);
static void execute_autoreconf                                   (AutotoolsOutput        *output);

static void run_command                                          (AutotoolsOutput        *output,
                                                                  gchar                  *command);

static AutotoolsConfiguration* get_configuration_by_project_key  (AutotoolsEngine        *engine, 
                                                                  const gchar            *project_key);
static void execute_configure                                    (AutotoolsOutput        *output);
static AutotoolsOutput* get_output_by_active_editor              (AutotoolsEngine        *engine);

static AutotoolsOutput* get_output_by_project                    (AutotoolsEngine        *engine, 
                                                                  CodeSlayerProject      *project);

static void project_properties_opened_action                     (AutotoolsEngine        *engine,
                                                                  CodeSlayerProject      *project);
static void project_properties_saved_action                      (AutotoolsEngine        *engine,
                                                                  CodeSlayerProject      *project);
                                                                        
static void save_configuration_action                            (AutotoolsEngine        *engine,
                                                                  AutotoolsConfiguration *configuration);
                                                                  
static CodeSlayerProject* get_selections_project                 (GList                  *selections);

static gchar* get_configuration_file_path                        (AutotoolsEngine        *engine);
                                                   
#define AUTOTOOLS_ENGINE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), AUTOTOOLS_ENGINE_TYPE, AutotoolsEnginePrivate))

typedef struct _AutotoolsEnginePrivate AutotoolsEnginePrivate;

struct _AutotoolsEnginePrivate
{
  CodeSlayer *codeslayer;
  GtkWidget  *menu;
  GtkWidget  *project_properties;
  GtkWidget  *projects_menu;
  GtkWidget  *notebook;
  GList      *configurations;
  gulong      properties_opened_id;
  gulong      properties_saved_id;
};

G_DEFINE_TYPE (AutotoolsEngine, autotools_engine, G_TYPE_OBJECT)

static void
autotools_engine_class_init (AutotoolsEngineClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) autotools_engine_finalize;
  g_type_class_add_private (klass, sizeof (AutotoolsEnginePrivate));
}

static void
autotools_engine_init (AutotoolsEngine *engine) 
{
  AutotoolsEnginePrivate *priv;
  priv = AUTOTOOLS_ENGINE_GET_PRIVATE (engine);
  priv->configurations = NULL;
}

static void
autotools_engine_finalize (AutotoolsEngine *engine)
{
  AutotoolsEnginePrivate *priv;
  priv = AUTOTOOLS_ENGINE_GET_PRIVATE (engine);
  if (priv->configurations != NULL)
    {
      g_list_foreach (priv->configurations, (GFunc) g_object_unref, NULL);
      g_list_free (priv->configurations);
      priv->configurations = NULL;    
    }
  
  g_signal_handler_disconnect (priv->codeslayer, priv->properties_opened_id);
  g_signal_handler_disconnect (priv->codeslayer, priv->properties_saved_id);

  G_OBJECT_CLASS (autotools_engine_parent_class)->finalize (G_OBJECT(engine));
}

AutotoolsEngine*
autotools_engine_new (CodeSlayer *codeslayer,
                      GtkWidget  *menu, 
                      GtkWidget  *project_properties,
                      GtkWidget  *projects_menu,
                      GtkWidget  *notebook)
{
  AutotoolsEnginePrivate *priv;
  AutotoolsEngine *engine;

  engine = AUTOTOOLS_ENGINE (g_object_new (autotools_engine_get_type (), NULL));
  priv = AUTOTOOLS_ENGINE_GET_PRIVATE (engine);

  priv->codeslayer = codeslayer;
  priv->menu = menu;
  priv->project_properties = project_properties;
  priv->projects_menu = projects_menu;
  priv->notebook = notebook;
  
  g_signal_connect_swapped (G_OBJECT (menu), "make",
                            G_CALLBACK (make_action), engine);

  g_signal_connect_swapped (G_OBJECT (projects_menu), "make",
                            G_CALLBACK (project_make_action), engine);

  g_signal_connect_swapped (G_OBJECT (menu), "make-install",
                            G_CALLBACK (make_install_action), engine);

  g_signal_connect_swapped (G_OBJECT (projects_menu), "make-install",
                            G_CALLBACK (project_make_install_action), engine);

  g_signal_connect_swapped (G_OBJECT (menu), "make-clean",
                            G_CALLBACK (make_clean_action), engine);

  g_signal_connect_swapped (G_OBJECT (projects_menu), "make-clean",
                            G_CALLBACK (project_make_clean_action), engine);

  g_signal_connect_swapped (G_OBJECT (projects_menu), "configure",
                            G_CALLBACK (project_configure_action), engine);

  g_signal_connect_swapped (G_OBJECT (projects_menu), "autoreconf",
                            G_CALLBACK (project_autoreconf_action), engine);

  priv->properties_opened_id =  g_signal_connect_swapped (G_OBJECT (codeslayer), "project-properties-opened",
                                                          G_CALLBACK (project_properties_opened_action), engine);

  priv->properties_saved_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "project-properties-saved",
                                                        G_CALLBACK (project_properties_saved_action), engine);

  g_signal_connect_swapped (G_OBJECT (project_properties), "save-configuration",
                            G_CALLBACK (save_configuration_action), engine);

  return engine;
}

void 
autotools_engine_load_configurations (AutotoolsEngine *engine)
{
  AutotoolsEnginePrivate *priv;
  GList *configurations;
  gchar *file_path;

  priv = AUTOTOOLS_ENGINE_GET_PRIVATE (engine);
  
  file_path = get_configuration_file_path (engine);
  configurations = codeslayer_utils_get_gobjects (AUTOTOOLS_CONFIGURATION_TYPE,
                                                  FALSE,
                                                  file_path, 
                                                  "autotool",
                                                  "project_key", G_TYPE_STRING,
                                                  "configure_file", G_TYPE_STRING, 
                                                  "configure_parameters", G_TYPE_STRING, 
                                                  "build_directory", G_TYPE_STRING, 
                                                  NULL);
  priv->configurations = configurations;
  g_free (file_path);
}

static AutotoolsConfiguration*
get_configuration_by_project_key (AutotoolsEngine *engine, 
                                  const gchar     *project_key)
{
  AutotoolsEnginePrivate *priv;
  GList *list;

  priv = AUTOTOOLS_ENGINE_GET_PRIVATE (engine);

  list = priv->configurations;
  while (list != NULL)
    {
      AutotoolsConfiguration *configuration = list->data;
      const gchar *key;
      
      key = autotools_configuration_get_project_key (configuration);
      
      if (g_strcmp0 (project_key, key) == 0)
        return configuration;

      list = g_list_next (list);
    }

  return NULL;
}

static void
project_properties_opened_action (AutotoolsEngine   *engine,
                                  CodeSlayerProject *project)
{
  AutotoolsEnginePrivate *priv;
  const gchar *project_key;
  AutotoolsConfiguration *configuration;
  
  priv = AUTOTOOLS_ENGINE_GET_PRIVATE (engine);

  project_key = codeslayer_project_get_key (project);
  configuration = get_configuration_by_project_key (engine, project_key);
  
  autotools_project_properties_opened (AUTOTOOLS_PROJECT_PROPERTIES (priv->project_properties),
                                       configuration, project);
}

static void
project_properties_saved_action (AutotoolsEngine   *engine,
                                 CodeSlayerProject *project)
{
  AutotoolsEnginePrivate *priv;
  const gchar *project_key;
  AutotoolsConfiguration *configuration;
  
  priv = AUTOTOOLS_ENGINE_GET_PRIVATE (engine);

  project_key = codeslayer_project_get_key (project);
  configuration = get_configuration_by_project_key (engine, project_key);
  
  autotools_project_properties_saved (AUTOTOOLS_PROJECT_PROPERTIES (priv->project_properties),
                                      configuration, project);
}

static void
save_configuration_action (AutotoolsEngine        *engine,
                           AutotoolsConfiguration *configuration)
{
  AutotoolsEnginePrivate *priv;
  GList *list;
  GList *tmp;
  gchar *file_path;
    
  priv = AUTOTOOLS_ENGINE_GET_PRIVATE (engine);
  
  if (configuration)
    priv->configurations = g_list_prepend (priv->configurations, configuration);

  list = g_list_copy (priv->configurations);
  tmp = list;
  
  while (tmp != NULL)
    {
      AutotoolsConfiguration *configuration = tmp->data;
      const gchar *configure_file;
      const gchar *configure_parameters;
      const gchar *build_directory;

      configure_file = autotools_configuration_get_configure_file (configuration);
      configure_parameters = autotools_configuration_get_configure_parameters (configuration);
      build_directory = autotools_configuration_get_build_directory (configuration);
      
      if (g_utf8_strlen (configure_file, -1) == 0 &&
          g_utf8_strlen (configure_parameters, -1) == 0 &&
          g_utf8_strlen (build_directory, -1) == 0)
        priv->configurations = g_list_remove (priv->configurations, configuration);
      tmp = g_list_next (tmp);
    }
    
  g_list_free (list);    
  
  file_path = get_configuration_file_path (engine);  
  codeslayer_utils_save_gobjects (priv->configurations,
                                  file_path, 
                                  "autotool",
                                  "project_key", G_TYPE_STRING,
                                  "configure_file", G_TYPE_STRING, 
                                  "configure_parameters", G_TYPE_STRING, 
                                  "build_directory", G_TYPE_STRING, 
                                  NULL);  
  g_free (file_path);
}

static gchar*
get_configuration_file_path (AutotoolsEngine *engine)
{
  AutotoolsEnginePrivate *priv;
  gchar *folder_path;
  gchar *file_path;
  
  priv = AUTOTOOLS_ENGINE_GET_PRIVATE (engine);

  folder_path = codeslayer_get_active_group_folder_path (priv->codeslayer);  
  file_path = g_build_filename (folder_path, "autotools.xml", NULL);
  g_free (folder_path);
  
  return file_path;
}
                            
static void
make_action (AutotoolsEngine *engine)
{
  AutotoolsEnginePrivate *priv;
  AutotoolsOutput *output;  

  priv = AUTOTOOLS_ENGINE_GET_PRIVATE (engine);

  output =  get_output_by_active_editor (engine);
  if (output)
    {
      codeslayer_show_bottom_pane (priv->codeslayer, priv->notebook);
      autotools_notebook_select_page_by_output (AUTOTOOLS_NOTEBOOK (priv->notebook), 
                                                GTK_WIDGET (output));
      g_thread_create ((GThreadFunc) execute_make, output, FALSE, NULL);    
    }
}

static void
project_make_action (AutotoolsEngine *engine, 
                     GList           *selections)
{
  AutotoolsEnginePrivate *priv;
  AutotoolsOutput *output;
  CodeSlayerProject *project;

  priv = AUTOTOOLS_ENGINE_GET_PRIVATE (engine);
  
  project = get_selections_project (selections);
  output = get_output_by_project (engine, project);
  if (output)
    {
      codeslayer_show_bottom_pane (priv->codeslayer, priv->notebook);
      autotools_notebook_select_page_by_output (AUTOTOOLS_NOTEBOOK (priv->notebook), 
                                                GTK_WIDGET (output));
      g_thread_create ((GThreadFunc) execute_make, output, FALSE, NULL);    
    }
}

static void
make_install_action (AutotoolsEngine *engine)
{
  AutotoolsEnginePrivate *priv;
  AutotoolsOutput *output;  

  priv = AUTOTOOLS_ENGINE_GET_PRIVATE (engine);

  output =  get_output_by_active_editor (engine);
  if (output)
    {
      codeslayer_show_bottom_pane (priv->codeslayer, priv->notebook);
      autotools_notebook_select_page_by_output (AUTOTOOLS_NOTEBOOK (priv->notebook), 
                                                GTK_WIDGET (output));
      g_thread_create ((GThreadFunc) execute_make_install, output, FALSE, NULL);
    }
}   

static void
project_make_install_action (AutotoolsEngine *engine, 
                             GList           *selections)
{
  AutotoolsEnginePrivate *priv;
  AutotoolsOutput *output;  
  CodeSlayerProject *project;

  priv = AUTOTOOLS_ENGINE_GET_PRIVATE (engine);
  
  project = get_selections_project (selections);
  output =  get_output_by_project (engine, project);
  if (output)
    {
      codeslayer_show_bottom_pane (priv->codeslayer, priv->notebook);
      autotools_notebook_select_page_by_output (AUTOTOOLS_NOTEBOOK (priv->notebook), 
                                                GTK_WIDGET (output));
      g_thread_create ((GThreadFunc) execute_make_install, output, FALSE, NULL);
    }
}   

static void
make_clean_action (AutotoolsEngine *engine)
{
  AutotoolsEnginePrivate *priv;
  AutotoolsOutput *output;  

  priv = AUTOTOOLS_ENGINE_GET_PRIVATE (engine);

  output =  get_output_by_active_editor (engine);
  if (output)
    {
      codeslayer_show_bottom_pane (priv->codeslayer, priv->notebook);
      autotools_notebook_select_page_by_output (AUTOTOOLS_NOTEBOOK (priv->notebook), 
                                                GTK_WIDGET (output));
      g_thread_create ((GThreadFunc) execute_make_clean, output, FALSE, NULL);
    }
}

static void
project_make_clean_action (AutotoolsEngine *engine, 
                           GList           *selections)
{
  AutotoolsEnginePrivate *priv;
  AutotoolsOutput *output;  
  CodeSlayerProject *project;

  priv = AUTOTOOLS_ENGINE_GET_PRIVATE (engine);
  
  project = get_selections_project (selections);
  output =  get_output_by_project (engine, project);
  if (output)
    {
      codeslayer_show_bottom_pane (priv->codeslayer, priv->notebook);
      autotools_notebook_select_page_by_output (AUTOTOOLS_NOTEBOOK (priv->notebook), 
                                                GTK_WIDGET (output));
      g_thread_create ((GThreadFunc) execute_make_clean, output, FALSE, NULL);
    }
}

static void
project_configure_action (AutotoolsEngine *engine, 
                          GList           *selections)
{
  AutotoolsEnginePrivate *priv;
  AutotoolsOutput *output;  
  CodeSlayerProject *project;

  priv = AUTOTOOLS_ENGINE_GET_PRIVATE (engine);
  
  project = get_selections_project (selections);
  output =  get_output_by_project (engine, project);
  if (output)
    {
      codeslayer_show_bottom_pane (priv->codeslayer, priv->notebook);
      autotools_notebook_select_page_by_output (AUTOTOOLS_NOTEBOOK (priv->notebook), 
                                                GTK_WIDGET (output));
      g_thread_create ((GThreadFunc) execute_configure, output, FALSE, NULL);
    }
}

static void
project_autoreconf_action (AutotoolsEngine *engine, 
                           GList           *selections)
{
  AutotoolsEnginePrivate *priv;
  AutotoolsOutput *output;  
  CodeSlayerProject *project;

  priv = AUTOTOOLS_ENGINE_GET_PRIVATE (engine);
  
  project = get_selections_project (selections);
  output =  get_output_by_project (engine, project);
  if (output)
    {
      codeslayer_show_bottom_pane (priv->codeslayer, priv->notebook);
      autotools_notebook_select_page_by_output (AUTOTOOLS_NOTEBOOK (priv->notebook), 
                                                GTK_WIDGET (output));
      g_thread_create ((GThreadFunc) execute_autoreconf, output, FALSE, NULL);
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

static void
execute_make (AutotoolsOutput *output)
{
  AutotoolsConfiguration *configuration;
  const gchar *build_directory;             
  gchar *command;
  
  configuration = autotools_output_get_configuration (output);
  build_directory = autotools_configuration_get_build_directory (configuration);
  
  command = g_strconcat ("cd ", build_directory, ";make 2>&1", NULL);
  run_command (output, command);
  g_free (command);
}

static void
execute_make_install (AutotoolsOutput *output)
{
  AutotoolsConfiguration *configuration;
  const gchar *build_directory;             
  gchar *command;
  
  configuration = autotools_output_get_configuration (output);
  build_directory = autotools_configuration_get_build_directory (configuration);
  
  command = g_strconcat ("cd ", build_directory, ";make install 2>&1", NULL);
  run_command (output, command);
  g_free (command);    
}

static void
execute_make_clean (AutotoolsOutput *output)
{
  AutotoolsConfiguration *configuration;
  const gchar *build_directory;             
  gchar *command;
  
  configuration = autotools_output_get_configuration (output);
  build_directory = autotools_configuration_get_build_directory (configuration);
  
  command = g_strconcat ("cd ", build_directory, ";make clean 2>&1", NULL);
  run_command (output, command);
  g_free (command);
}

static void
execute_configure (AutotoolsOutput *output)
{
  AutotoolsConfiguration *configuration;
  const gchar *build_directory;
  const gchar *configure_file;             
  const gchar *configure_parameters;             
  gchar *configure_file_path;             
  gchar *command;
  
  configuration = autotools_output_get_configuration (output);
  build_directory = autotools_configuration_get_build_directory (configuration);
  configure_file = autotools_configuration_get_configure_file (configuration);
  configure_file_path = g_path_get_dirname (configure_file);
  configure_parameters = autotools_configuration_get_configure_parameters (configuration);
  
  command = g_strconcat ("cd ", build_directory, ";", configure_file_path, 
                         G_DIR_SEPARATOR_S, "configure ", configure_parameters, " 2>&1", NULL);
  g_free (configure_file_path);    

  run_command (output, command);
  g_free (command);    
}

static void
execute_autoreconf (AutotoolsOutput *output)
{
  AutotoolsConfiguration *configuration;
  GtkTextBuffer *buffer;
  GtkTextIter iter;
  GtkTextMark *text_mark;
  const gchar *configure_file;             
  gchar *configure_file_path;             
  gchar *command;
  
  configuration = autotools_output_get_configuration (output);
  configure_file = autotools_configuration_get_configure_file (configuration);
  configure_file_path = g_path_get_dirname (configure_file);
  
  command = g_strconcat ("cd ", configure_file_path, ";autoreconf 2>&1", NULL);
  g_free (configure_file_path);

  run_command (output, command);
  g_free (command);
  
  gdk_threads_enter ();
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (output));
  gtk_text_buffer_get_end_iter (buffer, &iter);
  gtk_text_buffer_insert (buffer, &iter, "autoreconf finished\n", -1);
  text_mark = gtk_text_buffer_create_mark (buffer, NULL, &iter, TRUE);
  gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (output), text_mark, 0.0, FALSE, 0, 0);
  gdk_threads_leave ();  
}

static AutotoolsOutput*
get_output_by_active_editor (AutotoolsEngine *engine)
{
  AutotoolsEnginePrivate *priv;
  CodeSlayer *codeslayer;
  CodeSlayerProject *project;
  
  priv = AUTOTOOLS_ENGINE_GET_PRIVATE (engine);
  codeslayer = priv->codeslayer;
  
  project = codeslayer_get_active_editor_project (codeslayer);
  if (project == NULL)
    {
      GtkWidget *dialog;
      dialog =  gtk_message_dialog_new (NULL, 
                                        GTK_DIALOG_MODAL,
                                        GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                        "There are no open editors. Not able to determine what project to execute command against.");
      gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (dialog);
      return NULL;
    }

  return get_output_by_project (engine, project);
}

static AutotoolsOutput*
get_output_by_project (AutotoolsEngine   *engine, 
                       CodeSlayerProject *project)
{
  AutotoolsEnginePrivate *priv;
  GtkWidget  *notebook;
  const gchar *project_key;
  const gchar *project_name;
  AutotoolsConfiguration *configuration;
  GtkWidget *output;
  
  priv = AUTOTOOLS_ENGINE_GET_PRIVATE (engine);
  notebook = priv->notebook;
  
  project_key = codeslayer_project_get_key (project);
  project_name = codeslayer_project_get_name (project);
  configuration = get_configuration_by_project_key (engine, project_key);
  
  if (configuration == NULL)
    {
      GtkWidget *dialog;
      gchar *msg;
      msg = g_strconcat ("There is no configuration for project ", 
                         project_name, ".", NULL);      
      dialog =  gtk_message_dialog_new (NULL, 
                                        GTK_DIALOG_MODAL,
                                        GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                        msg, 
                                        NULL);
      gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (dialog);
      g_free (msg);
      return NULL;
    }
  
  output = autotools_notebook_get_output_by_configuration (AUTOTOOLS_NOTEBOOK (notebook), 
                                                           configuration);
  if (output == NULL)
    {
      output = autotools_output_new (configuration);
      autotools_notebook_add_output (AUTOTOOLS_NOTEBOOK (priv->notebook), output, project_name);
    }                                                           

  return AUTOTOOLS_OUTPUT (output);
}

static void
run_command (AutotoolsOutput *output,
             gchar           *command)
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
