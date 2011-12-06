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

#ifndef __AUTOTOOLS_MENU_H__
#define	__AUTOTOOLS_MENU_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define AUTOTOOLS_MENU_TYPE            (autotools_menu_get_type ())
#define AUTOTOOLS_MENU(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), AUTOTOOLS_MENU_TYPE, AutotoolsMenu))
#define AUTOTOOLS_MENU_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), AUTOTOOLS_MENU_TYPE, AutotoolsMenuClass))
#define IS_AUTOTOOLS_MENU(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AUTOTOOLS_MENU_TYPE))
#define IS_AUTOTOOLS_MENU_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), AUTOTOOLS_MENU_TYPE))

typedef struct _AutotoolsMenu AutotoolsMenu;
typedef struct _AutotoolsMenuClass AutotoolsMenuClass;

struct _AutotoolsMenu
{
  GtkMenuItem parent_instance;
};

struct _AutotoolsMenuClass
{
  GtkMenuItemClass parent_class;

  void (*make) (AutotoolsMenu *menu);
  void (*make_install) (AutotoolsMenu *menu);
  void (*make_clean) (AutotoolsMenu *menu);
};

GType autotools_menu_get_type (void) G_GNUC_CONST;
  
GtkWidget*  autotools_menu_new  (GtkAccelGroup *accel_group);
                                            
G_END_DECLS

#endif /* __AUTOTOOLS_MENU_H__ */
