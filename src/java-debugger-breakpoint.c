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

#include "java-debugger-breakpoint.h"

static void java_debugger_breakpoint_class_init    (JavaDebuggerBreakpointClass *klass);
static void java_debugger_breakpoint_init          (JavaDebuggerBreakpoint      *breakpoint);
static void java_debugger_breakpoint_finalize      (JavaDebuggerBreakpoint      *breakpoint);
static void java_debugger_breakpoint_get_property  (GObject                     *object, 
                                                    guint                        prop_id,
                                                    GValue                      *value,
                                                    GParamSpec                  *pspec);
static void java_debugger_breakpoint_set_property  (GObject                     *object, 
                                                    guint                        prop_id,
                                                    const GValue                *value,
                                                    GParamSpec                  *pspec);

#define JAVA_DEBUGGER_BREAKPOINT_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_DEBUGGER_BREAKPOINT_TYPE, JavaDebuggerBreakpointPrivate))

typedef struct _JavaDebuggerBreakpointPrivate JavaDebuggerBreakpointPrivate;

struct _JavaDebuggerBreakpointPrivate
{
  gchar *class_name;
  gint line_number;
};

enum
{
  PROP_0,
  PROP_CLASS_NAME,
  PROP_LINE_NUMBER
};

G_DEFINE_TYPE (JavaDebuggerBreakpoint, java_debugger_breakpoint, G_TYPE_OBJECT)
     
static void 
java_debugger_breakpoint_class_init (JavaDebuggerBreakpointClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = (GObjectFinalizeFunc) java_debugger_breakpoint_finalize;

  gobject_class->get_property = java_debugger_breakpoint_get_property;
  gobject_class->set_property = java_debugger_breakpoint_set_property;

  g_type_class_add_private (klass, sizeof (JavaDebuggerBreakpointPrivate));

  g_object_class_install_property (gobject_class, 
                                   PROP_CLASS_NAME,
                                   g_param_spec_string ("class_name", 
                                                        "Class Name",
                                                        "Class Name", "",
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, 
                                   PROP_LINE_NUMBER,
                                   g_param_spec_string ("line_number",
                                                        "Line Lumber",
                                                        "Line Lumber", "",
                                                        G_PARAM_READWRITE));
}

static void
java_debugger_breakpoint_init (JavaDebuggerBreakpoint *breakpoint)
{
  JavaDebuggerBreakpointPrivate *priv;
  priv = JAVA_DEBUGGER_BREAKPOINT_GET_PRIVATE (breakpoint);
  priv->class_name = NULL;
  priv->line_number = 0;
}

static void
java_debugger_breakpoint_finalize (JavaDebuggerBreakpoint *breakpoint)
{
  JavaDebuggerBreakpointPrivate *priv;
  priv = JAVA_DEBUGGER_BREAKPOINT_GET_PRIVATE (breakpoint);
  if (priv->class_name)
    {
      g_free (priv->class_name);
      priv->class_name = NULL;
    }
  G_OBJECT_CLASS (java_debugger_breakpoint_parent_class)->finalize (G_OBJECT (breakpoint));
}

static void
java_debugger_breakpoint_get_property (GObject    *object, 
                                       guint       prop_id,
                                       GValue     *value, 
                                       GParamSpec *pspec)
{
  JavaDebuggerBreakpoint *breakpoint;
  JavaDebuggerBreakpointPrivate *priv;
  
  breakpoint = JAVA_DEBUGGER_BREAKPOINT (object);
  priv = JAVA_DEBUGGER_BREAKPOINT_GET_PRIVATE (breakpoint);

  switch (prop_id)
    {
    case PROP_CLASS_NAME:
      g_value_set_string (value, priv->class_name);
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
java_debugger_breakpoint_set_property (GObject      *object, 
                                       guint         prop_id,
                                       const GValue *value, 
                                       GParamSpec   *pspec)
{
  JavaDebuggerBreakpoint *breakpoint;
  breakpoint = JAVA_DEBUGGER_BREAKPOINT (object);

  switch (prop_id)
    {
    case PROP_CLASS_NAME:
      java_debugger_breakpoint_set_class_name (breakpoint, g_value_get_string (value));
      break;
    case PROP_LINE_NUMBER:
      java_debugger_breakpoint_set_line_number (breakpoint, g_value_get_int (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

JavaDebuggerBreakpoint*
java_debugger_breakpoint_new (void)
{
  return JAVA_DEBUGGER_BREAKPOINT (g_object_new (java_debugger_breakpoint_get_type (), NULL));
}

const gchar*
java_debugger_breakpoint_get_class_name (JavaDebuggerBreakpoint *breakpoint)
{
  return JAVA_DEBUGGER_BREAKPOINT_GET_PRIVATE (breakpoint)->class_name;
}

void
java_debugger_breakpoint_set_class_name (JavaDebuggerBreakpoint *breakpoint, 
                                         const gchar            *class_name)
{
  JavaDebuggerBreakpointPrivate *priv;
  priv = JAVA_DEBUGGER_BREAKPOINT_GET_PRIVATE (breakpoint);
  if (priv->class_name)
    {
      g_free (priv->class_name);
      priv->class_name = NULL;
    }
  priv->class_name = g_strdup (class_name);
}

const gint
java_debugger_breakpoint_get_line_number (JavaDebuggerBreakpoint *breakpoint)
{
  return JAVA_DEBUGGER_BREAKPOINT_GET_PRIVATE (breakpoint)->line_number;
}

void
java_debugger_breakpoint_set_line_number (JavaDebuggerBreakpoint *breakpoint,
                                          gint                    line_number)
{
  JavaDebuggerBreakpointPrivate *priv;
  priv = JAVA_DEBUGGER_BREAKPOINT_GET_PRIVATE (breakpoint);
  priv->line_number = line_number;
}
