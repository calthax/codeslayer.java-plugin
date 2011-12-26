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

#ifndef __JAVA_DEBUGGER_BREAKPOINTS_H__
#define	__JAVA_DEBUGGER_BREAKPOINTS_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>
#include "java-debugger-breakpoint.h"

G_BEGIN_DECLS

#define JAVA_DEBUGGER_BREAKPOINTS_TYPE            (java_debugger_breakpoints_get_type ())
#define JAVA_DEBUGGER_BREAKPOINTS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_DEBUGGER_BREAKPOINTS_TYPE, JavaDebuggerBreakpoints))
#define JAVA_DEBUGGER_BREAKPOINTS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_DEBUGGER_BREAKPOINTS_TYPE, JavaDebuggerBreakpointsClass))
#define IS_JAVA_DEBUGGER_BREAKPOINTS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_DEBUGGER_BREAKPOINTS_TYPE))
#define IS_JAVA_DEBUGGER_BREAKPOINTS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_DEBUGGER_BREAKPOINTS_TYPE))

typedef struct _JavaDebuggerBreakpoints JavaDebuggerBreakpoints;
typedef struct _JavaDebuggerBreakpointsClass JavaDebuggerBreakpointsClass;

struct _JavaDebuggerBreakpoints
{
  GObject parent_instance;
};

struct _JavaDebuggerBreakpointsClass
{
  GObjectClass parent_class;
};

GType java_debugger_breakpoints_get_type (void) G_GNUC_CONST;

JavaDebuggerBreakpoints*  java_debugger_breakpoints_new                (void);

void                      java_debugger_breakpoints_add_breakpoint     (JavaDebuggerBreakpoints *breakpoints, 
                                                                        JavaDebuggerBreakpoint  *breakpoint);
                                                     
void                      java_debugger_breakpoints_remove_breakpoint  (JavaDebuggerBreakpoints *breakpoints, 
                                                                        JavaDebuggerBreakpoint  *breakpoint);

JavaDebuggerBreakpoint*   java_debugger_breakpoints_find_breakpoint    (JavaDebuggerBreakpoints *breakpoints,
                                                                        const gchar             *class_name,
                                                                        gint                     line_number);
GList*                    java_debugger_breakpoints_get_list           (JavaDebuggerBreakpoints *breakpoints);

G_END_DECLS

#endif /* __JAVA_DEBUGGER_BREAKPOINTS_H__ */
