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

#ifndef __JAVA_DEBUGGER_COLUMN_H__
#define	__JAVA_DEBUGGER_COLUMN_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define JAVA_DEBUGGER_COLUMN_TYPE            (java_debugger_column_get_type ())
#define JAVA_DEBUGGER_COLUMN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_DEBUGGER_COLUMN_TYPE, JavaDebuggerColumn))
#define JAVA_DEBUGGER_COLUMN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_DEBUGGER_COLUMN_TYPE, JavaDebuggerColumnClass))
#define IS_JAVA_DEBUGGER_COLUMN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_DEBUGGER_COLUMN_TYPE))
#define IS_JAVA_DEBUGGER_COLUMN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_DEBUGGER_COLUMN_TYPE))

typedef struct _JavaDebuggerColumn JavaDebuggerColumn;
typedef struct _JavaDebuggerColumnClass JavaDebuggerColumnClass;

struct _JavaDebuggerColumn
{
  GObject parent_instance;
};

struct _JavaDebuggerColumnClass
{
  GObjectClass parent_class;
};

GType java_debugger_column_get_type (void) G_GNUC_CONST;

JavaDebuggerColumn*  java_debugger_column_new              (void);

const gchar*         java_debugger_column_get_name   (JavaDebuggerColumn *column);
void                 java_debugger_column_set_name   (JavaDebuggerColumn *column,
                                                      const gchar        *name);
const gchar*         java_debugger_column_get_value  (JavaDebuggerColumn *column);
void                 java_debugger_column_set_value  (JavaDebuggerColumn *column,
                                                      const gchar        *value);

G_END_DECLS

#endif /* __JAVA_DEBUGGER_COLUMN_H__ */
