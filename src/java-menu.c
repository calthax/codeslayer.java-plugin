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
#include "java-menu.h"

static void java_menu_class_init   (JavaMenuClass  *klass);
static void java_menu_init         (JavaMenu       *menu);
static void java_menu_finalize     (JavaMenu       *menu);

static void add_menu_items          (JavaMenu      *menu, 
                                     GtkWidget     *submenu,
                                     GtkAccelGroup *accel_group);
                                        
static void compile_action          (JavaMenu      *menu);
static void clean_action            (JavaMenu      *menu);
static void test_file_action        (JavaMenu      *menu);
static void debug_test_file_action  (JavaMenu      *menu);
static void attach_debugger_action  (JavaMenu      *menu);
static void navigate_action      (JavaMenu      *menu);
static void search_action     (JavaMenu      *menu);
static void index_projects_action   (JavaMenu      *menu);
static void index_libs_action       (JavaMenu      *menu);
static void method_usage_action     (JavaMenu      *menu);
                                        
enum
{
  COMPILE,
  CLEAN,
  TEST_FILE,
  DEBUG_TEST_FILE,
  ATTACH_DEBUGGER,
  NAVIGATE,
  SEARCH,
  INDEX_PRODUCTS,
  INDEX_LIBS,
  METHOD_USAGE,
  LAST_SIGNAL
};

static guint java_menu_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (JavaMenu, java_menu, GTK_TYPE_MENU_ITEM)

