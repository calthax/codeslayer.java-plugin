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
#include "java-debugger.h"
#include "java-debugger-column.h"
#include "java-debugger-pane.h"
#include "java-debugger-service.h"
#include "java-debugger-breakpoints.h"
#include "java-notebook.h"
#include "java-configuration.h"
#include "java-utils.h"

static void java_debugger_class_init  (JavaDebuggerClass      *klass);
static void java_debugger_init        (JavaDebugger           *debugger);
static void java_debugger_finalize    (JavaDebugger           *debugger);

static void editor_added_action       (JavaDebugger           *debugger,
                                       CodeSlayerEditor       *editor);                                    
static void editor_removed_action     (JavaDebugger           *debugger,
                                       CodeSlayerEditor       *editor);                                    
static void line_activated_action     (GtkSourceView          *sourceview,
                                       GtkTextIter            *iter,
                                       GdkEvent               *event,
                                       JavaDebugger           *debugger);                                       
static void debug_test_file_action    (JavaDebugger           *debugger);
static void query_action              (JavaDebugger           *debugger, 
                                       gchar                  *text);
static void cont_action               (JavaDebugger           *debugger);
static void quit_action               (JavaDebugger           *debugger);
static void step_over_action          (JavaDebugger           *debugger);
static void step_into_action          (JavaDebugger           *debugger);
static void step_out_action           (JavaDebugger           *debugger);

static void add_breakpoint            (JavaDebugger           *debugger,
                                       JavaDebuggerBreakpoint *breakpoint);
static void read_channel_action       (JavaDebugger           *debugger, 
                                       gchar                  *text);
static void select_editor             (CodeSlayer             *codeslayer, 
                                       CodeSlayerDocument     *document);
                                       
static void initialize_editors        (JavaDebugger           *debugger);
static void uninitialize_editors      (JavaDebugger           *debugger);
static GList* get_debugger_rows       (GList                  *columns);                                       
                                       
#define BREAKPOINT "breakpoint"
#define LINE_ACTIVATED "LINE_ACTIVATED"

#define JAVA_DEBUGGER_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_DEBUGGER_TYPE, JavaDebuggerPrivate))

typedef struct _JavaDebuggerPrivate JavaDebuggerPrivate;

struct _JavaDebuggerPrivate
{
  CodeSlayer              *codeslayer;
  GtkWidget               *debugger_pane;
  JavaConfigurations      *configurations;
  GtkWidget               *menu;
  JavaDebuggerService     *service;
  JavaDebuggerBreakpoints *breakpoints;
  GtkSourceMarkAttributes *attributes;
  GtkWidget               *notebook;
  gulong                   editor_added_id;
  gulong                   editor_removed_id;
};

enum
{
  PROP_0,
  PROP_CLASS_NAME,
  PROP_LINE_NUMBER
};

G_DEFINE_TYPE (JavaDebugger, java_debugger, G_TYPE_OBJECT)
     
static void 
java_debugger_class_init (JavaDebuggerClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) java_debugger_finalize;
  g_type_class_add_private (klass, sizeof (JavaDebuggerPrivate));
}

static void
java_debugger_init (JavaDebugger *debugger)
{
  JavaDebuggerPrivate *priv;
  priv = JAVA_DEBUGGER_GET_PRIVATE (debugger);
  priv->breakpoints = NULL;
}

static void
java_debugger_finalize (JavaDebugger *debugger)
{
  JavaDebuggerPrivate *priv;
  priv = JAVA_DEBUGGER_GET_PRIVATE (debugger);  
  uninitialize_editors (debugger);
  g_signal_handler_disconnect (priv->codeslayer, priv->editor_added_id);
  g_signal_handler_disconnect (priv->codeslayer, priv->editor_removed_id);
  g_object_unref (priv->breakpoints);
  g_object_unref (priv->service);
  g_object_unref (priv->attributes);
  G_OBJECT_CLASS (java_debugger_parent_class)->finalize (G_OBJECT (debugger));
}

