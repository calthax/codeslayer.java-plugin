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

#include "java-completion.h"
#include "java-completion-method.h"
#include "java-completion-class.h"
#include "java-completion-word.h"

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
  CodeSlayer         *codeslayer;
  JavaClient         *client;
  gulong              editor_added_id;
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
java_completion_new (CodeSlayer         *codeslayer, 
                     JavaClient         *client)
{
  JavaCompletionPrivate *priv;
  JavaCompletion *completion;

  completion = JAVA_COMPLETION (g_object_new (java_completion_get_type (), NULL));
  priv = JAVA_COMPLETION_GET_PRIVATE (completion);
  priv->codeslayer = codeslayer;
  priv->client = client;
  
  priv->editor_added_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "editor-added",
                                                    G_CALLBACK (editor_added_action), completion);
                     
  return completion;
}

static void
editor_added_action (JavaCompletion   *completion,
                     CodeSlayerEditor *editor)
{
  JavaCompletionPrivate *priv;
  JavaCompletionWord *word;
  JavaCompletionMethod *method;
  JavaCompletionKlass *class;
  
  priv = JAVA_COMPLETION_GET_PRIVATE (completion);

  word = java_completion_word_new (editor);
  method = java_completion_method_new (priv->codeslayer, editor, priv->client);
  class = java_completion_klass_new (priv->codeslayer, editor, priv->client);
  
  codeslayer_editor_add_completion_provider (editor, 
                                             CODESLAYER_COMPLETION_PROVIDER (word));
                                             
  codeslayer_editor_add_completion_provider (editor, 
                                             CODESLAYER_COMPLETION_PROVIDER (method));

  codeslayer_editor_add_completion_provider (editor, 
                                             CODESLAYER_COMPLETION_PROVIDER (class));  
}
