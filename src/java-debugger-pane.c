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

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "java-debugger-pane.h"
#include "java-page.h"

static void java_page_interface_init                            (gpointer               debugger_pane, 
                                                                 gpointer               data);
static void java_debugger_pane_class_init                       (JavaDebuggerPaneClass *klass);
static void java_debugger_pane_init                             (JavaDebuggerPane      *debugger_pane);
static void java_debugger_pane_finalize                         (JavaDebuggerPane      *debugger_pane);

static JavaPageType java_debugger_pane_get_page_type            (JavaDebuggerPane      *debugger_pane);                                      
static JavaConfiguration* java_debugger_pane_get_configuration  (JavaDebuggerPane      *debugger_pane);
static void java_debugger_pane_set_configuration                (JavaDebuggerPane      *debugger_pane, 
                                                                 JavaConfiguration     *configuration);
static CodeSlayerDocument* java_debugger_pane_get_document      (JavaDebuggerPane      *debugger_pane);
static void java_debugger_pane_set_document                     (JavaDebuggerPane      *debugger_pane, 
                                                                 CodeSlayerDocument    *document);
static void query_action                                        (JavaDebuggerPane      *debugger_pane);
static void step_over_action                                    (JavaDebuggerPane      *debugger_pane);
static void step_into_action                                    (JavaDebuggerPane      *debugger_pane);
static void step_out_action                                     (JavaDebuggerPane      *debugger_pane);
static void cont_action                                         (JavaDebuggerPane      *debugger_pane);
static void quit_action                                         (JavaDebuggerPane      *debugger_pane);

#define JAVA_DEBUGGER_PANE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_DEBUGGER_PANE_TYPE, JavaDebuggerPanePrivate))

typedef struct _JavaDebuggerPanePrivate JavaDebuggerPanePrivate;

struct _JavaDebuggerPanePrivate
{
  JavaConfiguration  *configuration;
  CodeSlayerDocument *document;
  GtkWidget          *hpaned;
  GtkWidget          *text_view;
  GtkWidget          *tree;
  GtkListStore       *store;
};

enum
{
  QUERY,  
  QUIT,  
  CONT,  
  STEP_OVER,  
  STEP_INTO,  
  STEP_OUT,  
  LAST_SIGNAL
};

static guint debugger_pane_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE_EXTENDED (JavaDebuggerPane,
                        java_debugger_pane,
                        GTK_TYPE_VBOX,
                        0,
                        G_IMPLEMENT_INTERFACE (JAVA_PAGE_TYPE ,
                                               java_page_interface_init));
     
static void
java_page_interface_init (gpointer page, 
                          gpointer data)
{
  JavaPageInterface *page_interface = (JavaPageInterface*) page;
  page_interface->get_page_type = (JavaPageType (*) (JavaPage *obj)) java_debugger_pane_get_page_type;
  page_interface->get_configuration = (JavaConfiguration* (*) (JavaPage *obj)) java_debugger_pane_get_configuration;
  page_interface->set_configuration = (void (*) (JavaPage *obj, JavaConfiguration*)) java_debugger_pane_set_configuration;
  page_interface->get_document = (CodeSlayerDocument* (*) (JavaPage *obj)) java_debugger_pane_get_document;
  page_interface->set_document = (void (*) (JavaPage *obj, CodeSlayerDocument*)) java_debugger_pane_set_document;
}

