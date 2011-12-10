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

#ifndef __JAVA_NOTEBOOK_PAGE_H__
#define	__JAVA_NOTEBOOK_PAGE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define JAVA_NOTEBOOK_PAGE_TYPE            (java_notebook_page_get_type ())
#define JAVA_NOTEBOOK_PAGE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_NOTEBOOK_PAGE_TYPE, JavaNotebookPage))
#define JAVA_NOTEBOOK_PAGE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_NOTEBOOK_PAGE_TYPE, JavaNotebookPageClass))
#define IS_JAVA_NOTEBOOK_PAGE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_NOTEBOOK_PAGE_TYPE))
#define IS_JAVA_NOTEBOOK_PAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_NOTEBOOK_PAGE_TYPE))

typedef struct _JavaNotebookPage JavaNotebookPage;
typedef struct _JavaNotebookPageClass JavaNotebookPageClass;

struct _JavaNotebookPage
{
  GtkHBox parent_instance;
};

struct _JavaNotebookPageClass
{
  GtkHBoxClass parent_class;
};

GType java_notebook_page_get_type (void) G_GNUC_CONST;
     
GtkWidget*  java_notebook_page_new         (GtkWidget             *output);

GtkWidget*  java_notebook_page_get_output  (JavaNotebookPage *notebook_page);

G_END_DECLS

#endif /* __JAVA_NOTEBOOK_PAGE_H__ */
