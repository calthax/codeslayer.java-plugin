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

#ifndef __JAVA_PAGE_H__
#define	__JAVA_PAGE_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>
#include "java-configuration.h"

G_BEGIN_DECLS

typedef enum
{
  JAVA_PAGE_TYPE_COMPILER,
  JAVA_PAGE_TYPE_TESTER, 
  JAVA_PAGE_TYPE_DEBUGGER, 
} JavaPageType;

#define JAVA_PAGE_TYPE                (java_page_get_type ())
#define JAVA_PAGE(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_PAGE_TYPE, JavaPage))
#define IS_JAVA_PAGE(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_PAGE_TYPE))
#define JAVA_PAGE_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE ((inst), JAVA_PAGE_TYPE, JavaPageInterface))

typedef struct _JavaPage JavaPage;
typedef struct _JavaPageInterface JavaPageInterface;

struct _JavaPageInterface
{
  GTypeInterface parent;
  
  JavaPageType (*get_page_type) (JavaPage *page);
  JavaConfiguration* (*get_configuration) (JavaPage *page);
  void (*set_configuration) (JavaPage *page, JavaConfiguration *configuration);
  CodeSlayerDocument* (*get_document) (JavaPage *page);
  void (*set_document) (JavaPage *page, CodeSlayerDocument *document);
};

GType java_page_get_type (void);

JavaPageType        java_page_get_page_type      (JavaPage           *page);

JavaConfiguration*  java_page_get_configuration  (JavaPage           *page);
void                java_page_set_configuration  (JavaPage           *page, 
                                                  JavaConfiguration  *configuration);
CodeSlayerDocument* java_page_get_document       (JavaPage           *page);
void                java_page_set_document       (JavaPage           *page, 
                                                  CodeSlayerDocument *document);

G_END_DECLS

#endif /* __JAVA_PAGE_H__ */
