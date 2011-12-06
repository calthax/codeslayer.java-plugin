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

#ifndef __AUTOTOOLS_NOTEBOOK_PAGE_H__
#define	__AUTOTOOLS_NOTEBOOK_PAGE_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-document.h>

G_BEGIN_DECLS

#define AUTOTOOLS_NOTEBOOK_PAGE_TYPE            (autotools_notebook_page_get_type ())
#define AUTOTOOLS_NOTEBOOK_PAGE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), AUTOTOOLS_NOTEBOOK_PAGE_TYPE, AutotoolsNotebookPage))
#define AUTOTOOLS_NOTEBOOK_PAGE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), AUTOTOOLS_NOTEBOOK_PAGE_TYPE, AutotoolsNotebookPageClass))
#define IS_AUTOTOOLS_NOTEBOOK_PAGE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AUTOTOOLS_NOTEBOOK_PAGE_TYPE))
#define IS_AUTOTOOLS_NOTEBOOK_PAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), AUTOTOOLS_NOTEBOOK_PAGE_TYPE))

typedef struct _AutotoolsNotebookPage AutotoolsNotebookPage;
typedef struct _AutotoolsNotebookPageClass AutotoolsNotebookPageClass;

struct _AutotoolsNotebookPage
{
  GtkHBox parent_instance;
};

struct _AutotoolsNotebookPageClass
{
  GtkHBoxClass parent_class;
};

GType autotools_notebook_page_get_type (void) G_GNUC_CONST;
     
GtkWidget*  autotools_notebook_page_new         (GtkWidget             *output);

GtkWidget*  autotools_notebook_page_get_output  (AutotoolsNotebookPage *notebook_page);

G_END_DECLS

#endif /* __AUTOTOOLS_NOTEBOOK_PAGE_H__ */
