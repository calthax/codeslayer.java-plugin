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
#include "java-build.h"
#include "java-build-pane.h"
#include "java-page.h"
#include "java-configuration.h"
#include "java-notebook.h"

static void java_build_class_init                      (JavaBuildClass    *klass);
static void java_build_init                            (JavaBuild         *build);
static void java_build_finalize                        (JavaBuild         *build);

static void compile_action                             (JavaBuild         *build);
static void project_compile_action                     (JavaBuild         *build, 
                                                        GList             *selections);
static void clean_action                               (JavaBuild         *build);
static void project_clean_action                       (JavaBuild         *build, 
                                                        GList             *selections);
static void clean_compile_action                       (JavaBuild         *build);
static void project_clean_compile_action               (JavaBuild         *build, 
                                                        GList             *selections);
static void test_action                                (JavaBuild         *build);
static void project_test_action                        (JavaBuild         *build, 
                                                        GList             *selections);
static void execute                                    (JavaBuild         *build,
                                                        JavaBuildPane     *build_pane,
                                                        JavaPageType       page_type, 
                                                        GThreadFunc        thread_func);
static void execute_clean                              (JavaBuildPane     *build_pane);
static void execute_compile                            (JavaBuildPane     *build_pane);
static void execute_clean_compile                      (JavaBuildPane     *build_pane);
static void execute_test                               (JavaBuildPane     *build_pane);
static void execute_project_test                       (JavaBuildPane     *build_pane);
static void run_command                                (JavaBuildPane     *build_pane,
                                                        gchar             *command);
static CodeSlayerProject* get_selections_project       (GList             *selections);
static JavaBuildPane* get_build_pane_by_active_editor  (JavaBuild         *build,
                                                        JavaPageType       page_type);
static JavaBuildPane* get_build_pane_by_project        (JavaBuild         *build, 
                                                        CodeSlayerProject *project,
                                                        JavaPageType       page_type);
                          
#define JAVA_BUILD_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_BUILD_TYPE, JavaBuildPrivate))

typedef struct _JavaBuildPrivate JavaBuildPrivate;

struct _JavaBuildPrivate
{
  CodeSlayer         *codeslayer;
  JavaConfigurations *configurations;
  JavaDebugger       *debugger;
  GtkWidget          *menu;
  GtkWidget          *project_properties;
  GtkWidget          *projects_popup;
  GtkWidget          *notebook;
};

G_DEFINE_TYPE (JavaBuild, java_build, G_TYPE_OBJECT)

static void
java_build_class_init (JavaBuildClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) java_build_finalize;
  g_type_class_add_private (klass, sizeof (JavaBuildPrivate));
}

static void
java_build_init (JavaBuild *build) {}

static void
java_build_finalize (JavaBuild *build)
{
  G_OBJECT_CLASS (java_build_parent_class)->finalize (G_OBJECT(build));
}

JavaBuild*
java_build_new (CodeSlayer         *codeslayer,
                JavaConfigurations *configurations,
                GtkWidget          *menu, 
                GtkWidget          *projects_popup,
                GtkWidget          *notebook)
{
  JavaBuildPrivate *priv;
  JavaBuild *build;

  build = JAVA_BUILD (g_object_new (java_build_get_type (), NULL));
  priv = JAVA_BUILD_GET_PRIVATE (build);

  priv->codeslayer = codeslayer;
  priv->configurations = configurations;
  priv->menu = menu;
  priv->projects_popup = projects_popup;
  priv->notebook = notebook;
  
  g_signal_connect_swapped (G_OBJECT (menu), "compile",
                            G_CALLBACK (compile_action), build);

  g_signal_connect_swapped (G_OBJECT (projects_popup), "compile",
                            G_CALLBACK (project_compile_action), build);

  g_signal_connect_swapped (G_OBJECT (menu), "clean",
                            G_CALLBACK (clean_action), build);

  g_signal_connect_swapped (G_OBJECT (projects_popup), "clean",
                            G_CALLBACK (project_clean_action), build);

  g_signal_connect_swapped (G_OBJECT (menu), "clean-compile",
                            G_CALLBACK (clean_compile_action), build);

  g_signal_connect_swapped (G_OBJECT (projects_popup), "clean-compile",
                            G_CALLBACK (project_clean_compile_action), build);

  g_signal_connect_swapped (G_OBJECT (menu), "test-file",
                            G_CALLBACK (test_action), build);

  g_signal_connect_swapped (G_OBJECT (projects_popup), "test-project",
                            G_CALLBACK (project_test_action), build);

  return build;
}

