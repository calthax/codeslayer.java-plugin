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

#ifndef __JAVA_BREAKPOINT_H__
#define	__JAVA_BREAKPOINT_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define JAVA_BREAKPOINT_TYPE            (java_breakpoint_get_type ())
#define JAVA_BREAKPOINT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_BREAKPOINT_TYPE, JavaBreakpoint))
#define JAVA_BREAKPOINT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_BREAKPOINT_TYPE, JavaBreakpointClass))
#define IS_JAVA_BREAKPOINT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_BREAKPOINT_TYPE))
#define IS_JAVA_BREAKPOINT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_BREAKPOINT_TYPE))

typedef struct _JavaBreakpoint JavaBreakpoint;
typedef struct _JavaBreakpointClass JavaBreakpointClass;

struct _JavaBreakpoint
{
  GObject parent_instance;
};

struct _JavaBreakpointClass
{
  GObjectClass parent_class;
};

GType java_breakpoint_get_type (void) G_GNUC_CONST;

JavaBreakpoint*  java_breakpoint_new              (void);

const gchar*     java_breakpoint_get_class_name   (JavaBreakpoint *breakpoint);
void             java_breakpoint_set_class_name   (JavaBreakpoint *breakpoint,
                                                   const gchar    *class_name);
const gint       java_breakpoint_get_line_number  (JavaBreakpoint *breakpoint);
void             java_breakpoint_set_line_number  (JavaBreakpoint *breakpoint,
                                                   gint            line_number);

G_END_DECLS

#endif /* __JAVA_BREAKPOINT_H__ */
