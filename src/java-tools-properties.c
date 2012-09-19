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

#include "java-tools-properties.h"

static void java_tools_properties_class_init  (JavaToolsPropertiesClass *klass);
static void java_tools_properties_init        (JavaToolsProperties      *tools_properties);
static void java_tools_properties_finalize    (JavaToolsProperties      *tools_properties);

static void properties_action                 (JavaToolsProperties      *tools_properties);
static void jdk_folder_icon_action            (GtkEntry                 *lib_folder_entry,
                                               GtkEntryIconPosition      icon_pos,
                                               GdkEvent                 *event,
                                               JavaToolsProperties      *tools_properties);
static void suppressions_file_icon_action     (GtkEntry                 *suppressions_file_entry,
                                               GtkEntryIconPosition      icon_pos,
                                               GdkEvent                 *event,
                                               JavaToolsProperties      *tools_properties);

#define JAVA_TOOLS_PROPERTIES_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_TOOLS_PROPERTIES_TYPE, JavaToolsPropertiesPrivate))

typedef struct _JavaToolsPropertiesPrivate JavaToolsPropertiesPrivate;

struct _JavaToolsPropertiesPrivate
{
  CodeSlayer *codeslayer;
  GtkWidget  *menu;
  GtkWidget  *dialog;
  GtkWidget  *jdk_folder_entry;
  GtkWidget  *suppressions_file_entry;  
};

G_DEFINE_TYPE (JavaToolsProperties, java_tools_properties, G_TYPE_OBJECT)

static void
java_tools_properties_class_init (JavaToolsPropertiesClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) java_tools_properties_finalize;
  g_type_class_add_private (klass, sizeof (JavaToolsPropertiesPrivate));
}

static void
java_tools_properties_init (JavaToolsProperties *tools_properties) 
{
  JavaToolsPropertiesPrivate *priv;
  priv = JAVA_TOOLS_PROPERTIES_GET_PRIVATE (tools_properties);
  priv->dialog = NULL;
}

static void
java_tools_properties_finalize (JavaToolsProperties *tools_properties)
{
  JavaToolsPropertiesPrivate *priv;
  priv = JAVA_TOOLS_PROPERTIES_GET_PRIVATE (tools_properties);
  if (priv->dialog != NULL)
    gtk_widget_destroy (priv->dialog);
  G_OBJECT_CLASS (java_tools_properties_parent_class)->finalize (G_OBJECT(tools_properties));
}

JavaToolsProperties*
java_tools_properties_new (CodeSlayer *codeslayer,
                     GtkWidget  *menu)
{
  JavaToolsPropertiesPrivate *priv;
  JavaToolsProperties *tools_properties;

  tools_properties = JAVA_TOOLS_PROPERTIES (g_object_new (java_tools_properties_get_type (), NULL));
  priv = JAVA_TOOLS_PROPERTIES_GET_PRIVATE (tools_properties);

  priv->codeslayer = codeslayer;
  priv->menu = menu;

  g_signal_connect_swapped (G_OBJECT (menu), "properties",
                            G_CALLBACK (properties_action), tools_properties);
                            
  return tools_properties;
}

