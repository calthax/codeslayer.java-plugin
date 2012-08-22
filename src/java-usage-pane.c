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

#include "java-usage-pane.h"
#include "java-usage-method.h"

static void java_page_interface_init                         (gpointer            page, 
                                                              gpointer            data);
static void java_usage_pane_class_init                       (JavaUsagePaneClass *klass);
static void java_usage_pane_init                             (JavaUsagePane      *usage_pane);
static void java_usage_pane_finalize                         (JavaUsagePane      *usage_pane);
static JavaPageType java_usage_pane_get_page_type            (JavaUsagePane      *usage_pane);                                      
static JavaConfiguration* java_usage_pane_get_configuration  (JavaUsagePane      *usage_pane);
static void java_usage_pane_set_configuration                (JavaUsagePane      *usage_pane, 
                                                              JavaConfiguration   *configuration);
static CodeSlayerDocument* java_usage_pane_get_document      (JavaUsagePane      *usage_pane);
static void java_usage_pane_set_document                     (JavaUsagePane      *usage_pane, 
                                                              CodeSlayerDocument *document);
static gint sort_iter_compare_func                           (GtkTreeModel       *model, 
                                                              GtkTreeIter        *a,
                                                              GtkTreeIter        *b, 
                                                              gpointer            userdata);
static gboolean select_usage                                 (JavaUsagePane      *usage_pane,
                                                              GtkTreeIter        *treeiter,
                                                              GtkTreeViewColumn  *column);

#define JAVA_USAGE_PANE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_USAGE_PANE_TYPE, JavaUsagePanePrivate))

typedef struct _JavaUsagePanePrivate JavaUsagePanePrivate;

struct _JavaUsagePanePrivate
{
  CodeSlayer         *codeslayer;
  JavaPageType        page_type;
  JavaConfiguration  *configuration;
  CodeSlayerDocument *document;
  GtkWidget          *treeview;
  GtkTreeStore       *treestore;
  GtkCellRenderer    *renderer;
};

enum
{
  FILE_PATH = 0,
  LINE_NUMBER,
  TEXT,
  USAGE,
  COLUMNS
};

G_DEFINE_TYPE_EXTENDED (JavaUsagePane,
                        java_usage_pane,
                        GTK_TYPE_HBOX,
                        0,
                        G_IMPLEMENT_INTERFACE (JAVA_PAGE_TYPE ,
                                               java_page_interface_init));
      
static void
java_page_interface_init (gpointer page, 
                          gpointer data)
{
  JavaPageInterface *page_interface = (JavaPageInterface*) page;
  page_interface->get_page_type = (JavaPageType (*) (JavaPage *obj)) java_usage_pane_get_page_type;
  page_interface->get_configuration = (JavaConfiguration* (*) (JavaPage *obj)) java_usage_pane_get_configuration;
  page_interface->set_configuration = (void (*) (JavaPage *obj, JavaConfiguration*)) java_usage_pane_set_configuration;
  page_interface->get_document = (CodeSlayerDocument* (*) (JavaPage *obj)) java_usage_pane_get_document;
  page_interface->set_document = (void (*) (JavaPage *obj, CodeSlayerDocument*)) java_usage_pane_set_document;
}
      
static void 
java_usage_pane_class_init (JavaUsagePaneClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) java_usage_pane_finalize;
  g_type_class_add_private (klass, sizeof (JavaUsagePanePrivate));
}

static void
java_usage_pane_init (JavaUsagePane *usage_pane) 
{
  JavaUsagePanePrivate *priv;
  GtkWidget *treeview;
  GtkTreeStore *treestore;
  GtkTreeSortable *sortable;
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;
  GtkWidget *scrolled_window;

  priv = JAVA_USAGE_PANE_GET_PRIVATE (usage_pane);

  treeview = gtk_tree_view_new ();
  priv->treeview = treeview;

  treestore = gtk_tree_store_new (COLUMNS, G_TYPE_STRING, G_TYPE_INT, 
                                  G_TYPE_STRING, G_TYPE_POINTER);
  
  priv->treestore = treestore;

  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
  gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (treestore));
  g_object_unref (treestore);

  sortable = GTK_TREE_SORTABLE (treestore);
  gtk_tree_sortable_set_sort_func (sortable, TEXT, sort_iter_compare_func,
                                   GINT_TO_POINTER (TEXT), NULL);
  gtk_tree_sortable_set_sort_column_id (sortable, TEXT, GTK_SORT_ASCENDING);

  column = gtk_tree_view_column_new ();

  renderer = gtk_cell_renderer_text_new ();
  priv->renderer = renderer;

  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_set_attributes (column, renderer, "text", TEXT, NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (treeview));
  gtk_box_pack_start (GTK_BOX (usage_pane), GTK_WIDGET (scrolled_window), 
                      TRUE, TRUE, 2);

  g_signal_connect_swapped (G_OBJECT (treeview), "row_activated",
                            G_CALLBACK (select_usage), usage_pane);
}

static void
java_usage_pane_finalize (JavaUsagePane *usage_pane)
{
  G_OBJECT_CLASS (java_usage_pane_parent_class)->finalize (G_OBJECT (usage_pane));
}

GtkWidget*
java_usage_pane_new (CodeSlayer   *codeslayer, 
                     JavaPageType  page_type)
{
  JavaUsagePanePrivate *priv;
  GtkWidget *usage_pane;
 
  usage_pane = g_object_new (java_usage_pane_get_type (), NULL);
  priv = JAVA_USAGE_PANE_GET_PRIVATE (usage_pane);
  priv->codeslayer = codeslayer;
  priv->page_type = page_type;
  
  return usage_pane;
}