static void
compile_action (JavaBuild *build)
{
  JavaBuildPane *build_pane;  
  build_pane = get_build_pane_by_active_editor (build, JAVA_PAGE_TYPE_COMPILER);
  execute (build, build_pane, JAVA_PAGE_TYPE_COMPILER, (GThreadFunc) execute_compile);
}

static void
project_compile_action (JavaBuild *build, 
                        GList      *selections)
{
  JavaBuildPane *build_pane;
  CodeSlayerProject *project;
  project = get_selections_project (selections);
  build_pane = get_build_pane_by_project (build, project, JAVA_PAGE_TYPE_COMPILER);
  execute (build, build_pane, JAVA_PAGE_TYPE_COMPILER, (GThreadFunc) execute_compile);
}

static void
clean_action (JavaBuild *build)
{
  JavaBuildPane *build_pane;  
  build_pane = get_build_pane_by_active_editor (build, JAVA_PAGE_TYPE_COMPILER);
  execute (build, build_pane, JAVA_PAGE_TYPE_COMPILER, (GThreadFunc) execute_clean);
}

static void
project_clean_action (JavaBuild *build, 
                      GList      *selections)
{
  JavaBuildPane *build_pane;
  CodeSlayerProject *project;
  project = get_selections_project (selections);
  build_pane = get_build_pane_by_project (build, project, JAVA_PAGE_TYPE_COMPILER);
  execute (build, build_pane, JAVA_PAGE_TYPE_COMPILER, (GThreadFunc) execute_clean);
}

static void
clean_compile_action (JavaBuild *build)
{
  JavaBuildPane *build_pane;  
  build_pane = get_build_pane_by_active_editor (build, JAVA_PAGE_TYPE_COMPILER);
  execute (build, build_pane, JAVA_PAGE_TYPE_COMPILER, (GThreadFunc) execute_clean_compile);
}

static void
project_clean_compile_action (JavaBuild *build, 
                              GList      *selections)
{
  JavaBuildPane *build_pane;
  CodeSlayerProject *project;
  project = get_selections_project (selections);
  build_pane = get_build_pane_by_project (build, project, JAVA_PAGE_TYPE_COMPILER);
  execute (build, build_pane, JAVA_PAGE_TYPE_COMPILER, (GThreadFunc) execute_clean_compile);
}

static void
test_action (JavaBuild *build)
{
  JavaBuildPrivate *priv;
  JavaBuildPane *build_pane;  
  CodeSlayerDocument *document;
  priv = JAVA_BUILD_GET_PRIVATE (build);
  build_pane = get_build_pane_by_active_editor (build, JAVA_PAGE_TYPE_TESTER);
  document = codeslayer_get_active_editor_document (priv->codeslayer);
  java_page_set_document (JAVA_PAGE (build_pane), document); 
  execute (build, build_pane, JAVA_PAGE_TYPE_TESTER, (GThreadFunc) execute_test);
}

