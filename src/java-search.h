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

#ifndef __JAVA_SEARCH_H__
#define	__JAVA_SEARCH_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>

G_BEGIN_DECLS

#define JAVA_SEARCH_TYPE            (java_search_get_type ())
#define JAVA_SEARCH(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), JAVA_SEARCH_TYPE, JavaSearch))
#define JAVA_SEARCH_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), JAVA_SEARCH_TYPE, JavaSearchClass))
#define IS_JAVA_SEARCH(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JAVA_SEARCH_TYPE))
#define IS_JAVA_SEARCH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), JAVA_SEARCH_TYPE))

typedef struct _JavaSearch JavaSearch;
typedef struct _JavaSearchClass JavaSearchClass;

struct _JavaSearch
{
  GObject parent_instance;
};

struct _JavaSearchClass
{
  GObjectClass parent_class;
};

GType java_search_get_type (void) G_GNUC_CONST;
     
JavaSearch*  java_search_new  (CodeSlayer *codeslayer,                                          
                               GtkWidget  *menu);
                                     
G_END_DECLS

#endif /* __JAVA_SEARCH_H__ */
