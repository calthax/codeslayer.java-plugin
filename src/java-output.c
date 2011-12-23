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
#include "java-output.h"
#include "java-output-pane.h"
#include "java-page.h"
#include "java-configuration.h"
#include "java-notebook.h"

static void java_output_class_init                       (JavaOutputClass   *klass);
static void java_output_init                             (JavaOutput        *output);
static void java_output_finalize                         (JavaOutput        *output);

static void compile_action                               (JavaOutput        *output);
static void project_compile_action                       (JavaOutput        *output, 
                                                          GList             *selections);
static void clean_action                                 (JavaOutput        *output);
static void project_clean_action                         (JavaOutput        *output, 
                                                          GList             *selections);
static void test_action                                  (JavaOutput        *output);
static void project_test_action                          (JavaOutput        *output, 
                                                          GList             *selections);
static void execute                                      (JavaOutput        *output,
                                                          JavaOutputPane    *output_pane,
                                                          JavaPageType       page_type, 
                                                          GThreadFunc        thread_func);
static void execute_clean                                (JavaOutputPane    *output_pane);
static void execute_compile                              (JavaOutputPane    *output_pane);
static void execute_test                                 (JavaOutputPane    *output_pane);
static void execute_project_test                         (JavaOutputPane    *output_pane);
static void run_command                                  (JavaOutputPane    *output_pane,
                                                          gchar             *command);
static CodeSlayerProject* get_selections_project         (GList             *selections);
static JavaOutputPane* get_output_pane_by_active_editor  (JavaOutput        *output,
                                                          JavaPageType       page_type);
static JavaOutputPane* get_output_pane_by_project        (JavaOutput        *output, 
                                                          CodeSlayerProject *project,
                                                          JavaPageType       page_type);
                          
#define JAVA_OUTPUT_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_OUTPUT_TYPE, JavaOutputPrivate))

typedef struct _JavaOutputPrivate JavaOutputPrivate;

struct _JavaOutputPrivate
{
  CodeSlayer         *codeslayer;
  JavaConfigurations *configurations;
  JavaDebugger       *debugger;
  GtkWidget          *menu;
  GtkWidget          *project_properties;
  GtkWidget          *projects_popup;
  GtkWidget          *notebook;
  gulong              properties_opened_id;
  gulong              properties_saved_id;
};

G_DEFINE_TYPE (JavaOutput, java_output, G_TYPE_OBJECT)

static void
java_output_class_init (JavaOutputClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) java_output_finalize;
  g_type_class_add_private (klass, sizeof (JavaOutputPrivate));
}

static void
java_output_init (JavaOutput *output) {}

static void
java_output_finalize (JavaOutput *output)
{
  JavaOutputPrivate *priv;
  priv = JAVA_OUTPUT_GET_PRIVATE (output);
  g_signal_handler_disconnect (priv->codeslayer, priv->properties_opened_id);
  g_signal_handler_disconnect (priv->codeslayer, priv->properties_saved_id);
  G_OBJECT_CLASS (java_output_parent_class)->finalize (G_OBJECT(output));
}

JavaOutput*
java_output_new (CodeSlayer         *codeslayer,
                 JavaConfigurations *configurations,
                 GtkWidget          *menu, 
                 GtkWidget          *projects_popup,
                 GtkWidget          *notebook)
{
  JavaOutputPrivate *priv;
  JavaOutput *output;

  output = JAVA_OUTPUT (g_object_new (java_output_get_type (), NULL));
  priv = JAVA_OUTPUT_GET_PRIVATE (output);

  priv->codeslayer = codeslayer;
  priv->configurations = configurations;
  priv->menu = menu;
  priv->projects_popup = projects_popup;
  priv->notebook = notebook;
  
  g_signal_connect_swapped (G_OBJECT (menu), "compile",
                            G_CALLBACK (compile_action), output);

  g_signal_connect_swapped (G_OBJECT (projects_popup), "compile",
                            G_CALLBACK (project_compile_action), output);

  g_signal_connect_swapped (G_OBJECT (menu), "clean",
                            G_CALLBACK (clean_action), output);

  g_signal_connect_swapped (G_OBJECT (projects_popup), "clean",
                            G_CALLBACK (project_clean_action), output);

  g_signal_connect_swapped (G_OBJECT (menu), "test-file",
                            G_CALLBACK (test_action), output);

  g_signal_connect_swapped (G_OBJECT (projects_popup), "test-project",
                            G_CALLBACK (project_test_action), output);

  return output;
}

