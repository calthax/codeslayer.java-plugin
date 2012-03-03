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
static void find_symbol_action      (JavaMenu      *menu);
static void class_search_action      (JavaMenu      *menu);
                                        
enum
{
  COMPILE,
  CLEAN,
  TEST_FILE,
  DEBUG_TEST_FILE,
  ATTACH_DEBUGGER,
  FIND_SYMBOL,
  CLASS_SEARCH,
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

  java_menu_signals[FIND_SYMBOL] =
    g_signal_new ("find-symbol", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (JavaMenuClass, find_symbol),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  java_menu_signals[CLASS_SEARCH] =
    g_signal_new ("class-search", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (JavaMenuClass, class_search),
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
  GtkWidget *find_symbol_item;
  GtkWidget *class_search_item;
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

  find_symbol_item = codeslayer_menu_item_new_with_label ("Find Symbol");
  gtk_widget_add_accelerator (find_symbol_item, "activate", 
                              accel_group, GDK_KEY_F4, 0, GTK_ACCEL_VISIBLE);  
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), find_symbol_item);

  class_search_item = codeslayer_menu_item_new_with_label ("Class Search");
  gtk_widget_add_accelerator (class_search_item, "activate", 
                              accel_group, GDK_KEY_E, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);  
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), class_search_item);

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
   
  g_signal_connect_swapped (G_OBJECT (find_symbol_item), "activate", 
                            G_CALLBACK (find_symbol_action), menu);
   
  g_signal_connect_swapped (G_OBJECT (class_search_item), "activate", 
                            G_CALLBACK (class_search_action), menu);
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
find_symbol_action (JavaMenu *menu) 
{
  g_signal_emit_by_name ((gpointer) menu, "find-symbol");
}

static void 
class_search_action (JavaMenu *menu) 
{
  g_signal_emit_by_name ((gpointer) menu, "class-search");
}
