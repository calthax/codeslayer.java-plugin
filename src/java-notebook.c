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

#include "java-notebook.h"
#include "java-notebook-tab.h"

static void java_notebook_class_init  (JavaNotebookClass *klass);
static void java_notebook_init        (JavaNotebook      *notebook);
static void java_notebook_finalize    (JavaNotebook      *notebook);

static void close_action              (JavaNotebookTab   *notebook_tab,
                                       JavaNotebook      *notebook);

#define JAVA_NOTEBOOK_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_NOTEBOOK_TYPE, JavaNotebookPrivate))

typedef struct _JavaNotebookPrivate JavaNotebookPrivate;

struct _JavaNotebookPrivate
{
  gchar *foo;
};

G_DEFINE_TYPE (JavaNotebook, java_notebook, GTK_TYPE_NOTEBOOK)

static void
java_notebook_class_init (JavaNotebookClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) java_notebook_finalize;
  g_type_class_add_private (klass, sizeof (JavaNotebookPrivate));
}

static void
java_notebook_init (JavaNotebook *notebook)
{
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);
}

static void
java_notebook_finalize (JavaNotebook *notebook)
{
  G_OBJECT_CLASS (java_notebook_parent_class)->finalize (G_OBJECT (notebook));
}

GtkWidget*
java_notebook_new (void)
{
  GtkWidget *notebook;
  notebook = g_object_new (java_notebook_get_type (), NULL);
  return notebook;
}

void 
java_notebook_add_page (JavaNotebook *notebook, 
                        GtkWidget    *notebook_page,
                        const gchar  *label)
{
  GtkWidget *notebook_tab;
  
  notebook_tab = java_notebook_tab_new (GTK_WIDGET (notebook), label);
  
  java_notebook_tab_set_notebook_page (JAVA_NOTEBOOK_TAB (notebook_tab), 
                                       notebook_page);
                                            
  g_signal_connect (G_OBJECT (notebook_tab), "close",
                    G_CALLBACK (close_action), notebook);
                                            
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), notebook_page, notebook_tab);
  gtk_notebook_set_tab_reorderable (GTK_NOTEBOOK (notebook), notebook_page, TRUE);
  
  gtk_widget_show_all (notebook_tab);
  gtk_widget_show_all (notebook_page);  
}

static void
close_action (JavaNotebookTab *notebook_tab,
              JavaNotebook    *notebook)
{
  GtkWidget *notebook_page;
  gint page_num;
  
  notebook_page = java_notebook_tab_get_notebook_page (notebook_tab);
  page_num = gtk_notebook_page_num (GTK_NOTEBOOK (notebook),
                                    GTK_WIDGET (notebook_page));
  gtk_notebook_remove_page (GTK_NOTEBOOK (notebook), page_num);
}

GtkWidget*  
java_notebook_get_page_by_type (JavaNotebook *notebook, 
                                JavaPageType  page_type)
{
  gint pages;
  gint i;
  
  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));

  for (i = 0; i < pages; i++)
    {
      GtkWidget *notebook_page;
      notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), i);
      if (java_page_get_page_type (JAVA_PAGE (notebook_page)) == page_type)
        return notebook_page;
    }
    
  return NULL;
}

void        
java_notebook_select_page_by_type (JavaNotebook *notebook, 
                                   JavaPageType  page_type)
{
  gint pages;
  gint i;
  
  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));

  for (i = 0; i < pages; i++)
    {
      GtkWidget *notebook_page;
      notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), i);
      if (java_page_get_page_type (JAVA_PAGE (notebook_page)) == page_type)
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), i);
    }
}
