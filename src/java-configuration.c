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

#include "java-configuration.h"

static void java_configuration_class_init    (JavaConfigurationClass *klass);
static void java_configuration_init          (JavaConfiguration      *configuration);
static void java_configuration_finalize      (JavaConfiguration      *configuration);
static void java_configuration_get_property  (GObject                *object, 
                                              guint                   prop_id,
                                              GValue                 *value,
                                              GParamSpec             *pspec);
static void java_configuration_set_property  (GObject                *object, 
                                              guint                   prop_id,
                                              const GValue           *value,
                                              GParamSpec             *pspec);

#define JAVA_CONFIGURATION_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_CONFIGURATION_TYPE, JavaConfigurationPrivate))

typedef struct _JavaConfigurationPrivate JavaConfigurationPrivate;

struct _JavaConfigurationPrivate
{
  gchar *project_key;
  gchar *ant_file;
  gchar *build_folder;
  gchar *lib_folder;
  gchar *source_folder;
  gchar *test_folder;
};

enum
{
  PROP_0,
  PROP_PROJECT_KEY,
  PROP_ANT_FILE,
  PROP_BUILD_FOLDER,
  PROP_LIB_FOLDER,
  PROP_SOURCE_FOLDER,
  PROP_TEST_FOLDER
};

G_DEFINE_TYPE (JavaConfiguration, java_configuration, G_TYPE_OBJECT)
     
