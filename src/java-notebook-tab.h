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

#ifndef __JAVA_NOTEBOOK_TAB_H__
#define	__JAVA_NOTEBOOK_TAB_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define JAVA_NOTEBOOK_TAB_TYPE            (java_notebook_tab_get_type ())
#define JAVA_NOTEBOOK_TAB(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_NOTEBOOK_TAB_TYPE, JavaNotebookTab))
#define JAVA_NOTEBOOK_TAB_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_NOTEBOOK_TAB_TYPE, JavaNotebookTabClass))
#define IS_JAVA_NOTEBOOK_TAB(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_NOTEBOOK_TAB_TYPE))
#define IS_JAVA_NOTEBOOK_TAB_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_NOTEBOOK_TAB_TYPE))

typedef struct _JavaNotebookTab JavaNotebookTab;
typedef struct _JavaNotebookTabClass JavaNotebookTabClass;

struct _JavaNotebookTab
{
  GtkHBox parent_instance;
};

struct _JavaNotebookTabClass
{
  GtkHBoxClass parent_class;

  void (*close) (JavaNotebookTab *notebook_tab);
};

GType java_notebook_tab_get_type (void) G_GNUC_CONST;
  
GtkWidget*  java_notebook_tab_new                (GtkWidget        *notebook, 
                                                  const gchar      *label);

GtkWidget*  java_notebook_tab_get_notebook_page  (JavaNotebookTab  *notebook_tab);
void        java_notebook_tab_set_notebook_page  (JavaNotebookTab  *notebook_tab, 
                                                  GtkWidget        *notebook_page);

G_END_DECLS

#endif /* __JAVA_NOTEBOOK_TAB_H__ */
