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

#ifndef __JAVA_INDEXER_UTILS_H__
#define	__JAVA_INDEXER_UTILS_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>
#include "java-indexer.h"

G_BEGIN_DECLS

gchar*  java_indexer_utils_find_path                     (gchar            *text);
gchar*  java_indexer_utils_strip_path_comments           (gchar            *text);
gchar*  java_indexer_utils_strip_path_parameters         (gchar            *text);
gchar*  java_indexer_utils_get_text_to_search            (CodeSlayerEditor *editor, 
                                                          GtkTextIter       iter);
gchar*  java_indexer_utils_search_text_for_class_symbol  (const gchar      *text, 
                                                          gchar            *class_symbol);
gchar*  java_indexer_utils_search_text_for_import        (JavaIndexer      *indexer, 
                                                          gchar            *group_folder_path,
                                                          const gchar      *text, 
                                                          gchar            *class_symbol);
GList*  java_indexer_utils_get_package_indexes           (JavaIndexer      *indexer,
                                                          gchar            *group_folder_path,
                                                          gchar            *package_name);

G_END_DECLS

#endif /* __JAVA_INDEXER_UTILS_H__ */
