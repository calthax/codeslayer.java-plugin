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

#include "java-project-properties.h"

static void java_project_properties_class_init  (JavaProjectPropertiesClass *klass);
static void java_project_properties_init        (JavaProjectProperties      *project_properties);
static void java_project_properties_finalize    (JavaProjectProperties      *project_properties);

static void add_form                            (JavaProjectProperties      *project_properties);

static void ant_file_icon_action                (GtkEntry                   *ant_file_entry, 
                                                 GtkEntryIconPosition        icon_pos, 
                                                 GdkEvent                   *event,
                                                 JavaProjectProperties      *project_properties);

static void build_folder_icon_action            (GtkEntry                   *build_folder_entry,
                                                 GtkEntryIconPosition        icon_pos,
                                                 GdkEvent                   *event,
                                                 JavaProjectProperties      *project_properties);
static void lib_folder_icon_action              (GtkEntry                   *lib_folder_entry,
                                                 GtkEntryIconPosition        icon_pos,
                                                 GdkEvent                   *event,
                                                 JavaProjectProperties      *project_properties);
static void source_folder_icon_action             (GtkEntry                   *source_folder_entry,
                                                 GtkEntryIconPosition        icon_pos,
                                                 GdkEvent                   *event,
                                                 JavaProjectProperties      *project_properties);
static void test_folder_icon_action               (GtkEntry                   *test_folder_entry,
                                                 GtkEntryIconPosition        icon_pos,
                                                 GdkEvent                   *event,
                                                 JavaProjectProperties      *project_properties);
static gboolean entry_has_text                  (GtkWidget                  *entry);


#define JAVA_PROJECT_PROPERTIES_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_PROJECT_PROPERTIES_TYPE, JavaProjectPropertiesPrivate))

typedef struct _JavaProjectPropertiesPrivate JavaProjectPropertiesPrivate;

struct _JavaProjectPropertiesPrivate
{
  CodeSlayerProject *project;
  GtkWidget         *ant_file_entry;
  GtkWidget         *build_folder_entry;
  GtkWidget         *lib_folder_entry;
  GtkWidget         *source_folder_entry;
  GtkWidget         *test_folder_entry;
};

enum
{
  SAVE_CONFIGURATION,
  LAST_SIGNAL
};

static guint java_project_properties_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (JavaProjectProperties, java_project_properties, GTK_TYPE_VBOX)

static void
java_project_properties_class_init (JavaProjectPropertiesClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  java_project_properties_signals[SAVE_CONFIGURATION] =
    g_signal_new ("save-configuration", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (JavaProjectPropertiesClass, save_configuration), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, JAVA_CONFIGURATION_TYPE);

  gobject_class->finalize = (GObjectFinalizeFunc) java_project_properties_finalize;
  g_type_class_add_private (klass, sizeof (JavaProjectPropertiesPrivate));
}

static void
java_project_properties_init (JavaProjectProperties *project_properties) {}

static void
java_project_properties_finalize (JavaProjectProperties *project_properties)
{
  G_OBJECT_CLASS (java_project_properties_parent_class)->finalize (G_OBJECT(project_properties));
}

GtkWidget*
java_project_properties_new (void)
{
  GtkWidget *project_properties;
  project_properties = g_object_new (java_project_properties_get_type (), NULL);
  add_form (JAVA_PROJECT_PROPERTIES (project_properties));
  return project_properties;
}

