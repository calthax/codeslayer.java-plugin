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

#include <gtk/gtk.h>
#include "java-debugger-service.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void java_debugger_service_class_init  (JavaDebuggerServiceClass *klass);
static void java_debugger_service_init        (JavaDebuggerService      *debugger);
static void java_debugger_service_finalize    (JavaDebuggerService      *debugger);

static gboolean iochannel_read                (GIOChannel               *channel, 
                                               GIOCondition              condition, 
                                               JavaDebuggerService      *service);

#define JAVA_DEBUGGER_SERVICE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_DEBUGGER_SERVICE_TYPE, JavaDebuggerServicePrivate))

typedef struct _JavaDebuggerServicePrivate JavaDebuggerServicePrivate;

struct _JavaDebuggerServicePrivate
{
  GIOChannel *channel_read;
  GIOChannel *channel_write;
};

G_DEFINE_TYPE (JavaDebuggerService, java_debugger_service, G_TYPE_OBJECT)
     
static void 
java_debugger_service_class_init (JavaDebuggerServiceClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) java_debugger_service_finalize;
  g_type_class_add_private (klass, sizeof (JavaDebuggerServicePrivate));
}

static void
java_debugger_service_init (JavaDebuggerService *service)
{
  JavaDebuggerServicePrivate *priv;
  priv = JAVA_DEBUGGER_SERVICE_GET_PRIVATE (service);
  priv->channel_read = NULL;
  priv->channel_write = NULL;
}

static void
java_debugger_service_finalize (JavaDebuggerService *service)
{
  G_OBJECT_CLASS (java_debugger_service_parent_class)->finalize (G_OBJECT (service));
}

JavaDebuggerService*
java_debugger_service_new ()
{
  JavaDebuggerService *service;
  service = JAVA_DEBUGGER_SERVICE (g_object_new (java_debugger_service_get_type (), NULL));
  return service;
}

void 
java_debugger_service_start (JavaDebuggerService *service)
{
  JavaDebuggerServicePrivate *priv;

  int	stdin_pipe[2];
  int	stdout_pipe[2];

  int pid;

  priv = JAVA_DEBUGGER_SERVICE_GET_PRIVATE (service);

  if (pipe (stdin_pipe) != 0	|| pipe (stdout_pipe) != 0)
    {
      g_warning ("Not able to create the pipes.");  
      return;
    }

  pid = fork ();

  if (pid == -1)
    {
      g_warning ("Not able to fork the process.");
      return;
    }

  if (pid == 0)
    {
      close (0);
      dup (stdin_pipe[0]);
      close (stdin_pipe[0]); 
      close (stdin_pipe[1]); 

      close (1);
      dup (stdout_pipe[1]);
      close (stdout_pipe[0]);
      close (stdout_pipe[1]);

      execlp ("ejdb", "ejdb", "-interactive", "true", "-launch", "org.junit.runner.JUnitCore org.jmesa.core.CoreContextTest", "-sourcepath", "/home/jeff/workspace/jmesaWeb/src:/home/jeff/workspace/jmesa/src:/home/jeff/workspace/jmesa/test", "-classpath", "/home/jeff/workspace/jmesa/classes:/home/jeff/workspace/jmesa/lib/*", (char *)NULL);

      g_warning ("Not able to communicate with ejdb.");
    }
  else
    {
      priv->channel_read = g_io_channel_unix_new (stdout_pipe[0]);
      priv->channel_write = g_io_channel_unix_new (stdin_pipe[1]);
      
      close (stdin_pipe[0]);
      close (stdout_pipe[1]);

      if (priv->channel_read == NULL || priv->channel_write == NULL)
        g_error ("The channels could not be created.");
        
      g_io_add_watch (priv->channel_read, G_IO_IN | G_IO_HUP, (GIOFunc) iochannel_read, NULL);        
    }
}

void 
java_debugger_service_stop (JavaDebuggerService *service)
{
  java_debugger_service_send_command (service, "q\n");
}

void 
java_debugger_service_send_command (JavaDebuggerService *service, 
                                    char                *cmd)
{
  JavaDebuggerServicePrivate *priv;
  GIOStatus ret_value;
  gsize length;

  priv = JAVA_DEBUGGER_SERVICE_GET_PRIVATE (service);

  g_print ("%s\n", cmd);
  
  ret_value = g_io_channel_write_chars (priv->channel_write, cmd, -1, &length, NULL);
  if (ret_value == G_IO_STATUS_ERROR)
    g_error ("The changes could not be written to the pipe.");
  else
    g_io_channel_flush (priv->channel_write, NULL);
}

static gboolean
iochannel_read (GIOChannel          *channel, 
                GIOCondition         condition, 
                JavaDebuggerService *service)
{
  JavaDebuggerServicePrivate *priv;
  GIOStatus ret_value;
  gchar *message;
  gsize length;
  
  priv = JAVA_DEBUGGER_SERVICE_GET_PRIVATE (service);

  if (condition == G_IO_HUP)
    g_error ("The pipe has died.");

  ret_value = g_io_channel_read_line (priv->channel_read, &message, &length, NULL, NULL);
  if (ret_value == G_IO_STATUS_ERROR)
    g_error ("Could not read from the pipe.");

  g_print ("read: %s", message);

  return TRUE;
}
