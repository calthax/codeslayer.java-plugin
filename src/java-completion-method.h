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

#ifndef __JAVA_COMPLETION_METHOD_H__
#define	__JAVA_COMPLETION_METHOD_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>
#include "java-debugger-breakpoint.h"
#include "java-indexer.h"

G_BEGIN_DECLS

#define JAVA_COMPLETION_METHOD_TYPE            (java_completion_method_get_type ())
#define JAVA_COMPLETION_METHOD(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_COMPLETION_METHOD_TYPE, JavaCompletionMethod))
#define JAVA_COMPLETION_METHOD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_COMPLETION_METHOD_TYPE, JavaCompletionMethodClass))
#define IS_JAVA_COMPLETION_METHOD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_COMPLETION_METHOD_TYPE))
#define IS_JAVA_COMPLETION_METHOD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_COMPLETION_METHOD_TYPE))

typedef struct _JavaCompletionMethod JavaCompletionMethod;
typedef struct _JavaCompletionMethodClass JavaCompletionMethodClass;

struct _JavaCompletionMethod
{
  GObject parent_instance;
};

struct _JavaCompletionMethodClass
{
  GObjectClass parent_class;
};

GType java_completion_method_get_type (void) G_GNUC_CONST;

JavaCompletionMethod*  java_completion_method_new  (CodeSlayerEditor *editor, 
                                                    JavaIndexer      *indexer);

G_END_DECLS

#endif /* __JAVA_COMPLETION_METHOD_H__ */
