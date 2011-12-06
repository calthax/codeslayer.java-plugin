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

#include "autotools-notebook-tab.h"

static void autotools_notebook_tab_class_init    (AutotoolsNotebookTabClass *klass);
static void autotools_notebook_tab_init          (AutotoolsNotebookTab      *notebook_tab);
static void autotools_notebook_tab_finalize      (AutotoolsNotebookTab      *notebook_tab);

static void close_action                         (AutotoolsNotebookTab      *notebook_tab);

#define AUTOTOOLS_NOTEBOOK_TAB_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), AUTOTOOLS_NOTEBOOK_TAB_TYPE, AutotoolsNotebookTabPrivate))

typedef struct _AutotoolsNotebookTabPrivate AutotoolsNotebookTabPrivate;

struct _AutotoolsNotebookTabPrivate
{
  GtkWidget *notebook_page;
  GtkWidget *notebook;
};

enum
{
  CLOSE,
  LAST_SIGNAL
};

static guint autotools_notebook_tab_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (AutotoolsNotebookTab, autotools_notebook_tab, GTK_TYPE_HBOX)
     
static void
autotools_notebook_tab_class_init (AutotoolsNotebookTabClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  autotools_notebook_tab_signals[CLOSE] =
    g_signal_new ("close", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (AutotoolsNotebookTabClass, close),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  gobject_class->finalize = (GObjectFinalizeFunc) autotools_notebook_tab_finalize;

  g_type_class_add_private (klass, sizeof (AutotoolsNotebookTabPrivate));
}

static void
autotools_notebook_tab_init (AutotoolsNotebookTab *notebook_tab)
{
  gtk_box_set_homogeneous (GTK_BOX (notebook_tab), FALSE);
  gtk_box_set_spacing (GTK_BOX (notebook_tab), 0);
}

static void
autotools_notebook_tab_finalize (AutotoolsNotebookTab *notebook_tab)
{
  G_OBJECT_CLASS (autotools_notebook_tab_parent_class)->finalize ( G_OBJECT (notebook_tab));
}

GtkWidget*
autotools_notebook_tab_new (GtkWidget   *notebook, 
                            const gchar *label)
{
  AutotoolsNotebookTabPrivate *priv;
  GtkWidget *notebook_tab;
  GtkWidget *text;
  GtkWidget *button;
  GtkWidget *image;
  
  notebook_tab = g_object_new (autotools_notebook_tab_get_type (), NULL);
  priv = AUTOTOOLS_NOTEBOOK_TAB_GET_PRIVATE (notebook_tab);
  priv->notebook = notebook;

  text = gtk_label_new (label);

  button = gtk_button_new ();

  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
  image = gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (button), image);

  gtk_rc_parse_string ("style \"my-button-style\"\n"
                       "{\n"
                       "  GtkWidget::focus-padding = 0\n"
                       "  GtkWidget::focus-line-width = 0\n"
                       "  xthickness = 0\n"
                       "  ythickness = 0\n"
                       "}\n"
                       "widget \"*.my-close-button\" style \"my-button-style\"");
  gtk_widget_set_name (button, "my-close-button");

  g_signal_connect_swapped (G_OBJECT (button), "clicked",
                            G_CALLBACK (close_action), notebook_tab);

  gtk_box_pack_start (GTK_BOX (notebook_tab), text, TRUE, TRUE, 1);

  gtk_box_pack_start (GTK_BOX (notebook_tab), button, FALSE, FALSE, 1);

  return notebook_tab;
}

static void
close_action (AutotoolsNotebookTab *notebook_tab)
{
  g_signal_emit_by_name ((gpointer) notebook_tab, "close");
}

GtkWidget*
autotools_notebook_tab_get_notebook_page (AutotoolsNotebookTab *notebook_tab)
{
  return AUTOTOOLS_NOTEBOOK_TAB_GET_PRIVATE (notebook_tab)->notebook_page;
}

void
autotools_notebook_tab_set_notebook_page (AutotoolsNotebookTab *notebook_tab,
                                          GtkWidget            *notebook_page)
{
  AutotoolsNotebookTabPrivate *priv;
  priv = AUTOTOOLS_NOTEBOOK_TAB_GET_PRIVATE (notebook_tab);
  priv->notebook_page = notebook_page;
}