static void
properties_action (JavaToolsProperties *tools_properties)
{
  JavaToolsPropertiesPrivate *priv;
  
  priv = JAVA_TOOLS_PROPERTIES_GET_PRIVATE (tools_properties);

  if (priv->dialog == NULL)
    {
      GtkWidget *content_area;
      GtkWidget *table;
      GtkWidget  *jdk_folder_entry;
      GtkWidget *jdk_folder_label;
      GtkWidget  *suppressions_file_entry;  
      GtkWidget *suppressions_file_label;
      
      priv->dialog = gtk_dialog_new_with_buttons ("Properties", 
                                                  NULL,
                                                  GTK_DIALOG_MODAL,
                                                  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                  GTK_STOCK_OK, GTK_RESPONSE_OK,
                                                  NULL);
      gtk_window_set_skip_taskbar_hint (GTK_WINDOW (priv->dialog), TRUE);
      gtk_window_set_skip_pager_hint (GTK_WINDOW (priv->dialog), TRUE);  

      content_area = gtk_dialog_get_content_area (GTK_DIALOG (priv->dialog));

      table = gtk_table_new (4, 2, FALSE);

      jdk_folder_label = gtk_label_new ("JDK Folder:");
      gtk_misc_set_alignment (GTK_MISC (jdk_folder_label), 1, .5);
      gtk_table_attach (GTK_TABLE (table), jdk_folder_label, 0, 1, 0, 1, 
                        GTK_FILL, GTK_FILL, 4, 1);

      jdk_folder_entry = gtk_entry_new ();
      priv->jdk_folder_entry = jdk_folder_entry;
      gtk_entry_set_activates_default (GTK_ENTRY (jdk_folder_entry), TRUE);
      gtk_entry_set_width_chars (GTK_ENTRY (jdk_folder_entry), 50);  
      gtk_entry_set_icon_from_stock (GTK_ENTRY (jdk_folder_entry), 
                                     GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_DIRECTORY);
      gtk_table_attach (GTK_TABLE (table), jdk_folder_entry, 1, 2, 0, 1,
                        GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL, 4, 1);
                        
      suppressions_file_label = gtk_label_new ("Suppressions File:");
      gtk_label_set_width_chars (GTK_LABEL (suppressions_file_label), 10);
      gtk_misc_set_alignment (GTK_MISC (suppressions_file_label), .97, .50);
      gtk_table_attach (GTK_TABLE (table), suppressions_file_label, 0, 1, 1, 2, 
                        GTK_FILL, GTK_FILL, 4, 1);

      suppressions_file_entry = gtk_entry_new ();
      priv->suppressions_file_entry = suppressions_file_entry;
      gtk_entry_set_width_chars (GTK_ENTRY (suppressions_file_entry), 50);
      gtk_entry_set_icon_from_stock (GTK_ENTRY (suppressions_file_entry), 
                                     GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_FILE);
      gtk_table_attach (GTK_TABLE (table), suppressions_file_entry, 1, 2, 1, 2,
                        GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL, 4, 1);
                        
      gtk_box_pack_start (GTK_BOX (content_area), table, TRUE, TRUE, 0);
      gtk_widget_show_all (content_area);
      
      g_signal_connect (G_OBJECT (jdk_folder_entry), "icon-press",
                        G_CALLBACK (jdk_folder_icon_action), tools_properties);

      g_signal_connect (G_OBJECT (suppressions_file_entry), "icon-press",
                        G_CALLBACK (suppressions_file_icon_action), tools_properties);
    }

  gtk_dialog_run (GTK_DIALOG (priv->dialog));
  gtk_widget_hide (priv->dialog);
}

static void 
jdk_folder_icon_action (GtkEntry             *jdk_folder_entry,
                        GtkEntryIconPosition  icon_pos,
                        GdkEvent             *event,
                        JavaToolsProperties  *tools_properties)
{
  GtkWidget *dialog;
  gint response;
  
  dialog = gtk_file_chooser_dialog_new ("Select JDK Folder",
                                        NULL,
                                        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                        GTK_STOCK_CANCEL,
                                        GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OPEN,
                                        GTK_RESPONSE_OK, 
                                        NULL);

  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
  
  response = gtk_dialog_run (GTK_DIALOG (dialog));
  if (response == GTK_RESPONSE_OK)
    {
      GFile *file;
      char *file_path;
      file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog));
      file_path = g_file_get_path (file);
      gtk_entry_set_text (jdk_folder_entry, file_path);
      g_free (file_path);
      g_object_unref (file);
    }

  gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void 
suppressions_file_icon_action (GtkEntry             *suppressions_file_entry,
                               GtkEntryIconPosition  icon_pos,
                               GdkEvent             *event,
                               JavaToolsProperties  *tools_properties)
{
  GtkWidget *dialog;
  gint response;
  
  dialog = gtk_file_chooser_dialog_new ("Select Suppressions File", 
                                        NULL,
                                        GTK_FILE_CHOOSER_ACTION_OPEN,
                                        GTK_STOCK_CANCEL,
                                        GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OPEN,
                                        GTK_RESPONSE_OK, 
                                        NULL);

  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

  response = gtk_dialog_run (GTK_DIALOG (dialog));
  if (response == GTK_RESPONSE_OK)
    {
      GFile *file;
      char *file_path;
      file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog));
      file_path = g_file_get_path (file);
      gtk_entry_set_text (suppressions_file_entry, file_path);
      g_free (file_path);
      g_object_unref (file);
    }

  gtk_widget_destroy (GTK_WIDGET (dialog));  
}                        
