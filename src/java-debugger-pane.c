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


#define JAVA_DEBUGGER_PANE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_DEBUGGER_PANE_TYPE, JavaDebuggerPanePrivate))

typedef struct _JavaDebuggerPanePrivate JavaDebuggerPanePrivate;

struct _JavaDebuggerPanePrivate
{
  JavaConfiguration  *configuration;
  CodeSlayerDocument *document;
  GtkWidget          *hpaned; 
};

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
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) java_debugger_pane_finalize;
  g_type_class_add_private (klass, sizeof (JavaDebuggerPanePrivate));
}

static void
java_debugger_pane_init (JavaDebuggerPane *debugger_pane) 
{
  JavaDebuggerPanePrivate *priv;
  GtkWidget *toolbar;
  GtkWidget *vbox;
  GtkToolItem *query_item;
  GtkToolItem *forward_item;
  GtkToolItem *down_item;
  GtkToolItem *up_item;
  
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
  forward_item = gtk_tool_button_new_from_stock (GTK_STOCK_GO_FORWARD);
  down_item = gtk_tool_button_new_from_stock (GTK_STOCK_GO_DOWN);
  up_item = gtk_tool_button_new_from_stock (GTK_STOCK_GO_UP);
  
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), query_item, -1);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), forward_item, -1);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), down_item, -1);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), up_item, -1);
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
