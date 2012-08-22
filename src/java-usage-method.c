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

#include "java-usage-method.h"

static void java_usage_method_class_init    (JavaUsageMethodClass *klass);
static void java_usage_method_init          (JavaUsageMethod      *method);
static void java_usage_method_finalize      (JavaUsageMethod      *method);
static void java_usage_method_get_property  (GObject              *object, 
                                             guint                 prop_id,
                                             GValue               *value,
                                             GParamSpec           *pspec);
static void java_usage_method_set_property  (GObject              *object, 
                                             guint                 prop_id,
                                             const GValue         *value,
                                             GParamSpec           *pspec);

#define JAVA_USAGE_METHOD_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_USAGE_METHOD_TYPE, JavaUsageMethodPrivate))

typedef struct _JavaUsageMethodPrivate JavaUsageMethodPrivate;

struct _JavaUsageMethodPrivate
{
  gchar *class_name;
  gchar *file_path;
  gint   line_number;
};

enum
{
  PROP_0,
  PROP_CLASS_NAME,
  PROP_FILE_PATH,
  PROP_LINE_NUMBER
};

G_DEFINE_TYPE (JavaUsageMethod, java_usage_method, G_TYPE_OBJECT)
     
static void 
java_usage_method_class_init (JavaUsageMethodClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = (GObjectFinalizeFunc) java_usage_method_finalize;

  gobject_class->get_property = java_usage_method_get_property;
  gobject_class->set_property = java_usage_method_set_property;

  g_type_class_add_private (klass, sizeof (JavaUsageMethodPrivate));

  g_object_class_install_property (gobject_class, 
                                   PROP_CLASS_NAME,
                                   g_param_spec_string ("class_name", 
                                                        "Class Name",
                                                        "Class Name", "",
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
java_usage_method_init (JavaUsageMethod *method)
{
  JavaUsageMethodPrivate *priv;
  priv = JAVA_USAGE_METHOD_GET_PRIVATE (method);
  priv->class_name = NULL;
  priv->file_path = NULL;
  priv->line_number = 0;
}

static void
java_usage_method_finalize (JavaUsageMethod *method)
{
  JavaUsageMethodPrivate *priv;
  priv = JAVA_USAGE_METHOD_GET_PRIVATE (method);
  if (priv->class_name)
    {
      g_free (priv->class_name);
      priv->class_name = NULL;
    }
  if (priv->file_path)
    {
      g_free (priv->file_path);
      priv->file_path = NULL;
    }
  G_OBJECT_CLASS (java_usage_method_parent_class)->finalize (G_OBJECT (method));
}

static void
java_usage_method_get_property (GObject    *object, 
                                guint       prop_id,
                                GValue     *value, 
                                GParamSpec *pspec)
{
  JavaUsageMethod *method;
  JavaUsageMethodPrivate *priv;
  
  method = JAVA_USAGE_METHOD (object);
  priv = JAVA_USAGE_METHOD_GET_PRIVATE (method);

  switch (prop_id)
    {
    case PROP_CLASS_NAME:
      g_value_set_string (value, priv->class_name);
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
java_usage_method_set_property (GObject      *object, 
                                guint         prop_id,
                                const GValue *value, 
                                GParamSpec   *pspec)
{
  JavaUsageMethod *method;
  method = JAVA_USAGE_METHOD (object);

  switch (prop_id)
    {
    case PROP_CLASS_NAME:
      java_usage_method_set_class_name (method, g_value_get_string (value));
      break;
    case PROP_FILE_PATH:
      java_usage_method_set_file_path (method, g_value_get_string (value));
      break;
    case PROP_LINE_NUMBER:
      java_usage_method_set_line_number (method, g_value_get_int (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

JavaUsageMethod*
java_usage_method_new (void)
{
  return JAVA_USAGE_METHOD (g_object_new (java_usage_method_get_type (), NULL));
}

const gchar*
java_usage_method_get_class_name (JavaUsageMethod *method)
{
  return JAVA_USAGE_METHOD_GET_PRIVATE (method)->class_name;
}

void
java_usage_method_set_class_name (JavaUsageMethod *method, 
                                  const gchar     *class_name)
{
  JavaUsageMethodPrivate *priv;
  priv = JAVA_USAGE_METHOD_GET_PRIVATE (method);
  if (priv->class_name)
    {
      g_free (priv->class_name);
      priv->class_name = NULL;
    }
  priv->class_name = g_strdup (class_name);
}

const gchar*
java_usage_method_get_file_path (JavaUsageMethod *method)
{
  return JAVA_USAGE_METHOD_GET_PRIVATE (method)->file_path;
}

void
java_usage_method_set_file_path (JavaUsageMethod *method, 
                                 const gchar     *file_path)
{
  JavaUsageMethodPrivate *priv;
  priv = JAVA_USAGE_METHOD_GET_PRIVATE (method);
  if (priv->file_path)
    {
      g_free (priv->file_path);
      priv->file_path = NULL;
    }
  priv->file_path = g_strdup (file_path);
}

const gint
java_usage_method_get_line_number (JavaUsageMethod *method)
{
  return JAVA_USAGE_METHOD_GET_PRIVATE (method)->line_number;
}

void
java_usage_method_set_line_number (JavaUsageMethod *method,
                                   gint             line_number)
{
  JavaUsageMethodPrivate *priv;
  priv = JAVA_USAGE_METHOD_GET_PRIVATE (method);
  priv->line_number = line_number;
}
