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

#ifndef __JAVA_CLASS_IMPORT_H__
#define	__JAVA_CLASS_IMPORT_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>

G_BEGIN_DECLS

#define JAVA_CLASS_IMPORT_TYPE            (java_class_import_get_type ())
#define JAVA_CLASS_IMPORT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_CLASS_IMPORT_TYPE, JavaClassImport))
#define JAVA_CLASS_IMPORT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_CLASS_IMPORT_TYPE, JavaClassImportClass))
#define IS_JAVA_CLASS_IMPORT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_CLASS_IMPORT_TYPE))
#define IS_JAVA_CLASS_IMPORT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_CLASS_IMPORT_TYPE))

typedef struct _JavaClassImport JavaClassImport;
typedef struct _JavaClassImportClass JavaClassImportClass;

struct _JavaClassImport
{
  GObject parent_instance;
};

struct _JavaClassImportClass
{
  GObjectClass parent_class;
};

GType java_class_import_get_type (void) G_GNUC_CONST;
     
JavaClassImport*  java_class_import_new  (CodeSlayer *codeslayer, 
                                          GtkWidget  *menu);
                                     
G_END_DECLS

#endif /* __JAVA_CLASS_IMPORT_H__ */