static void
compile_action (JavaOutput *output)
{
  JavaOutputPane *output_pane;  
  output_pane = get_output_pane_by_active_editor (output, JAVA_PAGE_TYPE_COMPILER);
  execute (output, output_pane, JAVA_PAGE_TYPE_COMPILER, (GThreadFunc) execute_compile);
}

static void
project_compile_action (JavaOutput *output, 
                        GList      *selections)
{
  JavaOutputPane *output_pane;
  CodeSlayerProject *project;
  project = get_selections_project (selections);
  output_pane = get_output_pane_by_project (output, project, JAVA_PAGE_TYPE_COMPILER);
  execute (output, output_pane, JAVA_PAGE_TYPE_COMPILER, (GThreadFunc) execute_compile);
}

static void
clean_action (JavaOutput *output)
{
  JavaOutputPane *output_pane;  
  output_pane = get_output_pane_by_active_editor (output, JAVA_PAGE_TYPE_COMPILER);
  execute (output, output_pane, JAVA_PAGE_TYPE_COMPILER, (GThreadFunc) execute_clean);
}

static void
project_clean_action (JavaOutput *output, 
                      GList      *selections)
{
  JavaOutputPane *output_pane;
  CodeSlayerProject *project;
  project = get_selections_project (selections);
  output_pane = get_output_pane_by_project (output, project, JAVA_PAGE_TYPE_COMPILER);
  execute (output, output_pane, JAVA_PAGE_TYPE_COMPILER, (GThreadFunc) execute_clean);
}

static void
test_action (JavaOutput *output)
{
  JavaOutputPrivate *priv;
  JavaOutputPane *output_pane;  
  CodeSlayerDocument *document;
  priv = JAVA_OUTPUT_GET_PRIVATE (output);
  output_pane = get_output_pane_by_active_editor (output, JAVA_PAGE_TYPE_TESTER);
  document = codeslayer_get_active_editor_document (priv->codeslayer);
  java_page_set_document (JAVA_PAGE (output_pane), document); 
  execute (output, output_pane, JAVA_PAGE_TYPE_TESTER, (GThreadFunc) execute_test);
}

