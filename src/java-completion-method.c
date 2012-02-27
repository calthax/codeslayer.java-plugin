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
#include "java-completion-method.h"
#include "java-indexer-index.h"

static void java_completion_provider_interface_init  (gpointer                    page, 
                                                      gpointer                    data);
static void java_completion_method_class_init        (JavaCompletionMethodClass  *klass);
static void java_completion_method_init              (JavaCompletionMethod       *method);
static void java_completion_method_finalize          (JavaCompletionMethod       *method);

static gboolean java_completion_has_match            (JavaCompletionMethod       *method, 
                                                      GtkTextIter                 iter);
static GList* java_completion_get_proposals          (JavaCompletionMethod       *method, 
                                                      GtkTextIter                 iter);

#define JAVA_COMPLETION_METHOD_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_COMPLETION_METHOD_TYPE, JavaCompletionMethodPrivate))

typedef struct _JavaCompletionMethodPrivate JavaCompletionMethodPrivate;

struct _JavaCompletionMethodPrivate
{
  CodeSlayerEditor *editor;
  JavaIndexer      *indexer;
};

G_DEFINE_TYPE_EXTENDED (JavaCompletionMethod,
                        java_completion_method,
                        G_TYPE_INITIALLY_UNOWNED,
                        0,
                        G_IMPLEMENT_INTERFACE (CODESLAYER_COMPLETION_PROVIDER_TYPE,
                                               java_completion_provider_interface_init));

static void
java_completion_provider_interface_init (gpointer provider, 
                                         gpointer data)
{
  CodeSlayerCompletionProviderInterface *provider_interface = (CodeSlayerCompletionProviderInterface*) provider;
  provider_interface->has_match = (gboolean (*) (CodeSlayerCompletionProvider *obj, GtkTextIter iter)) java_completion_has_match;
  provider_interface->get_proposals = (GList* (*) (CodeSlayerCompletionProvider *obj, GtkTextIter iter)) java_completion_get_proposals;
}

static void 
java_completion_method_class_init (JavaCompletionMethodClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) java_completion_method_finalize;
  g_type_class_add_private (klass, sizeof (JavaCompletionMethodPrivate));
}

static void
java_completion_method_init (JavaCompletionMethod *method)
{
}

static void
java_completion_method_finalize (JavaCompletionMethod *method)
{
  G_OBJECT_CLASS (java_completion_method_parent_class)->finalize (G_OBJECT (method));
}

JavaCompletionMethod*
java_completion_method_new (CodeSlayerEditor *editor, 
                            JavaIndexer      *indexer)
{
  JavaCompletionMethodPrivate *priv;
  JavaCompletionMethod *method;

  method = JAVA_COMPLETION_METHOD (g_object_new (java_completion_method_get_type (), NULL));
  priv = JAVA_COMPLETION_METHOD_GET_PRIVATE (method);
  priv->editor = editor;
  priv->indexer = indexer;

  return method;
}

static gboolean 
java_completion_has_match (JavaCompletionMethod *method, 
                           GtkTextIter           iter)
{
  GtkTextIter start;
  gchar *text;
  
  start = iter;
  
  gtk_text_iter_backward_char (&start);
  
  text = gtk_text_iter_get_text (&start, &iter);
  
  if (g_strcmp0 (text, ".") == 0)
    {
      g_free (text);
      return TRUE;
    }

  g_free (text);  
  return FALSE;
}

static GList* 
java_completion_get_proposals (JavaCompletionMethod *method, 
                               GtkTextIter           iter)
{
  JavaCompletionMethodPrivate *priv;
  GList *proposals = NULL;
  GtkTextBuffer *buffer;
  GtkTextMark *mark;
  GList *indexes; 
  GList *list; 

  priv = JAVA_COMPLETION_METHOD_GET_PRIVATE (method);
  
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->editor));
  mark = gtk_text_buffer_create_mark (buffer, NULL, &iter, TRUE);

  indexes = java_indexer_get_indexes (priv->indexer, priv->editor, iter);
  list = indexes;
  
  while (indexes != NULL)
    {
      JavaIndexerIndex *index = indexes->data;
      const gchar *name;
      name = java_indexer_index_get_method_name (index);

      if (g_strcmp0 (name, "<init>") != 0)
        {
          CodeSlayerCompletionProposal *proposal;
          gchar *method_parameters;
          gchar *method_completion;
          
          method_parameters = g_strconcat (name, 
                                           java_indexer_index_get_method_parameters (index),
                                           " ",
                                           java_indexer_index_get_method_return_type (index),
                                           NULL);
          method_completion = g_strconcat (name, 
                                           java_indexer_index_get_method_completion (index), 
                                           NULL);
          
          proposal = codeslayer_completion_proposal_new (method_parameters, method_completion, mark);
          proposals = g_list_prepend (proposals, proposal);
                                                                
          g_free (method_parameters);	                                              
          g_free (method_completion);	                                              
        }
      
      indexes = g_list_next (indexes);
    }
    
  if (list != NULL)
    {
      g_list_foreach (list, (GFunc) g_object_unref, NULL);
      g_list_free (list);
    }
    
  return proposals; 
}
