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
#include <stdlib.h>
#include "java-usage.h"
#include "java-usage-method.h"
#include "java-usage-pane.h"
#include "java-utils.h"
#include "java-page.h"
#include "java-client.h"

typedef struct
{
  JavaUsage *usage;
  gchar     *text;
} Output;

static void java_usage_class_init              (JavaUsageClass  *klass);
static void java_usage_init                    (JavaUsage       *usage);
static void java_usage_finalize                (JavaUsage       *usage);

static void method_usage_action                (JavaUsage       *usage);
static gchar* get_input                        (JavaUsage       *usage, 
                                                const gchar     *file_path, 
                                                gchar           *symbol, 
                                                gint             line_number);
static GList* get_usage_methods_from_output    (gchar           *output);
static gint sort_usage_methods                 (JavaUsageMethod *usage_method1, 
                                                JavaUsageMethod *usage_method2);
static JavaUsageMethod* get_java_usage_method  (gchar           *text);

static void add_idle                           (gchar           *text, 
                                                JavaUsage       *usage);
static void destroy_output                     (Output          *output);
static void render_output                      (Output          *output);

#define JAVA_USAGE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_USAGE_TYPE, JavaUsagePrivate))

typedef struct _JavaUsagePrivate JavaUsagePrivate;

struct _JavaUsagePrivate
{
  CodeSlayer *codeslayer;
  GtkWidget *notebook;
  JavaConfigurations *configurations;
  JavaClient *client;
  gint process_id;
};

G_DEFINE_TYPE (JavaUsage, java_usage, G_TYPE_OBJECT)
     
static void 
java_usage_class_init (JavaUsageClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) java_usage_finalize;
  g_type_class_add_private (klass, sizeof (JavaUsagePrivate));
}

static void
java_usage_init (JavaUsage *usage){}

static void
java_usage_finalize (JavaUsage *usage)
{
  JavaUsagePrivate *priv;
  priv = JAVA_USAGE_GET_PRIVATE (usage);

  if (priv->client)
    g_object_unref (priv->client);

  G_OBJECT_CLASS (java_usage_parent_class)->finalize (G_OBJECT (usage));
}

JavaUsage*
java_usage_new (CodeSlayer         *codeslayer,
                GtkWidget          *menu,
                GtkWidget          *notebook,
                JavaConfigurations *configurations)
{
  JavaUsagePrivate *priv;
  JavaUsage *usage;

  usage = JAVA_USAGE (g_object_new (java_usage_get_type (), NULL));
  priv = JAVA_USAGE_GET_PRIVATE (usage);
  priv->codeslayer = codeslayer;
  priv->notebook = notebook;
  priv->configurations = configurations;

  priv->client = java_client_new (codeslayer);

  g_signal_connect_swapped (G_OBJECT (menu), "method-usage",
                            G_CALLBACK (method_usage_action), usage);

  return usage;
}

static void 
method_usage_action (JavaUsage *usage)
{  
  JavaUsagePrivate *priv;
  CodeSlayerEditor *editor;
  const gchar *file_path;
  GtkTextBuffer *buffer;

  GtkTextMark *insert_mark;
  GtkTextMark *selection_mark;
  gchar *symbol;
  gint line_number;
  gchar *input;

  GtkTextIter start, end;

  priv = JAVA_USAGE_GET_PRIVATE (usage);
  
  priv->process_id = codeslayer_add_to_processes (priv->codeslayer, "Method Usage", NULL, NULL);

  editor = codeslayer_get_active_editor (priv->codeslayer);
  
  if (editor == NULL)
    return;
    
  file_path = codeslayer_editor_get_file_path (editor);
  if (!g_str_has_suffix (file_path, ".java"))
    return;
  
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));

  if (!gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
    {
      insert_mark = gtk_text_buffer_get_insert (buffer);    
      selection_mark = gtk_text_buffer_get_selection_bound (buffer);

      gtk_text_buffer_get_iter_at_mark (buffer, &start, insert_mark);
      gtk_text_buffer_get_iter_at_mark (buffer, &end, selection_mark);
    }

  symbol = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
  line_number = gtk_text_iter_get_line (&start);
  
  if (symbol != NULL)
    g_strstrip (symbol);
  
  input = get_input (usage, file_path, symbol, line_number);
  
  g_print ("input %s\n", input);
  
  java_client_send_with_callback (priv->client, input, (ClientCallbackFunc) add_idle, usage);
                                              
  g_free (symbol);
  g_free (input);                                         
}

