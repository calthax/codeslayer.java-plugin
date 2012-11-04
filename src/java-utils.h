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
 * You should have received a copy of the GNU General Public Licens
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __JAVA_UTILS_H__
#define	__JAVA_UTILS_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>
#include "java-configurations.h"

G_BEGIN_DECLS

gchar*  java_utils_get_class_name        (JavaConfiguration  *configuration,
                                          CodeSlayerDocument *document);                                    
gchar*  java_utils_get_text_to_search    (GtkTextView        *text_view, 
                                          GtkTextIter         iter);                                    
void    java_utils_move_iter_word_start  (GtkTextIter        *iter);  
gchar*  get_source_indexes_folders       (CodeSlayer         *codeslayer, 
                                          JavaConfigurations *configurations);
gchar*  get_lib_indexes_folders          (CodeSlayer         *codeslayer, 
                                          JavaConfigurations *configurations);
gchar*  java_utils_get_indexes_folder    (CodeSlayer         *codeslayer);
gchar*  java_utils_get_expression        (gchar              *text);
                               
G_END_DECLS

#endif /* __JAVA_UTILS_H__ */
