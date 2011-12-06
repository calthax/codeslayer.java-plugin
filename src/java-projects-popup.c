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
#include "autotools-projects-popup.h"

static void autotools_projects_popup_class_init  (AutotoolsProjectsPopupClass *klass);
static void autotools_projects_popup_init        (AutotoolsProjectsPopup      *projects_popup);
static void autotools_projects_popup_finalize    (AutotoolsProjectsPopup      *projects_popup);

static void add_menu_items                       (AutotoolsProjectsPopup      *projects_popup,
                                                  GtkWidget                   *submenu);
                                        
static void make_action                          (AutotoolsProjectsPopup      *projects_popup, 
                                                  GList                      *selections);
static void make_clean_action                    (AutotoolsProjectsPopup      *projects_popup, 
                                                  GList                      *selections);
static void make_install_action                  (AutotoolsProjectsPopup      *projects_popup, 
                                                  GList                      *selections);
static void configure_action                     (AutotoolsProjectsPopup      *projects_popup, 
                                                  GList                      *selections);
static void autoreconf_action                    (AutotoolsProjectsPopup      *projects_popup, 
                                                  GList                      *selections);
                                        
enum
{
  MAKE,
  MAKE_INSTALL,
  MAKE_CLEAN,
  AUTORECONF,
  CONFIGURE,
  LAST_SIGNAL
};

static guint autotools_projects_popup_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (AutotoolsProjectsPopup, autotools_projects_popup, GTK_TYPE_MENU_ITEM)

static void
autotools_projects_popup_class_init (AutotoolsProjectsPopupClass *klass)
{
  autotools_projects_popup_signals[MAKE] =
    g_signal_new ("make", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (AutotoolsProjectsPopupClass, make),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);

  autotools_projects_popup_signals[MAKE_INSTALL] =
    g_signal_new ("make-install", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (AutotoolsProjectsPopupClass, make_install),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);

  autotools_projects_popup_signals[MAKE_CLEAN] =
    g_signal_new ("make-clean", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (AutotoolsProjectsPopupClass, make_clean),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);

  autotools_projects_popup_signals[CONFIGURE] =
    g_signal_new ("configure", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (AutotoolsProjectsPopupClass, configure),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);

  autotools_projects_popup_signals[AUTORECONF] =
    g_signal_new ("autoreconf", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (AutotoolsProjectsPopupClass, autoreconf),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);

  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) autotools_projects_popup_finalize;
}

static void
autotools_projects_popup_init (AutotoolsProjectsPopup *projects_popup)
{
  gtk_menu_item_set_label (GTK_MENU_ITEM (projects_popup), "Autotools");
}

static void
autotools_projects_popup_finalize (AutotoolsProjectsPopup *projects_popup)
{
  G_OBJECT_CLASS (autotools_projects_popup_parent_class)->finalize (G_OBJECT (projects_popup));
}

GtkWidget*
autotools_projects_popup_new ()
{
  GtkWidget *projects_popup;
  GtkWidget *submenu;
  
  projects_popup = g_object_new (autotools_projects_popup_get_type (), NULL);

  submenu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (projects_popup), submenu);

  add_menu_items (AUTOTOOLS_PROJECTS_POPUP (projects_popup), submenu);

  return projects_popup;
}

static void
add_menu_items (AutotoolsProjectsPopup *projects_popup, 
                GtkWidget              *submenu)
{
  GtkWidget *make_item;
  GtkWidget *make_clean_item;
  GtkWidget *make_install_item;
  GtkWidget *separator;
  GtkWidget *configure_item;
  GtkWidget *autoreconf_item;

  make_item = codeslayer_menu_item_new_with_label ("make");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), make_item);

  make_install_item = codeslayer_menu_item_new_with_label ("make install");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), make_install_item);
  
  make_clean_item = codeslayer_menu_item_new_with_label ("make clean");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), make_clean_item);
  
  separator = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), separator);

  configure_item = codeslayer_menu_item_new_with_label ("configure");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), configure_item);

  autoreconf_item = codeslayer_menu_item_new_with_label ("autoreconf");
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), autoreconf_item);

  g_signal_connect_swapped (G_OBJECT (make_item), "projects-menu-selected", 
                            G_CALLBACK (make_action), projects_popup);
   
  g_signal_connect_swapped (G_OBJECT (make_clean_item), "projects-menu-selected", 
                            G_CALLBACK (make_clean_action), projects_popup);
   
  g_signal_connect_swapped (G_OBJECT (make_install_item), "projects-menu-selected", 
                            G_CALLBACK (make_install_action), projects_popup);

  g_signal_connect_swapped (G_OBJECT (configure_item), "projects-menu-selected", 
                            G_CALLBACK (configure_action), projects_popup);

  g_signal_connect_swapped (G_OBJECT (autoreconf_item), "projects-menu-selected", 
                            G_CALLBACK (autoreconf_action), projects_popup);
}

static void 
make_action (AutotoolsProjectsPopup *projects_popup, 
             GList                 *selections) 
{
  g_signal_emit_by_name ((gpointer) projects_popup, "make", selections);
}

static void 
make_clean_action (AutotoolsProjectsPopup *projects_popup, 
                   GList                  *selections) 
{
  g_signal_emit_by_name ((gpointer) projects_popup, "make-clean", selections);
}

static void 
make_install_action (AutotoolsProjectsPopup *projects_popup, 
                     GList                  *selections) 
{
  g_signal_emit_by_name ((gpointer) projects_popup, "make-install", selections);
}

static void 
configure_action (AutotoolsProjectsPopup *projects_popup, 
                  GList                  *selections) 
{
  g_signal_emit_by_name ((gpointer) projects_popup, "configure", selections);
}

static void 
autoreconf_action (AutotoolsProjectsPopup *projects_popup, 
                   GList                  *selections) 
{
  g_signal_emit_by_name ((gpointer) projects_popup, "autoreconf", selections);
}