static void 
add_form (JavaProjectProperties *project_properties)
{
  JavaProjectPropertiesPrivate *priv;
  GtkWidget *table;

  GtkWidget *ant_file_label;
  GtkWidget *ant_file_entry;

  GtkWidget *build_folder_label;
  GtkWidget *build_folder_entry;

  GtkWidget *lib_folder_label;
  GtkWidget *lib_folder_entry;

  GtkWidget *source_folder_label;
  GtkWidget *source_folder_entry;

  GtkWidget *test_folder_label;
  GtkWidget *test_folder_entry;

  priv = JAVA_PROJECT_PROPERTIES_GET_PRIVATE (project_properties);

  table = gtk_table_new (4, 2, FALSE);

  ant_file_label = gtk_label_new ("Ant File:");
  gtk_misc_set_alignment (GTK_MISC (ant_file_label), 1, .5);
  gtk_table_attach (GTK_TABLE (table), ant_file_label, 
                    0, 1, 0, 1, GTK_FILL, GTK_SHRINK, 4, 0);
                    
  ant_file_entry = gtk_entry_new ();
  priv->ant_file_entry = ant_file_entry;
  gtk_entry_set_width_chars (GTK_ENTRY (ant_file_entry), 50);
  gtk_entry_set_icon_from_stock (GTK_ENTRY (ant_file_entry), 
                                 GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_FILE);
  gtk_table_attach (GTK_TABLE (table), ant_file_entry, 1, 2, 0, 1,
                    GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL, 4, 1);
                      
  build_folder_label = gtk_label_new ("Build Folder:");
  gtk_misc_set_alignment (GTK_MISC (build_folder_label), 1, .5);
  gtk_table_attach (GTK_TABLE (table), build_folder_label, 
                    0, 1, 2, 3, GTK_FILL, GTK_SHRINK, 4, 0);
  
  build_folder_entry = gtk_entry_new ();
  priv->build_folder_entry = build_folder_entry;
  gtk_entry_set_width_chars (GTK_ENTRY (build_folder_entry), 50);
  gtk_entry_set_icon_from_stock (GTK_ENTRY (build_folder_entry), 
                                 GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_DIRECTORY);
  gtk_table_attach (GTK_TABLE (table), build_folder_entry, 1, 2, 2, 3,
                    GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL, 4, 1);
                      
  lib_folder_label = gtk_label_new ("Lib Folder:");
  gtk_misc_set_alignment (GTK_MISC (lib_folder_label), 1, .5);
  gtk_table_attach (GTK_TABLE (table), lib_folder_label, 
                    0, 1, 3, 4, GTK_FILL, GTK_SHRINK, 4, 0);
  
  lib_folder_entry = gtk_entry_new ();
  priv->lib_folder_entry = lib_folder_entry;
  gtk_entry_set_width_chars (GTK_ENTRY (lib_folder_entry), 50);
  gtk_entry_set_icon_from_stock (GTK_ENTRY (lib_folder_entry), 
                                 GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_DIRECTORY);
  gtk_table_attach (GTK_TABLE (table), lib_folder_entry, 1, 2, 3, 4,
                    GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL, 4, 1);
                      
  source_folder_label = gtk_label_new ("Source Folder:");
  gtk_misc_set_alignment (GTK_MISC (source_folder_label), 1, .5);
  gtk_table_attach (GTK_TABLE (table), source_folder_label, 
                    0, 1, 4, 5, GTK_FILL, GTK_SHRINK, 4, 0);
  
  source_folder_entry = gtk_entry_new ();
  priv->source_folder_entry = source_folder_entry;
  gtk_entry_set_width_chars (GTK_ENTRY (source_folder_entry), 50);
  gtk_entry_set_icon_from_stock (GTK_ENTRY (source_folder_entry), 
                                 GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_DIRECTORY);
  gtk_table_attach (GTK_TABLE (table), source_folder_entry, 1, 2, 4, 5,
                    GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL, 4, 1);
                      
  test_folder_label = gtk_label_new ("Test Folder:");
  gtk_misc_set_alignment (GTK_MISC (test_folder_label), 1, .5);
  gtk_table_attach (GTK_TABLE (table), test_folder_label, 
                    0, 1, 5, 6, GTK_FILL, GTK_SHRINK, 4, 0);
  
  test_folder_entry = gtk_entry_new ();
  priv->test_folder_entry = test_folder_entry;
  gtk_entry_set_width_chars (GTK_ENTRY (test_folder_entry), 50);
  gtk_entry_set_icon_from_stock (GTK_ENTRY (test_folder_entry), 
                                 GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_DIRECTORY);
  gtk_table_attach (GTK_TABLE (table), test_folder_entry, 1, 2, 5, 6,
                    GTK_FILL | GTK_EXPAND | GTK_SHRINK, GTK_FILL, 4, 1);
                      
  gtk_box_pack_start (GTK_BOX (project_properties), table, FALSE, FALSE, 2);
  

  g_signal_connect (G_OBJECT (ant_file_entry), "icon-press",
                    G_CALLBACK (ant_file_icon_action), project_properties);

  g_signal_connect (G_OBJECT (build_folder_entry), "icon-press",
                    G_CALLBACK (build_folder_icon_action), project_properties);

  g_signal_connect (G_OBJECT (lib_folder_entry), "icon-press",
                    G_CALLBACK (lib_folder_icon_action), project_properties);

  g_signal_connect (G_OBJECT (source_folder_entry), "icon-press",
                    G_CALLBACK (source_folder_icon_action), project_properties);

  g_signal_connect (G_OBJECT (test_folder_entry), "icon-press",
                    G_CALLBACK (test_folder_icon_action), project_properties);
}

