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
#include "java-navigate.h"
#include "java-utils.h"
#include "java-page.h"
#include "java-indexer-utils.h"

static void java_navigate_class_init  (JavaNavigateClass *klass);
static void java_navigate_init        (JavaNavigate      *navigate);
static void java_navigate_finalize    (JavaNavigate      *navigate);

static void navigate_action           (JavaNavigate      *navigate);
static gchar* get_input               (JavaNavigate      *navigate, 
                                       const gchar       *file_path, 
                                       gchar             *expression, 
                                       gint               line_number);
static void render_output             (JavaNavigate      *navigate, 
                                       gchar             *output);
                                       
static gchar* get_text                (GtkTextBuffer     *buffer, 
                                       GtkTextIter        iter);

#define JAVA_NAVIGATE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_NAVIGATE_TYPE, JavaNavigatePrivate))

typedef struct _JavaNavigatePrivate JavaNavigatePrivate;

struct _JavaNavigatePrivate
{
  CodeSlayer *codeslayer;
  JavaConfigurations *configurations;
  JavaClient *client;
};

G_DEFINE_TYPE (JavaNavigate, java_navigate, G_TYPE_OBJECT)
     
static void 
java_navigate_class_init (JavaNavigateClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) java_navigate_finalize;
  g_type_class_add_private (klass, sizeof (JavaNavigatePrivate));
}

static void
java_navigate_init (JavaNavigate *navigate){}

static void
java_navigate_finalize (JavaNavigate *navigate)
{
  G_OBJECT_CLASS (java_navigate_parent_class)->finalize (G_OBJECT (navigate));
}

JavaNavigate*
java_navigate_new (CodeSlayer         *codeslayer,
                   GtkWidget          *menu,
                   JavaConfigurations *configurations,
                   JavaClient         *client)
{
  JavaNavigatePrivate *priv;
  JavaNavigate *navigate;

  navigate = JAVA_NAVIGATE (g_object_new (java_navigate_get_type (), NULL));
  priv = JAVA_NAVIGATE_GET_PRIVATE (navigate);
  priv->codeslayer = codeslayer;
  priv->configurations = configurations;
  priv->client = client;

  g_signal_connect_swapped (G_OBJECT (menu), "find-symbol",
                            G_CALLBACK (navigate_action), navigate);

  return navigate;
}

static void 
navigate_action (JavaNavigate *navigate)
{
  JavaNavigatePrivate *priv;
  CodeSlayerEditor *editor;
  const gchar *file_path;
  GtkTextBuffer *buffer;

  gchar *text;
  gchar *expression;
  GtkTextMark *insert_mark;
  GtkTextMark *selection_mark;
  gint line_number;
  gchar *input;
  gchar *output;

  GtkTextIter start, end;

  priv = JAVA_NAVIGATE_GET_PRIVATE (navigate);

  editor = codeslayer_get_active_editor (priv->codeslayer);
  
  if (editor == NULL)
    return;
    
  file_path = codeslayer_editor_get_file_path (editor);
  if (!g_str_has_suffix (file_path, ".java"))
    return;
  
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));

  insert_mark = gtk_text_buffer_get_insert (buffer);    
  selection_mark = gtk_text_buffer_get_selection_bound (buffer);

  gtk_text_buffer_get_iter_at_mark (buffer, &start, insert_mark);
  gtk_text_buffer_get_iter_at_mark (buffer, &end, selection_mark);

  line_number = gtk_text_iter_get_line (&start);
  
  text = get_text (buffer, end);
  expression = java_indexer_utils_get_expression (text);
  
  input = get_input (navigate, file_path, expression, line_number);

  g_print ("input: %s\n", input);
  
  output = java_client_send (priv->client, input);
  
  if (output != NULL)
    {
      g_print ("output: %s\n", output);
      render_output (navigate, output);
      g_free (output);
    }

  g_free (input);
}

static gchar*
get_text (GtkTextBuffer *buffer, 
          GtkTextIter    iter)
{
  GtkTextIter start;
  gchar *text;
  
  gtk_text_buffer_get_start_iter (buffer, &start);
  
  text = gtk_text_buffer_get_text (buffer, &start, &iter, FALSE);

  return text;
}

static gchar* 
get_input (JavaNavigate *navigate, 
           const gchar  *file_path, 
           gchar        *expression, 
           gint          line_number)
{
  JavaNavigatePrivate *priv;

  gchar *source_indexes_folders;
  gchar *line_number_str;
  gchar *result;
  
  priv = JAVA_NAVIGATE_GET_PRIVATE (navigate);
  
  line_number_str = g_strdup_printf ("%d", (line_number + 1));
  
  source_indexes_folders = get_source_indexes_folders (priv->codeslayer, priv->configurations);
  
  result = g_strconcat ("-program navigate", 
                        " -sourcefile ", file_path,
                        " -expression ", expression,
                        " -linenumber ", line_number_str,
                        source_indexes_folders, 
                        NULL);
  
  g_free (source_indexes_folders);
  g_free (line_number_str);

  return result;
}

static void
render_output (JavaNavigate *navigate, 
               gchar        *output)
{
  JavaNavigatePrivate *priv;
  gchar **split;
  gchar **tmp;
  
  priv = JAVA_NAVIGATE_GET_PRIVATE (navigate);

  if (!codeslayer_utils_has_text (output))
    return;
  
  split = g_strsplit (output, "\t", -1);
  if (split != NULL)
    {
      gchar *file_path;
      gchar *line_number;

      tmp = split;

      file_path = *tmp;
      line_number = *++tmp;
      
      if (file_path != NULL && 
          line_number != NULL)
        {
          CodeSlayerDocument *document;
          CodeSlayerProject *project;
          
          document = codeslayer_document_new ();
          codeslayer_document_set_file_path (document, file_path);
          codeslayer_document_set_line_number (document, atoi(line_number));
          
          project = codeslayer_get_project_by_file_path (priv->codeslayer, file_path);
          
          if (project != NULL)
            {
              codeslayer_document_set_project (document, project);
              codeslayer_select_editor (priv->codeslayer, document);
            }

          g_object_unref (document);
        
        }
        
      g_strfreev (split);
    }
}
