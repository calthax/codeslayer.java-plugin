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

#ifndef __JAVA_INDEXER_METHOD_H__
#define	__JAVA_INDEXER_METHOD_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define JAVA_INDEXER_METHOD_TYPE            (java_indexer_method_get_type ())
#define JAVA_INDEXER_METHOD(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_INDEXER_METHOD_TYPE, JavaIndexerMethod))
#define JAVA_INDEXER_METHOD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_INDEXER_METHOD_TYPE, JavaIndexerMethodClass))
#define IS_JAVA_INDEXER_METHOD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_INDEXER_METHOD_TYPE))
#define IS_JAVA_INDEXER_METHOD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_INDEXER_METHOD_TYPE))

typedef struct _JavaIndexerMethod JavaIndexerMethod;
typedef struct _JavaIndexerMethodClass JavaIndexerMethodClass;

struct _JavaIndexerMethod
{
  GObject parent_instance;
};

struct _JavaIndexerMethodClass
{
  GObjectClass parent_class;
};

GType java_indexer_method_get_type (void) G_GNUC_CONST;

JavaIndexerMethod*  java_indexer_method_new               (void);

const gchar*        java_indexer_method_get_name          (JavaIndexerMethod *method);
void                java_indexer_method_set_name          (JavaIndexerMethod *method,
                                                           const gchar       *name);
const gchar*        java_indexer_method_get_parameters    (JavaIndexerMethod *method);
void                java_indexer_method_set_parameters    (JavaIndexerMethod *method,
                                                           const gchar       *parameters);
const gchar*        java_indexer_method_get_modifier      (JavaIndexerMethod *method);
void                java_indexer_method_set_modifier      (JavaIndexerMethod *method,
                                                           const gchar       *modifier);
const gchar*        java_indexer_method_get_class_name    (JavaIndexerMethod *method);
void                java_indexer_method_set_class_name    (JavaIndexerMethod *method,
                                                           const gchar       *class_name);
const gchar*        java_indexer_method_get_package_name  (JavaIndexerMethod *method);
void                java_indexer_method_set_package_name  (JavaIndexerMethod *method,
                                                           const gchar       *package_name);
const gchar*        java_indexer_method_get_file_path     (JavaIndexerMethod *method);
void                java_indexer_method_set_file_path     (JavaIndexerMethod *method,
                                                           const gchar       *file_path);
const gint          java_indexer_method_get_line_number   (JavaIndexerMethod *method);
void                java_indexer_method_set_line_number   (JavaIndexerMethod *method,
                                                           gint               line_number);

G_END_DECLS

#endif /* __JAVA_INDEXER_METHOD_H__ */
