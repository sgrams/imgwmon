/* 
 *  imgwmon 0.1-git
 *  (C) 2016-2017 Stanisław J. Grams <sjg@fmdx.pl>
 * 
 *	data.h
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 3
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

#ifndef IMGWMON_DATA_H_
#define IMGWMON_DATA_H_

typedef struct
{
	gchar *memory;
	gsize size;
} memory_object;

void
data_get (memory_object *mem, gshort target_object, gint target_id);
void
data_process (memory_object *mem, gshort data_index, gshort data_type, gchar *target_time)
gsize
data_write_callback (void *contents, gsize size, gsize nmemb, void *userp);

#endif