JavaDebugger*
java_debugger_new (CodeSlayer         *codeslayer,
                   JavaConfigurations *configurations, 
                   GtkWidget          *menu,
                   GtkWidget          *notebook)
{
  JavaDebuggerPrivate *priv;
  JavaDebugger *debugger;

  debugger = JAVA_DEBUGGER (g_object_new (java_debugger_get_type (), NULL));
  priv = JAVA_DEBUGGER_GET_PRIVATE (debugger);
  priv->codeslayer = codeslayer;
  priv->configurations = configurations;
  priv->menu = menu;
  priv->notebook = notebook;
  
  priv->service = java_debugger_service_new ();
  priv->breakpoints = java_debugger_breakpoints_new ();
  
  priv->attributes = gtk_source_mark_attributes_new ();
  gtk_source_mark_attributes_set_stock_id (priv->attributes, GTK_STOCK_MEDIA_RECORD);
  
  priv->debugger_pane = java_debugger_pane_new ();
  java_notebook_add_page (JAVA_NOTEBOOK (priv->notebook), priv->debugger_pane, "Debugger");
  
  priv->editor_added_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "editor-added",
                                                    G_CALLBACK (editor_added_action), debugger);

  priv->editor_removed_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "editor-removed",
                                                      G_CALLBACK (editor_removed_action), debugger);

  g_signal_connect_swapped (G_OBJECT (menu), "debug-test-file",
                            G_CALLBACK (debug_test_file_action), debugger);

  g_signal_connect_swapped (G_OBJECT (priv->debugger_pane), "query",
                            G_CALLBACK (query_action), debugger);

  g_signal_connect_swapped (G_OBJECT (priv->debugger_pane), "cont",
                            G_CALLBACK (cont_action), debugger);

  g_signal_connect_swapped (G_OBJECT (priv->debugger_pane), "quit",
                            G_CALLBACK (quit_action), debugger);

  g_signal_connect_swapped (G_OBJECT (priv->debugger_pane), "step-over",
                            G_CALLBACK (step_over_action), debugger);

  g_signal_connect_swapped (G_OBJECT (priv->debugger_pane), "step-into",
                            G_CALLBACK (step_into_action), debugger);

  g_signal_connect_swapped (G_OBJECT (priv->debugger_pane), "step-out",
                            G_CALLBACK (step_out_action), debugger);

  g_signal_connect_swapped (G_OBJECT (priv->service), "read-channel",
                            G_CALLBACK (read_channel_action), debugger);
                            
  initialize_editors (debugger);

  return debugger;
}

static void
initialize_editors (JavaDebugger *debugger)
{
  JavaDebuggerPrivate *priv;
  GList *editors;
  GList *list;
  
  priv = JAVA_DEBUGGER_GET_PRIVATE (debugger);
  
  editors = codeslayer_get_all_editors (priv->codeslayer);
  list = editors;
  
  while (list != NULL)
    {
      CodeSlayerEditor *editor = list->data;
      gchar *line_activated_id = NULL;
      line_activated_id = (gchar*)g_object_get_data (G_OBJECT (editor), LINE_ACTIVATED);
      if (line_activated_id == NULL)
        editor_added_action (debugger, editor);
      list = g_list_next (list);
    }
    
  if (editors != NULL)
    g_list_free (editors);    
}

static void
uninitialize_editors (JavaDebugger *debugger)
{
  JavaDebuggerPrivate *priv;
  GList *editors;
  GList *list;

  priv = JAVA_DEBUGGER_GET_PRIVATE (debugger);

  editors = codeslayer_get_all_editors (priv->codeslayer);
  list = editors;

  while (list != NULL)
    {
      CodeSlayerEditor *editor = list->data;
      editor_removed_action (debugger, editor);
      list = g_list_next (list);
    }
  
  if (editors != NULL)
    g_list_free (editors);    
}

static void
editor_added_action (JavaDebugger     *debugger,
                     CodeSlayerEditor *editor)
{
  JavaDebuggerPrivate *priv;
  gulong line_activated_id;
  
  priv = JAVA_DEBUGGER_GET_PRIVATE (debugger);

  gtk_source_view_set_mark_attributes (GTK_SOURCE_VIEW (editor), BREAKPOINT, 
                                       priv->attributes, 1);
  gtk_source_view_set_show_line_marks (GTK_SOURCE_VIEW (editor), TRUE);
  
  line_activated_id = g_signal_connect (G_OBJECT (editor), "line-mark-activated",
                                        G_CALLBACK (line_activated_action), debugger);
                                        
  g_object_set_data (G_OBJECT (editor), LINE_ACTIVATED, 
                     g_strdup_printf ("%lu", line_activated_id));
}

static void
editor_removed_action (JavaDebugger     *debugger,
                       CodeSlayerEditor *editor)
{
  gchar *line_activated_id = NULL;
  line_activated_id = (gchar*)g_object_get_data (G_OBJECT (editor), LINE_ACTIVATED);
  if (line_activated_id != NULL)
    {        
      g_signal_handler_disconnect (editor, atol (line_activated_id));
      g_free (line_activated_id);
      g_object_set_data (G_OBJECT (editor), LINE_ACTIVATED, NULL);
      gtk_source_view_set_show_line_marks (GTK_SOURCE_VIEW (editor), FALSE);
    }
}

