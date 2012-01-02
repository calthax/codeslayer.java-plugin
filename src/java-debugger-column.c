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

#include "java-debugger-column.h"

static void java_debugger_column_class_init    (JavaDebuggerColumnClass *klass);
static void java_debugger_column_init          (JavaDebuggerColumn      *column);
static void java_debugger_column_finalize      (JavaDebuggerColumn      *column);
static void java_debugger_column_get_property  (GObject                 *object, 
                                                guint                    prop_id,
                                                GValue                  *value,
                                                GParamSpec              *pspec);
static void java_debugger_column_set_property  (GObject                 *object, 
                                                guint                    prop_id,
                                                const GValue            *value,
                                                GParamSpec              *pspec);

#define JAVA_DEBUGGER_COLUMN_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_DEBUGGER_COLUMN_TYPE, JavaDebuggerColumnPrivate))

typedef struct _JavaDebuggerColumnPrivate JavaDebuggerColumnPrivate;

struct _JavaDebuggerColumnPrivate
{
  gchar *name;
  gchar *value;
};

enum
{
  PROP_0,
  PROP_NAME,
  PROP_VALUE
};

G_DEFINE_TYPE (JavaDebuggerColumn, java_debugger_column, G_TYPE_OBJECT)
     
static void 
java_debugger_column_class_init (JavaDebuggerColumnClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = (GObjectFinalizeFunc) java_debugger_column_finalize;

  gobject_class->get_property = java_debugger_column_get_property;
  gobject_class->set_property = java_debugger_column_set_property;

  g_type_class_add_private (klass, sizeof (JavaDebuggerColumnPrivate));

  g_object_class_install_property (gobject_class, 
                                   PROP_NAME,
                                   g_param_spec_string ("name", 
                                                        "Class Name",
                                                        "Class Name", "",
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, 
                                   PROP_VALUE,
                                   g_param_spec_string ("value",
                                                        "File Path",
                                                        "File Path", "",
                                                        G_PARAM_READWRITE));
}

static void
java_debugger_column_init (JavaDebuggerColumn *column)
{
  JavaDebuggerColumnPrivate *priv;
  priv = JAVA_DEBUGGER_COLUMN_GET_PRIVATE (column);
  priv->name = NULL;
  priv->value = NULL;
}

static void
java_debugger_column_finalize (JavaDebuggerColumn *column)
{
  JavaDebuggerColumnPrivate *priv;
  priv = JAVA_DEBUGGER_COLUMN_GET_PRIVATE (column);
  if (priv->name)
    {
      g_free (priv->name);
      priv->name = NULL;
    }
  if (priv->value)
    {
      g_free (priv->value);
      priv->value = NULL;
    }
  G_OBJECT_CLASS (java_debugger_column_parent_class)->finalize (G_OBJECT (column));
}

static void
java_debugger_column_get_property (GObject    *object, 
                                   guint       prop_id,
                                   GValue     *value, 
                                   GParamSpec *pspec)
{
  JavaDebuggerColumn *column;
  JavaDebuggerColumnPrivate *priv;
  
  column = JAVA_DEBUGGER_COLUMN (object);
  priv = JAVA_DEBUGGER_COLUMN_GET_PRIVATE (column);

  switch (prop_id)
    {
    case PROP_NAME:
      g_value_set_string (value, priv->name);
      break;
    case PROP_VALUE:
      g_value_set_string (value, priv->value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
java_debugger_column_set_property (GObject      *object, 
                                       guint         prop_id,
                                       const GValue *value, 
                                       GParamSpec   *pspec)
{
  JavaDebuggerColumn *column;
  column = JAVA_DEBUGGER_COLUMN (object);

  switch (prop_id)
    {
    case PROP_NAME:
      java_debugger_column_set_name (column, g_value_get_string (value));
      break;
    case PROP_VALUE:
      java_debugger_column_set_value (column, g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

JavaDebuggerColumn*
java_debugger_column_new (void)
{
  return JAVA_DEBUGGER_COLUMN (g_object_new (java_debugger_column_get_type (), NULL));
}

const gchar*
java_debugger_column_get_name (JavaDebuggerColumn *column)
{
  return JAVA_DEBUGGER_COLUMN_GET_PRIVATE (column)->name;
}

void
java_debugger_column_set_name (JavaDebuggerColumn *column, 
                               const gchar        *name)
{
  JavaDebuggerColumnPrivate *priv;
  priv = JAVA_DEBUGGER_COLUMN_GET_PRIVATE (column);
  if (priv->name)
    {
      g_free (priv->name);
      priv->name = NULL;
    }
  priv->name = g_strdup (name);
}

const gchar*
java_debugger_column_get_value (JavaDebuggerColumn *column)
{
  return JAVA_DEBUGGER_COLUMN_GET_PRIVATE (column)->value;
}

void
java_debugger_column_set_value (JavaDebuggerColumn *column, 
                                const gchar        *value)
{
  JavaDebuggerColumnPrivate *priv;
  priv = JAVA_DEBUGGER_COLUMN_GET_PRIVATE (column);
  if (priv->value)
    {
      g_free (priv->value);
      priv->value = NULL;
    }
  priv->value = g_strdup (value);
}
