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
static gchar* find_completion_path                   (JavaCompletionMethod       *method,
                                                      GtkTextIter                 iter);
static gchar* strip_completion_path_comments         (gchar                      *text);
static gchar* strip_completion_path_parameters       (gchar                      *text);
static GList* get_completion_list                    (JavaCompletionMethod       *method,
                                                      GtkTextIter                 iter, 
                                                      gchar                      *text);
static gchar* get_text_to_search                     (GtkTextIter                 iter,
                                                      CodeSlayerEditor           *editor);
static gchar* search_text_for_variable               (gchar                      *variable, 
                                                      const gchar                *string);
static GList* search_text_for_potential_imports      (gchar                      *variable, 
                                                      const gchar                *string);
static GList* get_valid_import_indexes               (JavaCompletionMethod       *method, 
                                                      GList                      *imports);
static gint sort_indexes                             (JavaIndexerIndex           *index1, 
                                                      JavaIndexerIndex           *index2);

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
                        G_TYPE_OBJECT,
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
  GList *proposals = NULL;
  gchar *completion_path;

  completion_path = find_completion_path (method, iter);
  
  if (codeslayer_utils_has_text (completion_path))
    {
      gchar *comments_stripped;
      gchar *parameters_stripped;
      GList *list;
      
      comments_stripped = strip_completion_path_comments (completion_path);
      parameters_stripped = strip_completion_path_parameters (comments_stripped);
      parameters_stripped = g_strreverse (parameters_stripped);
      
      list = get_completion_list (method, iter, parameters_stripped);
      if (list != NULL)
        list = g_list_sort (list, (GCompareFunc) sort_indexes);
      
      while (list != NULL)
        {
          JavaIndexerIndex *index = list->data;
          const gchar *name;
          name = java_indexer_index_get_name (index);

          if (g_strcmp0 (name, "<init>") != 0)
            {
              CodeSlayerCompletionProposal *proposal;
              const gchar *parameters;
              gchar *concat;
              
              parameters = java_indexer_index_get_parameters (index);
              concat = g_strconcat (name, "(", parameters, ")", NULL);
              
              proposal = codeslayer_completion_proposal_new (concat, concat);
              
              proposals = g_list_prepend (proposals, proposal);
	                                                                  
              g_free (concat);	                                              
            }
          
          list = g_list_next (list);
        }
      
      g_free (completion_path);
      g_free (comments_stripped);
      g_free (parameters_stripped);
    }
    
  return proposals; 
}

static gint                
sort_indexes (JavaIndexerIndex *index1, 
              JavaIndexerIndex *index2)
{
  const gchar *name1;
  const gchar *name2;
  name1 = java_indexer_index_get_name (index1);
  name2 = java_indexer_index_get_name (index2);  
  return g_strcmp0 (name2, name1);
}

/*
 * Walk backwards and match braces to find the completion path.
 *
 * For example at any point in the following path you would be able to 
 * find the context, namely the tableModel, or the presidentService.
 *
 * tableModel.setItems(presidentService.getPresidents())
 */
static gchar*
find_completion_path (JavaCompletionMethod *method,
                      GtkTextIter           iter)
{
  gchar *result;
  GString *string;
  gchar *text;
  gchar *variable;
  GtkTextIter start;
  int brace = 0;
  
  string = g_string_new ("");

  start = iter;
  gtk_text_iter_backward_chars (&start, 100);
  
  text = gtk_text_iter_get_text (&start, &iter);
  variable = text;
  variable = g_strreverse (variable);  
  
  for (; *variable != '\0'; ++variable)
    {
      if (*variable == '=' ||
          *variable == ';' ||
          *variable == '{')
        break;
      
      if (*variable == ')')
        brace++;
      if (*variable == '(')
        {
          brace--;
          if (brace == -1)
            break;
        }
        
      string = g_string_append_c (string, *variable);
    }
  
  g_free (text);
  
  result = g_string_free (string, FALSE);
  g_strstrip (result);
    
  return result;
}

/*
 * Take out the comments so that they do not effect how the completion
 * path is walked to find the context of the variable and methods.
 *
 * new DateFilterMatcher("MM/yyyy") becomes new DateFilterMatcher()
 */
static gchar*
strip_completion_path_comments (gchar *text)
{
  gchar *result;
  GRegex *regex;
  GError *error = NULL;

  regex = g_regex_new ("\"([^\"\\\\]*(\\.[^\"\\\\]*)*)\"", 0, 0, &error);
  
  if (error != NULL)
    {
      g_printerr (error->message);
      g_error_free (error);
    }
  
  result = g_regex_replace (regex, text, -1, 0, "", 0, NULL);
  
  g_regex_unref (regex);

  return result;
}

/*
 * Take out everything in the methods (between the braces) that do not 
 * belong in the context of the path being matched so that they do not 
 * effect how the completion path is walked to find the context of the 
 * variable and methods.
 *
 * tableModel.setItems(presidentService.getPresidents()) 
 * becomes tableModel.setItems() if we are in the context of the tableModel.
 */
static gchar*
strip_completion_path_parameters (gchar *text)
{
  gchar *result;
  GString *string;
  int brace = 0;
  
  string = g_string_new ("");

  for (; *text != '\0'; ++text)
    {
      if (*text == ')')
        {
          brace++;
          if (brace == 1)
            string = g_string_append_c (string, *text);
          continue;
        }
      if (*text == '(')
        {
          brace--;
          if (brace == 0)
            string = g_string_append_c (string, *text);
          continue;
        }
      
      if (brace == 0)
        string = g_string_append_c (string, *text);
    }
  
  result = g_string_free (string, FALSE);
  g_strstrip(result);
    
  return result;
}

