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
