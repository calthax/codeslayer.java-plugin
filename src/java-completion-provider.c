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
#include <gtksourceview/gtksourcecompletion.h>
#include <gtksourceview/gtksourcecompletioninfo.h>
#include <gtksourceview/gtksourcecompletionitem.h>
#include <gtksourceview/gtksourcelanguagemanager.h>
#include "java-completion-provider.h"

static void java_completion_provider_class_init  (JavaCompletionProviderClass      *klass);
static void java_completion_provider_init        (JavaCompletionProvider           *completion_provider);
static void java_completion_provider_finalize    (JavaCompletionProvider           *completion_provider);

static void provider_interface_init              (GtkSourceCompletionProviderIface *iface);

static gchar* provider_get_name                  (GtkSourceCompletionProvider      *provider);
static gint provider_get_priority                (GtkSourceCompletionProvider      *provider);
static gboolean provider_match                   (GtkSourceCompletionProvider      *provider,
                                                  GtkSourceCompletionContext       *context);
static void provider_populate                    (GtkSourceCompletionProvider      *provider,
                                                  GtkSourceCompletionContext       *context);
static gchar* find_start_of_context              (GtkSourceCompletionProvider      *provider,
                                                  GtkSourceCompletionContext       *context);

#define JAVA_COMPLETION_PROVIDER_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JAVA_COMPLETION_PROVIDER_TYPE, JavaCompletionProviderPrivate))

typedef struct _JavaCompletionProviderPrivate JavaCompletionProviderPrivate;

struct _JavaCompletionProviderPrivate
{
  CodeSlayerEditor *editor;
  GList            *proposals;
};

G_DEFINE_TYPE_EXTENDED (JavaCompletionProvider,
                        java_completion_provider,
                        G_TYPE_OBJECT,
                        0,
                        G_IMPLEMENT_INTERFACE (GTK_SOURCE_TYPE_COMPLETION_PROVIDER,
                                               provider_interface_init));

static void
provider_interface_init (GtkSourceCompletionProviderIface *iface)
{
	iface->get_name = provider_get_name;
	iface->populate = provider_populate;
	iface->match = provider_match;
	iface->get_priority = provider_get_priority;
}
     
static void 
java_completion_provider_class_init (JavaCompletionProviderClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) java_completion_provider_finalize;
  g_type_class_add_private (klass, sizeof (JavaCompletionProviderPrivate));
}

static void
java_completion_provider_init (JavaCompletionProvider *completion_provider)
{
  JavaCompletionProviderPrivate *priv;
  
  priv = JAVA_COMPLETION_PROVIDER_GET_PRIVATE (completion_provider);
  priv->proposals = NULL;
  
	priv->proposals = g_list_prepend (priv->proposals,
	                            gtk_source_completion_item_new ("Proposal", "Proposal", NULL, NULL)); 
}

static void
java_completion_provider_finalize (JavaCompletionProvider *completion_provider)
{
  G_OBJECT_CLASS (java_completion_provider_parent_class)->finalize (G_OBJECT (completion_provider));
}

JavaCompletionProvider*
java_completion_provider_new (CodeSlayerEditor *editor)
{
  JavaCompletionProviderPrivate *priv;
  JavaCompletionProvider *completion_provider;

  completion_provider = JAVA_COMPLETION_PROVIDER (g_object_new (java_completion_provider_get_type (), NULL));
  priv = JAVA_COMPLETION_PROVIDER_GET_PRIVATE (completion_provider);
  priv->editor = editor;

  return completion_provider;
}

static gchar*
provider_get_name (GtkSourceCompletionProvider *provider)
{
	return g_strdup ("CodeSlayer");
}

static gint
provider_get_priority (GtkSourceCompletionProvider *provider)
{
	return 0;
}

static gboolean
provider_match (GtkSourceCompletionProvider *provider,
                GtkSourceCompletionContext  *context)
{
  GtkTextIter iter;
  GtkTextIter start;
  gchar *text;
  
  gtk_source_completion_context_get_iter (context, &iter);
  start = iter;
  
  gtk_text_iter_backward_char (&start);
  
  text = gtk_text_iter_get_text (&start, &iter);
  
  if (g_strcmp0 (text, ".") == 0)
    {
      g_free (text);
      return TRUE;
    }

  g_free (text);    
	return FALSE;
}

static void
provider_populate (GtkSourceCompletionProvider *provider,
                   GtkSourceCompletionContext  *context)
{
  JavaCompletionProviderPrivate *priv;
  gchar *variable;

  priv = JAVA_COMPLETION_PROVIDER_GET_PRIVATE (provider);

  variable = find_start_of_context (provider, context);
  
  if (variable != NULL)
    {
      g_print ("variable->%s\n", variable);
      g_free (variable);
    }
  
	gtk_source_completion_context_add_proposals (context, 
	                                             provider, 
	                                             priv->proposals, TRUE);
}

static gchar*
find_start_of_context (GtkSourceCompletionProvider *provider,
                       GtkSourceCompletionContext  *context)
{
  gchar *result;
  GString *string;
  gchar *text;
  gchar *variable;
  GtkTextIter iter;
  GtkTextIter start;
  
  string = g_string_new ("");

  gtk_source_completion_context_get_iter (context, &iter);

  start = iter;
  gtk_text_iter_backward_chars (&start, 100);
  
  text = gtk_text_iter_get_text (&start, &iter);
  variable = text;
  variable = g_strreverse (variable);  
  
  for (; *variable != '\0'; ++variable)
    {
      if (*variable == '=')
        break;
        
      string = g_string_append_c (string, *variable);
    }
  
  g_free (text);
  
  result = g_string_free (string, FALSE);
  result = g_strreverse (result);
  g_strstrip(result);
    
  return result;
}
