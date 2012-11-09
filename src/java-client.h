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

#ifndef __JAVA_CLIENT_H__
#define	__JAVA_CLIENT_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>

G_BEGIN_DECLS

#define JAVA_CLIENT_TYPE            (java_client_get_type ())
#define JAVA_CLIENT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_CLIENT_TYPE, JavaClient))
#define JAVA_CLIENT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_CLIENT_TYPE, JavaClientClass))
#define IS_JAVA_CLIENT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_CLIENT_TYPE))
#define IS_JAVA_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_CLIENT_TYPE))

typedef void (*ClientCallbackFunc) (gchar *output, gpointer data);

typedef struct _JavaClient JavaClient;
typedef struct _JavaClientClass JavaClientClass;

struct _JavaClient
{
  GObject parent_instance;
};

struct _JavaClientClass
{
  GObjectClass parent_class;
};

GType java_client_get_type (void) G_GNUC_CONST;

JavaClient*  java_client_new                 (CodeSlayer         *codeslayer);
                  
void         java_client_connect             (JavaClient         *client);
gchar*       java_client_send                (JavaClient         *client, 
                                              gchar              *input);
void         java_client_send_with_callback  (JavaClient         *client, 
                                              gchar              *input,
                                              ClientCallbackFunc  func, 
                                              gpointer            data);

G_END_DECLS

#endif /* _JAVA_CLIENT_H */
