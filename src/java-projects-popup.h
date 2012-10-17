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

#ifndef __JAVA_PROJECTS_POPUP_H__
#define	__JAVA_PROJECTS_POPUP_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define JAVA_PROJECTS_POPUP_TYPE            (java_projects_popup_get_type ())
#define JAVA_PROJECTS_POPUP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_PROJECTS_POPUP_TYPE, JavaProjectsPopup))
#define JAVA_PROJECTS_POPUP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_PROJECTS_POPUP_TYPE, JavaProjectsPopupClass))
#define IS_JAVA_PROJECTS_POPUP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_PROJECTS_POPUP_TYPE))
#define IS_JAVA_PROJECTS_POPUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_PROJECTS_POPUP_TYPE))

typedef struct _JavaProjectsPopup JavaProjectsPopup;
typedef struct _JavaProjectsPopupClass JavaProjectsPopupClass;

struct _JavaProjectsPopup
{
  GtkMenuItem parent_instance;
};

struct _JavaProjectsPopupClass
{
  GtkMenuItemClass parent_class;

  void (*compile) (JavaProjectsPopup *projects_popup);
  void (*clean) (JavaProjectsPopup *projects_popup);
  void (*clean_compile) (JavaProjectsPopup *projects_popup);
  void (*test_project) (JavaProjectsPopup *projects_popup);
};

GType java_projects_popup_get_type (void) G_GNUC_CONST;
  
GtkWidget*  java_projects_popup_new  (void);
                                            
G_END_DECLS

#endif /* __JAVA_PROJECTS_POPUP_H__ */
