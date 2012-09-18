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

#include "java-properties.h"

static void java_properties_class_init  (JavaPropertiesClass *klass);
static void java_properties_init        (JavaProperties      *properties);
static void java_properties_finalize    (JavaProperties      *properties);

static void properties_action           (JavaProperties      *properties);


#define JAVA_PROPERTIES_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_PROPERTIES_TYPE, JavaPropertiesPrivate))

typedef struct _JavaPropertiesPrivate JavaPropertiesPrivate;

struct _JavaPropertiesPrivate
{
  CodeSlayer         *codeslayer;
  GtkWidget          *menu;
};

G_DEFINE_TYPE (JavaProperties, java_properties, G_TYPE_OBJECT)

static void
java_properties_class_init (JavaPropertiesClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) java_properties_finalize;
  g_type_class_add_private (klass, sizeof (JavaPropertiesPrivate));
}

static void
java_properties_init (JavaProperties *properties) {}

static void
java_properties_finalize (JavaProperties *properties)
{
  G_OBJECT_CLASS (java_properties_parent_class)->finalize (G_OBJECT(properties));
}

JavaProperties*
java_properties_new (CodeSlayer *codeslayer,
                     GtkWidget  *menu)
{
  JavaPropertiesPrivate *priv;
  JavaProperties *properties;

  properties = JAVA_PROPERTIES (g_object_new (java_properties_get_type (), NULL));
  priv = JAVA_PROPERTIES_GET_PRIVATE (properties);

  priv->codeslayer = codeslayer;
  priv->menu = menu;

  g_signal_connect_swapped (G_OBJECT (menu), "properties",
                            G_CALLBACK (properties_action), properties);
                            
  return properties;
}

static void
properties_action (JavaProperties *properties)
{
  GtkWidget *dialog;
  
  dialog = gtk_dialog_new_with_buttons ("Properties", 
                                        NULL,
                                        GTK_DIALOG_MODAL,
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OK, GTK_RESPONSE_OK,
                                        NULL);
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (dialog), TRUE);  

  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_hide (dialog);
}
