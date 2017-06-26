/* 
 *  imgwmon 0.1-git
 *  (C) 2016-2017 Stanisław J. Grams <sjg@fmdx.pl>
 * 
 *	stations.c
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

typedef struct station
{
  gint id;
  gchar *name;
  station *next;
} *station_list;

station_list station_get_list (gchar* custom_path, gint data_type)
{
  gchar *directory;
  if (custom_path)

}

gint station_get_id (gchar *name, gint data_type)
{
}

gint station_validate_id (gint id, gint data_type)
{	
}

void station_free_list (station_list list)
{
  g_free(list);
}
