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
#include "java-completion-class.h"
#include "java-indexer-index.h"
#include "java-utils.h"

static void java_completion_provider_interface_init  (gpointer                    page, 
                                                      gpointer                    data);
static void java_completion_klass_class_init         (JavaCompletionKlassClass  *klass);
static void java_completion_klass_init               (JavaCompletionKlass       *klass);
static void java_completion_klass_finalize           (JavaCompletionKlass       *klass);

static GList* java_completion_get_proposals          (JavaCompletionKlass       *klass, 
                                                      GtkTextIter                 iter);                                                      
static GList* get_class_indexes                      (JavaCompletionKlass       *klass, 
                                                      const gchar               *class_name);
static GList* get_class_indexes_by_file_name         (gchar                     *group_folder_path,
                                                      const gchar               *index_file_name,
                                                      const gchar               *class_name);
static gboolean has_match                            (GtkTextIter                 start);

#define JAVA_COMPLETION_KLASS_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_COMPLETION_KLASS_TYPE, JavaCompletionKlassPrivate))

typedef struct _JavaCompletionKlassPrivate JavaCompletionKlassPrivate;

struct _JavaCompletionKlassPrivate
{
  CodeSlayer       *codeslayer;
  CodeSlayerEditor *editor;
};

G_DEFINE_TYPE_EXTENDED (JavaCompletionKlass,
                        java_completion_klass,
                        G_TYPE_INITIALLY_UNOWNED,
                        0,
                        G_IMPLEMENT_INTERFACE (CODESLAYER_COMPLETION_PROVIDER_TYPE,
                                               java_completion_provider_interface_init));

static void
java_completion_provider_interface_init (gpointer provider, 
                                         gpointer data)
{
  CodeSlayerCompletionProviderInterface *provider_interface = (CodeSlayerCompletionProviderInterface*) provider;
  provider_interface->get_proposals = (GList* (*) (CodeSlayerCompletionProvider *obj, GtkTextIter iter)) java_completion_get_proposals;
}

static void 
java_completion_klass_class_init (JavaCompletionKlassClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) java_completion_klass_finalize;
  g_type_class_add_private (klass, sizeof (JavaCompletionKlassPrivate));
}

static void
java_completion_klass_init (JavaCompletionKlass *klass)
{
}

static void
java_completion_klass_finalize (JavaCompletionKlass *klass)
{
  G_OBJECT_CLASS (java_completion_klass_parent_class)->finalize (G_OBJECT (klass));
}

JavaCompletionKlass*
java_completion_klass_new (CodeSlayer       *codeslayer, 
                           CodeSlayerEditor *editor)
{
  JavaCompletionKlassPrivate *priv;
  JavaCompletionKlass *klass;

  klass = JAVA_COMPLETION_KLASS (g_object_new (java_completion_klass_get_type (), NULL));
  priv = JAVA_COMPLETION_KLASS_GET_PRIVATE (klass);
  priv->codeslayer = codeslayer;
  priv->editor = editor;

  return klass;
}

static GList* 
java_completion_get_proposals (JavaCompletionKlass *klass, 
                               GtkTextIter          iter)
{
  JavaCompletionKlassPrivate *priv;
  GList *proposals = NULL;
  GtkTextBuffer *buffer;
  GtkTextIter start;
  GtkTextMark *mark;
  GList *indexes;
  GList *list; 
  gchar *text;
  
  priv = JAVA_COMPLETION_KLASS_GET_PRIVATE (klass);
  
  start = iter;
  java_utils_move_iter_word_start (&start);
  
  if (!has_match (start))
    return NULL;

  text = gtk_text_iter_get_text (&start, &iter);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->editor));
  mark = gtk_text_buffer_create_mark (buffer, NULL, &start, TRUE);
  indexes = get_class_indexes (klass, text);
  
  list = indexes;
  
  while (indexes != NULL)
    {
      JavaIndexerIndex *index = indexes->data;
      CodeSlayerCompletionProposal *proposal;
      const gchar *name;
      name = java_indexer_index_get_class_name (index);
      proposal = codeslayer_completion_proposal_new (name, name, mark);
      proposals = g_list_prepend (proposals, proposal);
      indexes = g_list_next (indexes);
    }
    
  if (list != NULL)
    {
      g_list_foreach (list, (GFunc) g_object_unref, NULL);
      g_list_free (list);
    }

  g_free (text);  

  return proposals; 
}

static gboolean
has_match (GtkTextIter start)
{
  gboolean result = TRUE;
  GtkTextIter next;
  gchar *text;
  
  next = start;
  
  gtk_text_iter_forward_char (&next);
  text = gtk_text_iter_get_text (&start, &next);
  
  if (!g_ascii_isupper (*text))
    result = FALSE;
    
  g_free (text);
  
  return result;
}

static GList*
get_class_indexes (JavaCompletionKlass *klass, 
                   const gchar         *class_name)
{
  JavaCompletionKlassPrivate *priv;
  GList *indexes = NULL;
  GList *projects_indexes = NULL;
  GList *libs_indexes = NULL;
  gchar *group_folder_path;
  
  priv = JAVA_COMPLETION_KLASS_GET_PRIVATE (klass);

  group_folder_path = codeslayer_get_active_group_folder_path (priv->codeslayer);
  
  projects_indexes = get_class_indexes_by_file_name (group_folder_path, "projects.classes", class_name);
  libs_indexes = get_class_indexes_by_file_name (group_folder_path, "libs.classes", class_name);
  
  indexes = g_list_concat (projects_indexes, libs_indexes);
    
  g_free (group_folder_path);

  return indexes;
}

static GList*
get_class_indexes_by_file_name (gchar       *group_folder_path,
                                const gchar *index_file_name,
                                const gchar *class_name)
{
  GList *indexes = NULL;  
  gchar *file_name;
  GIOChannel *channel;
  gchar *text;
  GError *error = NULL;
  
  file_name = g_build_filename (group_folder_path, "indexes", index_file_name, NULL);

  if (!g_file_test (file_name, G_FILE_TEST_EXISTS))
    {
      g_warning ("There is no %s file.", index_file_name);
      g_free (file_name);
      return indexes;
    }
  
  channel = g_io_channel_new_file (file_name, "r", &error);
  
  while (g_io_channel_read_line (channel, &text, NULL, NULL, NULL) == G_IO_STATUS_NORMAL)
    {
      gchar **split;
      gchar **array;
    
      split = g_strsplit (text, "\t", -1);
      array = split;
      
      if (g_str_has_prefix (*array, class_name))
        {
          JavaIndexerIndex *index;
          index = java_indexer_index_new ();
          java_indexer_index_set_class_name (index, *array);
          java_indexer_index_set_package_name (index, *++array);
          java_indexer_index_set_file_path (index, g_strstrip(*++array));
          indexes = g_list_prepend (indexes, index);
        }
        
      g_strfreev (split);
      g_free (text);
    }
    
  g_io_channel_shutdown(channel, FALSE, NULL);
  g_io_channel_unref (channel);  
  g_free (file_name);

  return indexes;
}