static void
project_test_action (JavaBuild *build, 
                     GList     *selections)
{
  JavaBuildPane *build_pane;
  CodeSlayerProject *project;
  project = get_selections_project (selections);
  build_pane = get_build_pane_by_project (build, project, JAVA_PAGE_TYPE_TESTER);
  execute (build, build_pane, JAVA_PAGE_TYPE_TESTER, (GThreadFunc) execute_project_test);
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
execute (JavaBuild     *build,
         JavaBuildPane *build_pane,
         JavaPageType   page_type, 
         GThreadFunc    thread_func)
{
  JavaBuildPrivate *priv;
  priv = JAVA_BUILD_GET_PRIVATE (build);

  if (build_pane)
    {
      codeslayer_show_bottom_pane (priv->codeslayer, priv->notebook);
      java_notebook_select_page_by_type (JAVA_NOTEBOOK (priv->notebook), page_type);
      g_thread_create (thread_func, build_pane, FALSE, NULL);
    }
}

static void
execute_compile (JavaBuildPane *build_pane)
{
  JavaConfiguration *configuration;
  const gchar *ant_file;
  gchar *command;

  java_build_pane_start_process (build_pane, "Compile...");  
  
  configuration = java_page_get_configuration (JAVA_PAGE (build_pane));
  ant_file = java_configuration_get_ant_file (configuration);
  
  command = g_strconcat ("ant -f ", ant_file, " compile 2>&1", NULL);
  run_command (build_pane, command);
  g_free (command);
  
  java_build_pane_stop_process (build_pane);
}

static void
execute_clean (JavaBuildPane *build_pane)
{
  JavaConfiguration *configuration;
  const gchar *ant_file;
  gchar *command;
  
  java_build_pane_start_process (build_pane, "Clean...");  
  
  configuration = java_page_get_configuration (JAVA_PAGE (build_pane));
  ant_file = java_configuration_get_ant_file (configuration);
  
  command = g_strconcat ("ant -f ", ant_file, " clean 2>&1", NULL);
  run_command (build_pane, command);
  g_free (command);
  
  java_build_pane_stop_process (build_pane);
}

static void
execute_clean_compile (JavaBuildPane *build_pane)
{
  execute_clean (build_pane);
  execute_compile (build_pane);
}

static void
execute_test (JavaBuildPane *build_pane)
{
  JavaConfiguration *configuration;
  CodeSlayerDocument *document;
  const gchar *ant_file;
  const gchar *folder_path;
  const gchar *file_path;
  gchar *command;
  gchar *substr;
  gchar *replace;
  
  java_build_pane_start_process (build_pane, "Test...");  
  
  configuration = java_page_get_configuration (JAVA_PAGE (build_pane));
  document = java_page_get_document (JAVA_PAGE (build_pane));
  ant_file = java_configuration_get_ant_file (configuration);
  
  folder_path = java_configuration_get_test_folder (configuration);
  file_path = codeslayer_document_get_file_path (document);
  
  substr = codeslayer_utils_substr (file_path, strlen(folder_path) + 1, strlen(file_path) - 6);  
  replace = codeslayer_utils_strreplace (substr, G_DIR_SEPARATOR_S, ".");
  
  command = g_strconcat ("ant -f ", ant_file, " testfile -Dtestfile=", replace, " 2>&1", NULL);
  
  run_command (build_pane, command);

  g_free (command);
  g_free (substr);
  g_free (replace);
  
  java_build_pane_stop_process (build_pane);
}

static void
execute_project_test (JavaBuildPane *build_pane)
{
  JavaConfiguration *configuration;
  const gchar *ant_file;
  gchar *command;
  
  java_build_pane_start_process (build_pane, "Test Project...");  
  
  configuration = java_page_get_configuration (JAVA_PAGE (build_pane));
  ant_file = java_configuration_get_ant_file (configuration);
  
  command = g_strconcat ("ant -f ", ant_file, " testproject 2>&1", NULL);
  run_command (build_pane, command);
  g_free (command);
  
  java_build_pane_stop_process (build_pane);
}

static JavaBuildPane*
get_build_pane_by_active_editor (JavaBuild   *build, 
                                 JavaPageType page_type)
{
  JavaBuildPrivate *priv;
  CodeSlayer *codeslayer;
  CodeSlayerProject *project;
  
  priv = JAVA_BUILD_GET_PRIVATE (build);
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

  return get_build_pane_by_project (build, project, page_type);
}

static JavaBuildPane*
get_build_pane_by_project (JavaBuild         *build,
                           CodeSlayerProject *project,
                           JavaPageType       page_type)
{
  JavaBuildPrivate *priv;
  GtkWidget  *notebook;
  const gchar *project_key;
  const gchar *project_name;
  JavaConfiguration *configuration;
  GtkWidget *build_pane;
  
  priv = JAVA_BUILD_GET_PRIVATE (build);
  notebook = priv->notebook;
  
  project_key = codeslayer_project_get_key (project);
  project_name = codeslayer_project_get_name (project);
  configuration = java_configurations_find_configuration (priv->configurations, project_key);
  
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
  
  build_pane = java_notebook_get_page_by_type (JAVA_NOTEBOOK (notebook), page_type);
  
  if (build_pane == NULL)
    {
      gchar *label;
      build_pane = java_build_pane_new (page_type, priv->codeslayer);
      if (page_type == JAVA_PAGE_TYPE_COMPILER)
        label = "Compiler";
      else if (page_type == JAVA_PAGE_TYPE_TESTER)
        label = "Tester";
        
      java_notebook_add_page (JAVA_NOTEBOOK (priv->notebook), build_pane, label);
    }
    
  java_page_set_configuration (JAVA_PAGE (build_pane), configuration);

  return JAVA_BUILD_PANE (build_pane);
}

static void
run_command (JavaBuildPane *build_pane,
             gchar         *command)
{
  char out[BUFSIZ];
  FILE *file;
  
  gdk_threads_enter ();
  java_build_pane_clear_text (build_pane);
  gdk_threads_leave ();
  
  file = popen (command, "r");
  if (file != NULL)
    {
      while (fgets (out, BUFSIZ, file))
        {
          gdk_threads_enter ();
          java_build_pane_append_text (build_pane, out);
          gdk_threads_leave ();
        }
      pclose (file);
    }
    
  java_build_pane_create_links (build_pane);
}
