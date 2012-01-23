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
#include <gtksourceview/gtksourcecompletion.h>
#include <gtksourceview/gtksourcecompletioninfo.h>
#include <gtksourceview/gtksourcecompletionitem.h>
#include <gtksourceview/gtksourcelanguagemanager.h>
#include <codeslayer/codeslayer-utils.h>
#include "java-completion-provider.h"
#include "java-indexer-index.h"

static void java_completion_provider_class_init (JavaCompletionProviderClass      *klass);
static void java_completion_provider_init       (JavaCompletionProvider           *completion_provider);
static void java_completion_provider_finalize   (JavaCompletionProvider           *completion_provider);

static void provider_interface_init             (GtkSourceCompletionProviderIface *iface);
static gchar* provider_get_name                 (GtkSourceCompletionProvider      *provider);
static gint provider_get_priority               (GtkSourceCompletionProvider      *provider);
static gboolean provider_match                  (GtkSourceCompletionProvider      *provider,
                                                 GtkSourceCompletionContext       *context);
static void provider_populate                   (GtkSourceCompletionProvider      *provider,
                                                 GtkSourceCompletionContext       *context);

static gchar* find_completion_path              (GtkSourceCompletionProvider      *provider,
                                                 GtkSourceCompletionContext       *context);
static gchar* strip_completion_path_comments    (gchar                            *text);
static gchar* strip_completion_path_parameters  (gchar                            *text);
static gchar* get_completion_list               (GtkSourceCompletionProvider      *provider,
                                                 GtkSourceCompletionContext       *context, 
                                                 gchar                            *text);
static gchar* get_text_to_search                (GtkSourceCompletionContext       *context,
                                                 CodeSlayerEditor                 *editor);
static gchar* search_text_for_variable          (gchar                            *variable, 
                                                 const gchar                      *string);
static GList* search_text_for_potential_imports (gchar                            *variable, 
                                                 const gchar                      *string);

#define JAVA_COMPLETION_PROVIDER_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_COMPLETION_PROVIDER_TYPE, JavaCompletionProviderPrivate))

typedef struct _JavaCompletionProviderPrivate JavaCompletionProviderPrivate;

struct _JavaCompletionProviderPrivate
{
  CodeSlayerEditor *editor;
  JavaIndexer      *indexer;
  GList            *proposals;
};

G_DEFINE_TYPE_EXTENDED (JavaCompletionProvider,
                        java_completion_provider,
                        G_TYPE_OBJECT,
                        0,
                        G_IMPLEMENT_INTERFACE (GTK_SOURCE_TYPE_COMPLETION_PROVIDER,
                                               provider_interface_init));

static void
provider_interface_init (GtkSourceCompletionProviderIface *iface)
{
	iface->get_name = provider_get_name;
	iface->populate = provider_populate;
	iface->match = provider_match;
	iface->get_priority = provider_get_priority;
}
     
static void 
java_completion_provider_class_init (JavaCompletionProviderClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) java_completion_provider_finalize;
  g_type_class_add_private (klass, sizeof (JavaCompletionProviderPrivate));
}

static void
java_completion_provider_init (JavaCompletionProvider *completion_provider)
{
  JavaCompletionProviderPrivate *priv;
  
  priv = JAVA_COMPLETION_PROVIDER_GET_PRIVATE (completion_provider);
  priv->proposals = NULL;
  
	priv->proposals = g_list_prepend (priv->proposals,
	                            gtk_source_completion_item_new ("Proposal", "Proposal", NULL, NULL)); 
}

static void
java_completion_provider_finalize (JavaCompletionProvider *completion_provider)
{
  G_OBJECT_CLASS (java_completion_provider_parent_class)->finalize (G_OBJECT (completion_provider));
}

