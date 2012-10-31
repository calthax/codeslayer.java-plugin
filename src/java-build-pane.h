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

#ifndef __JAVA_BUILD_PANE_H__
#define	__JAVA_BUILD_PANE_H__

#include <gtk/gtk.h>
#include "java-page.h"
#include <codeslayer/codeslayer.h>

G_BEGIN_DECLS

#define JAVA_BUILD_PANE_TYPE            (java_build_pane_get_type ())
#define JAVA_BUILD_PANE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_BUILD_PANE_TYPE, JavaBuildPane))
#define JAVA_BUILD_PANE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_BUILD_PANE_TYPE, JavaBuildPaneClass))
#define IS_JAVA_BUILD_PANE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_BUILD_PANE_TYPE))
#define IS_JAVA_BUILD_PANE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_BUILD_PANE_TYPE))

typedef struct _JavaBuildPane JavaBuildPane;
typedef struct _JavaBuildPaneClass JavaBuildPaneClass;

struct _JavaBuildPane
{
  GtkHBox parent_instance;
};

struct _JavaBuildPaneClass
{
  GtkHBoxClass parent_class;
};

GType java_build_pane_get_type (void) G_GNUC_CONST;

GtkWidget*  java_build_pane_new             (JavaPageType  page_type,
                                               CodeSlayer   *codeslayer);

void        java_build_pane_clear_text     (JavaBuildPane *build_pane);
void        java_build_pane_append_text    (JavaBuildPane *build_pane, 
                                            gchar         *text);
void        java_build_pane_create_links   (JavaBuildPane *build_pane);
void        java_build_pane_start_process  (JavaBuildPane *build_pane, 
                                            gchar         *text);
void        java_build_pane_stop_process   (JavaBuildPane *build_pane);

G_END_DECLS

#endif /* __JAVA_BUILD_PANE_H__ */
