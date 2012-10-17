/*
 * Copyright (C) 2010 - Jeff Johnston
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.remove_group_item
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <gdk/gdkkeysyms.h>
#include <codeslayer/codeslayer.h>
#include "java-projects-popup.h"

static void java_projects_popup_class_init  (JavaProjectsPopupClass *klass);
static void java_projects_popup_init        (JavaProjectsPopup      *projects_popup);
static void java_projects_popup_finalize    (JavaProjectsPopup      *projects_popup);

static void add_menu_items                  (JavaProjectsPopup      *projects_popup,
                                             GtkWidget               *submenu);
                                        
static void compile_action                  (JavaProjectsPopup      *projects_popup, 
                                             GList                   *selections);
static void clean_action                    (JavaProjectsPopup      *projects_popup, 
                                             GList                   *selections);
static void clean_compile_action            (JavaProjectsPopup      *projects_popup, 
                                             GList                   *selections);
static void test_project_action             (JavaProjectsPopup      *projects_popup, 
                                             GList                   *selections);
                                        
enum
{
  COMPILE,
  CLEAN,
  CLEAN_COMPILE,
  TEST_PROJECT,
  LAST_SIGNAL
};

static guint java_projects_popup_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (JavaProjectsPopup, java_projects_popup, GTK_TYPE_MENU_ITEM)

static void
java_projects_popup_class_init (JavaProjectsPopupClass *klass)
{
  java_projects_popup_signals[COMPILE] =
    g_signal_new ("compile", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (JavaProjectsPopupClass, compile),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);

  java_projects_popup_signals[CLEAN] =
    g_signal_new ("clean", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (JavaProjectsPopupClass, clean),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);

  java_projects_popup_signals[CLEAN_COMPILE] =
    g_signal_new ("clean-compile", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (JavaProjectsPopupClass, clean_compile),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);

  java_projects_popup_signals[TEST_PROJECT] =
    g_signal_new ("test-project", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (JavaProjectsPopupClass, test_project),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);

  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) java_projects_popup_finalize;
}

static void
java_projects_popup_init (JavaProjectsPopup *projects_popup)
{
  gtk_menu_item_set_label (GTK_MENU_ITEM (projects_popup), "Java");
}

static void
java_projects_popup_finalize (JavaProjectsPopup *projects_popup)
{
  G_OBJECT_CLASS (java_projects_popup_parent_class)->finalize (G_OBJECT (projects_popup));
}

GtkWidget*
java_projects_popup_new ()
{
  GtkWidget *projects_popup;
  GtkWidget *submenu;
  
  projects_popup = g_object_new (java_projects_popup_get_type (), NULL);

  submenu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (projects_popup), submenu);

  add_menu_items (JAVA_PROJECTS_POPUP (projects_popup), submenu);

  return projects_popup;
}

static void
add_menu_items (JavaProjectsPopup *projects_popup, 
                GtkWidget         *submenu)
{
  GtkWidget *compile_item;
  GtkWidget *clean_item;
  GtkWidget *clean_compile_item;
  GtkWidget *separator_item;
  GtkWidget *test_project_item;

  compile_item = codeslayer_menu_item_new_with_label ("Compile");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), compile_item);

  clean_item = codeslayer_menu_item_new_with_label ("Clean");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), clean_item);
  
  clean_compile_item = codeslayer_menu_item_new_with_label ("Clean & Compile");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), clean_compile_item);
  
  separator_item = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), separator_item);
  
  test_project_item = codeslayer_menu_item_new_with_label ("test project");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), test_project_item);

  g_signal_connect_swapped (G_OBJECT (compile_item), "projects-menu-selected", 
                            G_CALLBACK (compile_action), projects_popup);
   
  g_signal_connect_swapped (G_OBJECT (clean_item), "projects-menu-selected", 
                            G_CALLBACK (clean_action), projects_popup);
   
  g_signal_connect_swapped (G_OBJECT (clean_compile_item), "projects-menu-selected", 
                            G_CALLBACK (clean_compile_action), projects_popup);
   
  g_signal_connect_swapped (G_OBJECT (test_project_item), "projects-menu-selected", 
                            G_CALLBACK (test_project_action), projects_popup);
}

static void 
compile_action (JavaProjectsPopup *projects_popup, 
                GList             *selections) 
{
  g_signal_emit_by_name ((gpointer) projects_popup, "compile", selections);
}

static void 
clean_action (JavaProjectsPopup *projects_popup, 
              GList             *selections) 
{
  g_signal_emit_by_name ((gpointer) projects_popup, "clean", selections);
}

static void 
clean_compile_action (JavaProjectsPopup *projects_popup, 
                      GList             *selections) 
{
  g_signal_emit_by_name ((gpointer) projects_popup, "clean-compile", selections);
}

static void 
test_project_action (JavaProjectsPopup *projects_popup, 
                     GList             *selections) 
{
  g_signal_emit_by_name ((gpointer) projects_popup, "test-project", selections);
}