static void
line_activated_action (GtkSourceView *view,
                       GtkTextIter   *iter,
                       GdkEvent      *event,
                       JavaDebugger  *debugger)
{
  JavaDebuggerPrivate *priv;
	GtkSourceBuffer *buffer;
	CodeSlayerProject *project;
	CodeSlayerDocument *document;
	JavaConfiguration *configuration;
	const gchar *project_key;
	gchar *class_name;
	gint line_number;
	GSList *marks;

  priv = JAVA_DEBUGGER_GET_PRIVATE (debugger);

	buffer = GTK_SOURCE_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (view)));

  line_number = gtk_text_iter_get_line (iter);

	marks = gtk_source_buffer_get_source_marks_at_line (buffer, line_number, BREAKPOINT);
	
	project = codeslayer_get_active_editor_project (priv->codeslayer);
	document = codeslayer_get_active_editor_document (priv->codeslayer);
	project_key = codeslayer_project_get_key (project);
	configuration = java_configurations_find_configuration (priv->configurations, 
	                                                        project_key);

  class_name = java_utils_get_class_name (configuration, document);
  
	if (marks != NULL)
	  {
      JavaDebuggerBreakpoint *breakpoint;	    
      breakpoint = java_debugger_breakpoints_find_breakpoint (priv->breakpoints, 
                                                              class_name, 
                                                              line_number + 1);
      if (breakpoint)
        java_debugger_breakpoints_remove_breakpoint (priv->breakpoints, breakpoint);

		  gtk_text_buffer_delete_mark (GTK_TEXT_BUFFER (buffer), 
		                               GTK_TEXT_MARK (marks->data));

      g_print ("line removed %s:%d\n", class_name, line_number + 1);
  	}
	else
	  {
      JavaDebuggerBreakpoint *breakpoint;	    
      breakpoint = java_debugger_breakpoint_new ();            
      java_debugger_breakpoint_set_class_name (breakpoint, class_name);
      java_debugger_breakpoint_set_line_number (breakpoint, line_number + 1);
      
      java_debugger_breakpoints_add_breakpoint (priv->breakpoints, breakpoint);
	  
	  	gtk_source_buffer_create_source_mark (buffer, NULL, BREAKPOINT, iter);
	  		  	
      g_print ("line added %s:%d\n", class_name, line_number + 1);
      
      if (java_debugger_service_get_running (priv->service))
        add_breakpoint (debugger, breakpoint);
  	}

  g_free (class_name);
	g_slist_free (marks);
}

static void
debug_test_file_action (JavaDebugger *debugger)
{
  JavaDebuggerPrivate *priv;
  priv = JAVA_DEBUGGER_GET_PRIVATE (debugger);

  if (java_debugger_service_get_running (priv->service))
    java_debugger_service_send_command (priv->service, "q\n");
    
  java_debugger_service_start (priv->service);
}

static void
query_action (JavaDebugger *debugger, 
              gchar        *text)
{
  JavaDebuggerPrivate *priv;
  gchar *query;
  
  priv = JAVA_DEBUGGER_GET_PRIVATE (debugger);

  if (!java_debugger_service_get_running (priv->service))
    return;

  query = g_strconcat (text, "\n", NULL);  
  java_debugger_service_send_command (priv->service, query);
  g_free (query);  
}

static void
cont_action (JavaDebugger *debugger)
{
  JavaDebuggerPrivate *priv;
  priv = JAVA_DEBUGGER_GET_PRIVATE (debugger);

  if (java_debugger_service_get_running (priv->service))
    java_debugger_service_send_command (priv->service, "c\n");
}

static void
quit_action (JavaDebugger *debugger)
{
  JavaDebuggerPrivate *priv;
  priv = JAVA_DEBUGGER_GET_PRIVATE (debugger);

  if (java_debugger_service_get_running (priv->service))
    java_debugger_service_send_command (priv->service, "q\n");
}

static void
step_over_action (JavaDebugger *debugger)
{
  JavaDebuggerPrivate *priv;
  priv = JAVA_DEBUGGER_GET_PRIVATE (debugger);

  if (java_debugger_service_get_running (priv->service))
    java_debugger_service_send_command (priv->service, "n\n");
}

static void
step_into_action (JavaDebugger *debugger)
{
  JavaDebuggerPrivate *priv;
  priv = JAVA_DEBUGGER_GET_PRIVATE (debugger);

  if (!java_debugger_service_get_running (priv->service))
    return;

  java_debugger_service_send_command (priv->service, "s\n");
}

static void
step_out_action (JavaDebugger *debugger)
{
  JavaDebuggerPrivate *priv;
  priv = JAVA_DEBUGGER_GET_PRIVATE (debugger);

  if (!java_debugger_service_get_running (priv->service))
    return;

  java_debugger_service_send_command (priv->service, "f\n");
}

