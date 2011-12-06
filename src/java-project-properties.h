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

#ifndef __AUTOTOOLS_PROJECT_PROPERTIES_H__
#define	__AUTOTOOLS_PROJECT_PROPERTIES_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>
#include "autotools-configuration.h"

G_BEGIN_DECLS

#define AUTOTOOLS_PROJECT_PROPERTIES_TYPE            (autotools_project_properties_get_type ())
#define AUTOTOOLS_PROJECT_PROPERTIES(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), AUTOTOOLS_PROJECT_PROPERTIES_TYPE, AutotoolsProjectProperties))
#define AUTOTOOLS_PROJECT_PROPERTIES_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), AUTOTOOLS_PROJECT_PROPERTIES_TYPE, AutotoolsProjectPropertiesClass))
#define IS_AUTOTOOLS_PROJECT_PROPERTIES(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AUTOTOOLS_PROJECT_PROPERTIES_TYPE))
#define IS_AUTOTOOLS_PROJECT_PROPERTIES_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), AUTOTOOLS_PROJECT_PROPERTIES_TYPE))

typedef struct _AutotoolsProjectProperties AutotoolsProjectProperties;
typedef struct _AutotoolsProjectPropertiesClass AutotoolsProjectPropertiesClass;

struct _AutotoolsProjectProperties
{
  GtkVBox parent_instance;
};

struct _AutotoolsProjectPropertiesClass
{
  GtkVBoxClass parent_class;
  
  void (*save_configuration) (AutotoolsProjectProperties *project_properties);  
};

GType autotools_project_properties_get_type (void) G_GNUC_CONST;
     
GtkWidget*  autotools_project_properties_new  (void);

void autotools_project_properties_opened      (AutotoolsProjectProperties *project_properties,
                                               AutotoolsConfiguration     *configuration, 
                                               CodeSlayerProject          *project);

void autotools_project_properties_saved       (AutotoolsProjectProperties *project_properties,
                                               AutotoolsConfiguration     *configuration, 
                                               CodeSlayerProject          *project);

G_END_DECLS

#endif /* __AUTOTOOLS_PROJECT_PROPERTIES_H__ */
