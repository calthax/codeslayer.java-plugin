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

#ifndef __JAVA_USAGE_METHOD_H__
#define	__JAVA_USAGE_METHOD_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define JAVA_USAGE_METHOD_TYPE            (java_usage_method_get_type ())
#define JAVA_USAGE_METHOD(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_USAGE_METHOD_TYPE, JavaUsageMethod))
#define JAVA_USAGE_METHOD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_USAGE_METHOD_TYPE, JavaUsageMethodClass))
#define IS_JAVA_USAGE_METHOD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_USAGE_METHOD_TYPE))
#define IS_JAVA_USAGE_METHOD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_USAGE_METHOD_TYPE))

typedef struct _JavaUsageMethod JavaUsageMethod;
typedef struct _JavaUsageMethodClass JavaUsageMethodClass;

struct _JavaUsageMethod
{
  GObject parent_instance;
};

struct _JavaUsageMethodClass
{
  GObjectClass parent_class;
};

GType java_usage_method_get_type (void) G_GNUC_CONST;

JavaUsageMethod*  java_usage_method_new                (void);

const gchar*  java_usage_method_get_class_name   (JavaUsageMethod *method);
void          java_usage_method_set_class_name   (JavaUsageMethod *method,
                                                  const gchar     *class_name);
const gchar*  java_usage_method_get_file_path    (JavaUsageMethod *method);
void          java_usage_method_set_file_path    (JavaUsageMethod *method,
                                                  const gchar     *file_path);
const gint    java_usage_method_get_line_number  (JavaUsageMethod *method);
void          java_usage_method_set_line_number  (JavaUsageMethod *method,
                                                  gint             line_number);

G_END_DECLS

#endif /* __JAVA_USAGE_METHOD_H__ */
