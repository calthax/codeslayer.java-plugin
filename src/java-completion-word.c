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
#include "java-completion-word.h"
#include "java-utils.h"

static void java_completion_provider_interface_init  (gpointer                  page, 
                                                      gpointer                  data);
static void java_completion_word_class_init          (JavaCompletionWordClass  *klass);
static void java_completion_word_init                (JavaCompletionWord       *word);
static void java_completion_word_finalize            (JavaCompletionWord       *word);

static GList* java_completion_get_proposals          (JavaCompletionWord       *word, 
                                                      GtkTextIter               iter);
static GList* find_matches                           (gchar                    *text,
                                                      gchar                    *word);
static gint compare_match                            (gchar                    *a,
                                                      gchar                    *b);
static gboolean has_match                            (GtkTextIter                 start);

#define JAVA_COMPLETION_WORD_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_COMPLETION_WORD_TYPE, JavaCompletionWordPrivate))

typedef struct _JavaCompletionWordPrivate JavaCompletionWordPrivate;

struct _JavaCompletionWordPrivate
{
  CodeSlayerEditor *editor;
};

G_DEFINE_TYPE_EXTENDED (JavaCompletionWord,
                        java_completion_word,
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
java_completion_word_class_init (JavaCompletionWordClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) java_completion_word_finalize;
  g_type_class_add_private (klass, sizeof (JavaCompletionWordPrivate));
}

static void
java_completion_word_init (JavaCompletionWord *word)
{
}

static void
java_completion_word_finalize (JavaCompletionWord *word)
{
  G_OBJECT_CLASS (java_completion_word_parent_class)->finalize (G_OBJECT (word));
}

JavaCompletionWord*
java_completion_word_new (CodeSlayerEditor *editor)
{
  JavaCompletionWordPrivate *priv;
  JavaCompletionWord *word;

  word = JAVA_COMPLETION_WORD (g_object_new (java_completion_word_get_type (), NULL));
  priv = JAVA_COMPLETION_WORD_GET_PRIVATE (word);
  priv->editor = editor;

  return word;
}

static GList* 
java_completion_get_proposals (JavaCompletionWord *word, 
                               GtkTextIter         iter)
{
  JavaCompletionWordPrivate *priv;
  GList *proposals = NULL;
  GtkTextBuffer *buffer;
  GtkTextMark *mark;
  GList *list = NULL;
  GList *tmp = NULL;
  GtkTextIter start;
  gchar *start_word;
  gchar *text;
  
  priv = JAVA_COMPLETION_WORD_GET_PRIVATE (word);

  start = iter;
  java_utils_move_iter_word_start (&start);
  
  if (!has_match (start))
    return NULL;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->editor));
  mark = gtk_text_buffer_create_mark (buffer, NULL, &start, TRUE);
  
  start_word = gtk_text_iter_get_text (&start, &iter);
  
  text = java_utils_get_text_to_search (GTK_TEXT_VIEW (priv->editor), start);
  list = find_matches (text, start_word);
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
  
  if (start_word != NULL)
    g_free (start_word);
    
  g_free (text);

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
find_matches (gchar *text, 
              gchar *word)
{
  GList *results = NULL;
  GRegex *regex;
  gchar *concat;
  GMatchInfo *match_info;
  GError *error = NULL;
  
  concat = g_strconcat ("(\\s", word, "[a-zA-Z0-9_]+)", NULL);
  
  regex = g_regex_new (concat, 0, 0, NULL);
  
  g_regex_match_full (regex, text, -1, 0, 0, &match_info, &error);
  
  while (g_match_info_matches (match_info))
    {
      gchar *match_text = NULL;
      match_text = g_match_info_fetch (match_info, 0);
      
      if (g_list_find_custom (results, match_text, (GCompareFunc) compare_match) == NULL)
        results = g_list_prepend (results, g_strdup (match_text));
        
      g_free (match_text);
      g_match_info_next (match_info, &error);
    }
  
  g_match_info_free (match_info);
  g_regex_unref (regex);
  g_free (concat);
  
  if (error != NULL)
    {
      g_printerr ("search text for import error: %s\n", error->message);
      g_error_free (error);
    }
    
  return results;   
}

static gint
compare_match (gchar *a,
               gchar *b)
{
  return g_strcmp0 (a, b);
}
