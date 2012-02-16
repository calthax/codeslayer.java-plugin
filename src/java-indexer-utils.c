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
#include "java-indexer-index.h"
#include "java-indexer-utils.h"

static GList* get_indexes                   (JavaIndexer      *indexer,
                                             CodeSlayerEditor *editor,
                                             gchar            *text, 
                                             gchar            *path);
static gint sort_indexes                    (JavaIndexerIndex *index1, 
                                             JavaIndexerIndex *index2);
static gchar* find_path                     (gchar            *text);
static gchar* strip_path_comments           (gchar            *text);
static gchar* strip_path_parameters         (gchar            *text);
static gchar* get_text_to_search            (CodeSlayerEditor *editor, 
                                             GtkTextIter       iter);
static gchar* search_text_for_class_symbol  (const gchar      *text, 
                                             gchar            *class_symbol);
static gchar* search_text_for_import        (JavaIndexer      *indexer, 
                                             const gchar      *text, 
                                             gchar            *class_symbol);

GList* 
java_indexer_utils_completion_indexes  (JavaIndexer      *indexer, 
                                        CodeSlayerEditor *editor,
                                        GtkTextIter       iter)
{
  GList *list = NULL;
  gchar *path;
  gchar *text;   

  text = get_text_to_search (editor, iter);
  path = find_path (text);

  if (codeslayer_utils_has_text (path))
    {
      gchar *comments_stripped;
      gchar *parameters_stripped;
      
      comments_stripped = strip_path_comments (path);
      parameters_stripped = strip_path_parameters (comments_stripped);
      parameters_stripped = g_strreverse (parameters_stripped);
      
      g_print ("path %s\n", parameters_stripped);

      list = get_indexes (indexer, editor, text, parameters_stripped);
      
      if (list != NULL)
        list = g_list_sort (list, (GCompareFunc) sort_indexes);
      
      g_free (path);
      g_free (comments_stripped);
      g_free (parameters_stripped);
    }
    
  g_free (text);
    
  return list; 
}

/*
 * Take the path, start at the beginning, and resolve the 
 * actual classes and methods types.
 *
 * For example after the title figure out that this starts as a Column 
 * object and that the title() method also returns a Column object.
 *
 * new Column().title().
 *
 */
static GList*
get_indexes (JavaIndexer      *indexer,
             CodeSlayerEditor *editor,
             gchar            *text,
             gchar            *path)
{
  gchar **split;
  gchar **array;
  GList *indexes = NULL;
  
  split = g_strsplit (path, ".", -1);
  array = split;
  
  if (codeslayer_utils_has_text (*array))
    {
      gchar *class_symbol;
      class_symbol = search_text_for_class_symbol (text, *array);
      if (class_symbol != NULL)
        {
          gchar *import;
          g_print ("class_symbol %s\n", class_symbol);
          import = search_text_for_import (indexer, text, class_symbol);
          if (import != NULL)
            {
              indexes = java_indexer_get_package_indexes (indexer, import);
              g_free (import);
            }
          g_free (class_symbol);
        }      
      array++;
    }

  while (codeslayer_utils_has_text (*array))
    {
      g_print ("other %s\n", *array);
      array++;
    }
  
  if (split != NULL)
    g_strfreev (split);
  
  return indexes;
}
                           
static gint                
sort_indexes (JavaIndexerIndex *index1, 
              JavaIndexerIndex *index2)
{
  const gchar *name1;
  const gchar *name2;
  name1 = java_indexer_index_get_name (index1);
  name2 = java_indexer_index_get_name (index2);  
  return g_strcmp0 (name2, name1);
}

/*
 * Walk backwards and match braces to find the path.
 *
 * For example at any point in the following path you would be able to 
 * find the context, namely the tableModel, or the presidentService.
 *
 * tableModel.setItems(presidentService.getPresidents())
 */
static gchar*
find_path (gchar *text)
{
  gchar *result;
  GString *string;
  gchar *text_cpy;
  gchar *tmp;
  int brace = 0;
  
  string = g_string_new ("");

  text_cpy = g_strdup (text);
  tmp = text_cpy;
  text_cpy = g_strreverse (text_cpy);  
  
  for (; *text_cpy != '\0'; ++text_cpy)
    {
      if (*text_cpy == '=' ||
          *text_cpy == ';' ||
          *text_cpy == '{')
        break;
      
      if (*text_cpy == ')')
        brace++;
      if (*text_cpy == '(')
        {
          brace--;
          if (brace == -1)
            break;
        }
        
      string = g_string_append_c (string, *text_cpy);
    }
  
  result = g_string_free (string, FALSE);
  g_strstrip (result);
  g_free (tmp);
    
  return result;
}

