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
#include "java-utils.h"

static gchar* find_path              (gchar *text);
static gchar* strip_comments         (gchar *text);
static gchar* strip_path_parameters  (gchar *text);

gchar*
java_utils_get_class_name (JavaConfiguration  *configuration,
                           CodeSlayerDocument *document)
{
  const gchar *folder_path;
  const gchar *file_path;
  gchar *substr;
  gchar *replace;
  
  folder_path = java_configuration_get_source_folder (configuration);
  file_path = codeslayer_document_get_file_path (document);
  
  if (!g_str_has_prefix (file_path, folder_path))
    {
      folder_path = java_configuration_get_test_folder (configuration);          
    }
  
  if (!g_str_has_prefix (file_path, folder_path))
    {
      g_warning ("Not able to determine file.");
      return NULL;    
    }

  substr = codeslayer_utils_substr (file_path, strlen(folder_path) + 1, strlen(file_path) - 6);  
  replace = codeslayer_utils_strreplace (substr, G_DIR_SEPARATOR_S, ".");
  
  g_free (substr);
  return replace;
}

/*
 * Start at the current place in the editor and get all the previous text.
 * The idea is that any variables that need to be resolved will come 
 * before this place in the editor.
 */
gchar*
java_utils_get_text_to_search (GtkTextView *text_view, 
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

void
java_utils_move_iter_word_start (GtkTextIter *iter)
{
  if (!gtk_text_iter_inside_word (iter) && !gtk_text_iter_ends_word (iter))
    return;
    
  if (gtk_text_iter_starts_word (iter))
    return;
    
  while (!gtk_text_iter_starts_word (iter))
    gtk_text_iter_backward_char (iter);
}

gchar*
get_source_indexes_folders (CodeSlayer         *codeslayer, 
                            JavaConfigurations *configurations)
{
  gchar *group_folder_path;
  gchar *index_file_name;
  GList *list;
  GString *string;

  group_folder_path = codeslayer_get_active_group_folder_path (codeslayer);
  index_file_name = g_build_filename (group_folder_path, "indexes", NULL);
  
  string = g_string_new (" -sourcefolder ");
  
  list = java_configurations_get_list (configurations);
  while (list != NULL)
    {
      JavaConfiguration *configuration = list->data;
      const gchar *source_folder;
      const gchar *test_folder;
      source_folder = java_configuration_get_source_folder (configuration);
      test_folder = java_configuration_get_test_folder (configuration);
      if (codeslayer_utils_has_text (source_folder))
        {
          string = g_string_append (string, source_folder);
          string = g_string_append (string, ":");        
        }
      if (codeslayer_utils_has_text (test_folder))
        {
          string = g_string_append (string, test_folder);
          string = g_string_append (string, ":");        
        }
      list = g_list_next (list);
    }

  string = g_string_append (string, " -indexesfolder ");
  string = g_string_append (string, index_file_name);

  g_free (group_folder_path);
  g_free (index_file_name);
  
  return g_string_free (string, FALSE);
}

gchar*
get_lib_indexes_folders (CodeSlayer         *codeslayer, 
                         JavaConfigurations *configurations)
{
  gchar *group_folder_path;
  gchar *index_file_name;
  GList *list;
  GString *string;

  group_folder_path = codeslayer_get_active_group_folder_path (codeslayer);
  index_file_name = g_build_filename (group_folder_path, "indexes", NULL);
  
  string = g_string_new (" -libfolder ");
  
  list = java_configurations_get_list (configurations);
  while (list != NULL)
    {
      JavaConfiguration *configuration = list->data;
      const gchar *lib_folder;
      lib_folder = java_configuration_get_lib_folder (configuration);
      if (codeslayer_utils_has_text (lib_folder))
        {
          string = g_string_append (string, lib_folder);
          string = g_string_append (string, ":");        
        }
      list = g_list_next (list);
    }

  string = g_string_append (string, " -indexesfolder ");
  string = g_string_append (string, index_file_name);

  g_free (group_folder_path);
  g_free (index_file_name);
  
  return g_string_free (string, FALSE);
}

gchar*
java_utils_get_indexes_folder (CodeSlayer *codeslayer)
{
  gchar *group_folder_path;
  gchar *index_file_name;
  GString *string;

  group_folder_path = codeslayer_get_active_group_folder_path (codeslayer);
  index_file_name = g_build_filename (group_folder_path, "indexes", NULL);
  
  string = g_string_new ("");
  string = g_string_append (string, " -indexesfolder ");
  string = g_string_append (string, index_file_name);

  g_free (group_folder_path);
  g_free (index_file_name);
  
  return g_string_free (string, FALSE);
}

gchar*
java_utils_get_expression (gchar *text)
{
  gchar *result = NULL;
  gchar *comments_stripped;
  gchar *path;
  
  comments_stripped = strip_comments (text);
  
  path = find_path (comments_stripped);
  if (path != NULL)
    {
      gchar *parameters_stripped;
      
      parameters_stripped = strip_path_parameters (path);
      parameters_stripped = g_strreverse (parameters_stripped);
      
      result = g_strdup (parameters_stripped);
      
      g_free (path);
      g_free (parameters_stripped);
    }
    
  g_free (comments_stripped);

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
          g_ascii_isspace (*text_cpy) ||
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
strip_comments (gchar *text)
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
