/* 
 *  imgwmon 0.1-git
 *  (C) 2016-2017 Stanisław J. Grams <sjg@fmdx.pl>
 * 
 *	data.c
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

#include <glib/gstdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <yajl/yajl_tree.h>
#include <errno.h>

#include "data.h"

#define DATA_METEO_URL	"http://monitor.pogodynka.pl/api/station/meteo/?id="
#define DATA_HYDRO_URL	"http://monitor.pogodynka.pl/api/station/hydro/?id="

void
data_get (memory_object *mem, gshort target_object, gint target_id)
{
	gchar			*url;
	CURL			*curl_handle;
	CURLcode		res;
	
	mem->size = 0;
	
	if (!target_id)
	{
		g_free (mem);
		fprintf(stderr, "Error, ID not specified!\n");
		exit(EXIT_FAILURE);
	}
	
	/* Both meteo and hydro types use 9-digit id number */
	if (!target_object)
	{
		url = g_malloc ((strlen(DATA_METEO_URL)+10)*sizeof(gchar));
		url[0]='\0';
		url = g_strconcat (url, DATA_METEO_URL, g_strdup_printf("%i", target_id), NULL);
	}
	else
	{
		url = g_malloc ((strlen(DATA_HYDRO_URL)+10)*sizeof(gchar));
		url[0]='\0';
		url = g_strconcat (url, DATA_HYDRO_URL, g_strdup_printf("%i", target_id), NULL);
	}
	
	
	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();

	curl_easy_setopt(curl_handle, CURLOPT_URL, url);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "imgwmon-agent/0.1");
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, data_write_callback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)mem);
	res = curl_easy_perform(curl_handle);
	
	if (res != CURLE_OK)
	{
			g_fprintf(stderr, "Failed to fetch data: %s\n", curl_easy_strerror(res));
			exit(EXIT_FAILURE);
	}
	curl_easy_cleanup(curl_handle);
	curl_global_cleanup();
	
	g_free(url);
}