void
java_usage_pane_set_usages (JavaUsagePane *usage_pane, 
                            GList         *usages)
{
  JavaUsagePanePrivate *priv;
  
  priv = JAVA_USAGE_PANE_GET_PRIVATE (usage_pane);
  
  while (usages != NULL)
    {
      JavaUsageMethod *usage_method = usages->data;
      GtkTreeIter iter;
      gchar *line_text;
      gchar *full_text;
      const gchar *class_name;
      gint line_number;
      
      class_name = java_usage_method_get_class_name (usage_method);
      line_number = java_usage_method_get_line_number (usage_method);
      
      line_text = g_strdup_printf ("%d", line_number);
      full_text = g_strconcat ("(", line_text, ") ", class_name, NULL);
      
      g_print ("%s\n", full_text);
      
      gtk_tree_store_append (priv->treestore, &iter, NULL);

      gtk_tree_store_set (priv->treestore, &iter, 
                          FILE_PATH, java_usage_method_get_file_path (usage_method), 
                          LINE_NUMBER, java_usage_method_get_line_number (usage_method), 
                          TEXT, full_text, 
                          USAGE, usage_method, 
                          -1);
                          
      g_free (line_text);
      g_free (full_text);
                          
      usages = g_list_next (usages);
    }
}

static gboolean 
select_usage (JavaUsagePane     *usage_pane,
              GtkTreeIter       *treeiter,
              GtkTreeViewColumn *column)
{
  JavaUsagePanePrivate *priv;
  GtkTreeModel *model;
  GtkTreeIter iter;
  GtkTreeSelection *treeselection;
  
  priv = JAVA_USAGE_PANE_GET_PRIVATE (usage_pane);

  treeselection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview));
  if (gtk_tree_selection_get_selected (treeselection, &model, &iter))
    {
      gchar *file_path = NULL;
      gint line_number;
      JavaUsageMethod *usage;
      CodeSlayerDocument *document;
      CodeSlayerProject *project;

      gtk_tree_model_get (GTK_TREE_MODEL (priv->treestore), &iter,
                          FILE_PATH, &file_path,
                          LINE_NUMBER, &line_number, 
                          USAGE, &usage, -1);

      /*if (file_path == NULL)
        {
          GtkTreePath *tree_path;
          tree_path = gtk_tree_model_get_path (model, &iter);
          if (gtk_tree_view_row_expanded (GTK_TREE_VIEW (priv->treeview), tree_path))
            gtk_tree_view_collapse_row (GTK_TREE_VIEW (priv->treeview), tree_path);
          else
            gtk_tree_view_expand_row (GTK_TREE_VIEW (priv->treeview), tree_path, FALSE);
          return FALSE;
        }*/

      document = codeslayer_document_new ();
      codeslayer_document_set_file_path (document, file_path);
      codeslayer_document_set_line_number (document, line_number);
      
      project = codeslayer_get_project_by_file_path (priv->codeslayer, file_path);
      codeslayer_document_set_project (document, project);
      
      codeslayer_select_editor (priv->codeslayer, document);
      
      g_object_unref (document);
      g_free (file_path);
    }

  return FALSE;
}              

static gint
sort_iter_compare_func (GtkTreeModel *model, 
                        GtkTreeIter  *a,
                        GtkTreeIter  *b, 
                        gpointer      userdata)
{
  gint ret = 0;

  gint sortcol;
  sortcol = GPOINTER_TO_INT (userdata);

  switch (sortcol)
    {
    case TEXT:
      {
        gchar *text1, *text2;
        gint line_number1, line_number2;

        gtk_tree_model_get (model, a, 
                            LINE_NUMBER, &line_number1, 
                            TEXT, &text1, -1);
        gtk_tree_model_get (model, b, 
                            LINE_NUMBER, &line_number2, 
                            TEXT, &text2, -1);

        if (line_number1 < line_number2)
          ret = -1;
        else if (line_number1 > line_number2)
          ret = 1;
        else
          ret = g_strcmp0 (text1, text2);

        g_free (text1);
        g_free (text2);
      }
      break;
    }

  return ret;
}

static JavaPageType 
java_usage_pane_get_page_type (JavaUsagePane *usage_pane)
{
  JavaUsagePanePrivate *priv;
  priv = JAVA_USAGE_PANE_GET_PRIVATE (usage_pane);
  return priv->page_type;
}

static JavaConfiguration* 
java_usage_pane_get_configuration (JavaUsagePane *usage_pane)
{
  JavaUsagePanePrivate *priv;
  priv = JAVA_USAGE_PANE_GET_PRIVATE (usage_pane);
  return priv->configuration;
}

static void 
java_usage_pane_set_configuration (JavaUsagePane    *usage_pane, 
                                   JavaConfiguration *configuration)
{
  JavaUsagePanePrivate *priv;
  priv = JAVA_USAGE_PANE_GET_PRIVATE (usage_pane);
  priv->configuration = configuration;
}                               

static CodeSlayerDocument* 
java_usage_pane_get_document (JavaUsagePane *usage_pane)
{
  JavaUsagePanePrivate *priv;
  priv = JAVA_USAGE_PANE_GET_PRIVATE (usage_pane);
  return priv->document;
}

static void 
java_usage_pane_set_document (JavaUsagePane     *usage_pane, 
                              CodeSlayerDocument *document)
{
  JavaUsagePanePrivate *priv;
  priv = JAVA_USAGE_PANE_GET_PRIVATE (usage_pane);
  priv->document = document;
}                         
