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

#include "java-breakpoints.h"
#include <codeslayer/codeslayer-utils.h>

static void java_breakpoints_class_init  (JavaBreakpointsClass *klass);
static void java_breakpoints_init        (JavaBreakpoints      *breakpoints);
static void java_breakpoints_finalize    (JavaBreakpoints      *breakpoints);

#define JAVA_BREAKPOINTS_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_BREAKPOINTS_TYPE, JavaBreakpointsPrivate))

typedef struct _JavaBreakpointsPrivate JavaBreakpointsPrivate;

struct _JavaBreakpointsPrivate
{
  GList *list;
};

G_DEFINE_TYPE (JavaBreakpoints, java_breakpoints, G_TYPE_OBJECT)
     
static void 
java_breakpoints_class_init (JavaBreakpointsClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) java_breakpoints_finalize;
  g_type_class_add_private (klass, sizeof (JavaBreakpointsPrivate));
}

static void
java_breakpoints_init (JavaBreakpoints *breakpoints)
{
  JavaBreakpointsPrivate *priv;
  priv = JAVA_BREAKPOINTS_GET_PRIVATE (breakpoints);
  priv->list = NULL;
}

static void
java_breakpoints_finalize (JavaBreakpoints *breakpoints)
{
  JavaBreakpointsPrivate *priv;
  priv = JAVA_BREAKPOINTS_GET_PRIVATE (breakpoints);
  if (priv->list != NULL)
    {
      g_list_foreach (priv->list, (GFunc) g_object_unref, NULL);
      g_list_free (priv->list);
      priv->list = NULL;    
    }
  G_OBJECT_CLASS (java_breakpoints_parent_class)->finalize (G_OBJECT (breakpoints));
}

JavaBreakpoints*
java_breakpoints_new ()
{
  JavaBreakpoints *breakpoints;
  breakpoints = JAVA_BREAKPOINTS  (g_object_new (java_breakpoints_get_type (), NULL));
  return breakpoints;
}

void
java_breakpoints_add_breakpoint (JavaBreakpoints *breakpoints, 
                                 JavaBreakpoint  *breakpoint)
{
  JavaBreakpointsPrivate *priv;
  priv = JAVA_BREAKPOINTS_GET_PRIVATE (breakpoints);
  priv->list = g_list_prepend (priv->list, breakpoint);
}

void
java_breakpoints_remove_breakpoint (JavaBreakpoints *breakpoints,
                                    JavaBreakpoint  *breakpoint)
{
  JavaBreakpointsPrivate *priv;
  priv = JAVA_BREAKPOINTS_GET_PRIVATE (breakpoints);
  priv->list = g_list_remove (priv->list, breakpoint);
  g_object_unref (breakpoint);
}

JavaBreakpoint*
java_breakpoints_find_breakpoint (JavaBreakpoints *breakpoints, 
                                  const gchar     *class_name, 
                                  gint             line_number)
{
  JavaBreakpointsPrivate *priv;
  GList *list;

  priv = JAVA_BREAKPOINTS_GET_PRIVATE (breakpoints);

  list = priv->list;
  while (list != NULL)
    {
      JavaBreakpoint *breakpoint = list->data;
      const gchar *name;
      gint number = 0;
      
      name = java_breakpoint_get_class_name (breakpoint);
      
      if (g_strcmp0 (class_name, name) == 0 && line_number == number)
        return breakpoint;

      list = g_list_next (list);
    }

  return NULL;
}
