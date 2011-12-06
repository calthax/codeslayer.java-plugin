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

static void java_menu_class_init  (JavaMenuClass *klass);
static void java_menu_init        (JavaMenu      *menu);
static void java_menu_finalize    (JavaMenu      *menu);

static void add_menu_items         (JavaMenu      *menu, 
                                    GtkWidget      *submenu,
                                    GtkAccelGroup  *accel_group);
                                        
static void compile_action         (JavaMenu      *menu);
static void clean_action           (JavaMenu      *menu);
                                        
enum
{
  COMPILE,
  CLEAN,
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
add_menu_items (JavaMenu *menu,
                GtkWidget     *submenu,
                GtkAccelGroup *accel_group)
{
  GtkWidget *compile_item;
  GtkWidget *clean_item;

  compile_item = codeslayer_menu_item_new_with_label ("compile");
  gtk_widget_add_accelerator (compile_item, "activate", 
                              accel_group, GDK_KEY_F9, 0, GTK_ACCEL_VISIBLE);  
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), compile_item);

  clean_item = codeslayer_menu_item_new_with_label ("clean");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), clean_item);
  
  g_signal_connect_swapped (G_OBJECT (compile_item), "activate", 
                            G_CALLBACK (compile_action), menu);
   
  g_signal_connect_swapped (G_OBJECT (clean_item), "activate", 
                            G_CALLBACK (clean_action), menu);
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
