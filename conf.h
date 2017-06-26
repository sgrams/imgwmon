/* 
 *  imgwmon 0.1-git
 *  (C) 2016-2017 Stanisław J. Grams <sjg@fmdx.pl>
 * 
 *	conf.h
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

#ifndef IMGWMON_CONF_H_
#define IMGWMON_CONF_H_

void conf_init(const gchar*);
void conf_save(void);
void conf_destroy(void);

gint
conf_read_integer (const gchar*, const gchar*, gint);

gint conf_get_main_id_meteo (void);
void conf_set_main_id_meteo (gint);

gint conf_get_main_id_hydro (void);
void conf_set_main_id_hydro (gint);

#endif