static void
java_menu_class_init (JavaMenuClass *klass)
{
  java_menu_signals[COMPILE] =
    g_signal_new ("compile", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (JavaMenuClass, compile),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  java_menu_signals[CLEAN] =
    g_signal_new ("clean", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (JavaMenuClass, clean),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  java_menu_signals[TEST_FILE] =
    g_signal_new ("test-file", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (JavaMenuClass, test_file),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  java_menu_signals[DEBUG_TEST_FILE] =
    g_signal_new ("debug-test-file", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (JavaMenuClass, debug_test_file),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  java_menu_signals[ATTACH_DEBUGGER] =
    g_signal_new ("attach-debugger", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (JavaMenuClass, attach_debugger),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  java_menu_signals[NAVIGATE] =
    g_signal_new ("navigate", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (JavaMenuClass, find_symbol),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  java_menu_signals[SEARCH] =
    g_signal_new ("search", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (JavaMenuClass, class_search),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  java_menu_signals[INDEX_PRODUCTS] =
    g_signal_new ("index-projects", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (JavaMenuClass, index_projects),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  java_menu_signals[INDEX_LIBS] =
    g_signal_new ("index-libs", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (JavaMenuClass, index_libs),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  java_menu_signals[METHOD_USAGE] =
    g_signal_new ("method-usage", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (JavaMenuClass, method_usage),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) java_menu_finalize;
}

static void
java_menu_init (JavaMenu *menu)
{
  gtk_menu_item_set_label (GTK_MENU_ITEM (menu), "Java");
}

static void
java_menu_finalize (JavaMenu *menu)
{
  G_OBJECT_CLASS (java_menu_parent_class)->finalize (G_OBJECT (menu));
}

GtkWidget*
java_menu_new (GtkAccelGroup *accel_group)
{
  GtkWidget *menu;
  GtkWidget *submenu;
  
  menu = g_object_new (java_menu_get_type (), NULL);
  
  submenu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), submenu);

  add_menu_items (JAVA_MENU (menu), submenu, accel_group);

  return menu;
}

static void
add_menu_items (JavaMenu      *menu,
                GtkWidget     *submenu,
                GtkAccelGroup *accel_group)
{
  GtkWidget *compile_item;
  GtkWidget *clean_item;
  GtkWidget *test_file_item;
  GtkWidget *debug_test_file_item;
  GtkWidget *attach_debugger_item;
  GtkWidget *navigate_item;
  GtkWidget *search_item;
  GtkWidget *index_projects_item;
  GtkWidget *index_libs_item;
  GtkWidget *method_usage_item;
  GtkWidget *separator_item;

  compile_item = codeslayer_menu_item_new_with_label ("Compile");
  gtk_widget_add_accelerator (compile_item, "activate", 
                              accel_group, GDK_KEY_F9, 0, GTK_ACCEL_VISIBLE);  
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), compile_item);

  clean_item = codeslayer_menu_item_new_with_label ("Clean");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), clean_item);
  
  separator_item = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), separator_item);
  
  test_file_item = codeslayer_menu_item_new_with_label ("Test File");
  gtk_widget_add_accelerator (test_file_item, "activate", 
                              accel_group, GDK_KEY_F6, 0, GTK_ACCEL_VISIBLE);  
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), test_file_item);

  debug_test_file_item = codeslayer_menu_item_new_with_label ("Debug Test File");
  gtk_widget_add_accelerator (debug_test_file_item, "activate", 
                              accel_group, GDK_KEY_F6, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);  
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), debug_test_file_item);

  separator_item = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), separator_item);

  attach_debugger_item = codeslayer_menu_item_new_with_label ("Attach Debugger");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), attach_debugger_item);
  
  separator_item = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), separator_item);

  navigate_item = codeslayer_menu_item_new_with_label ("Navigate");
  gtk_widget_add_accelerator (navigate_item, "activate", 
                              accel_group, GDK_KEY_F4, 0, GTK_ACCEL_VISIBLE);  
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), navigate_item);

  search_item = codeslayer_menu_item_new_with_label ("Search");
  gtk_widget_add_accelerator (search_item, "activate", 
                              accel_group, GDK_KEY_E, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);  
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), search_item);

  separator_item = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), separator_item);
  
  index_projects_item = codeslayer_menu_item_new_with_label ("Index Projects");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), index_projects_item);

  index_libs_item = codeslayer_menu_item_new_with_label ("Index Libs");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), index_libs_item);

  separator_item = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), separator_item);
  
  method_usage_item = codeslayer_menu_item_new_with_label ("Method Usage");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), method_usage_item);

  g_signal_connect_swapped (G_OBJECT (compile_item), "activate", 
                            G_CALLBACK (compile_action), menu);
   
  g_signal_connect_swapped (G_OBJECT (clean_item), "activate", 
                            G_CALLBACK (clean_action), menu);
   
  g_signal_connect_swapped (G_OBJECT (test_file_item), "activate", 
                            G_CALLBACK (test_file_action), menu);
   
  g_signal_connect_swapped (G_OBJECT (debug_test_file_item), "activate", 
                            G_CALLBACK (debug_test_file_action), menu);
   
  g_signal_connect_swapped (G_OBJECT (attach_debugger_item), "activate", 
                            G_CALLBACK (attach_debugger_action), menu);
   
  g_signal_connect_swapped (G_OBJECT (navigate_item), "activate", 
                            G_CALLBACK (navigate_action), menu);
   
  g_signal_connect_swapped (G_OBJECT (search_item), "activate", 
                            G_CALLBACK (search_action), menu);
   
  g_signal_connect_swapped (G_OBJECT (index_projects_item), "activate", 
                            G_CALLBACK (index_projects_action), menu);
   
  g_signal_connect_swapped (G_OBJECT (index_libs_item), "activate", 
                            G_CALLBACK (index_libs_action), menu);
   
  g_signal_connect_swapped (G_OBJECT (method_usage_item), "activate", 
                            G_CALLBACK (method_usage_action), menu);
}

static void 
compile_action (JavaMenu *menu) 
{
  g_signal_emit_by_name ((gpointer) menu, "compile");
}

static void 
clean_action (JavaMenu *menu) 
{
  g_signal_emit_by_name ((gpointer) menu, "clean");
}

static void 
test_file_action (JavaMenu *menu) 
{
  g_signal_emit_by_name ((gpointer) menu, "test-file");
}

static void 
debug_test_file_action (JavaMenu *menu) 
{
  g_signal_emit_by_name ((gpointer) menu, "debug-test-file");
}

static void 
attach_debugger_action (JavaMenu *menu) 
{
  g_signal_emit_by_name ((gpointer) menu, "attach-debugger");
}

static void 
navigate_action (JavaMenu *menu) 
{
  g_signal_emit_by_name ((gpointer) menu, "navigate");
}

static void 
search_action (JavaMenu *menu) 
{
  g_signal_emit_by_name ((gpointer) menu, "search");
}

static void 
index_projects_action (JavaMenu *menu) 
{
  g_signal_emit_by_name ((gpointer) menu, "index-projects");
}

static void 
index_libs_action (JavaMenu *menu) 
{
  g_signal_emit_by_name ((gpointer) menu, "index-libs");
}

static void 
method_usage_action (JavaMenu *menu) 
{
  g_signal_emit_by_name ((gpointer) menu, "method-usage");
}
