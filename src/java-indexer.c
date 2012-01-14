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
#include "java-indexer.h"
#include "java-indexer-methods.h"
#include "java-indexer-method.h"
#include "java-configuration.h"
#include "java-utils.h"

static void java_indexer_class_init  (JavaIndexerClass      *klass);
static void java_indexer_init        (JavaIndexer           *indexer);
static void java_indexer_finalize    (JavaIndexer           *indexer);

static void editor_added_action      (JavaIndexer           *indexer,
                                      CodeSlayerEditor      *editor);
                                       
#define JAVA_INDEXER_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_INDEXER_TYPE, JavaIndexerPrivate))

typedef struct _JavaIndexerPrivate JavaIndexerPrivate;

struct _JavaIndexerPrivate
{
  CodeSlayer         *codeslayer;
  JavaConfigurations *configurations;
  gulong              editor_added_id;
  JavaIndexerMethods *methods;
};

G_DEFINE_TYPE (JavaIndexer, java_indexer, G_TYPE_OBJECT)
     
static void 
java_indexer_class_init (JavaIndexerClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) java_indexer_finalize;
  g_type_class_add_private (klass, sizeof (JavaIndexerPrivate));
}

static void
java_indexer_init (JavaIndexer *indexer)
{
}

static void
java_indexer_finalize (JavaIndexer *indexer)
{
  JavaIndexerPrivate *priv;
  priv = JAVA_INDEXER_GET_PRIVATE (indexer);  
  g_signal_handler_disconnect (priv->codeslayer, priv->editor_added_id);
  G_OBJECT_CLASS (java_indexer_parent_class)->finalize (G_OBJECT (indexer));
}

JavaIndexer*
java_indexer_new (CodeSlayer         *codeslayer,
                  JavaConfigurations *configurations)
{
  JavaIndexerPrivate *priv;
  JavaIndexer *indexer;

  indexer = JAVA_INDEXER (g_object_new (java_indexer_get_type (), NULL));
  priv = JAVA_INDEXER_GET_PRIVATE (indexer);
  priv->codeslayer = codeslayer;
  priv->configurations = configurations;
  priv->methods = java_indexer_methods_new ();
  
  priv->editor_added_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "editor-added",
                                                    G_CALLBACK (editor_added_action), indexer);
                     
  return indexer;
}

static void
editor_added_action (JavaIndexer     *indexer,
                     CodeSlayerEditor *editor)
{
	GtkSourceCompletionWords *prov_words;
	GtkSourceCompletion *comp;
  
	comp = gtk_source_view_get_completion (GTK_SOURCE_VIEW (editor));
	prov_words = gtk_source_completion_words_new (NULL, NULL);
	
	gtk_source_completion_words_register (prov_words,
	                                      gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor)));
	
	gtk_source_completion_add_provider (comp, 
	                                    GTK_SOURCE_COMPLETION_PROVIDER (prov_words), 
	                                    NULL);

	g_object_set (prov_words, "priority", 10, NULL);
}
