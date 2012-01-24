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

#include <codeslayer/codeslayer-utils.h>
#include <gtksourceview/gtksourceview.h>
#include <gtksourceview/gtksourcecompletion.h>
#include <gtksourceview/gtksourcecompletioninfo.h>
#include <gtksourceview/gtksourcecompletionitem.h>
#include <gtksourceview/gtksourcelanguagemanager.h>
#include <gtksourceview/completion-providers/words/gtksourcecompletionwords.h>
#include <stdio.h>
#include <stdlib.h>
#include "java-completion.h"
#include "java-completion-method.h"
#include "java-utils.h"

static void java_completion_class_init  (JavaCompletionClass *klass);
static void java_completion_init        (JavaCompletion      *completion);
static void java_completion_finalize    (JavaCompletion      *completion);
static void editor_added_action         (JavaCompletion      *completion,
                                         CodeSlayerEditor    *editor);
                                       
#define JAVA_COMPLETION_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_COMPLETION_TYPE, JavaCompletionPrivate))

typedef struct _JavaCompletionPrivate JavaCompletionPrivate;

struct _JavaCompletionPrivate
{
  CodeSlayer  *codeslayer;
  JavaIndexer *indexer;
  gulong       editor_added_id;
};

G_DEFINE_TYPE (JavaCompletion, java_completion, G_TYPE_OBJECT)
     
static void 
java_completion_class_init (JavaCompletionClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) java_completion_finalize;
  g_type_class_add_private (klass, sizeof (JavaCompletionPrivate));
}

static void
java_completion_init (JavaCompletion *completion)
{
}

static void
java_completion_finalize (JavaCompletion *completion)
{
  JavaCompletionPrivate *priv;
  priv = JAVA_COMPLETION_GET_PRIVATE (completion);  
  g_signal_handler_disconnect (priv->codeslayer, priv->editor_added_id);
  G_OBJECT_CLASS (java_completion_parent_class)->finalize (G_OBJECT (completion));
}

JavaCompletion*
java_completion_new (CodeSlayer  *codeslayer, 
                     JavaIndexer *indexer)
{
  JavaCompletionPrivate *priv;
  JavaCompletion *completion;

  completion = JAVA_COMPLETION (g_object_new (java_completion_get_type (), NULL));
  priv = JAVA_COMPLETION_GET_PRIVATE (completion);
  priv->codeslayer = codeslayer;
  priv->indexer = indexer;
  
  priv->editor_added_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "editor-added",
                                                    G_CALLBACK (editor_added_action), completion);
                     
  return completion;
}

static void
editor_added_action (JavaCompletion   *completion,
                     CodeSlayerEditor *editor)
{
  JavaCompletionPrivate *priv;
	GtkSourceCompletion *source_completion;
	JavaCompletionMethod *completion_method;
  
  priv = JAVA_COMPLETION_GET_PRIVATE (completion);

	source_completion = gtk_source_view_get_completion (GTK_SOURCE_VIEW (editor));
	g_object_set (source_completion, "show-headers", FALSE, NULL);
	g_object_set (source_completion, "show-icons", FALSE, NULL);
	
	completion_method = java_completion_method_new (editor, priv->indexer);
	
	gtk_source_completion_add_provider (source_completion, 
	                                    GTK_SOURCE_COMPLETION_PROVIDER (completion_method), 
	                                    NULL);
}
