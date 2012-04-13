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

#ifndef __JAVA_BUILD_H__
#define	__JAVA_BUILD_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>
#include "java-configurations.h"
#include "java-debugger.h"

G_BEGIN_DECLS

#define JAVA_BUILD_TYPE            (java_build_get_type ())
#define JAVA_BUILD(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_BUILD_TYPE, JavaBuild))
#define JAVA_BUILD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_BUILD_TYPE, JavaBuildClass))
#define IS_JAVA_BUILD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_BUILD_TYPE))
#define IS_JAVA_BUILD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_BUILD_TYPE))

typedef struct _JavaBuild JavaBuild;
typedef struct _JavaBuildClass JavaBuildClass;

struct _JavaBuild
{
  GObject parent_instance;
};

struct _JavaBuildClass
{
  GObjectClass parent_class;
};

GType java_build_get_type (void) G_GNUC_CONST;

JavaBuild*  java_build_new (CodeSlayer         *codeslayer,
                            JavaConfigurations *configurations,
                            GtkWidget          *menu,
                            GtkWidget          *projects_popup,
                            GtkWidget          *notebook);

G_END_DECLS

#endif /* _JAVA_BUILD_H */
