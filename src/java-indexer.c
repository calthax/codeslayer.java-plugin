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
#include <stdio.h>
#include <stdlib.h>
#include "java-indexer.h"
#include "java-indexer-index.h"
#include "java-configuration.h"
#include "java-utils.h"

static void java_indexer_class_init  (JavaIndexerClass *klass);
static void java_indexer_init        (JavaIndexer      *indexer);
static void java_indexer_finalize    (JavaIndexer      *indexer);

#define JAVA_INDEXER_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_INDEXER_TYPE, JavaIndexerPrivate))

typedef struct _JavaIndexerPrivate JavaIndexerPrivate;

struct _JavaIndexerPrivate
{
  CodeSlayer         *codeslayer;
  JavaConfigurations *configurations;
  gulong              editor_added_id;
  GList              *list;
  GHashTable         *package_hash_table;
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
  JavaIndexerPrivate *priv;
  priv = JAVA_INDEXER_GET_PRIVATE (indexer);  
  priv->package_hash_table = g_hash_table_new (g_str_hash, g_str_equal);
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

  return indexer;
}

void
java_indexer_load_index (JavaIndexer *indexer)
{
  JavaIndexerPrivate *priv;
  GList *list;

  priv = JAVA_INDEXER_GET_PRIVATE (indexer);

  list = codeslayer_utils_get_gobjects (JAVA_INDEXER_INDEX_TYPE,
                                             FALSE,
                                             "/home/jeff/.codeslayer-dev/groups/java/indexes/jmesa.xml", 
                                             "index",
                                             "name", G_TYPE_STRING, 
                                             "parameters", G_TYPE_STRING, 
                                             "modifier", G_TYPE_STRING, 
                                             "class_name", G_TYPE_STRING, 
                                             "package_name", G_TYPE_STRING, 
                                             "parameters", G_TYPE_STRING, 
                                             "file_path", G_TYPE_STRING, 
                                             "line_number", G_TYPE_INT, 
                                             NULL);
  priv->list = list;
  
  while (list != NULL)
    {
      JavaIndexerIndex *index = list->data;
      gchar *full_package_name;
      GList *indexes = NULL;
      full_package_name = java_indexer_index_get_full_package_name (index);
      indexes = g_hash_table_lookup (priv->package_hash_table, full_package_name);
      indexes = g_list_prepend (indexes, index);
      g_hash_table_insert (priv->package_hash_table, full_package_name, indexes);
      list = g_list_next (list);
    }
}

GList*
java_indexer_get_package_indexes (JavaIndexer *indexer, 
                                  gchar       *package_name)
{
  JavaIndexerPrivate *priv;
  GList *indexes = NULL;
  priv = JAVA_INDEXER_GET_PRIVATE (indexer);
  indexes = g_hash_table_lookup (priv->package_hash_table, package_name);
  return indexes;
}
