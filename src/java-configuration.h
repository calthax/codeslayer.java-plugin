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

#ifndef __JAVA_CONFIGURATION_H__
#define	__JAVA_CONFIGURATION_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define JAVA_CONFIGURATION_TYPE            (java_configuration_get_type ())
#define JAVA_CONFIGURATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_CONFIGURATION_TYPE, JavaConfiguration))
#define JAVA_CONFIGURATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_CONFIGURATION_TYPE, JavaConfigurationClass))
#define IS_JAVA_CONFIGURATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_CONFIGURATION_TYPE))
#define IS_JAVA_CONFIGURATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_CONFIGURATION_TYPE))

typedef struct _JavaConfiguration JavaConfiguration;
typedef struct _JavaConfigurationClass JavaConfigurationClass;

struct _JavaConfiguration
{
  GObject parent_instance;
};

struct _JavaConfigurationClass
{
  GObjectClass parent_class;
};

GType java_configuration_get_type (void) G_GNUC_CONST;

JavaConfiguration*  java_configuration_new                (void);

const gchar*        java_configuration_get_project_key    (JavaConfiguration *configuration);
void                java_configuration_set_project_key    (JavaConfiguration *configuration,
                                                           const gchar       *project_key);
const gchar*        java_configuration_get_ant_file       (JavaConfiguration *configuration);
void                java_configuration_set_ant_file       (JavaConfiguration *configuration,
                                                           const gchar       *ant_file);
const gchar*        java_configuration_get_build_folder   (JavaConfiguration *configuration);
void                java_configuration_set_build_folder   (JavaConfiguration *configuration,
                                                           const gchar       *build_folder);
const gchar*        java_configuration_get_lib_folder     (JavaConfiguration *configuration);
void                java_configuration_set_lib_folder     (JavaConfiguration *configuration,
                                                           const gchar       *lib_folder);
const gchar*        java_configuration_get_source_folder  (JavaConfiguration *configuration);
void                java_configuration_set_source_folder  (JavaConfiguration *configuration,
                                                           const gchar       *source_folder);
const gchar*        java_configuration_get_test_folder    (JavaConfiguration *configuration);
void                java_configuration_set_test_folder    (JavaConfiguration *configuration,
                                                           const gchar       *test_folder);

G_END_DECLS

#endif /* __JAVA_CONFIGURATION_H__ */
