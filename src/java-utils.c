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