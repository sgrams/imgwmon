/* 
 *  imgwmon 0.2-git
 *  (C) 2016-2017 Stanisław J. Grams <sjg@fmdx.pl>
 * 
 *	stations.h
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

#ifndef IMGWMON_STATIONS_H_
#define IMGWMON_STATIONS_H_

gint station_get_id       (gchar *name, gint data_type);
gint station_validate_id  (gint id, gint data_type);

#endif