/*
 * Take out the comments so that they do not effect how the 
 * completion path is walked to find the context of the methods.
 *
 * new DateFilterMatcher("MM/yyyy") becomes new DateFilterMatcher()
 */
static gchar*
strip_path_comments (gchar *path)
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
  
  result = g_regex_replace (regex, path, -1, 0, "", 0, NULL);
  
  g_regex_unref (regex);

  return result;
}

/*
 * Take out everything in the methods (between the braces) that do not 
 * belong in the context of the path being matched so that they do not 
 * effect how the completion path is walked to find the context of the 
 * methods.
 *
 * tableModel.setItems(presidentService.getPresidents()) 
 * becomes tableModel.setItems() if we are in the context of the tableModel.
 */
static gchar*
strip_path_parameters (gchar *path)
{
  gchar *result;
  GString *string;
  int brace = 0;
  
  string = g_string_new ("");

  for (; *path != '\0'; ++path)
    {
      if (*path == ')')
        {
          brace++;
          if (brace == 1)
            string = g_string_append_c (string, *path);
          continue;
        }
      if (*path == '(')
        {
          brace--;
          if (brace == 0)
            string = g_string_append_c (string, *path);
          continue;
        }
      
      if (brace == 0)
        string = g_string_append_c (string, *path);
    }
  
  result = g_string_free (string, FALSE);
  g_strstrip(result);
    
  return result;
}

/*
 * Start at the current place in the editor and get all the previous text.
 * The idea is that any variables that need to be resolved will come 
 * before this place in the editor.
 */
static gchar*
get_text_to_search (CodeSlayerEditor *editor, 
                    GtkTextIter       iter)
{
  GtkTextBuffer *buffer;
  GtkTextIter start;
  gchar *text;      

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));      
  gtk_text_buffer_get_start_iter (buffer, &start);
  text = gtk_text_buffer_get_text (buffer, &start, &iter, FALSE);

  return text;
}

/*
 * Take the class_symbol and find the type. For instance in the following
 * example the tableModel will be resolved to TableModel.
 *  
 * TableModel tableModel
 * tableModel.addFilterMatcher()
 */
static gchar*
search_text_for_class_symbol (const gchar *text, 
                              gchar       *class_symbol)
{
  gchar *result = NULL;
  GRegex *regex;
  gchar *concat;
  GMatchInfo *match_info;
  GError *error = NULL;
  
  concat = g_strconcat ("[a-zA-Z0-9_]+\\s+\\b", class_symbol, "\\b", NULL); 
   
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
  g_free (concat);
  
  if (error != NULL)
    {
      g_printerr ("search text for class symbol error: %s\n", error->message);
      g_error_free (error);
    }
    
  return result;   
}

/*
 * Take the class symbol and find the right import. One thing to be 
 * aware of is we need to replace the '*' with the class in case
 * the '*' is used instead of the full package declaration.
 * 
 * import org.jmesa.model.*;
 * import org.jmesa.model.TableModel;
 */
static gchar*
search_text_for_import (JavaIndexer *indexer, 
                        const gchar *text, 
                        gchar       *class_symbol)
{
  gchar* result = NULL;
  GRegex *regex;
  GMatchInfo *match_info;
  GError *error = NULL;
  
  regex = g_regex_new ("import\\s+(.*?);", 0, 0, NULL);
  
  g_regex_match_full (regex, text, -1, 0, 0, &match_info, &error);
  
  while (result == NULL && g_match_info_matches (match_info))
    {
      gchar *import = NULL;
      import = g_match_info_fetch (match_info, 1);
      g_match_info_next (match_info, &error);
      g_strstrip(import);
      if (g_str_has_suffix (import, "*"))
        {
          gchar *replace; 
          replace = codeslayer_utils_strreplace (import, "*", class_symbol);
          if (java_indexer_get_package_indexes (indexer, replace))
            result = g_strdup (replace);
          g_free (replace);
        }
      else
        {
          gchar *suffix = NULL;
          suffix = g_strconcat (".", class_symbol, NULL);
          if (g_str_has_suffix (import, suffix))
            result = g_strdup (import);
          g_free (suffix);
        }
      g_free (import);
    }
  
  g_match_info_free (match_info);
  g_regex_unref (regex);
  
  if (error != NULL)
    {
      g_printerr ("search text for import error: %s\n", error->message);
      g_error_free (error);
    }
    
  return result;   
}