/*
 * Take the completion path, start at the beginning, and resolve the 
 * actual classes and methods types.
 *
 * For example after the title figure out that this starts as a Column 
 * object and that the title() method also returns a Column object.
 *
 * new Column().title().
 *
 */
static GList*
get_completion_list (JavaCompletionMethod *method,
                     GtkTextIter           iter, 
                     gchar                *string)
{
  JavaCompletionMethodPrivate *priv;
  gchar **split;
  gchar **array;
  gchar *variable = NULL;
  GList *indexes = NULL;

  priv = JAVA_COMPLETION_METHOD_GET_PRIVATE (method);
  
  split = g_strsplit (string, ".", -1);
  array = split;
  
  if (codeslayer_utils_has_text (*array))
    {
      gchar *text;   
      GList *imports;
      text = get_text_to_search (iter, priv->editor);
      variable = search_text_for_variable (*array, text);
      
      if (codeslayer_utils_has_text (variable))
        {
          imports = search_text_for_potential_imports (variable, text);
          indexes = get_valid_import_indexes (method, imports);
          if (imports != NULL)
            {
              g_list_foreach (imports, (GFunc) g_free, NULL);
              g_list_free (imports);
            }
        }
      
      g_free (text);
      array++;
    }

  /*g_print ("variable %s\n", variable);*/
  
  while (codeslayer_utils_has_text (*array))
    {
      /*g_print ("other %s\n", *array);*/
      array++;
    }
  
  if (split != NULL)
    g_strfreev (split);
  
  if (variable != NULL)
    g_free (variable);
    
  return indexes;
}

/*
 * Start at the current place in the editor and get all the previous text.
 * The idea is that any variables that need to be resolved will come 
 * before this place in the editor.
 */
static gchar*
get_text_to_search (GtkTextIter       iter, 
                    CodeSlayerEditor *editor)
{
  GtkTextBuffer *buffer;
  GtkTextIter start;
  gchar *text;      

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));      
  gtk_text_buffer_get_start_iter (buffer, &start);
  text = gtk_text_buffer_get_text (buffer, &start, &iter, FALSE);

  return text;
}

/*
 * Take the variable and find the type. For instance in the following
 * example the tableModel will be resolved to TableModel.
 *  
 * TableModel tableModel
 * tableModel.addFilterMatcher()
 */
static gchar*
search_text_for_variable (gchar       *variable, 
                          const gchar *text)
{
  gchar *result = NULL;
  GRegex *regex;
  gchar *concat;
  GMatchInfo *match_info;
  GError *error = NULL;
  
  concat = g_strconcat ("[a-zA-Z0-9_]+\\s+\\b", variable, "\\b", NULL); 
   
  regex = g_regex_new (concat, 0, 0, NULL);
  
  g_regex_match_full (regex, text, -1, 0, 0, &match_info, &error);
  
  while (g_match_info_matches (match_info))
    {
      result = g_match_info_fetch (match_info, 0);
      if (g_match_info_next (match_info, &error))
        {
          g_free (result);
        }
      else
        {
          gchar **split;      
          split = g_strsplit (result, " ", -1);
          g_free (result);
          result = g_strdup (*split);
          g_strstrip (result);
          g_strfreev (split);
        }
    }
  
  g_match_info_free (match_info);
  g_regex_unref (regex);
  g_free (concat);
  
  if (error != NULL)
    {
      g_printerr ("search text for variable error: %s\n", error->message);
      g_error_free (error);
    }
    
  return result;   
}

/*
 * Take the variable and find all the potential imports that could be of this type.
 * One thing to be aware of is we need to replace the '*' with the class in case
 * the '*' is used instead of the full package declaration.
 * 
 * import org.jmesa.model.*;
 * import org.jmesa.model.TableModel;
 */
static GList*
search_text_for_potential_imports (gchar       *variable, 
                                   const gchar *text)
{
  GList *results = NULL;
  GRegex *regex;
  GMatchInfo *match_info;
  GError *error = NULL;
  
  regex = g_regex_new ("import\\s+(.*?);", 0, 0, NULL);
  
  g_regex_match_full (regex, text, -1, 0, 0, &match_info, &error);
  
  while (g_match_info_matches (match_info))
    {
      gchar *import = NULL;
      import = g_match_info_fetch (match_info, 1);
      g_match_info_next (match_info, &error);
      g_strstrip(import);
      if (g_str_has_suffix (import, "*"))
        {
          gchar *replace; 
          replace = codeslayer_utils_strreplace (import, "*", variable);
          results = g_list_prepend (results, g_strdup (replace));
          g_free (replace);
        }
      else
        {
          gchar *suffix = NULL;
          suffix = g_strconcat (".", variable, NULL);
          if (g_str_has_suffix (import, suffix))
            {
              results = g_list_prepend (results, g_strdup (import));
            }
          g_free (suffix);
        }
      g_free (import);
    }
  
  g_match_info_free (match_info);
  g_regex_unref (regex);
  
  if (error != NULL)
    {
      g_printerr ("search text for import error: %s\n", error->message);
      g_error_free (error);
    }
    
  return results;   
}

/*
 * Take the potential imports and see which ones is real to find the class
 * indexes.
 */
static GList*
get_valid_import_indexes (JavaCompletionMethod *method, 
                          GList                *imports)
{
  JavaCompletionMethodPrivate *priv;
  priv = JAVA_COMPLETION_METHOD_GET_PRIVATE (method);
  
  while (imports != NULL)
    {
      gchar *import = imports->data;
      GList *indexes;
      indexes = java_indexer_get_package_indexes (priv->indexer, import);
      if (indexes != NULL)
        {
          /*g_print ("import %s\n", import);*/
          return indexes;
        }
      imports = g_list_next (imports);
    }
    
  return NULL;    
}
