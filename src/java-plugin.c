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

#include <codeslayer/codeslayer.h>
#include <gtk/gtk.h>
#include <gmodule.h>
#include <glib.h>
#include "java-engine.h"
#include "java-debugger.h"
#include "java-debugger-breakpoints.h"
#include "java-configurations.h"
#include "java-menu.h"
#include "java-notebook.h"
#include "java-project-properties.h"
#include "java-projects-popup.h"

G_MODULE_EXPORT void activate   (CodeSlayer *codeslayer);
G_MODULE_EXPORT void deactivate (CodeSlayer *codeslayer);

static GtkWidget *menu;
static GtkWidget *project_properties;
static GtkWidget *notebook;
static GtkWidget *projects_popup;
static JavaEngine *engine;

G_MODULE_EXPORT void
activate (CodeSlayer *codeslayer)
{
  GtkAccelGroup *accel_group;
  JavaConfigurations *configurations;
  JavaDebugger *debugger;
  accel_group = codeslayer_get_menubar_accel_group (codeslayer);
  menu = java_menu_new (accel_group);
  
  configurations = java_configurations_new (codeslayer);
  java_configurations_load (configurations);
  debugger = java_debugger_new (codeslayer, configurations);
  
  project_properties = java_project_properties_new ();
  projects_popup = java_projects_popup_new ();
  notebook = java_notebook_new ();
  engine = java_engine_new (codeslayer, configurations, debugger, menu, project_properties, projects_popup, notebook);
  
  codeslayer_add_to_menubar (codeslayer, GTK_MENU_ITEM (menu));
  codeslayer_add_to_projects_popup (codeslayer, GTK_MENU_ITEM (projects_popup));
  codeslayer_add_to_project_properties (codeslayer, project_properties, "Java");
  codeslayer_add_to_bottom_pane (codeslayer, notebook, "Java");
}

G_MODULE_EXPORT void 
deactivate (CodeSlayer *codeslayer)
{
  codeslayer_remove_from_menubar (codeslayer, GTK_MENU_ITEM (menu));
  codeslayer_remove_from_projects_popup (codeslayer, GTK_MENU_ITEM (projects_popup));
  codeslayer_remove_from_project_properties (codeslayer, project_properties);
  codeslayer_remove_from_bottom_pane (codeslayer, notebook);
  g_object_unref (engine);
}
