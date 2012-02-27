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

#ifndef __JAVA_COMPLETION_WORD_H__
#define	__JAVA_COMPLETION_WORD_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>

G_BEGIN_DECLS

#define JAVA_COMPLETION_WORD_TYPE            (java_completion_word_get_type ())
#define JAVA_COMPLETION_WORD(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_COMPLETION_WORD_TYPE, JavaCompletionWord))
#define JAVA_COMPLETION_WORD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_COMPLETION_WORD_TYPE, JavaCompletionWordClass))
#define IS_JAVA_COMPLETION_WORD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_COMPLETION_WORD_TYPE))
#define IS_JAVA_COMPLETION_WORD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_COMPLETION_WORD_TYPE))

typedef struct _JavaCompletionWord JavaCompletionWord;
typedef struct _JavaCompletionWordClass JavaCompletionWordClass;

struct _JavaCompletionWord
{
  GInitiallyUnowned parent_instance;
};

struct _JavaCompletionWordClass
{
  GInitiallyUnownedClass parent_class;
};

GType java_completion_word_get_type (void) G_GNUC_CONST;

JavaCompletionWord*  java_completion_word_new  (CodeSlayerEditor *editor);

G_END_DECLS

#endif /* __JAVA_COMPLETION_WORD_H__ */
