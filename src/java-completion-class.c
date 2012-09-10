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
#include "java-utils.h"

static void java_completion_provider_interface_init  (gpointer                    page, 
                                                      gpointer                    data);
static void java_completion_klass_class_init         (JavaCompletionKlassClass  *klass);
static void java_completion_klass_init               (JavaCompletionKlass       *klass);
static void java_completion_klass_finalize           (JavaCompletionKlass       *klass);

static GList* java_completion_get_proposals          (JavaCompletionKlass       *klass, 
                                                      GtkTextIter                 iter);                                                      
static gboolean has_match                            (GtkTextIter                 start);

static gchar* get_input                              (JavaCompletionKlass       *klass, 
                                                      const gchar               *file_path, 
                                                      gchar                     *expression, 
                                                      gint                       line_number);                                                      
static GList* render_output                          (JavaCompletionKlass       *klass, 
                                                      gchar                     *output, 
                                                      GtkTextMark               *mark);
static CodeSlayerCompletionProposal*  render_line    (JavaCompletionKlass       *klass, 
                                                      gchar                     *line, 
                                                      GtkTextMark               *mark);

#define JAVA_COMPLETION_KLASS_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_COMPLETION_KLASS_TYPE, JavaCompletionKlassPrivate))

typedef struct _JavaCompletionKlassPrivate JavaCompletionKlassPrivate;

struct _JavaCompletionKlassPrivate
{
  CodeSlayer       *codeslayer;
  CodeSlayerEditor *editor;
  JavaClient       *client;
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
                           CodeSlayerEditor *editor, 
                           JavaClient       *client)
{
  JavaCompletionKlassPrivate *priv;
  JavaCompletionKlass *klass;

  klass = JAVA_COMPLETION_KLASS (g_object_new (java_completion_klass_get_type (), NULL));
  priv = JAVA_COMPLETION_KLASS_GET_PRIVATE (klass);
  priv->codeslayer = codeslayer;
  priv->editor = editor;
  priv->client = client;

  return klass;
}

static GList* 
java_completion_get_proposals (JavaCompletionKlass *klass, 
                               GtkTextIter          iter)
{
  JavaCompletionKlassPrivate *priv;
  GList *proposals = NULL;
  const gchar *file_path;
  gchar *text;
  GtkTextIter start;
  GtkTextBuffer *buffer;
  gint line_number;
  gchar *input;
  gchar *output;
  
  priv = JAVA_COMPLETION_KLASS_GET_PRIVATE (klass);
  
  start = iter;
  java_utils_move_iter_word_start (&start);
  
  if (!has_match (start))
    return NULL;
    
  file_path = codeslayer_editor_get_file_path (priv->editor);
  if (!g_str_has_suffix (file_path, ".java"))
    return NULL;
  
  line_number = gtk_text_iter_get_line (&iter);
  
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->editor));

  text = gtk_text_iter_get_text (&start, &iter);
  
  input = get_input (klass, file_path, text, line_number);

  g_print ("input: %s\n", input);

  output = java_client_send (priv->client, input);
  
  if (output != NULL)
    {
      GtkTextMark *mark;
      mark = gtk_text_buffer_create_mark (buffer, NULL, &start, TRUE);
      g_print ("output: %s\n", output);
      proposals = render_output (klass, output, mark);
      g_free (output);
    }

  g_free (input);
  g_free (text);

  return proposals; 
}

static gchar* 
get_input (JavaCompletionKlass *klass, 
           const gchar         *file_path, 
           gchar               *expression, 
           gint                 line_number)
{
  JavaCompletionKlassPrivate *priv;

  gchar *indexes_folders;
  gchar *line_number_str;
  gchar *result;
  
  priv = JAVA_COMPLETION_KLASS_GET_PRIVATE (klass);
  
  line_number_str = g_strdup_printf ("%d", (line_number + 1));
  
  indexes_folders = java_utils_get_indexes_folder (priv->codeslayer);
  
  result = g_strconcat ("-program completion", 
                        " -type ", "class",
                        " -sourcefile ", file_path,
                        " -expression ", expression,
                        " -linenumber ", line_number_str,
                        indexes_folders, 
                        NULL);
  
  g_free (indexes_folders);
  g_free (line_number_str);

  return result;
}

static GList*
render_output (JavaCompletionKlass *klass, 
               gchar               *output, 
               GtkTextMark         *mark)
{
  GList *proposals = NULL;
  gchar **split;
  gchar **tmp;
  
  if (!codeslayer_utils_has_text (output))
    return NULL;
  
  split = g_strsplit (output, "\n", -1);

  if (split != NULL)
    {
      tmp = split;
      while (*tmp != NULL)
        {
          CodeSlayerCompletionProposal *proposal;
          proposal = render_line (klass, *tmp, mark);
          if (proposal != NULL)
            proposals = g_list_append (proposals, proposal);
          tmp++;
        }
      g_strfreev (split);
    }
    
   return proposals;   
}

static CodeSlayerCompletionProposal*
render_line (JavaCompletionKlass *klass, 
             gchar               *line, 
             GtkTextMark         *mark)
{
  gchar **split;
  gchar **tmp;
  
  if (!codeslayer_utils_has_text (line))
    return NULL;
  
  split = g_strsplit (line, "\t", -1);
  if (split != NULL)
    {
      CodeSlayerCompletionProposal *proposal = NULL;
      gchar *simple_class_name;  
      
      gchar *match_label;
      gchar *match_text;
      
      tmp = split;

      simple_class_name = *tmp;
      
      if (simple_class_name != NULL)
        {
          match_label = g_strdup_printf ("%s", simple_class_name);
          match_text = g_strdup_printf ("%s", simple_class_name);
          
          proposal = codeslayer_completion_proposal_new (match_label, g_strstrip (match_text), mark);
          
          g_free (match_label);
          g_free (match_text);

        }      

      g_strfreev (split);

      return proposal;
    }
  
  return NULL;
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
