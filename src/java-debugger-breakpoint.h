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

#ifndef __JAVA_DEBUGGER_BREAKPOINT_H__
#define	__JAVA_DEBUGGER_BREAKPOINT_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define JAVA_DEBUGGER_BREAKPOINT_TYPE            (java_debugger_breakpoint_get_type ())
#define JAVA_DEBUGGER_BREAKPOINT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_DEBUGGER_BREAKPOINT_TYPE, JavaDebuggerBreakpoint))
#define JAVA_DEBUGGER_BREAKPOINT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_DEBUGGER_BREAKPOINT_TYPE, JavaDebuggerBreakpointClass))
#define IS_JAVA_DEBUGGER_BREAKPOINT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_DEBUGGER_BREAKPOINT_TYPE))
#define IS_JAVA_DEBUGGER_BREAKPOINT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_DEBUGGER_BREAKPOINT_TYPE))

typedef struct _JavaDebuggerBreakpoint JavaDebuggerBreakpoint;
typedef struct _JavaDebuggerBreakpointClass JavaDebuggerBreakpointClass;

struct _JavaDebuggerBreakpoint
{
  GObject parent_instance;
};

struct _JavaDebuggerBreakpointClass
{
  GObjectClass parent_class;
};

GType java_debugger_breakpoint_get_type (void) G_GNUC_CONST;

JavaDebuggerBreakpoint*  java_debugger_breakpoint_new              (void);

const gchar*             java_debugger_breakpoint_get_class_name   (JavaDebuggerBreakpoint *breakpoint);
void                     java_debugger_breakpoint_set_class_name   (JavaDebuggerBreakpoint *breakpoint,
                                                                    const gchar            *class_name);
const gchar*             java_debugger_breakpoint_get_file_path    (JavaDebuggerBreakpoint *breakpoint);
void                     java_debugger_breakpoint_set_file_path    (JavaDebuggerBreakpoint *breakpoint,
                                                                    const gchar            *file_path);
const gint               java_debugger_breakpoint_get_line_number  (JavaDebuggerBreakpoint *breakpoint);
void                     java_debugger_breakpoint_set_line_number  (JavaDebuggerBreakpoint *breakpoint,
                                                                    gint                    line_number);

G_END_DECLS

#endif /* __JAVA_DEBUGGER_BREAKPOINT_H__ */
