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

#ifndef __JAVA_INDEXER_UTILS_H__
#define	__JAVA_INDEXER_UTILS_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>
#include "java-indexer.h"

G_BEGIN_DECLS

GList*  java_indexer_utils_completion_indexes  (JavaIndexer      *indexer, 
                                                CodeSlayerEditor *editor,
                                                GtkTextIter       iter);

G_END_DECLS

#endif /* __JAVA_INDEXER_UTILS_H__ */