static gchar* 
get_input (JavaUsage   *usage, 
           const gchar *file_path, 
           gchar       *symbol, 
           gint         line_number)
{
  JavaUsagePrivate *priv;

  gchar *source_indexes_folders;
  gchar *line_number_str;
  gchar *result;
  
  priv = JAVA_USAGE_GET_PRIVATE (usage);
  
  line_number_str = g_strdup_printf ("%d", (line_number + 1));
  
  source_indexes_folders = get_source_indexes_folders (priv->codeslayer, priv->configurations);
  
  result = g_strconcat ("-program usage", 
                        " -sourcefile ", file_path,
                        " -symbol ", symbol,
                        " -linenumber ", line_number_str,
                        source_indexes_folders, 
                        NULL);
  
  g_free (source_indexes_folders);
  g_free (line_number_str);

  return result;
}

static void
add_idle (gchar     *text, 
          JavaUsage *usage)
{
  Output *output;
  output = g_malloc (sizeof (Output));
  output->usage = usage;
  output->text = text;    
  g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, (GSourceFunc) render_output, output, (GDestroyNotify)destroy_output);
}

static void 
destroy_output (Output *output)
{
  g_free (output->text);
  g_free (output);
}

static void
render_output (Output *output)
{
  JavaUsagePrivate *priv;
  GList *usage_methods;
  
  priv = JAVA_USAGE_GET_PRIVATE (output->usage);
  
  g_print ("output %s\n", output->text);
  
  usage_methods = get_usage_methods_from_output (output->text);

  if (usage_methods != NULL)
    {
      GtkWidget *usage_pane;
      
      usage_pane = java_notebook_get_page_by_type (JAVA_NOTEBOOK (priv->notebook), 
                                                   JAVA_PAGE_TYPE_USAGE);
      if (usage_pane == NULL)
        {
          usage_pane = java_usage_pane_new (priv->codeslayer, JAVA_PAGE_TYPE_USAGE);
          java_notebook_add_page (JAVA_NOTEBOOK (priv->notebook), usage_pane, "Method Usage");
        }
      
      codeslayer_show_bottom_pane (priv->codeslayer, priv->notebook);
      java_usage_pane_set_usage_methods (JAVA_USAGE_PANE (usage_pane), usage_methods);
      java_notebook_select_page_by_type (JAVA_NOTEBOOK (priv->notebook), JAVA_PAGE_TYPE_USAGE);
    }
  else
    {
      GtkWidget *usage_pane;
      usage_pane = java_notebook_get_page_by_type (JAVA_NOTEBOOK (priv->notebook), 
                                                   JAVA_PAGE_TYPE_USAGE);
      if (usage_pane != NULL)
        java_usage_pane_clear_usage_methods (JAVA_USAGE_PANE (usage_pane));
    }
    
  codeslayer_remove_from_processes (priv->codeslayer, priv->process_id);
}

static GList*
get_usage_methods_from_output (gchar *output)
{
  GList *results = NULL;
  gchar **split;
  gchar **tmp;
  
  split = g_strsplit (output, "\n", -1);

  if (split != NULL)
    {
      tmp = split;
      while (*tmp != NULL)
        {
          JavaUsageMethod *usage_method;
          usage_method = get_java_usage_method (*tmp);
          if (usage_method != NULL)
            results = g_list_append (results, usage_method);
          tmp++;
        }
      g_strfreev (split);
    }
  
  results = g_list_sort (results, (GCompareFunc) sort_usage_methods);

  return results;
}

static gint                
sort_usage_methods (JavaUsageMethod *usage_method1, 
                    JavaUsageMethod *usage_method2)
{
  const gchar *class_name1;
  const gchar *class_name2;
  class_name1 = java_usage_method_get_class_name (usage_method1);
  class_name2 = java_usage_method_get_class_name (usage_method2);  
  return g_strcmp0 (class_name1, class_name2);
}

static JavaUsageMethod*
get_java_usage_method (gchar *text)
{
  JavaUsageMethod *usage_method = NULL;
  gchar **split;
  gchar **tmp;
  
  if (!codeslayer_utils_has_text (text))
    return NULL;
  
  split = g_strsplit (text, "\t", -1);

  if (g_strcmp0 (*split, "NO_RESULTS_FOUND") == 0)
    return NULL;
  
  if (split != NULL)
    {
      gchar *class_name;
      gchar *file_path;
      gchar *line_number;
    
      tmp = split;
      
      class_name = *tmp;
      file_path = *++tmp;
      line_number = *++tmp;

      if (class_name != NULL && 
          file_path != NULL &&
          line_number != NULL)
        {
          usage_method = java_usage_method_new ();
          java_usage_method_set_class_name (usage_method, class_name);
          java_usage_method_set_file_path (usage_method, file_path);
          java_usage_method_set_line_number (usage_method, atoi(line_number));
        }
              
      g_strfreev (split);
    }

  return usage_method;
}
