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

#include "java-indexer-methods.h"
#include "java-indexer-method.h"
#include <codeslayer/codeslayer-utils.h>

static void java_indexer_methods_class_init  (JavaIndexerMethodsClass *klass);
static void java_indexer_methods_init        (JavaIndexerMethods      *methods);
static void java_indexer_methods_finalize    (JavaIndexerMethods      *methods);

static void load_indexer                     (JavaIndexerMethods      *methods);                                      

#define JAVA_INDEXER_METHODS_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_INDEXER_METHODS_TYPE, JavaIndexerMethodsPrivate))

typedef struct _JavaIndexerMethodsPrivate JavaIndexerMethodsPrivate;

struct _JavaIndexerMethodsPrivate
{
  GList *list;
};

G_DEFINE_TYPE (JavaIndexerMethods, java_indexer_methods, G_TYPE_OBJECT)
     
static void 
java_indexer_methods_class_init (JavaIndexerMethodsClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) java_indexer_methods_finalize;
  g_type_class_add_private (klass, sizeof (JavaIndexerMethodsPrivate));
}

static void
java_indexer_methods_init (JavaIndexerMethods *methods)
{
  JavaIndexerMethodsPrivate *priv;
  priv = JAVA_INDEXER_METHODS_GET_PRIVATE (methods);
  priv->list = NULL;
}

static void
java_indexer_methods_finalize (JavaIndexerMethods *methods)
{
  JavaIndexerMethodsPrivate *priv;
  priv = JAVA_INDEXER_METHODS_GET_PRIVATE (methods);
  if (priv->list != NULL)
    {
      g_list_foreach (priv->list, (GFunc) g_object_unref, NULL);
      g_list_free (priv->list);
      priv->list = NULL;    
    }
  G_OBJECT_CLASS (java_indexer_methods_parent_class)->finalize (G_OBJECT (methods));
}

JavaIndexerMethods*
java_indexer_methods_new ()
{
  JavaIndexerMethods *methods;
  methods = JAVA_INDEXER_METHODS  (g_object_new (java_indexer_methods_get_type (), NULL));
  
  load_indexer (methods);
  
  return methods;
}

static void
load_indexer (JavaIndexerMethods *methods)
{
  JavaIndexerMethodsPrivate *priv;
  GList *list;

  priv = JAVA_INDEXER_METHODS_GET_PRIVATE (methods);

  priv->list = codeslayer_utils_get_gobjects (JAVA_INDEXER_METHOD_TYPE,
                                                 FALSE,
                                                 "/home/jeff/.codeslayer-dev/groups/java/indexes/methods/jmesa.xml", 
                                                 "method",
                                                 "name", G_TYPE_STRING, 
                                                 "parameters", G_TYPE_STRING, 
                                                 "modifier", G_TYPE_STRING, 
                                                 "class_name", G_TYPE_STRING, 
                                                 "parameters", G_TYPE_STRING, 
                                                 "file_path", G_TYPE_STRING, 
                                                 "line_number", G_TYPE_INT, 
                                                 NULL);
               
  list = priv->list;
  
  while (list != NULL)
    {
      JavaIndexerMethod *method = list->data;
      g_print ("method: %s\n", java_indexer_method_get_name (method));
      list = g_list_next (list);
    }                                                 
}
