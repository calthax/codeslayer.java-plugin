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
  gchar *name;
  gchar *parameters;
  gchar *modifier;
  gchar *class_name;
  gchar *package_name;
  gchar *file_path;
  gint   line_number;
};

enum
{
  PROP_0,
  PROP_NAME,
  PROP_PARAMETERS,
  PROP_MODIFIER,
  PROP_CLASS_NAME,
  PROP_PACKAGE_NAME,
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
                                   PROP_NAME,
                                   g_param_spec_string ("name", 
                                                        "Name",
                                                        "Name", "",
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, 
                                   PROP_PARAMETERS,
                                   g_param_spec_string ("parameters", 
                                                        "Parameters",
                                                        "Parameters", "",
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, 
                                   PROP_MODIFIER,
                                   g_param_spec_string ("modifier", 
                                                        "Modifier",
                                                        "Modifier", "",
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, 
                                   PROP_CLASS_NAME,
                                   g_param_spec_string ("class_name", 
                                                        "Class Name",
                                                        "Class Name", "",
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, 
                                   PROP_PACKAGE_NAME,
                                   g_param_spec_string ("package_name", 
                                                        "Package Name",
                                                        "Package Name", "",
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
  priv->name = NULL;
  priv->parameters = NULL;
  priv->modifier = NULL;
  priv->class_name = NULL;
  priv->package_name = NULL;
  priv->file_path = NULL;
  priv->line_number = 0;
}

static void
java_indexer_index_finalize (JavaIndexerIndex *index)
{
  JavaIndexerIndexPrivate *priv;
  priv = JAVA_INDEXER_INDEX_GET_PRIVATE (index);
  if (priv->name)
    {
      g_free (priv->name);
      priv->name = NULL;
    }
  if (priv->parameters)
    {
      g_free (priv->parameters);
      priv->parameters = NULL;
    }
  if (priv->modifier)
    {
      g_free (priv->modifier);
      priv->modifier = NULL;
    }
  if (priv->class_name)
    {
      g_free (priv->class_name);
      priv->class_name = NULL;
    }
  if (priv->package_name)
    {
      g_free (priv->package_name);
      priv->package_name = NULL;
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
    case PROP_NAME:
      g_value_set_string (value, priv->name);
      break;
    case PROP_PARAMETERS:
      g_value_set_string (value, priv->parameters);
      break;
    case PROP_MODIFIER:
      g_value_set_string (value, priv->modifier);
      break;
    case PROP_CLASS_NAME:
      g_value_set_string (value, priv->class_name);
      break;
    case PROP_PACKAGE_NAME:
      g_value_set_string (value, priv->package_name);
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
    case PROP_NAME:
      java_indexer_index_set_name (index, g_value_get_string (value));
      break;
    case PROP_PARAMETERS:
      java_indexer_index_set_parameters (index, g_value_get_string (value));
      break;
    case PROP_MODIFIER:
      java_indexer_index_set_modifier (index, g_value_get_string (value));
      break;
    case PROP_CLASS_NAME:
      java_indexer_index_set_class_name (index, g_value_get_string (value));
      break;
    case PROP_PACKAGE_NAME:
      java_indexer_index_set_package_name (index, g_value_get_string (value));
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
java_indexer_index_get_name (JavaIndexerIndex *index)
{
  return JAVA_INDEXER_INDEX_GET_PRIVATE (index)->name;
}

void
java_indexer_index_set_name (JavaIndexerIndex *index, 
                             const gchar      *name)
{
  JavaIndexerIndexPrivate *priv;
  priv = JAVA_INDEXER_INDEX_GET_PRIVATE (index);
  if (priv->name)
    {
      g_free (priv->name);
      priv->name = NULL;
    }
  priv->name = g_strdup (name);
}

const gchar*
java_indexer_index_get_parameters (JavaIndexerIndex *index)
{
  return JAVA_INDEXER_INDEX_GET_PRIVATE (index)->parameters;
}

void
java_indexer_index_set_parameters (JavaIndexerIndex *index, 
                                   const gchar      *parameters)
{
  JavaIndexerIndexPrivate *priv;
  priv = JAVA_INDEXER_INDEX_GET_PRIVATE (index);
  if (priv->parameters)
    {
      g_free (priv->parameters);
      priv->parameters = NULL;
    }
  priv->parameters = g_strdup (parameters);
}

const gchar*
java_indexer_index_get_modifier (JavaIndexerIndex *index)
{
  return JAVA_INDEXER_INDEX_GET_PRIVATE (index)->modifier;
}

void
java_indexer_index_set_modifier (JavaIndexerIndex *index, 
                                 const gchar      *modifier)
{
  JavaIndexerIndexPrivate *priv;
  priv = JAVA_INDEXER_INDEX_GET_PRIVATE (index);
  if (priv->modifier)
    {
      g_free (priv->modifier);
      priv->modifier = NULL;
    }
  priv->modifier = g_strdup (modifier);
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
