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

#ifndef __AUTOTOOLS_NOTEBOOK_H__
#define	__AUTOTOOLS_NOTEBOOK_H__

#include <gtk/gtk.h>
#include "autotools-configuration.h"

G_BEGIN_DECLS

#define AUTOTOOLS_NOTEBOOK_TYPE            (autotools_notebook_get_type ())
#define AUTOTOOLS_NOTEBOOK(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), AUTOTOOLS_NOTEBOOK_TYPE, AutotoolsNotebook))
#define AUTOTOOLS_NOTEBOOK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), AUTOTOOLS_NOTEBOOK_TYPE, AutotoolsNotebookClass))
#define IS_AUTOTOOLS_NOTEBOOK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AUTOTOOLS_NOTEBOOK_TYPE))
#define IS_AUTOTOOLS_NOTEBOOK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), AUTOTOOLS_NOTEBOOK_TYPE))

typedef struct _AutotoolsNotebook AutotoolsNotebook;
typedef struct _AutotoolsNotebookClass AutotoolsNotebookClass;

struct _AutotoolsNotebook
{
  GtkNotebook parent_instance;
};

struct _AutotoolsNotebookClass
{
  GtkNotebookClass parent_class;
};

GType
autotools_notebook_get_type (void) G_GNUC_CONST;
  
GtkWidget*  autotools_notebook_new         (void);

void        autotools_notebook_add_output                   (AutotoolsNotebook      *notebook, 
                                                             GtkWidget              *output, 
                                                             const gchar            *label);

GtkWidget*  autotools_notebook_get_output_by_configuration  (AutotoolsNotebook      *notebook, 
                                                             AutotoolsConfiguration *configuration);

void        autotools_notebook_select_page_by_output        (AutotoolsNotebook      *notebook, 
                                                             GtkWidget              *output);

G_END_DECLS

#endif /* __AUTOTOOLS_NOTEBOOK_H__ */
