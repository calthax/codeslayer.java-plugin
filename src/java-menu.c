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
#include "autotools-menu.h"

static void autotools_menu_class_init  (AutotoolsMenuClass *klass);
static void autotools_menu_init        (AutotoolsMenu      *menu);
static void autotools_menu_finalize    (AutotoolsMenu      *menu);

static void add_menu_items             (AutotoolsMenu      *menu, 
                                        GtkWidget          *submenu,
                                        GtkAccelGroup      *accel_group);
                                        
static void make_action                (AutotoolsMenu      *menu);
static void make_clean_action          (AutotoolsMenu      *menu);
static void make_install_action        (AutotoolsMenu      *menu);
                                        
enum
{
  MAKE,
  MAKE_INSTALL,
  MAKE_CLEAN,
  LAST_SIGNAL
};

static guint autotools_menu_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (AutotoolsMenu, autotools_menu, GTK_TYPE_MENU_ITEM)

static void
autotools_menu_class_init (AutotoolsMenuClass *klass)
{
  autotools_menu_signals[MAKE] =
    g_signal_new ("make", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (AutotoolsMenuClass, make),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  autotools_menu_signals[MAKE_INSTALL] =
    g_signal_new ("make-install", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (AutotoolsMenuClass, make_install),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  autotools_menu_signals[MAKE_CLEAN] =
    g_signal_new ("make-clean", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (AutotoolsMenuClass, make_clean),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) autotools_menu_finalize;
}

static void
autotools_menu_init (AutotoolsMenu *menu)
{
  gtk_menu_item_set_label (GTK_MENU_ITEM (menu), "Autotools");
}

static void
autotools_menu_finalize (AutotoolsMenu *menu)
{
  G_OBJECT_CLASS (autotools_menu_parent_class)->finalize (G_OBJECT (menu));
}

GtkWidget*
autotools_menu_new (GtkAccelGroup *accel_group)
{
  GtkWidget *menu;
  GtkWidget *submenu;
  
  menu = g_object_new (autotools_menu_get_type (), NULL);
  
  submenu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), submenu);

  add_menu_items (AUTOTOOLS_MENU (menu), submenu, accel_group);

  return menu;
}

static void
add_menu_items (AutotoolsMenu *menu,
                GtkWidget     *submenu,
                GtkAccelGroup *accel_group)
{
  GtkWidget *make_item;
  GtkWidget *make_clean_item;
  GtkWidget *make_install_item;

  make_item = codeslayer_menu_item_new_with_label ("make");
  gtk_widget_add_accelerator (make_item, "activate", 
                              accel_group, GDK_KEY_F9, 0, GTK_ACCEL_VISIBLE);  
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), make_item);

  make_install_item = codeslayer_menu_item_new_with_label ("make install");
  gtk_widget_add_accelerator (make_install_item, "activate", 
                              accel_group, GDK_KEY_F9, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);  
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), make_install_item);
  
  make_clean_item = codeslayer_menu_item_new_with_label ("make clean");
  gtk_widget_add_accelerator (make_clean_item, "activate", 
                              accel_group, GDK_KEY_F9, GDK_CONTROL_MASK | GDK_SHIFT_MASK, 
                              GTK_ACCEL_VISIBLE);  
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), make_clean_item);
  
  g_signal_connect_swapped (G_OBJECT (make_item), "activate", 
                            G_CALLBACK (make_action), menu);
   
  g_signal_connect_swapped (G_OBJECT (make_clean_item), "activate", 
                            G_CALLBACK (make_clean_action), menu);
   
  g_signal_connect_swapped (G_OBJECT (make_install_item), "activate", 
                            G_CALLBACK (make_install_action), menu);
}

static void 
make_action (AutotoolsMenu *menu) 
{
  g_signal_emit_by_name ((gpointer) menu, "make");
}

static void 
make_clean_action (AutotoolsMenu *menu) 
{
  g_signal_emit_by_name ((gpointer) menu, "make-clean");
}

static void 
make_install_action (AutotoolsMenu *menu) 
{
  g_signal_emit_by_name ((gpointer) menu, "make-install");
}
