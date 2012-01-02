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

#ifndef __JAVA_DEBUGGER_PANE_H__
#define	__JAVA_DEBUGGER_PANE_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>

G_BEGIN_DECLS

#define JAVA_DEBUGGER_PANE_TYPE            (java_debugger_pane_get_type ())
#define JAVA_DEBUGGER_PANE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_DEBUGGER_PANE_TYPE, JavaDebuggerPane))
#define JAVA_DEBUGGER_PANE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_DEBUGGER_PANE_TYPE, JavaDebuggerPaneClass))
#define IS_JAVA_DEBUGGER_PANE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_DEBUGGER_PANE_TYPE))
#define IS_JAVA_DEBUGGER_PANE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_DEBUGGER_PANE_TYPE))

typedef struct _JavaDebuggerPane JavaDebuggerPane;
typedef struct _JavaDebuggerPaneClass JavaDebuggerPaneClass;

struct _JavaDebuggerPane
{
  GtkVBox parent_instance;
};

struct _JavaDebuggerPaneClass
{
  GtkVBoxClass parent_class;
  
  void (*query) (JavaDebuggerPane *debugger_pane);
  void (*cont) (JavaDebuggerPane *debugger_pane);
  void (*quit) (JavaDebuggerPane *debugger_pane);
  void (*step_over) (JavaDebuggerPane *debugger_pane);
  void (*step_into) (JavaDebuggerPane *debugger_pane);
  void (*step_out) (JavaDebuggerPane *debugger_pane);
};

GType java_debugger_pane_get_type (void) G_GNUC_CONST;

GtkWidget*  java_debugger_pane_new  (void);

G_END_DECLS

#endif /* __JAVA_DEBUGGER_PANE_H__ */
