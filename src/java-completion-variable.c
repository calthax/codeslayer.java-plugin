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

#include <string.h>
#include <codeslayer/codeslayer-utils.h>
#include "java-completion-variable.h"
#include "java-utils.h"

static void java_completion_provider_interface_init  (gpointer                     page, 
                                                      gpointer                     data);
static void java_completion_variable_class_init      (JavaCompletionVariableClass *klass);
static void java_completion_variable_init            (JavaCompletionVariable      *variable);
static void java_completion_variable_finalize        (JavaCompletionVariable      *variable);

static GList* java_completion_get_proposals          (JavaCompletionVariable      *variable, 
                                                      GtkTextIter                  iter);
static GList* find_matches                           (GtkTextIter                  iter);
static gboolean has_match                            (GtkTextIter                  start);

#define JAVA_COMPLETION_VARIABLE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_COMPLETION_VARIABLE_TYPE, JavaCompletionVariablePrivate))

typedef struct _JavaCompletionVariablePrivate JavaCompletionVariablePrivate;

struct _JavaCompletionVariablePrivate
{
  CodeSlayerEditor *editor;
};

G_DEFINE_TYPE_EXTENDED (JavaCompletionVariable,
                        java_completion_variable,
                        G_TYPE_INITIALLY_UNOWNED,
                        0,
                        G_IMPLEMENT_INTERFACE (CODESLAYER_COMPLETION_PROVIDER_TYPE,
                                               java_completion_provider_interface_init));

static void
java_completion_provider_interface_init (gpointer provider, 
                                         gpointer data)
{
  CodeSlayerCompletionProviderInterface *provider_interface = (CodeSlayerCompletionProviderInterface*) provider;
  provider_interface->get_proposals = (GList* (*) (CodeSlayerCompletionProvider *obj, GtkTextIter iter)) java_completion_get_proposals;
}

static void 
java_completion_variable_class_init (JavaCompletionVariableClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) java_completion_variable_finalize;
  g_type_class_add_private (klass, sizeof (JavaCompletionVariablePrivate));
}

static void
java_completion_variable_init (JavaCompletionVariable *variable)
{
}

static void
java_completion_variable_finalize (JavaCompletionVariable *variable)
{
  G_OBJECT_CLASS (java_completion_variable_parent_class)->finalize (G_OBJECT (variable));
}

JavaCompletionVariable*
java_completion_variable_new (CodeSlayerEditor *editor)
{
  JavaCompletionVariablePrivate *priv;
  JavaCompletionVariable *variable;

  variable = JAVA_COMPLETION_VARIABLE (g_object_new (java_completion_variable_get_type (), NULL));
  priv = JAVA_COMPLETION_VARIABLE_GET_PRIVATE (variable);
  priv->editor = editor;

  return variable;
}

static GList* 
java_completion_get_proposals (JavaCompletionVariable *variable, 
                               GtkTextIter         iter)
{
  JavaCompletionVariablePrivate *priv;
  GList *proposals = NULL;
  GtkTextBuffer *buffer;
  GtkTextMark *mark;
  GList *list = NULL;
  GList *tmp = NULL;
  GtkTextIter start;
  gchar *start_variable;
  
  priv = JAVA_COMPLETION_VARIABLE_GET_PRIVATE (variable);

  start = iter;
  java_utils_move_iter_word_start (&start);
  
  if (!has_match (start))
    return NULL;

  start_variable = gtk_text_iter_get_text (&start, &iter);
  
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->editor));
  mark = gtk_text_buffer_create_mark (buffer, NULL, &start, TRUE);

  list = find_matches (start);
  tmp = list;

  while (list != NULL)
    {
      gchar *match_text = list->data;
      CodeSlayerCompletionProposal *proposal;
      proposal = codeslayer_completion_proposal_new (match_text, g_strstrip (match_text), mark);
      proposals = g_list_prepend (proposals, proposal);
      list = g_list_next (list);
    }

  if (tmp != NULL)
    {
      g_list_foreach (tmp, (GFunc) g_free, NULL);    
      g_list_free (tmp);
    }
  
  if (start_variable != NULL)
    g_free (start_variable);
    
  return proposals;
}

static gboolean
has_match (GtkTextIter start)
{
  gboolean result = TRUE;
  GtkTextIter prev;
  GtkTextIter next;
  gchar *prev_text;
  gchar *next_text;
  
  prev = start;
  next = start;

  gtk_text_iter_backward_char (&prev);
  gtk_text_iter_forward_char (&next);

  prev_text = gtk_text_iter_get_text (&prev, &start);
  next_text = gtk_text_iter_get_text (&start, &next);    
  
  if (g_strcmp0 (prev_text, ".") == 0 || 
      g_ascii_isupper (*next_text)) 
    result = FALSE;
    
  g_free (prev_text);
  g_free (next_text);

  return result;
}

static GList*
find_matches (GtkTextIter iter)
{
  GList *results = NULL;
  GtkTextIter start;
  gchar *text;
  gchar *tmp;
  gint i = 0;
  gint length = 0;
  
  start = iter;

  gtk_text_iter_backward_word_start (&start);
  
  text = gtk_text_iter_get_text (&start, &iter);  
  tmp = text;
  
  length = strlen (tmp);
  
  for (i = 0; *tmp != '\0'; ++tmp, ++i)
    {
      if (g_ascii_isupper (*tmp))
        {
          GString *string;
          gchar *substr;
          substr = codeslayer_utils_substr (text, i + 1, length);
          string = g_string_new ("");
          string = g_string_append_c (string, g_ascii_tolower (*tmp));
          string = g_string_append (string, substr);
          results = g_list_prepend (results, g_string_free (string, FALSE));
          g_free (substr);
        }
    }
    
  g_free (text);
    
  return results;   
}
