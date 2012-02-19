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

#include "java-indexer-index.h"

static void java_indexer_index_class_init    (JavaIndexerIndexClass *klass);
static void java_indexer_index_init          (JavaIndexerIndex      *index);
static void java_indexer_index_finalize      (JavaIndexerIndex      *index);
static void java_indexer_index_get_property  (GObject               *object, 
                                              guint                  prop_id,
                                              GValue                *value,
                                              GParamSpec            *pspec);
static void java_indexer_index_set_property  (GObject               *object, 
                                              guint                  prop_id,
                                              const GValue          *value,
                                              GParamSpec            *pspec);

#define JAVA_INDEXER_INDEX_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_INDEXER_INDEX_TYPE, JavaIndexerIndexPrivate))

typedef struct _JavaIndexerIndexPrivate JavaIndexerIndexPrivate;

struct _JavaIndexerIndexPrivate
{
  gchar *package_name;
  gchar *class_name;
  gchar *method_modifier;
  gchar *method_name;
  gchar *method_parameters;
  gchar *method_completion;
  gchar *method_return_type;
  gchar *file_path;
  gint   line_number;
};

enum
{
  PROP_0,
  PROP_PACKAGE_NAME,
  PROP_CLASS_NAME,
  PROP_METHOD_MODIFIER,
  PROP_METHOD_NAME,
  PROP_METHOD_PARAMETERS,
  PROP_METHOD_COMPLETION,
  PROP_METHOD_RETURN_TYPE,
  PROP_FILE_PATH,
  PROP_LINE_NUMBER
};

G_DEFINE_TYPE (JavaIndexerIndex, java_indexer_index, G_TYPE_OBJECT)
     