static void 
java_debugger_pane_class_init (JavaDebuggerPaneClass *klass)
{
  debugger_pane_signals[QUERY] =
    g_signal_new ("query", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (JavaDebuggerPaneClass, query),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);

  debugger_pane_signals[CONT] =
    g_signal_new ("cont", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (JavaDebuggerPaneClass, cont),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  debugger_pane_signals[QUIT] =
    g_signal_new ("quit", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (JavaDebuggerPaneClass, quit),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  debugger_pane_signals[STEP_OVER] =
    g_signal_new ("step-over", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (JavaDebuggerPaneClass, step_over),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  debugger_pane_signals[STEP_INTO] =
    g_signal_new ("step-into", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (JavaDebuggerPaneClass, step_into),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  debugger_pane_signals[STEP_OUT] =
    g_signal_new ("step-out", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (JavaDebuggerPaneClass, step_out),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) java_debugger_pane_finalize;
  g_type_class_add_private (klass, sizeof (JavaDebuggerPanePrivate));
}

static void
java_debugger_pane_init (JavaDebuggerPane *debugger_pane) 
{
  JavaDebuggerPanePrivate *priv;
  GtkWidget *toolbar;
  GtkWidget *vbox;
  GtkToolItem *query_item;
  GtkToolItem *query_separator_item;
  GtkToolItem *cont_item;
  GtkToolItem *quit_item;
  GtkToolItem *stop_separator_item;
  GtkToolItem *step_over_item;
  GtkToolItem *step_into_item;
  GtkToolItem *step_out_item;
  
  priv = JAVA_DEBUGGER_PANE_GET_PRIVATE (debugger_pane);
  
  priv->hpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_paned_set_position (GTK_PANED (priv->hpaned), 400);
  gtk_box_pack_start (GTK_BOX (debugger_pane), priv->hpaned, TRUE, TRUE, 0);
  
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_paned_add1 (GTK_PANED (priv->hpaned), vbox);
  toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_icon_size (GTK_TOOLBAR (toolbar), GTK_ICON_SIZE_MENU);
  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_ICONS);
  gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, FALSE, 0);
  
  query_item = gtk_tool_button_new (NULL, "Query");
  query_separator_item = gtk_separator_tool_item_new ();
  cont_item = gtk_tool_button_new_from_stock (GTK_STOCK_GOTO_LAST);
  quit_item = gtk_tool_button_new_from_stock (GTK_STOCK_STOP);
  step_over_item = gtk_tool_button_new_from_stock (GTK_STOCK_GO_FORWARD);
  step_into_item = gtk_tool_button_new_from_stock (GTK_STOCK_GO_DOWN);
  step_out_item = gtk_tool_button_new_from_stock (GTK_STOCK_GO_UP);
  stop_separator_item = gtk_separator_tool_item_new ();
  
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), query_item, -1);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), query_separator_item, -1);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), cont_item, -1);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), quit_item, -1);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), stop_separator_item, -1);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), step_over_item, -1);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), step_into_item, -1);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), step_out_item, -1);
  
  priv->text_view = gtk_text_view_new ();
  gtk_box_pack_start (GTK_BOX (vbox), priv->text_view, TRUE, TRUE, 0);
  
  priv->tree = gtk_tree_view_new ();
  gtk_paned_add2 (GTK_PANED (priv->hpaned), priv->tree);
  /*priv->store = gtk_list_store_new (COLUMNS, G_TYPE_STRING);
  gtk_tree_view_set_model (GTK_TREE_VIEW (priv->tree), GTK_TREE_MODEL (priv->store));
  g_object_unref (priv->store);*/
  
  g_signal_connect_swapped (G_OBJECT (query_item), "clicked",
                            G_CALLBACK (query_action), debugger_pane);
  
  g_signal_connect_swapped (G_OBJECT (cont_item), "clicked",
                            G_CALLBACK (cont_action), debugger_pane);
  
  g_signal_connect_swapped (G_OBJECT (quit_item), "clicked",
                            G_CALLBACK (quit_action), debugger_pane);
  
  g_signal_connect_swapped (G_OBJECT (step_over_item), "clicked",
                            G_CALLBACK (step_over_action), debugger_pane);
  
  g_signal_connect_swapped (G_OBJECT (step_into_item), "clicked",
                            G_CALLBACK (step_into_action), debugger_pane);
  
  g_signal_connect_swapped (G_OBJECT (step_out_item), "clicked",
                            G_CALLBACK (step_out_action), debugger_pane);
}

static void
java_debugger_pane_finalize (JavaDebuggerPane *debugger_pane)
{
  G_OBJECT_CLASS (java_debugger_pane_parent_class)->finalize (G_OBJECT (debugger_pane));
}

GtkWidget*
java_debugger_pane_new ()
{
  GtkWidget *debugger_pane;
  debugger_pane = g_object_new (java_debugger_pane_get_type (), NULL);
  return debugger_pane;
}

static JavaPageType 
java_debugger_pane_get_page_type (JavaDebuggerPane *debugger_pane)
{
  return JAVA_PAGE_TYPE_DEBUGGER;
}

static JavaConfiguration* 
java_debugger_pane_get_configuration (JavaDebuggerPane *debugger_pane)
{
  JavaDebuggerPanePrivate *priv;
  priv = JAVA_DEBUGGER_PANE_GET_PRIVATE (debugger_pane);
  return priv->configuration;
}

static void 
java_debugger_pane_set_configuration (JavaDebuggerPane  *debugger_pane, 
                                      JavaConfiguration *configuration)
{
  JavaDebuggerPanePrivate *priv;
  priv = JAVA_DEBUGGER_PANE_GET_PRIVATE (debugger_pane);
  priv->configuration = configuration;
}                               

static CodeSlayerDocument* 
java_debugger_pane_get_document (JavaDebuggerPane *debugger_pane)
{
  JavaDebuggerPanePrivate *priv;
  priv = JAVA_DEBUGGER_PANE_GET_PRIVATE (debugger_pane);
  return priv->document;
}

static void 
java_debugger_pane_set_document (JavaDebuggerPane   *debugger_pane, 
                                 CodeSlayerDocument *document)
{
  JavaDebuggerPanePrivate *priv;
  priv = JAVA_DEBUGGER_PANE_GET_PRIVATE (debugger_pane);
  priv->document = document;
}

static void
query_action (JavaDebuggerPane *debugger_pane)
{
  JavaDebuggerPanePrivate *priv;
  GtkTextBuffer *buffer;
  GtkTextIter start;
  GtkTextIter end;
  gchar *text;
  
  priv = JAVA_DEBUGGER_PANE_GET_PRIVATE (debugger_pane);
  
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->text_view));
  gtk_text_buffer_get_bounds (buffer, &start, &end);
  text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
  
  g_signal_emit_by_name ((gpointer) debugger_pane, "query", text);
  
  g_free (text);
}

static void
cont_action (JavaDebuggerPane *debugger_pane)
{
  g_signal_emit_by_name ((gpointer) debugger_pane, "cont");
}

static void
quit_action (JavaDebuggerPane *debugger_pane)
{
  g_signal_emit_by_name ((gpointer) debugger_pane, "quit");
}

static void
step_over_action (JavaDebuggerPane *debugger_pane)
{
  g_signal_emit_by_name ((gpointer) debugger_pane, "step-over");
}

static void
step_into_action (JavaDebuggerPane *debugger_pane)
{
  g_signal_emit_by_name ((gpointer) debugger_pane, "step-into");
}

static void
step_out_action (JavaDebuggerPane *debugger_pane)
{
  g_signal_emit_by_name ((gpointer) debugger_pane, "step-out");
}
