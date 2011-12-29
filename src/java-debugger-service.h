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

#ifndef __JAVA_DEBUGGER_SERVICE_H__
#define	__JAVA_DEBUGGER_SERVICE_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>
#include "java-configurations.h"
#include "java-debugger-breakpoints.h"

G_BEGIN_DECLS

#define JAVA_DEBUGGER_SERVICE_TYPE            (java_debugger_service_get_type ())
#define JAVA_DEBUGGER_SERVICE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_DEBUGGER_SERVICE_TYPE, JavaDebuggerService))
#define JAVA_DEBUGGER_SERVICE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_DEBUGGER_SERVICE_TYPE, JavaDebuggerServiceClass))
#define IS_JAVA_DEBUGGER_SERVICE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_DEBUGGER_SERVICE_TYPE))
#define IS_JAVA_DEBUGGER_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_DEBUGGER_SERVICE_TYPE))

typedef struct _JavaDebuggerService JavaDebuggerService;
typedef struct _JavaDebuggerServiceClass JavaDebuggerServiceClass;

struct _JavaDebuggerService
{
  GObject parent_instance;
};

struct _JavaDebuggerServiceClass
{
  GObjectClass parent_class;
  
  void (*read_channel) (JavaDebuggerService *service);
};

GType java_debugger_service_get_type (void) G_GNUC_CONST;

JavaDebuggerService*  java_debugger_service_new    (void);

void                  java_debugger_service_start         (JavaDebuggerService *service);
gboolean              java_debugger_service_get_running   (JavaDebuggerService *service);
void                  java_debugger_service_send_command  (JavaDebuggerService *service, 
                                                           gchar               *command);

G_END_DECLS

#endif /* __JAVA_DEBUGGER_SERVICE_H__ */
