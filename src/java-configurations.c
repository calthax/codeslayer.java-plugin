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

#include "java-configurations.h"
#include <codeslayer/codeslayer-utils.h>

static void java_configurations_class_init  (JavaConfigurationsClass *klass);
static void java_configurations_init        (JavaConfigurations      *configurations);
static void java_configurations_finalize    (JavaConfigurations      *configurations);
static gchar* get_configuration_file_path   (JavaConfigurations      *configurations);

#define JAVA_CONFIGURATIONS_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_CONFIGURATIONS_TYPE, JavaConfigurationsPrivate))

typedef struct _JavaConfigurationsPrivate JavaConfigurationsPrivate;

struct _JavaConfigurationsPrivate
{
  CodeSlayer *codeslayer;
  GList *list;
};

G_DEFINE_TYPE (JavaConfigurations, java_configurations, G_TYPE_OBJECT)
     
static void 
java_configurations_class_init (JavaConfigurationsClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) java_configurations_finalize;
  g_type_class_add_private (klass, sizeof (JavaConfigurationsPrivate));
}

static void
java_configurations_init (JavaConfigurations *configurations)
{
  JavaConfigurationsPrivate *priv;
  priv = JAVA_CONFIGURATIONS_GET_PRIVATE (configurations);
  priv->list = NULL;
}

static void
java_configurations_finalize (JavaConfigurations *configurations)
{
  JavaConfigurationsPrivate *priv;
  priv = JAVA_CONFIGURATIONS_GET_PRIVATE (configurations);
  if (priv->list != NULL)
    {
      g_list_foreach (priv->list, (GFunc) g_object_unref, NULL);
      g_list_free (priv->list);
      priv->list = NULL;    
    }
  G_OBJECT_CLASS (java_configurations_parent_class)->finalize (G_OBJECT (configurations));
}

JavaConfigurations*
java_configurations_new (CodeSlayer *codeslayer)
{
  JavaConfigurationsPrivate *priv;
  JavaConfigurations *configurations;

  configurations = JAVA_CONFIGURATIONS  (g_object_new (java_configurations_get_type (), NULL));
  priv = JAVA_CONFIGURATIONS_GET_PRIVATE (configurations);

  priv->codeslayer = codeslayer;

  return configurations;
}

void 
java_configurations_load (JavaConfigurations *configurations)
{
  JavaConfigurationsPrivate *priv;
  GList *list;
  gchar *file_path;

  priv = JAVA_CONFIGURATIONS_GET_PRIVATE (configurations);
  
  file_path = get_configuration_file_path (configurations);
  list = codeslayer_utils_get_gobjects (JAVA_CONFIGURATION_TYPE,
                                        FALSE,
                                        file_path, 
                                        "java",
                                        "project_key", G_TYPE_STRING,
                                        "ant_file", G_TYPE_STRING, 
                                        "build_folder", G_TYPE_STRING, 
                                        "lib_folder", G_TYPE_STRING, 
                                        "source_folder", G_TYPE_STRING, 
                                        "test_folder", G_TYPE_STRING, 
                                        NULL);
  priv->list = list;
  g_free (file_path);
}

JavaConfiguration*
java_configurations_find_configuration (JavaConfigurations *configurations, 
                                        const gchar        *project_key)
{
  JavaConfigurationsPrivate *priv;
  GList *list;

  priv = JAVA_CONFIGURATIONS_GET_PRIVATE (configurations);

  list = priv->list;
  while (list != NULL)
    {
      JavaConfiguration *configuration = list->data;
      const gchar *key;
      
      key = java_configuration_get_project_key (configuration);
      
      if (g_strcmp0 (project_key, key) == 0)
        return configuration;

      list = g_list_next (list);
    }

  return NULL;
}

void
java_configurations_save (JavaConfigurations *configurations,
                          JavaConfiguration  *configuration)
{
  JavaConfigurationsPrivate *priv;
  GList *list;
  GList *tmp;
  gchar *file_path;
    
  priv = JAVA_CONFIGURATIONS_GET_PRIVATE (configurations);
  
  if (configuration)
    priv->list = g_list_prepend (priv->list, configuration);

  list = g_list_copy (priv->list);
  tmp = list;
  
  while (tmp != NULL)
    {
      JavaConfiguration *configuration = tmp->data;
      const gchar *ant_file;
      const gchar *build_folder;

      ant_file = java_configuration_get_ant_file (configuration);
      build_folder = java_configuration_get_build_folder (configuration);
      
      if (g_utf8_strlen (ant_file, -1) == 0 &&
          g_utf8_strlen (build_folder, -1) == 0)
        priv->list = g_list_remove (priv->list, configuration);
      tmp = g_list_next (tmp);
    }
    
  g_list_free (list);    
  
  file_path = get_configuration_file_path (configurations);  
  codeslayer_utils_save_gobjects (priv->list,
                                  file_path, 
                                  "java",
                                  "project_key", G_TYPE_STRING,
                                  "ant_file", G_TYPE_STRING, 
                                  "build_folder", G_TYPE_STRING, 
                                  "lib_folder", G_TYPE_STRING, 
                                  "source_folder", G_TYPE_STRING, 
                                  "test_folder", G_TYPE_STRING, 
                                  NULL);  
  g_free (file_path);
}

static gchar*
get_configuration_file_path (JavaConfigurations *configurations)
{
  JavaConfigurationsPrivate *priv;
  gchar *folder_path;
  gchar *file_path;
  
  priv = JAVA_CONFIGURATIONS_GET_PRIVATE (configurations);

  folder_path = codeslayer_get_active_group_folder_path (priv->codeslayer);  
  file_path = g_build_filename (folder_path, "java.xml", NULL);
  g_free (folder_path);
  
  return file_path;
}
