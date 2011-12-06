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

#ifndef __AUTOTOOLS_NOTEBOOK_TAB_H__
#define	__AUTOTOOLS_NOTEBOOK_TAB_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define AUTOTOOLS_NOTEBOOK_TAB_TYPE            (autotools_notebook_tab_get_type ())
#define AUTOTOOLS_NOTEBOOK_TAB(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), AUTOTOOLS_NOTEBOOK_TAB_TYPE, AutotoolsNotebookTab))
#define AUTOTOOLS_NOTEBOOK_TAB_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), AUTOTOOLS_NOTEBOOK_TAB_TYPE, AutotoolsNotebookTabClass))
#define IS_AUTOTOOLS_NOTEBOOK_TAB(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AUTOTOOLS_NOTEBOOK_TAB_TYPE))
#define IS_AUTOTOOLS_NOTEBOOK_TAB_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), AUTOTOOLS_NOTEBOOK_TAB_TYPE))

typedef struct _AutotoolsNotebookTab AutotoolsNotebookTab;
typedef struct _AutotoolsNotebookTabClass AutotoolsNotebookTabClass;

struct _AutotoolsNotebookTab
{
  GtkHBox parent_instance;
};

struct _AutotoolsNotebookTabClass
{
  GtkHBoxClass parent_class;

  void (*close) (AutotoolsNotebookTab *notebook_tab);
};

GType autotools_notebook_tab_get_type (void) G_GNUC_CONST;
  
GtkWidget*  autotools_notebook_tab_new                (GtkWidget             *notebook, 
                                                       const gchar           *label);

GtkWidget*  autotools_notebook_tab_get_notebook_page  (AutotoolsNotebookTab  *notebook_tab);
void        autotools_notebook_tab_set_notebook_page  (AutotoolsNotebookTab  *notebook_tab, 
                                                       GtkWidget             *notebook_page);

G_END_DECLS

#endif /* __AUTOTOOLS_NOTEBOOK_TAB_H__ */
