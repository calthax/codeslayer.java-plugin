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

#ifndef __AUTOTOOLS_ENGINE_H__
#define	__AUTOTOOLS_ENGINE_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>

G_BEGIN_DECLS

#define AUTOTOOLS_ENGINE_TYPE            (autotools_engine_get_type ())
#define AUTOTOOLS_ENGINE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), AUTOTOOLS_ENGINE_TYPE, AutotoolsEngine))
#define AUTOTOOLS_ENGINE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), AUTOTOOLS_ENGINE_TYPE, AutotoolsEngineClass))
#define IS_AUTOTOOLS_ENGINE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AUTOTOOLS_ENGINE_TYPE))
#define IS_AUTOTOOLS_ENGINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), AUTOTOOLS_ENGINE_TYPE))

typedef struct _AutotoolsEngine AutotoolsEngine;
typedef struct _AutotoolsEngineClass AutotoolsEngineClass;

struct _AutotoolsEngine
{
  GObject parent_instance;
};

struct _AutotoolsEngineClass
{
  GObjectClass parent_class;
};

GType autotools_engine_get_type (void) G_GNUC_CONST;

AutotoolsEngine*  autotools_engine_new (CodeSlayer *codeslayer,
                                        GtkWidget  *menu,
                                        GtkWidget  *project_properties,
                                        GtkWidget  *projects_menu,
                                        GtkWidget  *notebook);
                                        
void autotools_engine_load_configurations (AutotoolsEngine *engine);

G_END_DECLS

#endif /* _AUTOTOOLS_ENGINE_H */
