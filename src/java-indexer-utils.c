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
#include <stdio.h>
#include <stdlib.h>
#include <codeslayer/codeslayer-utils.h>
#include "java-indexer-index.h"
#include "java-indexer-utils.h"

static gchar* find_path              (gchar       *text);
static gchar* strip_path_comments    (gchar       *text);
static gchar* strip_path_parameters  (gchar       *text);
static GList* get_package_indexes    (gchar       *group_folder_path,
                                      gchar       *index_file_name,
                                      gchar       *package_name);
gchar* get_local_package_name        (gchar       *group_folder_path, 
                                      const gchar *text, 
                                      gchar       *class_name);

/*
 * Start at the current place in the editor and get all the previous text.
 * The idea is that any variables that need to be resolved will come 
 * before this place in the editor.
 */
gchar*
java_indexer_utils_get_text_to_search (GtkTextView *text_view, 
                                       GtkTextIter  iter)
{
  GtkTextBuffer *buffer;
  GtkTextIter start;
  gchar *text;      

  buffer = gtk_text_view_get_buffer (text_view);      
  gtk_text_buffer_get_start_iter (buffer, &start);
  text = gtk_text_buffer_get_text (buffer, &start, &iter, FALSE);

  return text;
}

gchar*
java_indexer_utils_get_context_path (gchar *text)
{
  gchar *result = NULL;
  gchar *path;

  path = find_path (text);
  if (path != NULL)
    {
      gchar *comments_stripped;
      gchar *parameters_stripped;
      
      comments_stripped = strip_path_comments (path);
      parameters_stripped = strip_path_parameters (comments_stripped);
      parameters_stripped = g_strreverse (parameters_stripped);
      
      result = g_strdup (parameters_stripped);
      
      g_free (path);
      g_free (comments_stripped);
      g_free (parameters_stripped);
    }

  return result; 
}

/*
 * Walk backwards and match braces to find the context path.
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
          *text_cpy == ' ' ||
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
 * Take the variable and find the type. For instance in the following
 * example the tableModel will be resolved to TableModel.
 *  
 * TableModel tableModel
 * tableModel.addFilterMatcher()
 */
gchar*
java_indexer_utils_get_class_name (const gchar *text, 
                                   gchar       *variable)
{
  gchar *result = NULL;
  GRegex *regex;
  gchar *concat;
  GMatchInfo *match_info;
  GError *error = NULL;
  
  concat = g_strconcat ("([A-Z][a-zA-Z0-9_]+)(?:<[a-zA-Z0-9_<>,?\\s]*)?(\\s+\\b", variable, "\\b)", NULL);
   
  regex = g_regex_new (concat, 0, 0, NULL);
  
  g_regex_match_full (regex, text, -1, 0, 0, &match_info, &error);
  
  if (g_match_info_matches (match_info))
    result = g_match_info_fetch (match_info, 1);
  
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
gchar*
java_indexer_utils_get_package_name (gchar       *group_folder_path, 
                                     const gchar *text, 
                                     gchar       *class_name)
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
          replace = codeslayer_utils_strreplace (import, "*", class_name);
          if (java_indexer_utils_get_indexes (group_folder_path, replace))
            result = g_strdup (replace);
          g_free (replace);
        }
      else
        {
          gchar *suffix = NULL;
          suffix = g_strconcat (".", class_name, NULL);
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
    
  if (result == NULL)
    result = get_local_package_name (group_folder_path, text, class_name);
    
  return result;   
}

/*
 * Check to see if this is declared in the current package.
 */
 gchar*
get_local_package_name (gchar       *group_folder_path, 
                        const gchar *text, 
                        gchar       *class_name)
{
  gchar* result = NULL;
  GRegex *regex;
  GMatchInfo *match_info;
  GError *error = NULL;
  
  regex = g_regex_new ("package\\s(.*?);", 0, 0, NULL);
  
  g_regex_match_full (regex, text, -1, 0, 0, &match_info, &error);
  
  if (result == NULL && g_match_info_matches (match_info))
    {
      gchar *import = NULL;
      gchar *replace; 

      import = g_match_info_fetch (match_info, 1);
      g_strstrip(import);
      
      replace = g_strconcat (import, ".", class_name, NULL);
      if (java_indexer_utils_get_indexes (group_folder_path, replace))
        result = g_strdup (replace);

      g_free (replace);
      g_free (import);
    }
  
  g_match_info_free (match_info);
  g_regex_unref (regex);
  
  if (error != NULL)
    {
      g_printerr ("search text for package error: %s\n", error->message);
      g_error_free (error);
    }
    
  return result;   
}

GList*
java_indexer_utils_get_indexes (gchar *group_folder_path,
                                gchar *package_name)
{
  GList *indexes = NULL;
  indexes = get_package_indexes (group_folder_path, "projects.indexes", package_name);

  if (indexes == NULL)
    indexes = get_package_indexes (group_folder_path, "libs.indexes", package_name);

  return indexes;
}

static GList*
get_package_indexes (gchar *group_folder_path,
                     gchar *index_file_name,
                     gchar *package_name)
{
  GList *indexes = NULL;
  gchar *file_name;
  GIOChannel *channel;
  gchar *text;
  GError *error = NULL;
  
  file_name = g_build_filename (group_folder_path, "indexes", index_file_name, NULL);

  if (!g_file_test (file_name, G_FILE_TEST_EXISTS))
    {
      g_warning ("There is no projects indexes.");
      g_free (file_name);
      return indexes;
    }
  
  channel = g_io_channel_new_file (file_name, "r", &error);
  
  while (g_io_channel_read_line (channel, &text, NULL, NULL, NULL) == G_IO_STATUS_NORMAL)
    {
      gchar **split;
      gchar **array;
    
      split = g_strsplit (text, "\t", -1);
      array = split;

      if (g_strcmp0 (package_name, *array) == 0)
        {
          JavaIndexerIndex *index;
          index = java_indexer_index_new ();
          java_indexer_index_set_package_name (index, *array);
          java_indexer_index_set_class_name (index, *++array);
          java_indexer_index_set_method_modifier (index, *++array);
          java_indexer_index_set_method_name (index, *++array);
          java_indexer_index_set_method_parameters (index, *++array);
          java_indexer_index_set_method_completion (index, *++array);
          java_indexer_index_set_method_return_type (index, *++array);
          
          if (*++array != NULL)
            {
              java_indexer_index_set_file_path (index, *array);
              java_indexer_index_set_line_number (index, atoi (*++array));
            }
            
          indexes = g_list_prepend (indexes, index);
        }
        
      g_strfreev (split);
      g_free (text);
    }
    
  g_io_channel_shutdown(channel, FALSE, NULL);
  g_io_channel_unref (channel);  
  g_free (file_name);

  return indexes;
}
