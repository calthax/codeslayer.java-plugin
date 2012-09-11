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
#include "java-indexer-utils.h"

static gchar* find_path               (gchar       *text);
static gchar* strip_comments          (gchar       *text);
static gchar* strip_path_parameters   (gchar       *text);

gchar*
java_indexer_utils_get_expression (gchar *text)
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
