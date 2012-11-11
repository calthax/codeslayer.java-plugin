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
static void save_tools_properties             (JavaToolsProperties      *tools_properties);
static gchar* get_conf_path                   (JavaToolsProperties       *tools_properties);
static gboolean verify_conf_exists            (JavaToolsProperties       *tools_properties);

#define JAVA_TOOLS_PROPERTIES_FILE "java-tools.properties"
#define JDK_FOLDER "jdk_folder"
#define SUPPRESSIONS_FILE "suppressions_file"                                           
#define MAIN "main"                                           

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
  GKeyFile   *keyfile;  
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
  priv->keyfile = NULL;
}

static void
java_tools_properties_finalize (JavaToolsProperties *tools_properties)
{
  JavaToolsPropertiesPrivate *priv;
  priv = JAVA_TOOLS_PROPERTIES_GET_PRIVATE (tools_properties);

  if (priv->dialog != NULL)
    gtk_widget_destroy (priv->dialog);

  if (priv->keyfile)
    {
      g_key_file_free (priv->keyfile);
      priv->keyfile = NULL;
    }

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

const gchar*          
java_tools_properties_get_jdk_folder (JavaToolsProperties *tools_properties)
{
  JavaToolsPropertiesPrivate *priv;
  priv = JAVA_TOOLS_PROPERTIES_GET_PRIVATE (tools_properties);

  if (g_key_file_has_key (priv->keyfile, MAIN, JDK_FOLDER, NULL))
    {
      return g_key_file_get_string (priv->keyfile, MAIN, JDK_FOLDER, NULL);
    }
    
  return NULL;    
}

const gchar*
java_tools_properties_get_suppressions_file (JavaToolsProperties *tools_properties)
{
  JavaToolsPropertiesPrivate *priv;
  priv = JAVA_TOOLS_PROPERTIES_GET_PRIVATE (tools_properties);
  
  if (g_key_file_has_key (priv->keyfile, MAIN, SUPPRESSIONS_FILE, NULL))
    {
      return g_key_file_get_string (priv->keyfile, MAIN, SUPPRESSIONS_FILE, NULL);
    }
    
  return NULL;    
}

static void
properties_action (JavaToolsProperties *tools_properties)
{
  JavaToolsPropertiesPrivate *priv;
  gint response;
  
  priv = JAVA_TOOLS_PROPERTIES_GET_PRIVATE (tools_properties);

  if (priv->dialog == NULL)
    {
      GtkWidget *content_area;
      GtkWidget *grid;
      GtkWidget  *jdk_folder_entry;
      GtkWidget *jdk_folder_label;
      GtkWidget  *suppressions_file_entry;  
      GtkWidget *suppressions_file_label;
      
      priv->dialog = gtk_dialog_new_with_buttons ("Properties", 
                                                  codeslayer_get_toplevel_window (priv->codeslayer),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                  GTK_STOCK_OK, GTK_RESPONSE_OK,
                                                  NULL);
      gtk_window_set_skip_taskbar_hint (GTK_WINDOW (priv->dialog), TRUE);
      gtk_window_set_skip_pager_hint (GTK_WINDOW (priv->dialog), TRUE);  

      content_area = gtk_dialog_get_content_area (GTK_DIALOG (priv->dialog));

      grid = gtk_grid_new ();
      gtk_grid_set_row_spacing (GTK_GRID (grid), 2);

      jdk_folder_label = gtk_label_new ("JDK Folder:");
      gtk_misc_set_alignment (GTK_MISC (jdk_folder_label), 1, .5);
      gtk_misc_set_padding (GTK_MISC (jdk_folder_label), 4, 0);
      gtk_grid_attach (GTK_GRID (grid), jdk_folder_label, 0, 0, 1, 1);

      jdk_folder_entry = gtk_entry_new ();
      priv->jdk_folder_entry = jdk_folder_entry;
      gtk_entry_set_activates_default (GTK_ENTRY (jdk_folder_entry), TRUE);
      gtk_entry_set_width_chars (GTK_ENTRY (jdk_folder_entry), 50);  
      gtk_entry_set_icon_from_stock (GTK_ENTRY (jdk_folder_entry), 
                                     GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_DIRECTORY);
      gtk_grid_attach_next_to (GTK_GRID (grid), jdk_folder_entry, jdk_folder_label, 
                               GTK_POS_RIGHT, 1, 1);
                        
      suppressions_file_label = gtk_label_new ("Suppressions File:");
      gtk_label_set_width_chars (GTK_LABEL (suppressions_file_label), 10);
      gtk_misc_set_alignment (GTK_MISC (suppressions_file_label), 1, .50);
      gtk_misc_set_padding (GTK_MISC (suppressions_file_label), 4, 0);
      gtk_grid_attach (GTK_GRID (grid), suppressions_file_label, 0, 1, 1, 1);

      suppressions_file_entry = gtk_entry_new ();
      priv->suppressions_file_entry = suppressions_file_entry;
      gtk_entry_set_width_chars (GTK_ENTRY (suppressions_file_entry), 50);
      gtk_entry_set_icon_from_stock (GTK_ENTRY (suppressions_file_entry), 
                                     GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_FILE);
      gtk_grid_attach_next_to (GTK_GRID (grid), suppressions_file_entry, suppressions_file_label, 
                               GTK_POS_RIGHT, 1, 1);
                        
      gtk_box_pack_start (GTK_BOX (content_area), grid, TRUE, TRUE, 0);
      gtk_widget_show_all (content_area);
      
      g_signal_connect (G_OBJECT (jdk_folder_entry), "icon-press",
                        G_CALLBACK (jdk_folder_icon_action), tools_properties);

      g_signal_connect (G_OBJECT (suppressions_file_entry), "icon-press",
                        G_CALLBACK (suppressions_file_icon_action), tools_properties);
    }

  if (g_key_file_has_key (priv->keyfile, MAIN, JDK_FOLDER, NULL))
    {
      gtk_entry_set_text (GTK_ENTRY (priv->jdk_folder_entry), 
                        g_key_file_get_string (priv->keyfile, MAIN, JDK_FOLDER, NULL));
    }
  
  if (g_key_file_has_key (priv->keyfile, MAIN, SUPPRESSIONS_FILE, NULL))
    {
      gtk_entry_set_text (GTK_ENTRY (priv->suppressions_file_entry), 
                        g_key_file_get_string (priv->keyfile, MAIN, SUPPRESSIONS_FILE, NULL));
    }

  response = gtk_dialog_run (GTK_DIALOG (priv->dialog));
  if (response == GTK_RESPONSE_OK)
    {
      save_tools_properties (tools_properties);
    }
    
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

void
java_tools_properties_load (JavaToolsProperties *tools_properties)
{
  JavaToolsPropertiesPrivate *priv;
  GKeyFile *keyfile;
  gchar *conf;
  
  priv = JAVA_TOOLS_PROPERTIES_GET_PRIVATE (tools_properties);
  
  if (priv->keyfile)
    {
      g_key_file_free (priv->keyfile);
      priv->keyfile = NULL;
    }
  
  verify_conf_exists (tools_properties);
  keyfile = g_key_file_new ();

  conf = get_conf_path (tools_properties);
  g_key_file_load_from_file (keyfile, conf, G_KEY_FILE_NONE, NULL);
    
  priv->keyfile = keyfile;
  g_free (conf);
}

static void
save_tools_properties (JavaToolsProperties *tools_properties)
{
  JavaToolsPropertiesPrivate *priv;
  gchar *data;
  gchar *conf_path;
  gsize size;
  
  priv = JAVA_TOOLS_PROPERTIES_GET_PRIVATE (tools_properties);
  
  g_key_file_set_string (priv->keyfile, MAIN, JDK_FOLDER, 
                         gtk_entry_get_text (GTK_ENTRY (priv->jdk_folder_entry)));  
  
  g_key_file_set_string (priv->keyfile, MAIN, SUPPRESSIONS_FILE, 
                         gtk_entry_get_text (GTK_ENTRY (priv->suppressions_file_entry)));

  data = g_key_file_to_data (priv->keyfile, &size, NULL);

  conf_path = get_conf_path (tools_properties);

  g_file_set_contents (conf_path, data, size, NULL);

  g_free (conf_path);
  g_free (data);
}

static gboolean
verify_conf_exists (JavaToolsProperties *tools_properties)
{
  gboolean result = TRUE;
  gchar *conf_path;
  GFile *conf_file;

  conf_path = get_conf_path (tools_properties);
  conf_file = g_file_new_for_path (conf_path);
  if (!g_file_query_exists (conf_file, NULL))
    {
      GFileIOStream *stream;
      stream = g_file_create_readwrite (conf_file, G_FILE_CREATE_NONE, 
                                        NULL, NULL);
      g_io_stream_close (G_IO_STREAM (stream), NULL, NULL);
      g_object_unref (stream);
      result = FALSE;
    }

  g_free (conf_path);
  g_object_unref (conf_file);

  return result;
}

static gchar*
get_conf_path (JavaToolsProperties *tools_properties)
{
  JavaToolsPropertiesPrivate *priv;
  gchar *group_folder_path;
  gchar *result;
  priv = JAVA_TOOLS_PROPERTIES_GET_PRIVATE (tools_properties);
  group_folder_path = codeslayer_get_active_group_folder_path (priv->codeslayer);  
  result = g_build_filename (group_folder_path, JAVA_TOOLS_PROPERTIES_FILE, NULL);
  g_free (group_folder_path);
  return result;
}
