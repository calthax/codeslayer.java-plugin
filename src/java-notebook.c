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

#include "autotools-notebook.h"
#include "autotools-notebook-page.h"
#include "autotools-notebook-tab.h"
#include "autotools-output.h"

static void autotools_notebook_class_init  (AutotoolsNotebookClass *klass);
static void autotools_notebook_init        (AutotoolsNotebook      *notebook);
static void autotools_notebook_finalize    (AutotoolsNotebook      *notebook);

static void close_action                   (AutotoolsNotebookTab   *notebook_tab,
                                            AutotoolsNotebook      *notebook);

#define AUTOTOOLS_NOTEBOOK_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), AUTOTOOLS_NOTEBOOK_TYPE, AutotoolsNotebookPrivate))

typedef struct _AutotoolsNotebookPrivate AutotoolsNotebookPrivate;

struct _AutotoolsNotebookPrivate
{
  gchar *foo;
};

G_DEFINE_TYPE (AutotoolsNotebook, autotools_notebook, GTK_TYPE_NOTEBOOK)

static void
autotools_notebook_class_init (AutotoolsNotebookClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) autotools_notebook_finalize;
  g_type_class_add_private (klass, sizeof (AutotoolsNotebookPrivate));
}

static void
autotools_notebook_init (AutotoolsNotebook *notebook)
{
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);
}

static void
autotools_notebook_finalize (AutotoolsNotebook *notebook)
{
  G_OBJECT_CLASS (autotools_notebook_parent_class)->finalize (G_OBJECT (notebook));
}

GtkWidget*
autotools_notebook_new (void)
{
  GtkWidget *notebook;
  notebook = g_object_new (autotools_notebook_get_type (), NULL);
  return notebook;
}

void 
autotools_notebook_add_output (AutotoolsNotebook *notebook, 
                               GtkWidget         *output,
                               const gchar       *label)
{
  GtkWidget *notebook_page;
  GtkWidget *notebook_tab;
  
  notebook_page = autotools_notebook_page_new (output);
  notebook_tab = autotools_notebook_tab_new (GTK_WIDGET (notebook), label);
  
  autotools_notebook_tab_set_notebook_page (AUTOTOOLS_NOTEBOOK_TAB (notebook_tab), 
                                            notebook_page);
                                            
  g_signal_connect (G_OBJECT (notebook_tab), "close",
                    G_CALLBACK (close_action), notebook);
                                            
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), notebook_page, notebook_tab);
  gtk_notebook_set_tab_reorderable (GTK_NOTEBOOK (notebook), notebook_page, TRUE);
  
  gtk_widget_show_all (notebook_tab);
  gtk_widget_show_all (notebook_page);  
}

static void
close_action (AutotoolsNotebookTab *notebook_tab,
              AutotoolsNotebook    *notebook)
{
  GtkWidget *notebook_page;
  gint page_num;
  
  notebook_page = autotools_notebook_tab_get_notebook_page (notebook_tab);
  page_num = gtk_notebook_page_num (GTK_NOTEBOOK (notebook),
                                    GTK_WIDGET (notebook_page));
  gtk_notebook_remove_page (GTK_NOTEBOOK (notebook), page_num);
}

GtkWidget*  
autotools_notebook_get_output_by_configuration  (AutotoolsNotebook      *notebook, 
                                                 AutotoolsConfiguration *configuration)
{
  gint pages;
  gint i;
  
  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));

  for (i = 0; i < pages; i++)
    {
      GtkWidget *notebook_page;
      GtkWidget *output;
      notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), i);
      output = autotools_notebook_page_get_output (AUTOTOOLS_NOTEBOOK_PAGE (notebook_page));      
      if (autotools_output_get_configuration (AUTOTOOLS_OUTPUT (output)) == configuration)
        return output;
    }
    
  return NULL;    
}                                                 

void        
autotools_notebook_select_page_by_output (AutotoolsNotebook *notebook, 
                                          GtkWidget         *output)
{
  gint pages;
  gint i;
  
  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));

  for (i = 0; i < pages; i++)
    {
      GtkWidget *notebook_page;
      notebook_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), i);
      if (autotools_notebook_page_get_output (AUTOTOOLS_NOTEBOOK_PAGE (notebook_page)) == output)
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), i);
    }
}                                        