void
data_process (memory_object *mem, gshort data_index,
			  gshort data_type, gchar *target_time)
{
	
/*		data_type list (it's not data_index!)
		"precip.current",	0
		"precip.hourly",	1
		"precip.daily",		2
		"precip.10min",		3
		
		"temp.auto",		4
		"temp.obs",			5
		
		"temp.auto.min",	6
		"temp.auto.max",	7
		"temp.obs.min",		8
		"temp.obs.max",		9
		
		"wind.dir.tel",		10
		"wind.dir.obs",		11
		"wind.vel.tel",		12
		"wind.vel.obs",		13
		"wind.vel.max",		14
		
		"water.state.current",	15
		"water.state.auto",		16
		"water.state.obs",		17
		"water.discharge",		18
		"water.temp.auto",		19
		"water.temp.obs",		20
*/


	gchar errbuf[1024];

	gint	i, array_length;
	gfloat	target_result;

	yajl_val main_node;
	yajl_val object_data_type;
	yajl_val arrays_data_type;
	yajl_val keys_data_type;
	yajl_val values_data_type;

	errbuf[0] = '\0';

	main_node = yajl_tree_parse((const gchar *)mem->memory, errbuf, sizeof(errbuf));
	if (!main_node)
	{
		g_free(mem);
		exit(EXIT_FAILURE);
	}
	if (!(strcmp(target_type_of_data, "currentPrecipRecords") == 0 ||
			strcmp(target_type_of_data, "currentWaterStateRecords") == 0 ||
			strcmp(target_type_of_data, "maxTemperatureAutoRecords") == 0 ||
			strcmp(target_type_of_data, "minTemperatureAutoRecords") == 0 ||
			strcmp(target_type_of_data, "maxTemperatureObsRecords") == 0 ||
			strcmp(target_type_of_data, "minTemperatureObsRecords") == 0))
	{
		arrays_target_type_of_data = yajl_tree_get(main_node, main_path, yajl_t_array);
		if (arrays_target_type_of_data)
		{
			array_length = arrays_target_type_of_data->u.array.len;
			if (array_length)
			{
				for (i=0; i<array_length; i++)
				{
					keys_target_type_of_data = arrays_target_type_of_data->u.array.values[i];

					/*  waterStateRecords and waterStateObserverRecords have different fields in the JSON */
					if (strcmp(target_type_of_data, "waterStateRecords") == 0 ||
							strcmp(target_type_of_data, "waterStateObserverRecords") == 0)
						values_target_type_of_data = keys_target_type_of_data->u.object.values[1];
					else
						values_target_type_of_data = keys_target_type_of_data->u.object.values[0];


					if(strcmp(target_time, (gchar *)values_target_type_of_data->u.object.keys) == 0)
					{
						if (strcmp(target_type_of_data, "waterStateRecords") == 0 ||
								strcmp(target_type_of_data, "waterStateObserverRecords") == 0)
							values_target_type_of_data = keys_target_type_of_data->u.object.values[2];
						else
						values_target_type_of_data = keys_target_type_of_data->u.object.values[1];

						fprintf(stdout, "%.1lf ", values_target_type_of_data->u.number.d);

						if (strcmp(target_type_of_data, "hourlyPrecipRecords") == 0 ||
								strcmp(target_type_of_data, "dailyPrecipRecords") == 0 ||
								strcmp(target_type_of_data, "tenMinutesPrecipRecords") == 0)
							fprintf(stdout, "mm\n");
						else if (strcmp(target_type_of_data, "temperatureAutoRecords") == 0 ||
								strcmp(target_type_of_data, "temperatureObsRecords") == 0 ||
								strcmp(target_type_of_data, "waterTemperatureAutoRecords") == 0 ||
								strcmp(target_type_of_data, "waterTemperatureObsRecords") == 0)
							fprintf(stdout, "°C\n");
						else if (strcmp(target_type_of_data, "windDirectionTelRecords") == 0 ||
								strcmp(target_type_of_data, "windDirectionObsRecords") == 0)
							fprintf(stdout, "°\n");
					else if (strcmp(target_type_of_data, "windVelocityTelRecords") == 0 ||
								strcmp(target_type_of_data, "windVelocityObsRecords") == 0 ||
								strcmp(target_type_of_data, "windMaxVelocityRecords") == 0)
							fprintf(stdout, "m/s\n");
					else if (strcmp(target_type_of_data, "waterStateRecords") == 0 ||
								strcmp(target_type_of_data, "waterStateObserverRecords") == 0)
							fprintf(stdout, "cm\n");
					else if (strcmp(target_type_of_data, "dischargeRecords") == 0)
							fprintf(stdout, "m3/s\n");
					}
				}
			}
		}	
	}

	else if (strcmp(target_type_of_data, "currentPrecipRecords") == 0)
	{
		object_target_type_of_data = yajl_tree_get(main_node, current_precip_path, yajl_t_number);
		if (object_target_type_of_data)
		{
			if(object_target_type_of_data->u.number.d != 0)
				fprintf(stdout, "state: precipitation\n");
			else
				fprintf(stdout, "state: no precipitation\n");
		}
	}

	else if (strcmp(target_type_of_data, "currentWaterStateRecords") == 0)
	{
		object_target_type_of_data = yajl_tree_get(main_node, current_river_path, yajl_t_string);
		if (object_target_type_of_data)
			fprintf(stdout, "river: %s, ", (gchar *)object_target_type_of_data->u.object.keys);
		object_target_type_of_data = yajl_tree_get(main_node, current_state_path, yajl_t_string);
		if (object_target_type_of_data)
			fprintf(stdout, "state: %s, ", (gchar *)object_target_type_of_data->u.object.keys);
		object_target_type_of_data = yajl_tree_get(main_node, current_trend_path, yajl_t_string);
		if (object_target_type_of_data)
			fprintf(stdout, "trend: %s\n", (gchar *)object_target_type_of_data->u.object.keys);
	}
	
	else if (strcmp(target_type_of_data, "maxTemperatureAutoRecords") == 0 ||
				strcmp(target_type_of_data, "maxTemperatureObsRecords") == 0 ||
				strcmp(target_type_of_data, "minTemperatureAutoRecords") == 0 ||
				strcmp(target_type_of_data, "minTemperatureObsRecords") == 0)
	{
		if (strcmp(target_type_of_data, "maxTemperatureAutoRecords") == 0 ||
				strcmp(target_type_of_data, "minTemperatureAutoRecords") == 0)
			arrays_target_type_of_data = yajl_tree_get(main_node, temperatureAutoPath, yajl_t_array);
		else
			arrays_target_type_of_data = yajl_tree_get(main_node, temperatureObsPath, yajl_t_array);
			
		if (arrays_target_type_of_data)
		{
			array_length = arrays_target_type_of_data->u.array.len;
			if (array_length)
			{
				keys_target_type_of_data = arrays_target_type_of_data->u.array.values[0];
				values_target_type_of_data = keys_target_type_of_data->u.object.values[1];
				target_result = values_target_type_of_data->u.number.d;

				values_target_type_of_data = keys_target_type_of_data->u.object.values[0];
				target_time = strndup((gchar*)values_target_type_of_data->u.object.keys, 21);

				for (i=0; i<array_length; i++)
				{
					keys_target_type_of_data = arrays_target_type_of_data->u.array.values[i];
					values_target_type_of_data = keys_target_type_of_data->u.object.values[1];

					if (target_result >= values_target_type_of_data->u.number.d &&
							(strcmp(target_type_of_data, "minTemperatureAutoRecords") == 0 ||
							strcmp(target_type_of_data, "minTemperatureObsRecords") == 0))
					{
						target_result = values_target_type_of_data->u.number.d;
						values_target_type_of_data = keys_target_type_of_data->u.object.values[0];
						target_time = strndup((gchar*)values_target_type_of_data->u.object.keys, 21);
					}
								
					else if (target_result <= values_target_type_of_data->u.number.d &&
							(strcmp(target_type_of_data, "maxTemperatureAutoRecords") == 0 ||
							strcmp(target_type_of_data, "maxTemperatureObsRecords") == 0))
					{
						target_result = values_target_type_of_data->u.number.d;
						values_target_type_of_data = keys_target_type_of_data->u.object.values[0];
						target_time = strndup((gchar*)values_target_type_of_data->u.object.keys, 21);
					}
				}
			fprintf(stdout, "%.1f °C, %s\n", target_result, target_time);
			}
		}
	}
	yajl_tree_free(main_node);

}

gsize
data_write_callback (void *contents, size_t size, size_t nmemb, void *userp)
{
	gsize realsize = size * nmemb;
	memory_object *mem = userp;

	mem->memory = g_realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory == NULL)
	{
		g_fprintf(stderr, "Not enough memory, returned %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;
	
	return realsize;
}