JavaCompletionProvider*
java_completion_provider_new (CodeSlayerEditor *editor, 
                              JavaIndexer      *indexer)
{
  JavaCompletionProviderPrivate *priv;
  JavaCompletionProvider *completion_provider;

  completion_provider = JAVA_COMPLETION_PROVIDER (g_object_new (java_completion_provider_get_type (), NULL));
  priv = JAVA_COMPLETION_PROVIDER_GET_PRIVATE (completion_provider);
  priv->editor = editor;
  priv->indexer = indexer;

  return completion_provider;
}

static gchar*
provider_get_name (GtkSourceCompletionProvider *provider)
{
	return g_strdup ("CodeSlayer");
}

static gint
provider_get_priority (GtkSourceCompletionProvider *provider)
{
	return 0;
}

static gboolean
provider_match (GtkSourceCompletionProvider *provider,
                GtkSourceCompletionContext  *context)
{
  GtkTextIter iter;
  GtkTextIter start;
  gchar *text;
  
  gtk_source_completion_context_get_iter (context, &iter);
  start = iter;
  
  gtk_text_iter_backward_char (&start);
  
  text = gtk_text_iter_get_text (&start, &iter);
  
  if (g_strcmp0 (text, ".") == 0)
    {
      g_free (text);
      return TRUE;
    }

  g_free (text);    
	return FALSE;
}

static void
provider_populate (GtkSourceCompletionProvider *provider,
                   GtkSourceCompletionContext  *context)
{
  JavaCompletionProviderPrivate *priv;
  gchar *text;

  priv = JAVA_COMPLETION_PROVIDER_GET_PRIVATE (provider);

  text = find_completion_path (provider, context);
  
  if (text != NULL)
    {
      gchar *text_strip_comments;
      gchar *text_strip_parameters;
      gchar *completion_list;
      text_strip_comments = strip_completion_path_comments (text);
      text_strip_parameters = strip_completion_path_parameters (text_strip_comments);
      text_strip_parameters = g_strreverse (text_strip_parameters);
      completion_list = get_completion_list (provider, context, text_strip_parameters);
      g_print ("completion_list->%s\n", completion_list);
      g_free (text);
      g_free (text_strip_comments);
      g_free (text_strip_parameters);
    }
  
	gtk_source_completion_context_add_proposals (context, 
	                                             provider, 
	                                             priv->proposals, TRUE);
}

static gchar*
find_completion_path (GtkSourceCompletionProvider *provider,
                      GtkSourceCompletionContext  *context)
{
  gchar *result;
  GString *string;
  gchar *text;
  gchar *variable;
  GtkTextIter iter;
  GtkTextIter start;
  int brace = 0;
  
  string = g_string_new ("");

  gtk_source_completion_context_get_iter (context, &iter);

  start = iter;
  gtk_text_iter_backward_chars (&start, 100);
  
  text = gtk_text_iter_get_text (&start, &iter);
  variable = text;
  variable = g_strreverse (variable);  
  
  for (; *variable != '\0'; ++variable)
    {
      if (*variable == '=' ||
          *variable == ';' ||
          *variable == '{')
        break;
      
      if (*variable == ')')
        brace++;
      if (*variable == '(')
        {
          brace--;
          if (brace == -1)
            break;
        }
        
      string = g_string_append_c (string, *variable);
    }
  
  g_free (text);
  
  result = g_string_free (string, FALSE);
  g_strstrip(result);
    
  return result;
}

static gchar*
strip_completion_path_comments (gchar *text)
{
  gchar *result;
  GRegex *regex;
  GError *error = NULL;

  regex = g_regex_new ("\"([^\"\\\\]*(\\.[^\"\\\\]*)*)\"", 0, 0, &error);
  
  if (error != NULL)
    {
      g_printerr (error->message);
      g_error_free (error);
    }
  
  result = g_regex_replace (regex, text, -1, 0, "", 0, NULL);
  
  g_regex_unref (regex);

  return result;
}

