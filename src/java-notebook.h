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

#ifndef __JAVA_NOTEBOOK_H__
#define	__JAVA_NOTEBOOK_H__

#include <gtk/gtk.h>
#include "java-configuration.h"
#include "java-page.h"

G_BEGIN_DECLS

#define JAVA_NOTEBOOK_TYPE            (java_notebook_get_type ())
#define JAVA_NOTEBOOK(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_NOTEBOOK_TYPE, JavaNotebook))
#define JAVA_NOTEBOOK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_NOTEBOOK_TYPE, JavaNotebookClass))
#define IS_JAVA_NOTEBOOK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_NOTEBOOK_TYPE))
#define IS_JAVA_NOTEBOOK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_NOTEBOOK_TYPE))

typedef struct _JavaNotebook JavaNotebook;
typedef struct _JavaNotebookClass JavaNotebookClass;

struct _JavaNotebook
{
  GtkNotebook parent_instance;
};

struct _JavaNotebookClass
{
  GtkNotebookClass parent_class;
};

GType
java_notebook_get_type (void) G_GNUC_CONST;
  
GtkWidget*  java_notebook_new                  (void);

void        java_notebook_add_page             (JavaNotebook *notebook, 
                                                GtkWidget    *page, 
                                                const gchar  *label);
                                     
GtkWidget*  java_notebook_get_page_by_type     (JavaNotebook *notebook, 
                                                JavaPageType  page_type);

void        java_notebook_select_page_by_type  (JavaNotebook *notebook, 
                                                JavaPageType  page_type);

G_END_DECLS

#endif /* __JAVA_NOTEBOOK_H__ */
