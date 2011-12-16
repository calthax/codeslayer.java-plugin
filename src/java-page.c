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

#include "java-page.h"

static void java_page_class_init (gpointer page);

GType
java_page_get_type ()
{
  static GType type = 0;
  
  if (!type)
    {
      type = g_type_register_static_simple (G_TYPE_INTERFACE, "JavaPage", 
                                            sizeof (JavaPageInterface), 
                                            (GClassInitFunc) java_page_class_init,
                                            0, NULL, 0);
                                            
      g_type_interface_add_prerequisite (type, GTK_TYPE_WIDGET);
    }
  
  return type;  
}

static void
java_page_class_init (gpointer page)
{
}

JavaPageType
java_page_get_page_type (JavaPage *page)
{
  return JAVA_PAGE_GET_INTERFACE (page)->get_page_type (page);
}

JavaConfiguration*  
java_page_get_configuration (JavaPage *page)
{
  return JAVA_PAGE_GET_INTERFACE (page)->get_configuration (page);
}

void
java_page_set_configuration (JavaPage          *page, 
                             JavaConfiguration *configuration)
{
  JAVA_PAGE_GET_INTERFACE (page)->set_configuration (page, configuration);
}

CodeSlayerDocument*  
java_page_get_document (JavaPage *page)
{
  return JAVA_PAGE_GET_INTERFACE (page)->get_document (page);
}

void
java_page_set_document (JavaPage           *page, 
                        CodeSlayerDocument *document)
{
  JAVA_PAGE_GET_INTERFACE (page)->set_document (page, document);
}