static void 
java_indexer_index_class_init (JavaIndexerIndexClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = (GObjectFinalizeFunc) java_indexer_index_finalize;

  gobject_class->get_property = java_indexer_index_get_property;
  gobject_class->set_property = java_indexer_index_set_property;

  g_type_class_add_private (klass, sizeof (JavaIndexerIndexPrivate));

  g_object_class_install_property (gobject_class, 
                                   PROP_PACKAGE_NAME,
                                   g_param_spec_string ("package_name", 
                                                        "Package Name",
                                                        "Package Name", "",
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, 
                                   PROP_CLASS_NAME,
                                   g_param_spec_string ("class_name", 
                                                        "Class Name",
                                                        "Class Name", "",
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, 
                                   PROP_METHOD_MODIFIER,
                                   g_param_spec_string ("method_modifier", 
                                                        "Method Modifier",
                                                        "Method Modifier", "",
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, 
                                   PROP_METHOD_NAME,
                                   g_param_spec_string ("method_name", 
                                                        "Method Name",
                                                        "Method Name", "",
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, 
                                   PROP_METHOD_PARAMETERS,
                                   g_param_spec_string ("method_parameters", 
                                                        "Method Parameters",
                                                        "Method Parameters", "",
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, 
                                   PROP_METHOD_COMPLETION,
                                   g_param_spec_string ("method_completion", 
                                                        "Method Completion",
                                                        "Method Completion", "",
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, 
                                   PROP_METHOD_RETURN_TYPE,
                                   g_param_spec_string ("method_return_type", 
                                                        "Method Return Type",
                                                        "Method Return Type", "",
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, 
                                   PROP_FILE_PATH,
                                   g_param_spec_string ("file_path",
                                                        "File Path",
                                                        "File Path", "",
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, 
                                   PROP_LINE_NUMBER,
                                   g_param_spec_int ("line_number",
                                                     "Line Lumber",
                                                     "Line Lumber", 
                                                     0, 100000, 0,
                                                     G_PARAM_READWRITE));
}

static void
java_indexer_index_init (JavaIndexerIndex *index)
{
  JavaIndexerIndexPrivate *priv;
  priv = JAVA_INDEXER_INDEX_GET_PRIVATE (index);
  priv->package_name = NULL;
  priv->class_name = NULL;
  priv->method_modifier = NULL;
  priv->method_name = NULL;
  priv->method_parameters = NULL;
  priv->method_completion = NULL;
  priv->method_return_type = NULL;
  priv->file_path = NULL;
  priv->line_number = 0;
}

static void
java_indexer_index_finalize (JavaIndexerIndex *index)
{
  JavaIndexerIndexPrivate *priv;
  priv = JAVA_INDEXER_INDEX_GET_PRIVATE (index);
  if (priv->package_name)
    {
      g_free (priv->package_name);
      priv->package_name = NULL;
    }
  if (priv->class_name)
    {
      g_free (priv->class_name);
      priv->class_name = NULL;
    }
  if (priv->method_modifier)
    {
      g_free (priv->method_modifier);
      priv->method_modifier = NULL;
    }
  if (priv->method_name)
    {
      g_free (priv->method_name);
      priv->method_name = NULL;
    }
  if (priv->method_parameters)
    {
      g_free (priv->method_parameters);
      priv->method_parameters = NULL;
    }
  if (priv->method_completion)
    {
      g_free (priv->method_completion);
      priv->method_completion = NULL;
    }
  if (priv->method_return_type)
    {
      g_free (priv->method_return_type);
      priv->method_return_type = NULL;
    }
  if (priv->file_path)
    {
      g_free (priv->file_path);
      priv->file_path = NULL;
    }
  G_OBJECT_CLASS (java_indexer_index_parent_class)->finalize (G_OBJECT (index));
}

static void
java_indexer_index_get_property (GObject    *object, 
                                 guint       prop_id,
                                 GValue     *value, 
                                 GParamSpec *pspec)
{
  JavaIndexerIndex *index;
  JavaIndexerIndexPrivate *priv;
  
  index = JAVA_INDEXER_INDEX (object);
  priv = JAVA_INDEXER_INDEX_GET_PRIVATE (index);

  switch (prop_id)
    {
    case PROP_PACKAGE_NAME:
      g_value_set_string (value, priv->package_name);
      break;
    case PROP_CLASS_NAME:
      g_value_set_string (value, priv->class_name);
      break;
    case PROP_METHOD_MODIFIER:
      g_value_set_string (value, priv->method_modifier);
      break;
    case PROP_METHOD_NAME:
      g_value_set_string (value, priv->method_name);
      break;
    case PROP_METHOD_PARAMETERS:
      g_value_set_string (value, priv->method_parameters);
      break;
    case PROP_METHOD_COMPLETION:
      g_value_set_string (value, priv->method_completion);
      break;
    case PROP_METHOD_RETURN_TYPE:
      g_value_set_string (value, priv->method_return_type);
      break;
    case PROP_FILE_PATH:
      g_value_set_string (value, priv->file_path);
      break;
    case PROP_LINE_NUMBER:
      g_value_set_int (value, priv->line_number);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
java_indexer_index_set_property (GObject      *object, 
                                 guint         prop_id,
                                 const GValue *value, 
                                 GParamSpec   *pspec)
{
  JavaIndexerIndex *index;
  index = JAVA_INDEXER_INDEX (object);

  switch (prop_id)
    {
    case PROP_PACKAGE_NAME:
      java_indexer_index_set_package_name (index, g_value_get_string (value));
      break;
    case PROP_CLASS_NAME:
      java_indexer_index_set_class_name (index, g_value_get_string (value));
      break;
    case PROP_METHOD_MODIFIER:
      java_indexer_index_set_method_modifier (index, g_value_get_string (value));
      break;
    case PROP_METHOD_NAME:
      java_indexer_index_set_method_name (index, g_value_get_string (value));
      break;
    case PROP_METHOD_PARAMETERS:
      java_indexer_index_set_method_parameters (index, g_value_get_string (value));
      break;
    case PROP_METHOD_COMPLETION:
      java_indexer_index_set_method_completion (index, g_value_get_string (value));
      break;
    case PROP_METHOD_RETURN_TYPE:
      java_indexer_index_set_method_return_type (index, g_value_get_string (value));
      break;
    case PROP_FILE_PATH:
      java_indexer_index_set_file_path (index, g_value_get_string (value));
      break;
    case PROP_LINE_NUMBER:
      java_indexer_index_set_line_number (index, g_value_get_int (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

JavaIndexerIndex*
java_indexer_index_new (void)
{
  return JAVA_INDEXER_INDEX (g_object_new (java_indexer_index_get_type (), NULL));
}

const gchar*
java_indexer_index_get_package_name (JavaIndexerIndex *index)
{
  return JAVA_INDEXER_INDEX_GET_PRIVATE (index)->package_name;
}

void
java_indexer_index_set_package_name (JavaIndexerIndex *index, 
                                     const gchar      *package_name)
{
  JavaIndexerIndexPrivate *priv;
  priv = JAVA_INDEXER_INDEX_GET_PRIVATE (index);
  if (priv->package_name)
    {
      g_free (priv->package_name);
      priv->package_name = NULL;
    }
  priv->package_name = g_strdup (package_name);
}

const gchar*
java_indexer_index_get_class_name (JavaIndexerIndex *index)
{
  return JAVA_INDEXER_INDEX_GET_PRIVATE (index)->class_name;
}

void
java_indexer_index_set_class_name (JavaIndexerIndex *index, 
                                   const gchar      *class_name)
{
  JavaIndexerIndexPrivate *priv;
  priv = JAVA_INDEXER_INDEX_GET_PRIVATE (index);
  if (priv->class_name)
    {
      g_free (priv->class_name);
      priv->class_name = NULL;
    }
  priv->class_name = g_strdup (class_name);
}

const gchar*
java_indexer_index_get_method_modifier (JavaIndexerIndex *index)
{
  return JAVA_INDEXER_INDEX_GET_PRIVATE (index)->method_modifier;
}

void
java_indexer_index_set_method_modifier (JavaIndexerIndex *index, 
                                        const gchar      *method_modifier)
{
  JavaIndexerIndexPrivate *priv;
  priv = JAVA_INDEXER_INDEX_GET_PRIVATE (index);
  if (priv->method_modifier)
    {
      g_free (priv->method_modifier);
      priv->method_modifier = NULL;
    }
  priv->method_modifier = g_strdup (method_modifier);
}

const gchar*
java_indexer_index_get_method_name (JavaIndexerIndex *index)
{
  return JAVA_INDEXER_INDEX_GET_PRIVATE (index)->method_name;
}

void
java_indexer_index_set_method_name (JavaIndexerIndex *index, 
                                    const gchar      *method_name)
{
  JavaIndexerIndexPrivate *priv;
  priv = JAVA_INDEXER_INDEX_GET_PRIVATE (index);
  if (priv->method_name)
    {
      g_free (priv->method_name);
      priv->method_name = NULL;
    }
  priv->method_name = g_strdup (method_name);
}

const gchar*
java_indexer_index_get_method_parameters (JavaIndexerIndex *index)
{
  return JAVA_INDEXER_INDEX_GET_PRIVATE (index)->method_parameters;
}

void
java_indexer_index_set_method_parameters (JavaIndexerIndex *index, 
                                          const gchar      *method_parameters)
{
  JavaIndexerIndexPrivate *priv;
  priv = JAVA_INDEXER_INDEX_GET_PRIVATE (index);
  if (priv->method_parameters)
    {
      g_free (priv->method_parameters);
      priv->method_parameters = NULL;
    }
  priv->method_parameters = g_strdup (method_parameters);
}

const gchar*
java_indexer_index_get_method_completion (JavaIndexerIndex *index)
{
  return JAVA_INDEXER_INDEX_GET_PRIVATE (index)->method_completion;
}

void
java_indexer_index_set_method_completion (JavaIndexerIndex *index, 
                                          const gchar      *method_completion)
{
  JavaIndexerIndexPrivate *priv;
  priv = JAVA_INDEXER_INDEX_GET_PRIVATE (index);
  if (priv->method_completion)
    {
      g_free (priv->method_completion);
      priv->method_completion = NULL;
    }
  priv->method_completion = g_strdup (method_completion);
}

const gchar*
java_indexer_index_get_method_return_type (JavaIndexerIndex *index)
{
  return JAVA_INDEXER_INDEX_GET_PRIVATE (index)->method_return_type;
}

void
java_indexer_index_set_method_return_type (JavaIndexerIndex *index, 
                                           const gchar      *method_return_type)
{
  JavaIndexerIndexPrivate *priv;
  priv = JAVA_INDEXER_INDEX_GET_PRIVATE (index);
  if (priv->method_return_type)
    {
      g_free (priv->method_return_type);
      priv->method_return_type = NULL;
    }
  priv->method_return_type = g_strdup (method_return_type);
}

const gchar*
java_indexer_index_get_file_path (JavaIndexerIndex *index)
{
  return JAVA_INDEXER_INDEX_GET_PRIVATE (index)->file_path;
}

void
java_indexer_index_set_file_path (JavaIndexerIndex *index, 
                                  const gchar      *file_path)
{
  JavaIndexerIndexPrivate *priv;
  priv = JAVA_INDEXER_INDEX_GET_PRIVATE (index);
  if (priv->file_path)
    {
      g_free (priv->file_path);
      priv->file_path = NULL;
    }
  priv->file_path = g_strdup (file_path);
}

const gint
java_indexer_index_get_line_number (JavaIndexerIndex *index)
{
  return JAVA_INDEXER_INDEX_GET_PRIVATE (index)->line_number;
}

void
java_indexer_index_set_line_number (JavaIndexerIndex *index,
                                    gint              line_number)
{
  JavaIndexerIndexPrivate *priv;
  priv = JAVA_INDEXER_INDEX_GET_PRIVATE (index);
  priv->line_number = line_number;
}