static void
project_test_action (JavaOutput *output, 
                     GList      *selections)
{
  JavaOutputPane *output_pane;
  CodeSlayerProject *project;
  project = get_selections_project (selections);
  output_pane = get_output_pane_by_project (output, project, JAVA_PAGE_TYPE_TESTER);
  execute (output, output_pane, JAVA_PAGE_TYPE_TESTER, (GThreadFunc) execute_project_test);
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
execute (JavaOutput     *output,
         JavaOutputPane *output_pane,
         JavaPageType    page_type, 
         GThreadFunc     thread_func)
{
  JavaOutputPrivate *priv;
  priv = JAVA_OUTPUT_GET_PRIVATE (output);

  if (output_pane)
    {
      codeslayer_show_bottom_pane (priv->codeslayer, priv->notebook);
      java_notebook_select_page_by_type (JAVA_NOTEBOOK (priv->notebook), page_type);
      g_thread_create (thread_func, output_pane, FALSE, NULL);
    }
}

static void
execute_compile (JavaOutputPane *output_pane)
{
  JavaConfiguration *configuration;
  const gchar *ant_file;
  gchar *command;
  
  configuration = java_page_get_configuration (JAVA_PAGE (output_pane));
  ant_file = java_configuration_get_ant_file (configuration);
  
  command = g_strconcat ("ant -f ", ant_file, " compile 2>&1", NULL);
  run_command (output_pane, command);
  g_free (command);
}

static void
execute_clean (JavaOutputPane *output_pane)
{
  JavaConfiguration *configuration;
  const gchar *ant_file;
  gchar *command;
  
  configuration = java_page_get_configuration (JAVA_PAGE (output_pane));
  ant_file = java_configuration_get_ant_file (configuration);
  
  command = g_strconcat ("ant -f ", ant_file, " clean 2>&1", NULL);
  run_command (output_pane, command);
  g_free (command);
}

static void
execute_test (JavaOutputPane *output_pane)
{
  JavaConfiguration *configuration;
  CodeSlayerDocument *document;
  const gchar *ant_file;
  const gchar *folder_path;
  const gchar *file_path;
  gchar *command;
  gchar *substr;
  gchar *replace;
  
  configuration = java_page_get_configuration (JAVA_PAGE (output_pane));
  document = java_page_get_document (JAVA_PAGE (output_pane));
  ant_file = java_configuration_get_ant_file (configuration);
  
  folder_path = java_configuration_get_test_folder (configuration);
  file_path = codeslayer_document_get_file_path (document);
  
  substr = codeslayer_utils_substr (file_path, strlen(folder_path) + 1, strlen(file_path) - 6);  
  replace = codeslayer_utils_strreplace (substr, G_DIR_SEPARATOR_S, ".");
  
  command = g_strconcat ("ant -f ", ant_file, " testfile -Dtestfile=", replace, " 2>&1", NULL);
  
  run_command (output_pane, command);

  g_free (command);
  g_free (substr);
  g_free (replace);
}

static void
execute_project_test (JavaOutputPane *output_pane)
{
  JavaConfiguration *configuration;
  const gchar *ant_file;
  gchar *command;
  
  configuration = java_page_get_configuration (JAVA_PAGE (output_pane));
  ant_file = java_configuration_get_ant_file (configuration);
  
  command = g_strconcat ("ant -f ", ant_file, " testproject 2>&1", NULL);
  run_command (output_pane, command);
  g_free (command);
}

static JavaOutputPane*
get_output_pane_by_active_editor (JavaOutput   *output, 
                                  JavaPageType  page_type)
{
  JavaOutputPrivate *priv;
  CodeSlayer *codeslayer;
  CodeSlayerProject *project;
  
  priv = JAVA_OUTPUT_GET_PRIVATE (output);
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

  return get_output_pane_by_project (output, project, page_type);
}

static JavaOutputPane*
get_output_pane_by_project (JavaOutput        *output,
                            CodeSlayerProject *project,
                            JavaPageType       page_type)
{
  JavaOutputPrivate *priv;
  GtkWidget  *notebook;
  const gchar *project_key;
  const gchar *project_name;
  JavaConfiguration *configuration;
  GtkWidget *output_pane;
  
  priv = JAVA_OUTPUT_GET_PRIVATE (output);
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
  
  output_pane = java_notebook_get_page_by_type (JAVA_NOTEBOOK (notebook), page_type);
  
  if (output_pane == NULL)
    {
      gchar *label;
      output_pane = java_output_pane_new (page_type);
      if (page_type == JAVA_PAGE_TYPE_COMPILER)
        label = "Compiler";
      else if (page_type == JAVA_PAGE_TYPE_TESTER)
        label = "Tester";
        
      java_notebook_add_page (JAVA_NOTEBOOK (priv->notebook), output_pane, label);
    }
    
  java_page_set_configuration (JAVA_PAGE (output_pane), configuration);

  return JAVA_OUTPUT_PANE (output_pane);
}

static void
run_command (JavaOutputPane *output_pane,
             gchar          *command)
{
  GtkTextBuffer *buffer;
  GtkTextIter iter;
  GtkTextMark *text_mark;
  char out[BUFSIZ];
  FILE *file;
  
  gdk_threads_enter ();
  buffer = gtk_text_view_get_buffer (java_output_pane_get_text_view (output_pane));
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
          gtk_text_view_scroll_to_mark (java_output_pane_get_text_view (output_pane), 
                                        text_mark, 0.0, FALSE, 0, 0);
          gdk_threads_leave ();
        }
      pclose (file);
    }
}
