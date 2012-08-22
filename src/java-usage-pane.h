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

#ifndef __JAVA_USAGE_PANE_H__
#define	__JAVA_USAGE_PANE_H__

#include <codeslayer/codeslayer.h>
#include <gtk/gtk.h>
#include "java-page.h"

G_BEGIN_DECLS

#define JAVA_USAGE_PANE_TYPE            (java_usage_pane_get_type ())
#define JAVA_USAGE_PANE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_USAGE_PANE_TYPE, JavaUsagePane))
#define JAVA_USAGE_PANE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_USAGE_PANE_TYPE, JavaUsagePaneClass))
#define IS_JAVA_USAGE_PANE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_USAGE_PANE_TYPE))
#define IS_JAVA_USAGE_PANE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_USAGE_PANE_TYPE))

typedef struct _JavaUsagePane JavaUsagePane;
typedef struct _JavaUsagePaneClass JavaUsagePaneClass;

struct _JavaUsagePane
{
  GtkHBox parent_instance;
};

struct _JavaUsagePaneClass
{
  GtkHBoxClass parent_class;
};

GType java_usage_pane_get_type (void) G_GNUC_CONST;

GtkWidget*  java_usage_pane_new         (CodeSlayer *codeslayer, 
                                         JavaPageType   page_type);

void        java_usage_pane_set_usages  (JavaUsagePane *usage_pane, 
                                         GList         *usages);

G_END_DECLS

#endif /* __JAVA_USAGE_PANE_H__ */