static void 
ant_file_icon_action (GtkEntry              *ant_file_entry,
                      GtkEntryIconPosition   icon_pos,
                      GdkEvent              *event,
                      JavaProjectProperties *project_properties)
{
  JavaProjectPropertiesPrivate *priv;
  GtkWidget *dialog;
  GtkFileFilter *filter;
  gint response;
  const gchar *folder_path;
  
  priv = JAVA_PROJECT_PROPERTIES_GET_PRIVATE (project_properties);

  dialog = gtk_file_chooser_dialog_new ("Select Ant File", 
                                        NULL,
                                        GTK_FILE_CHOOSER_ACTION_OPEN,
                                        GTK_STOCK_CANCEL,
                                        GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OPEN,
                                        GTK_RESPONSE_OK, 
                                        NULL);

  filter = gtk_file_filter_new ();
  gtk_file_filter_add_pattern (filter, "*.xml");
  gtk_file_chooser_set_filter (GTK_FILE_CHOOSER(dialog), filter);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

  folder_path = codeslayer_project_get_folder_path (priv->project);
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(dialog), folder_path);

  response = gtk_dialog_run (GTK_DIALOG (dialog));
  if (response == GTK_RESPONSE_OK)
    {
      GFile *file;
      char *file_path;
      file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog));
      file_path = g_file_get_path (file);
      gtk_entry_set_text (ant_file_entry, file_path);
      g_free (file_path);
      g_object_unref (file);
    }

  gtk_widget_destroy (GTK_WIDGET (dialog));  
}                        

static void 
build_folder_icon_action (GtkEntry              *build_folder_entry,
                          GtkEntryIconPosition   icon_pos,
                          GdkEvent              *event,
                          JavaProjectProperties *project_properties)
{
  JavaProjectPropertiesPrivate *priv;
  GtkWidget *dialog;
  gint response;
  const gchar *folder_path;
  
  priv = JAVA_PROJECT_PROPERTIES_GET_PRIVATE (project_properties);
  
  dialog = gtk_file_chooser_dialog_new ("Select Build Folder",
                                        NULL,
                                        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                        GTK_STOCK_CANCEL,
                                        GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OPEN,
                                        GTK_RESPONSE_OK, 
                                        NULL);

  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
  
  folder_path = codeslayer_project_get_folder_path (priv->project);
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(dialog), folder_path);

  response = gtk_dialog_run (GTK_DIALOG (dialog));
  if (response == GTK_RESPONSE_OK)
    {
      GFile *file;
      char *file_path;
      file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog));
      file_path = g_file_get_path (file);
      gtk_entry_set_text (build_folder_entry, file_path);
      g_free (file_path);
      g_object_unref (file);
    }

  gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void 
lib_folder_icon_action (GtkEntry              *lib_folder_entry,
                        GtkEntryIconPosition   icon_pos,
                        GdkEvent              *event,
                        JavaProjectProperties *project_properties)
{
  JavaProjectPropertiesPrivate *priv;
  GtkWidget *dialog;
  gint response;
  const gchar *folder_path;
  
  priv = JAVA_PROJECT_PROPERTIES_GET_PRIVATE (project_properties);
  
  dialog = gtk_file_chooser_dialog_new ("Select Lib Folder",
                                        NULL,
                                        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                        GTK_STOCK_CANCEL,
                                        GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OPEN,
                                        GTK_RESPONSE_OK, 
                                        NULL);

  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
  
  folder_path = codeslayer_project_get_folder_path (priv->project);
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(dialog), folder_path);

  response = gtk_dialog_run (GTK_DIALOG (dialog));
  if (response == GTK_RESPONSE_OK)
    {
      GFile *file;
      char *file_path;
      file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog));
      file_path = g_file_get_path (file);
      gtk_entry_set_text (lib_folder_entry, file_path);
      g_free (file_path);
      g_object_unref (file);
    }

  gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void 
