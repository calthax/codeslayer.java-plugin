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

#ifndef __JAVA_INDEXER_INDEX_H__
#define	__JAVA_INDEXER_INDEX_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define JAVA_INDEXER_INDEX_TYPE            (java_indexer_index_get_type ())
#define JAVA_INDEXER_INDEX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_INDEXER_INDEX_TYPE, JavaIndexerIndex))
#define JAVA_INDEXER_INDEX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_INDEXER_INDEX_TYPE, JavaIndexerIndexClass))
#define IS_JAVA_INDEXER_INDEX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_INDEXER_INDEX_TYPE))
#define IS_JAVA_INDEXER_INDEX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_INDEXER_INDEX_TYPE))

typedef struct _JavaIndexerIndex JavaIndexerIndex;
typedef struct _JavaIndexerIndexClass JavaIndexerIndexClass;

struct _JavaIndexerIndex
{
  GObject parent_instance;
};

struct _JavaIndexerIndexClass
{
  GObjectClass parent_class;
};

GType java_indexer_index_get_type (void) G_GNUC_CONST;

JavaIndexerIndex*  java_indexer_index_new                     (void);

const gchar*        java_indexer_index_get_name               (JavaIndexerIndex *index);
void                java_indexer_index_set_name               (JavaIndexerIndex *index,
                                                               const gchar      *name);
const gchar*        java_indexer_index_get_parameters         (JavaIndexerIndex *index);
void                java_indexer_index_set_parameters         (JavaIndexerIndex *index,
                                                               const gchar      *parameters);
const gchar*        java_indexer_index_get_modifier           (JavaIndexerIndex *index);
void                java_indexer_index_set_modifier           (JavaIndexerIndex *index,
                                                               const gchar       *modifier);
const gchar*        java_indexer_index_get_class_name         (JavaIndexerIndex *index);
void                java_indexer_index_set_class_name         (JavaIndexerIndex *index,
                                                               const gchar      *class_name);
const gchar*        java_indexer_index_get_package_name       (JavaIndexerIndex *index);
gchar*              java_indexer_index_get_full_package_name  (JavaIndexerIndex *index);
void                java_indexer_index_set_package_name       (JavaIndexerIndex *index,
                                                               const gchar      *package_name);
const gchar*        java_indexer_index_get_file_path          (JavaIndexerIndex *index);
void                java_indexer_index_set_file_path          (JavaIndexerIndex *index,
                                                               const gchar      *file_path);
const gint          java_indexer_index_get_line_number        (JavaIndexerIndex *index);
void                java_indexer_index_set_line_number        (JavaIndexerIndex *index,
                                                               gint              line_number);

G_END_DECLS

#endif /* __JAVA_INDEXER_INDEX_H__ */