static void
add_breakpoint (JavaDebugger           *debugger,
                JavaDebuggerBreakpoint *breakpoint)
{
  JavaDebuggerPrivate *priv;
  gchar *command;
  const gchar *class_name = NULL;
  gint line_number = 0;
  
  priv = JAVA_DEBUGGER_GET_PRIVATE (debugger);

  class_name = java_debugger_breakpoint_get_class_name (breakpoint);
  line_number = java_debugger_breakpoint_get_line_number (breakpoint);
  command = g_strdup_printf ("break %s:%d\n", class_name, line_number);
  
  java_debugger_service_send_command (priv->service, command);
  
  g_free (command);
}

static void
read_channel_action (JavaDebugger *debugger, 
                     gchar        *contents)
{
  JavaDebuggerPrivate *priv;
  priv = JAVA_DEBUGGER_GET_PRIVATE (debugger);

  g_print ("contents %s\n", contents);

  if (g_str_has_prefix (contents, "<ready/>"))
    {
      GList *list;
      list = java_debugger_breakpoints_get_list (priv->breakpoints);  
      while (list != NULL)
        {
          JavaDebuggerBreakpoint *breakpoint = list->data;
          add_breakpoint (debugger, breakpoint);
          list = g_list_next (list);
        }
        
      java_debugger_service_send_command (priv->service, "c\n");
    }
  else if (g_str_has_prefix (contents, "<hit-breakpoint"))
    {
      GList *gobjects;
      GList *list;
      gobjects = codeslayer_utils_deserialize_gobjects (CODESLAYER_DOCUMENT_TYPE,
                                                        FALSE,
                                                        contents, 
                                                        "hit-breakpoint",
                                                        "file_path", G_TYPE_STRING, 
                                                        "line_number", G_TYPE_INT, 
                                                        NULL);
      list = gobjects;
      if (list != NULL)
        {
          CodeSlayerDocument *document = list->data;
          select_editor (priv->codeslayer, document);
          g_object_unref (document);
        }

      if (gobjects != NULL)
        g_list_free (gobjects);
    }
  else if (g_str_has_prefix (contents, "<step"))
    {
      GList *gobjects;
      GList *list;
      gobjects = codeslayer_utils_deserialize_gobjects (CODESLAYER_DOCUMENT_TYPE,
                                                        FALSE,
                                                        contents, 
                                                        "step",
                                                        "file_path", G_TYPE_STRING, 
                                                        "line_number", G_TYPE_INT, 
                                                        NULL);
      list = gobjects;
      if (list != NULL)
        {
          CodeSlayerDocument *document = list->data;
          select_editor (priv->codeslayer, document);
          g_object_unref (document);
        }

      if (gobjects != NULL)
        g_list_free (gobjects);
    }
  else if (g_str_has_prefix (contents, "<print-table"))
    {
      GList *gobjects;
      GList *rows;
      gobjects = codeslayer_utils_deserialize_gobjects (JAVA_DEBUGGER_COLUMN_TYPE,
                                                        FALSE,
                                                        contents, 
                                                        "print-column",
                                                        "name", G_TYPE_STRING, 
                                                        "value", G_TYPE_STRING, 
                                                        NULL);
      rows = get_debugger_rows (gobjects);
      java_debugger_pane_refresh_table (JAVA_DEBUGGER_PANE (priv->debugger_pane), rows);
    
      if (gobjects != NULL)
        g_list_free (gobjects);
    }
}

static GList*
get_debugger_rows (GList *columns)
{
  GList *rows = NULL;
  GList *row = NULL;
  const gchar *first_col = NULL; 
  
  while (columns != NULL)
    {
      JavaDebuggerColumn *column = columns->data;
      const gchar *name = java_debugger_column_get_name (column);
      if (first_col == NULL)
        first_col = name;
      
      if (g_strcmp0 (first_col, name) == 0)
        {
          row = NULL;
          row = g_list_append (row, column);
          rows = g_list_append (rows, row);
        }
      else
        {
          row = g_list_append (row, column);
        }
      
      columns = g_list_next (columns);
    }
  
  return rows;
}

static void
select_editor (CodeSlayer         *codeslayer, 
               CodeSlayerDocument *document)
{
  CodeSlayerProject *project;
  const gchar *file_path;

  file_path = codeslayer_document_get_file_path (document);
  project = codeslayer_get_project_by_file_path (codeslayer, file_path);
  codeslayer_document_set_project (document, project);

  gdk_threads_enter ();
  codeslayer_select_editor (codeslayer, document);
  gdk_threads_leave ();
}
