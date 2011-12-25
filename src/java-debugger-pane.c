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
static void stop_action                                         (JavaDebuggerPane      *debugger_pane);

#define JAVA_DEBUGGER_PANE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_DEBUGGER_PANE_TYPE, JavaDebuggerPanePrivate))

typedef struct _JavaDebuggerPanePrivate JavaDebuggerPanePrivate;

struct _JavaDebuggerPanePrivate
{
  JavaConfiguration  *configuration;
  CodeSlayerDocument *document;
  GtkWidget          *hpaned; 
};

enum
{
  STEP_OVER,  
  STOP,  
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
  debugger_pane_signals[STEP_OVER] =
    g_signal_new ("step-over", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (JavaDebuggerPaneClass, step_over),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  debugger_pane_signals[STOP] =
    g_signal_new ("stop", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (JavaDebuggerPaneClass, stop),
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
  GtkToolItem *step_over_item;
  GtkToolItem *step_into_item;
  GtkToolItem *step_out_item;
  GtkToolItem *stop_item;
  GtkToolItem *stop_separator_item;
  
  priv = JAVA_DEBUGGER_PANE_GET_PRIVATE (debugger_pane);
  
  priv->hpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_box_pack_start (GTK_BOX (debugger_pane), priv->hpaned, TRUE, TRUE, 0);
  
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_paned_add1 (GTK_PANED (priv->hpaned), vbox);
  toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_icon_size (GTK_TOOLBAR (toolbar), GTK_ICON_SIZE_MENU);
  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_ICONS);
  gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, FALSE, 0);
  
  query_item = gtk_tool_button_new (NULL, "Query");
  query_separator_item = gtk_separator_tool_item_new ();
  step_over_item = gtk_tool_button_new_from_stock (GTK_STOCK_GO_FORWARD);
  step_into_item = gtk_tool_button_new_from_stock (GTK_STOCK_GO_DOWN);
  step_out_item = gtk_tool_button_new_from_stock (GTK_STOCK_GO_UP);
  stop_item = gtk_tool_button_new_from_stock (GTK_STOCK_STOP);
  stop_separator_item = gtk_separator_tool_item_new ();
  
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), query_item, -1);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), query_separator_item, -1);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), step_over_item, -1);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), step_into_item, -1);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), step_out_item, -1);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), stop_separator_item, -1);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), stop_item, -1);
  
  g_signal_connect_swapped (G_OBJECT (query_item), "clicked",
                            G_CALLBACK (query_action), debugger_pane);
  
  g_signal_connect_swapped (G_OBJECT (step_over_item), "clicked",
                            G_CALLBACK (step_over_action), debugger_pane);
  
  g_signal_connect_swapped (G_OBJECT (step_into_item), "clicked",
                            G_CALLBACK (step_into_action), debugger_pane);
  
  g_signal_connect_swapped (G_OBJECT (step_out_item), "clicked",
                            G_CALLBACK (step_out_action), debugger_pane);
  
  g_signal_connect_swapped (G_OBJECT (stop_item), "clicked",
                            G_CALLBACK (stop_action), debugger_pane);
}

static void
java_debugger_pane_finalize (JavaDebuggerPane *debugger_pane)
{
  G_OBJECT_CLASS (java_debugger_pane_parent_class)->finalize (G_OBJECT (debugger_pane));
}

GtkWidget*
java_debugger_pane_new ()
{
  /*JavaDebuggerPanePrivate *priv;*/
  GtkWidget *debugger_pane;
  
  debugger_pane = g_object_new (java_debugger_pane_get_type (), NULL);
  /*priv = JAVA_DEBUGGER_PANE_GET_PRIVATE (debugger_pane);*/
  
  
  
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
  g_print ("query action\n");
}

static void
step_over_action (JavaDebuggerPane *debugger_pane)
{
  g_signal_emit_by_name ((gpointer) debugger_pane, "step-over");
}

static void
step_into_action (JavaDebuggerPane *debugger_pane)
{
  g_print ("step into action\n");
}

static void
step_out_action (JavaDebuggerPane *debugger_pane)
{
  g_print ("step out action\n");
}

static void
stop_action (JavaDebuggerPane *debugger_pane)
{
  g_signal_emit_by_name ((gpointer) debugger_pane, "stop");
}
