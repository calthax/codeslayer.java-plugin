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

#ifndef __JAVA_CONFIGURATIONS_H__
#define	__JAVA_CONFIGURATIONS_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>
#include "java-configuration.h"

G_BEGIN_DECLS

#define JAVA_CONFIGURATIONS_TYPE            (java_configurations_get_type ())
#define JAVA_CONFIGURATIONS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_CONFIGURATIONS_TYPE, JavaConfigurations))
#define JAVA_CONFIGURATIONS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_CONFIGURATIONS_TYPE, JavaConfigurationsClass))
#define IS_JAVA_CONFIGURATIONS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_CONFIGURATIONS_TYPE))
#define IS_JAVA_CONFIGURATIONS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_CONFIGURATIONS_TYPE))

typedef struct _JavaConfigurations JavaConfigurations;
typedef struct _JavaConfigurationsClass JavaConfigurationsClass;

struct _JavaConfigurations
{
  GObject parent_instance;
};

struct _JavaConfigurationsClass
{
  GObjectClass parent_class;
};

GType java_configurations_get_type (void) G_GNUC_CONST;

JavaConfigurations*  java_configurations_new                 (CodeSlayer         *codeslayer);


void                 java_configurations_load                (JavaConfigurations *configurations);

JavaConfiguration*   java_configurations_find_configuration  (JavaConfigurations *configurations,
                                                              const gchar        *project_key);
void                 java_configurations_save                (JavaConfigurations *configurations,
                                                              JavaConfiguration  *configuration);

G_END_DECLS

#endif /* __JAVA_CONFIGURATIONS_H__ */