source_folder_icon_action (GtkEntry            *source_folder_entry,
                         GtkEntryIconPosition   icon_pos,
                         GdkEvent              *event,
                         JavaProjectProperties *project_properties)
{
  JavaProjectPropertiesPrivate *priv;
  GtkWidget *dialog;
  gint response;
  const gchar *folder_path;
  
  priv = JAVA_PROJECT_PROPERTIES_GET_PRIVATE (project_properties);
  
  dialog = gtk_file_chooser_dialog_new ("Select Source Path",
                                        NULL,
                                        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                        GTK_STOCK_CANCEL,
                                        GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OPEN,
                                        GTK_RESPONSE_OK, 
                                        NULL);

  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
  
  folder_path = codeslayer_project_get_folder_path (priv->project);
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(dialog), folder_path);

  response = gtk_dialog_run (GTK_DIALOG (dialog));
  if (response == GTK_RESPONSE_OK)
    {
      GFile *file;
      char *file_path;
      file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog));
      file_path = g_file_get_path (file);
      gtk_entry_set_text (source_folder_entry, file_path);
      g_free (file_path);
      g_object_unref (file);
    }

  gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void 
test_folder_icon_action (GtkEntry            *test_folder_entry,
                       GtkEntryIconPosition   icon_pos,
                       GdkEvent              *event,
                       JavaProjectProperties *project_properties)
{
  JavaProjectPropertiesPrivate *priv;
  GtkWidget *dialog;
  gint response;
  const gchar *folder_path;
  
  priv = JAVA_PROJECT_PROPERTIES_GET_PRIVATE (project_properties);
  
  dialog = gtk_file_chooser_dialog_new ("Select Test Path",
                                        NULL,
                                        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                        GTK_STOCK_CANCEL,
                                        GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OPEN,
                                        GTK_RESPONSE_OK, 
                                        NULL);

  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
  
  folder_path = codeslayer_project_get_folder_path (priv->project);
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(dialog), folder_path);

  response = gtk_dialog_run (GTK_DIALOG (dialog));
  if (response == GTK_RESPONSE_OK)
    {
      GFile *file;
      char *file_path;
      file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog));
      file_path = g_file_get_path (file);
      gtk_entry_set_text (test_folder_entry, file_path);
      g_free (file_path);
      g_object_unref (file);
    }

  gtk_widget_destroy (GTK_WIDGET (dialog));
}

void 
java_project_properties_opened (JavaProjectProperties *project_properties,
                                JavaConfiguration     *configuration, 
                                CodeSlayerProject     *project)
{
  JavaProjectPropertiesPrivate *priv;

  priv = JAVA_PROJECT_PROPERTIES_GET_PRIVATE (project_properties);  
  priv->project = project;
  
  if (configuration)
    {
      const gchar *ant_file;
      const gchar *build_folder;
      const gchar *lib_folder;
      const gchar *source_folder;
      const gchar *test_folder;
    
      ant_file = java_configuration_get_ant_file (configuration);
      build_folder = java_configuration_get_build_folder (configuration);
      lib_folder = java_configuration_get_lib_folder (configuration);
      source_folder = java_configuration_get_source_folder (configuration);
      test_folder = java_configuration_get_test_folder (configuration);

      gtk_entry_set_text (GTK_ENTRY (priv->ant_file_entry), ant_file);
      gtk_entry_set_text (GTK_ENTRY (priv->build_folder_entry), build_folder);
      gtk_entry_set_text (GTK_ENTRY (priv->lib_folder_entry), lib_folder);
      gtk_entry_set_text (GTK_ENTRY (priv->source_folder_entry), source_folder);
      gtk_entry_set_text (GTK_ENTRY (priv->test_folder_entry), test_folder);
    }
  else
    {
      gtk_entry_set_text (GTK_ENTRY (priv->ant_file_entry), "");
      gtk_entry_set_text (GTK_ENTRY (priv->build_folder_entry), "");
      gtk_entry_set_text (GTK_ENTRY (priv->lib_folder_entry), "");
      gtk_entry_set_text (GTK_ENTRY (priv->source_folder_entry), "");
      gtk_entry_set_text (GTK_ENTRY (priv->test_folder_entry), "");
    }
}