static gchar*
strip_completion_path_parameters (gchar *text)
{
  gchar *result;
  GString *string;
  int brace = 0;
  
  string = g_string_new ("");

  for (; *text != '\0'; ++text)
    {
      if (*text == ')')
        {
          brace++;
          if (brace == 1)
            string = g_string_append_c (string, *text);
          continue;
        }
      if (*text == '(')
        {
          brace--;
          if (brace == 0)
            string = g_string_append_c (string, *text);
          continue;
        }
      
      if (brace == 0)
        string = g_string_append_c (string, *text);
    }
  
  result = g_string_free (string, FALSE);
  g_strstrip(result);
    
  return result;
}

static gchar*
get_completion_list (GtkSourceCompletionProvider *provider,
                     GtkSourceCompletionContext  *context, 
                     gchar                       *string)
{
  JavaCompletionProviderPrivate *priv;
  gchar **split;
  gchar **array;
  gchar *variable;

  priv = JAVA_COMPLETION_PROVIDER_GET_PRIVATE (provider);
  
  split = g_strsplit (string, ".", -1);
  array = split;
  
  if (codeslayer_utils_has_text (*array))
    {
      gchar *text;   
      GList *imports;
      text = get_text_to_search (context, priv->editor);
      variable = search_text_for_variable (*array, text);
      imports = search_text_for_potential_imports (variable, text);
      if (imports != NULL)
        g_list_free (imports);
      g_free (text);
      array++;
    }

  g_print ("variable %s\n", variable);
  
  while (codeslayer_utils_has_text (*array))
    {
      g_print ("other %s\n", *array);
      array++;
    }
  
  if (split != NULL)
    g_strfreev (split);
    
  return string;
}

static gchar*
get_text_to_search (GtkSourceCompletionContext *context, 
                    CodeSlayerEditor           *editor)
{
  GtkTextBuffer *buffer;
  GtkTextIter start;
  GtkTextIter end;
  gchar *text;      

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));      
  gtk_text_buffer_get_start_iter (buffer, &start);
  gtk_source_completion_context_get_iter (context, &end);      
  text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

  return text;
}

static gchar*
search_text_for_variable (gchar       *variable, 
                          const gchar *text)
{
  gchar *result = NULL;
  GRegex *regex;
  gchar *concat;
  GMatchInfo *match_info;
  GError *error = NULL;
  
  concat = g_strconcat ("[a-zA-Z0-9_]+\\s+\\b", variable, "\\b", NULL); 
   
  regex = g_regex_new (concat, 0, 0, NULL);
  
  g_regex_match_full (regex, text, -1, 0, 0, &match_info, &error);
  
  while (g_match_info_matches (match_info))
    {
      result = g_match_info_fetch (match_info, 0);
      if (g_match_info_next (match_info, &error))
        {
          g_free (result);        
        }
      else
        {
          gchar **split;      
          split = g_strsplit (result, " ", -1);
          g_free (result);
          result = g_strdup (*split);
          g_strstrip (result);
          g_strfreev (split);
        }
    }
  
  g_match_info_free (match_info);
  g_regex_unref (regex);
  
  if (error != NULL)
    {
      g_printerr ("search text for variable error: %s\n", error->message);
      g_error_free (error);
    }
    
  g_free (concat);
  return result;   
}

static GList*
search_text_for_potential_imports (gchar       *variable, 
                                   const gchar *text)
{
  GList *results = NULL;
  GRegex *regex;
  GMatchInfo *match_info;
  GError *error = NULL;
  
  regex = g_regex_new ("import\\s+.*?;", 0, 0, NULL);
  
  g_regex_match_full (regex, text, -1, 0, 0, &match_info, &error);
  
  while (g_match_info_matches (match_info))
    {
      gchar *import = NULL;
      import = g_match_info_fetch (match_info, 0);
      if (g_match_info_next (match_info, &error))
        {
          results = g_list_prepend (results, g_strdup (import));
          g_print ("import %s\n", import);
          g_free (import);
        }
    }
  
  g_match_info_free (match_info);
  g_regex_unref (regex);
  
  if (error != NULL)
    {
      g_printerr ("search text for import error: %s\n", error->message);
      g_error_free (error);
    }
    
  return results;   
}
