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

#ifndef __JAVA_COMPLETION_VARIABLE_H__
#define	__JAVA_COMPLETION_VARIABLE_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>

G_BEGIN_DECLS

#define JAVA_COMPLETION_VARIABLE_TYPE            (java_completion_variable_get_type ())
#define JAVA_COMPLETION_VARIABLE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_COMPLETION_VARIABLE_TYPE, JavaCompletionVariable))
#define JAVA_COMPLETION_VARIABLE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_COMPLETION_VARIABLE_TYPE, JavaCompletionVariableClass))
#define IS_JAVA_COMPLETION_VARIABLE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_COMPLETION_VARIABLE_TYPE))
#define IS_JAVA_COMPLETION_VARIABLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_COMPLETION_VARIABLE_TYPE))

typedef struct _JavaCompletionVariable JavaCompletionVariable;
typedef struct _JavaCompletionVariableClass JavaCompletionVariableClass;

struct _JavaCompletionVariable
{
  GInitiallyUnowned parent_instance;
};

struct _JavaCompletionVariableClass
{
  GInitiallyUnownedClass parent_class;
};

GType java_completion_variable_get_type (void) G_GNUC_CONST;

JavaCompletionVariable*  java_completion_variable_new  (CodeSlayerEditor *editor);

G_END_DECLS

#endif /* __JAVA_COMPLETION_VARIABLE_H__ */
