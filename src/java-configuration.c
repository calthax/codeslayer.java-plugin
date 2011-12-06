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
  gchar *build_file;
  gchar *build_directory;
};

enum
{
  PROP_0,
  PROP_PROJECT_KEY,
  PROP_BUILD_FILE,
  PROP_BUILD_DIRECTORY
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
                                                        "Project Key Object", "",
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, 
                                   PROP_BUILD_FILE,
                                   g_param_spec_string ("build_file",
                                                        "Build File",
                                                        "Build File Object",
                                                        "",
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, 
                                   PROP_BUILD_DIRECTORY,
                                   g_param_spec_string ("build_directory",
                                                        "Build Directory",
                                                        "Build Directory Object",
                                                        "",
                                                        G_PARAM_READWRITE));
}

static void
java_configuration_init (JavaConfiguration *configuration)
{
  JavaConfigurationPrivate *priv;
  priv = JAVA_CONFIGURATION_GET_PRIVATE (configuration);
  priv->project_key = NULL;
  priv->build_file = NULL;
  priv->build_directory = NULL;
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
  if (priv->build_file)
    {
      g_free (priv->build_file);
      priv->build_file = NULL;
    }
  if (priv->build_directory)
    {
      g_free (priv->build_directory);
      priv->build_directory = NULL;
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
    case PROP_BUILD_FILE:
      g_value_set_string (value, priv->build_file);
      break;
    case PROP_BUILD_DIRECTORY:
      g_value_set_string (value, priv->build_directory);
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
    case PROP_BUILD_FILE:
      java_configuration_set_build_file (configuration, g_value_get_string (value));
      break;
    case PROP_BUILD_DIRECTORY:
      java_configuration_set_build_directory (configuration, g_value_get_string (value));
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
                                         const gchar             *project_key)
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
java_configuration_get_build_file (JavaConfiguration *configuration)
{
  return JAVA_CONFIGURATION_GET_PRIVATE (configuration)->build_file;
}

void
java_configuration_set_build_file (JavaConfiguration *configuration,
                                   const gchar       *build_file)
{
  JavaConfigurationPrivate *priv;
  priv = JAVA_CONFIGURATION_GET_PRIVATE (configuration);
  if (priv->build_file)
    {
      g_free (priv->build_file);
      priv->build_file = NULL;
    }
  priv->build_file = g_strdup (build_file);
}

const gchar*
java_configuration_get_build_directory (JavaConfiguration *configuration)
{
  return JAVA_CONFIGURATION_GET_PRIVATE (configuration)->build_directory;
}

void
java_configuration_set_build_directory (JavaConfiguration *configuration,
                                             const gchar            *build_directory)
{
  JavaConfigurationPrivate *priv;
  priv = JAVA_CONFIGURATION_GET_PRIVATE (configuration);
  if (priv->build_directory)
    {
      g_free (priv->build_directory);
      priv->build_directory = NULL;
    }
  priv->build_directory = g_strdup (build_directory);
}
