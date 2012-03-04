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

#ifndef __JAVA_COMPLETION_KLASS_H__
#define	__JAVA_COMPLETION_KLASS_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>
#include "java-indexer.h"

G_BEGIN_DECLS

#define JAVA_COMPLETION_KLASS_TYPE            (java_completion_klass_get_type ())
#define JAVA_COMPLETION_KLASS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_COMPLETION_KLASS_TYPE, JavaCompletionKlass))
#define JAVA_COMPLETION_KLASS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_COMPLETION_KLASS_TYPE, JavaCompletionKlassClass))
#define IS_JAVA_COMPLETION_KLASS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_COMPLETION_KLASS_TYPE))
#define IS_JAVA_COMPLETION_KLASS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_COMPLETION_KLASS_TYPE))

typedef struct _JavaCompletionKlass JavaCompletionKlass;
typedef struct _JavaCompletionKlassClass JavaCompletionKlassClass;

struct _JavaCompletionKlass
{
  GInitiallyUnowned parent_instance;
};

struct _JavaCompletionKlassClass
{
  GInitiallyUnownedClass parent_class;
};

GType java_completion_klass_get_type (void) G_GNUC_CONST;

JavaCompletionKlass*  java_completion_klass_new  (CodeSlayer       *codeslayer, 
                                                  CodeSlayerEditor *editor);

G_END_DECLS

#endif /* __JAVA_COMPLETION_KLASS_H__ */