gboolean 
java_project_properties_saved (JavaProjectProperties *project_properties,
                               JavaConfiguration     *configuration, 
                               CodeSlayerProject     *project)
{
  JavaProjectPropertiesPrivate *priv;
  gchar *ant_file;
  gchar *build_folder;
  gchar *lib_folder;
  gchar *source_folder;
  gchar *test_folder;

  priv = JAVA_PROJECT_PROPERTIES_GET_PRIVATE (project_properties);
  
  ant_file = g_strdup (gtk_entry_get_text (GTK_ENTRY (priv->ant_file_entry)));
  build_folder = g_strdup (gtk_entry_get_text (GTK_ENTRY (priv->build_folder_entry)));
  lib_folder = g_strdup (gtk_entry_get_text (GTK_ENTRY (priv->lib_folder_entry)));
  source_folder = g_strdup (gtk_entry_get_text (GTK_ENTRY (priv->source_folder_entry)));
  test_folder = g_strdup (gtk_entry_get_text (GTK_ENTRY (priv->test_folder_entry)));
  
  g_strstrip (ant_file);
  g_strstrip (build_folder);
  g_strstrip (lib_folder);
  g_strstrip (source_folder);
  g_strstrip (test_folder);
  
  if (configuration)
    {
      if (g_strcmp0 (ant_file, java_configuration_get_ant_file (configuration)) == 0 &&
          g_strcmp0 (build_folder, java_configuration_get_build_folder (configuration)) == 0 &&
          g_strcmp0 (lib_folder, java_configuration_get_lib_folder (configuration)) == 0 &&
          g_strcmp0 (source_folder, java_configuration_get_source_folder (configuration)) == 0 &&
          g_strcmp0 (test_folder, java_configuration_get_test_folder (configuration)) == 0)
        {
          g_free (ant_file);
          g_free (build_folder);
          g_free (lib_folder);
          g_free (source_folder);
          g_free (test_folder);
          return FALSE;
        }

      java_configuration_set_ant_file (configuration, ant_file);
      java_configuration_set_build_folder (configuration, build_folder);
      java_configuration_set_lib_folder (configuration, lib_folder);
      java_configuration_set_source_folder (configuration, source_folder);
      java_configuration_set_test_folder (configuration, test_folder);
      g_signal_emit_by_name((gpointer)project_properties, "save-configuration", NULL);
    }
  else if (entry_has_text (priv->ant_file_entry) &&
           entry_has_text (priv->build_folder_entry) &&
           entry_has_text (priv->lib_folder_entry) &&
           entry_has_text (priv->source_folder_entry)
           )
    {
      JavaConfiguration *configuration;
      const gchar *project_key;
      configuration = java_configuration_new ();
      project_key = codeslayer_project_get_key (project);
      java_configuration_set_project_key (configuration, project_key);
      java_configuration_set_ant_file (configuration, ant_file);
      java_configuration_set_build_folder (configuration, build_folder);
      java_configuration_set_lib_folder (configuration, lib_folder);
      java_configuration_set_source_folder (configuration, source_folder);
      java_configuration_set_test_folder (configuration, test_folder);
      g_signal_emit_by_name((gpointer)project_properties, "save-configuration", configuration);
    }
    
  g_free (ant_file);
  g_free (build_folder);
  g_free (lib_folder);
  g_free (source_folder);
  g_free (test_folder);
  return TRUE;
}

static gboolean
entry_has_text (GtkWidget *entry)
{
  return gtk_entry_buffer_get_length (gtk_entry_get_buffer (GTK_ENTRY (entry))) > 0;
}
