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

#include <codeslayer/codeslayer-utils.h>
#include <string.h>
#include "java-client.h"

static void java_client_class_init (JavaClientClass *klass);
static void java_client_init       (JavaClient      *client);
static void java_client_finalize   (JavaClient      *client);
                          
#define JAVA_CLIENT_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_CLIENT_TYPE, JavaClientPrivate))

typedef struct _JavaClientPrivate JavaClientPrivate;

struct _JavaClientPrivate
{
  CodeSlayer *codeslayer;
  GSocket    *socket;
};

G_DEFINE_TYPE (JavaClient, java_client, G_TYPE_OBJECT)

static void
java_client_class_init (JavaClientClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) java_client_finalize;
  g_type_class_add_private (klass, sizeof (JavaClientPrivate));
}

static void
java_client_init (JavaClient *client) {}

static void
java_client_finalize (JavaClient *client)
{
  JavaClientPrivate *priv;
  priv = JAVA_CLIENT_GET_PRIVATE (client);
  if  (priv->socket)
    {
      g_object_unref (priv->socket);    
    }
  G_OBJECT_CLASS (java_client_parent_class)->finalize (G_OBJECT(client));
}

JavaClient*
java_client_new (CodeSlayer *codeslayer)
{
  JavaClientPrivate *priv;
  JavaClient *client;

  client = JAVA_CLIENT (g_object_new (java_client_get_type (), NULL));
  priv = JAVA_CLIENT_GET_PRIVATE (client);

  priv->codeslayer = codeslayer;

  return client;
}

void
java_client_connect (JavaClient *client)
{
  JavaClientPrivate *priv;
  GSocketClient *socketClient;
  GSocketConnection *socketConnection;
  GError *error = NULL;

  priv = JAVA_CLIENT_GET_PRIVATE (client);
  
  socketClient = g_socket_client_new ();  
  socketConnection = g_socket_client_connect_to_host (socketClient, "localhost", 4444, NULL, &error);
  
  if (error != NULL)
    {
      g_print ("%s\n", error->message);
      return;
    }
  
  priv->socket = g_socket_connection_get_socket (socketConnection);
}

gchar*
java_client_send (JavaClient *client, 
                  gchar      *message)
{
  JavaClientPrivate *priv;
  gchar buffer[1024];
  GError *error = NULL;
  gint received;
  gint position;
  GString *page;
  gchar *text;

  priv = JAVA_CLIENT_GET_PRIVATE (client);

  if (!priv->socket) 
    {
      java_client_connect (client);
      if (!priv->socket)
        {
          g_print ("Not connected to the CodeSlayer Java server!!");
          return NULL;
        }
    }

  text = g_strconcat (message, "\n", NULL);

  g_socket_send_with_blocking (priv->socket, text, strlen(text), FALSE, NULL, &error);
  
  g_free (text);

  if (error != NULL)
    {
      g_print ("%s\n", error->message);
      return NULL;
    }
    
  page = g_string_new ("");

  while ((received = g_socket_receive_with_blocking (priv->socket, buffer, 1024, TRUE, NULL, NULL)))
    {
      for (position = 0; position < received; position++) 
        {
          g_string_append_c (page, buffer[position]);
        }
      break;
    }
    
  return g_string_free (page, FALSE);
}
