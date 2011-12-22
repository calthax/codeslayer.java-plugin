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

#include <gtk/gtk.h>
#include "java-debugger-pane.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void java_debugger_pane_class_init  (JavaDebuggerPaneClass *klass);
static void java_debugger_pane_init        (JavaDebuggerPane      *pane);
static void java_debugger_pane_finalize    (JavaDebuggerPane      *pane);

#define JAVA_DEBUGGER_PANE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_DEBUGGER_PANE_TYPE, JavaDebuggerPanePrivate))

typedef struct _JavaDebuggerPanePrivate JavaDebuggerPanePrivate;

struct _JavaDebuggerPanePrivate
{
  gchar *foo;
};

G_DEFINE_TYPE (JavaDebuggerPane, java_debugger_pane, G_TYPE_OBJECT)
     
static void 
java_debugger_pane_class_init (JavaDebuggerPaneClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) java_debugger_pane_finalize;
  g_type_class_add_private (klass, sizeof (JavaDebuggerPanePrivate));
}

static void
java_debugger_pane_init (JavaDebuggerPane *pane)
{
  /*JavaDebuggerPanePrivate *priv;
  priv = JAVA_DEBUGGER_PANE_GET_PRIVATE (pane);*/
}

static void
java_debugger_pane_finalize (JavaDebuggerPane *pane)
{
  G_OBJECT_CLASS (java_debugger_pane_parent_class)->finalize (G_OBJECT (pane));
}

JavaDebuggerPane*
java_debugger_pane_new ()
{
  JavaDebuggerPane *pane;
  pane = JAVA_DEBUGGER_PANE (g_object_new (java_debugger_pane_get_type (), NULL));
  return pane;
}