static void 
java_configuration_class_init (JavaConfigurationClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = (GObjectFinalizeFunc) java_configuration_finalize;

  gobject_class->get_property = java_configuration_get_property;
  gobject_class->set_property = java_configuration_set_property;

  g_type_class_add_private (klass, sizeof (JavaConfigurationPrivate));

  g_object_class_install_property (gobject_class, 
                                   PROP_PROJECT_KEY,
                                   g_param_spec_string ("project_key", 
                                                        "Project Key",
                                                        "Project Key", "",
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, 
                                   PROP_ANT_FILE,
                                   g_param_spec_string ("ant_file",
                                                        "Ant File",
                                                        "Ant File", "",
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, 
                                   PROP_BUILD_FOLDER,
                                   g_param_spec_string ("build_folder",
                                                        "Build Folder",
                                                        "Build Folder", "",
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, 
                                   PROP_LIB_FOLDER,
                                   g_param_spec_string ("lib_folder",
                                                        "Lib Folder",
                                                        "Lib Folder", "",
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, 
                                   PROP_SOURCE_FOLDER,
                                   g_param_spec_string ("source_folder",
                                                        "Source Folder",
                                                        "Source Folder", "",
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, 
                                   PROP_TEST_FOLDER,
                                   g_param_spec_string ("test_folder",
                                                        "Test Path",
                                                        "Test Path", "",
                                                        G_PARAM_READWRITE));
}

static void
java_configuration_init (JavaConfiguration *configuration)
{
  JavaConfigurationPrivate *priv;
  priv = JAVA_CONFIGURATION_GET_PRIVATE (configuration);
  priv->project_key = NULL;
  priv->ant_file = NULL;
  priv->build_folder = NULL;
  priv->lib_folder = NULL;
  priv->source_folder = NULL;
  priv->test_folder = NULL;
}

static void
java_configuration_finalize (JavaConfiguration *configuration)
{
  JavaConfigurationPrivate *priv;
  priv = JAVA_CONFIGURATION_GET_PRIVATE (configuration);
  if (priv->project_key)
    {
      g_free (priv->project_key);
      priv->project_key = NULL;
    }
  if (priv->ant_file)
    {
      g_free (priv->ant_file);
      priv->ant_file = NULL;
    }
  if (priv->build_folder)
    {
      g_free (priv->build_folder);
      priv->build_folder = NULL;
    }
  if (priv->lib_folder)
    {
      g_free (priv->lib_folder);
      priv->lib_folder = NULL;
    }
  if (priv->source_folder)
    {
      g_free (priv->source_folder);
      priv->source_folder = NULL;
    }
  if (priv->test_folder)
    {
      g_free (priv->test_folder);
      priv->test_folder = NULL;
    }
  G_OBJECT_CLASS (java_configuration_parent_class)->finalize (G_OBJECT (configuration));
}

static void
java_configuration_get_property (GObject    *object, 
                                      guint       prop_id,
                                      GValue     *value, 
                                      GParamSpec *pspec)
{
  JavaConfiguration *configuration;
  JavaConfigurationPrivate *priv;
  
  configuration = JAVA_CONFIGURATION (object);
  priv = JAVA_CONFIGURATION_GET_PRIVATE (configuration);

  switch (prop_id)
    {
    case PROP_PROJECT_KEY:
      g_value_set_string (value, priv->project_key);
      break;
    case PROP_ANT_FILE:
      g_value_set_string (value, priv->ant_file);
      break;
    case PROP_BUILD_FOLDER:
      g_value_set_string (value, priv->build_folder);
      break;
    case PROP_LIB_FOLDER:
      g_value_set_string (value, priv->lib_folder);
      break;
    case PROP_SOURCE_FOLDER:
      g_value_set_string (value, priv->source_folder);
      break;
    case PROP_TEST_FOLDER:
      g_value_set_string (value, priv->test_folder);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
java_configuration_set_property (GObject      *object, 
                                      guint         prop_id,
                                      const GValue *value, 
                                      GParamSpec   *pspec)
{
  JavaConfiguration *configuration;
  configuration = JAVA_CONFIGURATION (object);

  switch (prop_id)
    {
    case PROP_PROJECT_KEY:
      java_configuration_set_project_key (configuration, g_value_get_string (value));
      break;
    case PROP_ANT_FILE:
      java_configuration_set_ant_file (configuration, g_value_get_string (value));
      break;
    case PROP_BUILD_FOLDER:
      java_configuration_set_build_folder (configuration, g_value_get_string (value));
      break;
    case PROP_LIB_FOLDER:
      java_configuration_set_lib_folder (configuration, g_value_get_string (value));
      break;
    case PROP_SOURCE_FOLDER:
      java_configuration_set_source_folder (configuration, g_value_get_string (value));
      break;
    case PROP_TEST_FOLDER:
      java_configuration_set_test_folder (configuration, g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

JavaConfiguration*
java_configuration_new (void)
{
  return JAVA_CONFIGURATION (g_object_new (java_configuration_get_type (), NULL));
}

const gchar*
java_configuration_get_project_key (JavaConfiguration *configuration)
{
  return JAVA_CONFIGURATION_GET_PRIVATE (configuration)->project_key;
}

void
java_configuration_set_project_key (JavaConfiguration *configuration, 
                                    const gchar       *project_key)
{
  JavaConfigurationPrivate *priv;
  priv = JAVA_CONFIGURATION_GET_PRIVATE (configuration);
  if (priv->project_key)
    {
      g_free (priv->project_key);
      priv->project_key = NULL;
    }
  priv->project_key = g_strdup (project_key);
}

const gchar*
java_configuration_get_ant_file (JavaConfiguration *configuration)
{
  return JAVA_CONFIGURATION_GET_PRIVATE (configuration)->ant_file;
}

void
java_configuration_set_ant_file (JavaConfiguration *configuration,
                                 const gchar       *ant_file)
{
  JavaConfigurationPrivate *priv;
  priv = JAVA_CONFIGURATION_GET_PRIVATE (configuration);
  if (priv->ant_file)
    {
      g_free (priv->ant_file);
      priv->ant_file = NULL;
    }
  priv->ant_file = g_strdup (ant_file);
}

const gchar*
java_configuration_get_build_folder (JavaConfiguration *configuration)
{
  return JAVA_CONFIGURATION_GET_PRIVATE (configuration)->build_folder;
}

void
java_configuration_set_build_folder (JavaConfiguration *configuration,
                                     const gchar       *build_folder)
{
  JavaConfigurationPrivate *priv;
  priv = JAVA_CONFIGURATION_GET_PRIVATE (configuration);
  if (priv->build_folder)
    {
      g_free (priv->build_folder);
      priv->build_folder = NULL;
    }
  priv->build_folder = g_strdup (build_folder);
}

const gchar*
java_configuration_get_lib_folder (JavaConfiguration *configuration)
{
  return JAVA_CONFIGURATION_GET_PRIVATE (configuration)->lib_folder;
}

void
java_configuration_set_lib_folder (JavaConfiguration *configuration,
                                   const gchar       *lib_folder)
{
  JavaConfigurationPrivate *priv;
  priv = JAVA_CONFIGURATION_GET_PRIVATE (configuration);
  if (priv->lib_folder)
    {
      g_free (priv->lib_folder);
      priv->lib_folder = NULL;
    }
  priv->lib_folder = g_strdup (lib_folder);
}

const gchar*
java_configuration_get_source_folder (JavaConfiguration *configuration)
{
  return JAVA_CONFIGURATION_GET_PRIVATE (configuration)->source_folder;
}

void
java_configuration_set_source_folder (JavaConfiguration *configuration,
                                    const gchar       *source_folder)
{
  JavaConfigurationPrivate *priv;
  priv = JAVA_CONFIGURATION_GET_PRIVATE (configuration);
  if (priv->source_folder)
    {
      g_free (priv->source_folder);
      priv->source_folder = NULL;
    }
  priv->source_folder = g_strdup (source_folder);
}

const gchar*
java_configuration_get_test_folder (JavaConfiguration *configuration)
{
  return JAVA_CONFIGURATION_GET_PRIVATE (configuration)->test_folder;
}

void
java_configuration_set_test_folder (JavaConfiguration *configuration,
                                  const gchar       *test_folder)
{
  JavaConfigurationPrivate *priv;
  priv = JAVA_CONFIGURATION_GET_PRIVATE (configuration);
  if (priv->test_folder)
    {
      g_free (priv->test_folder);
      priv->test_folder = NULL;
    }
  priv->test_folder = g_strdup (test_folder);
}
