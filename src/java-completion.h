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

#ifndef __JAVA_COMPLETION_H__
#define	__JAVA_COMPLETION_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>
#include "java-indexer.h"

G_BEGIN_DECLS

#define JAVA_COMPLETION_TYPE            (java_completion_get_type ())
#define JAVA_COMPLETION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_COMPLETION_TYPE, JavaCompletion))
#define JAVA_COMPLETION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_COMPLETION_TYPE, JavaCompletionClass))
#define IS_JAVA_COMPLETION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_COMPLETION_TYPE))
#define IS_JAVA_COMPLETION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_COMPLETION_TYPE))

typedef struct _JavaCompletion JavaCompletion;
typedef struct _JavaCompletionClass JavaCompletionClass;

struct _JavaCompletion
{
  GObject parent_instance;
};

struct _JavaCompletionClass
{
  GObjectClass parent_class;
};

GType java_completion_get_type (void) G_GNUC_CONST;

JavaCompletion*  java_completion_new  (CodeSlayer  *codeslayer, 
                                       JavaIndexer *indexer);

G_END_DECLS

#endif /* __JAVA_COMPLETION_H__ */
