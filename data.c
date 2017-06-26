/* 
 *  imgwmon 0.2-git
 *  (C) 2016-2017 Stanisław J. Grams <sjg@fmdx.pl>
 * 
 *  data.c
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

#define DATA_METEO_URL      "http://monitor.pogodynka.pl/api/station/meteo/?id="
#define DATA_HYDRO_URL      "http://monitor.pogodynka.pl/api/station/hydro/?id="

#define PATH_PRECIP_CUR     (const gchar *[]){"status", "precip", "value", NULL}
#define PATH_PRECIP_10MIN   (const gchar *[]){"tenMinutesPrecipRecords",   NULL}
#define PATH_PRECIP_HOURLY  (const gchar *[]){"hourlyPrecipRecords",       NULL}
#define PATH_PRECIP_DAILY   (const gchar *[]){"dailyPrecipRecords",        NULL}

#define PATH_TEMP_AUTO      (const gchar *[]){"temperatureAutoRecords",   NULL}
#define PATH_TEMP_OBS       (const gchar *[]){"temperatureObsRecords",    NULL}

#define PATH_WIND_DIR_TEL   (const gchar *[]){"windDirectionTelRecords",  NULL}
#define PATH_WIND_DIR_OBS   (const gchar *[]){"windDirectionObsRecords",  NULL}
#define PATH_WIND_VEL_TEL   (const gchar *[]){"windVelocityTelRecords",   NULL}
#define PATH_WIND_VEL_OBS   (const gchar *[]){"windVelocityObsRecords",   NULL}
#define PATH_WIND_VEL_MAX   (const gchar *[]){"windMaxVelocityRecords",   NULL}

#define PATH_WATER_STATE_AUTO (const gchar *[]){"waterStateRecords",            NULL}
#define PATH_WATER_STATE_OBS  (const gchar *[]){"waterStateObserverRecords",    NULL}
#define PATH_WATER_DISCHARGE  (const gchar *[]){"dischargeRecords",             NULL}
#define PATH_WATER_TEMP_AUTO  (const gchar *[]){"waterTemperatureAutoRecords",  NULL}
#define PATH_WATER_TEMP_OBS   (const gchar *[]){"waterTemperatureObsRecords",   NULL}

#define PATH_WATER_INFO_RIVER (const gchar *[]){"status", "river", NULL}
#define PATH_WATER_INFO_STATE (const gchar *[]){"state",           NULL}
#define PATH_WATER_INFO_TREND (const gchar *[]){"trend",           NULL}

void
data_get (memory_object *mem, gshort target_object, gint target_id)
{
  gchar     *url;
  CURL      *curl_handle;
  CURLcode    res;
  
  mem->size = 0;
  
  if (!target_id)
  {
    g_free (mem);
    fprintf(stderr, "Error, ID not specified!\n");
    exit(EXIT_FAILURE);
  }
    
  /* Both meteo and hydro types use 9-digit id number */
  if (!target_object)
    url = g_strdup_printf("%s%i", DATA_METEO_URL, target_id);
  else
    url = g_strdup_printf("%s%i", DATA_HYDRO_URL, target_id);

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

void data_process (memory_object *mem, gshort data_type, gchar *target_time)
{
  
  /*
    data_type list (it's not data_index!)
    
    "precip.cur",       0
    "precip.10min",     1
    "precip.hourly",    2
    "precip.daily",     3
    
    "temp.auto",        4
    "temp.obs",         5
    
    "temp.auto.min",    6
    "temp.auto.max",    7
    "temp.obs.min",     8
    "temp.obs.max",     9
    
    "wind.dir.tel",     10
    "wind.dir.obs",     11

    "wind.vel.tel",     12
    "wind.vel.obs",     13
    "wind.vel.max",     14
    
    "wind.vel.tel.max"  15
    "wind.vel.obs.max"  16

    "water.info",       17
  
    "water.state.auto", 18
    "water.state.obs",  19
    "water.discharge",  20

    "water.temp.auto",  21
    "water.temp.obs",   22

  */
  
  gchar errbuf[1024];

  gint i, array_length;
  
  // for min/max temp/wind vel. search
  gfloat search_result;
  gchar  *search_result_time;
  
  yajl_val main_node;
  yajl_val object_data_type;
  yajl_val arrays_data_type;
  yajl_val keys_data_type;
  yajl_val values_data_type;

  errbuf[0] = '\0';

  main_node = yajl_tree_parse(mem->memory, errbuf, sizeof(errbuf));
  
  if (!main_node)
  {
    g_free(mem);
    exit(EXIT_FAILURE);
  }
  
  if (data_type == 0)
  {
      object_data_type = yajl_tree_get (main_node, PATH_PRECIP_CUR, yajl_t_number);
      if (object_data_type)
      {
        if(object_data_type->u.number.d != 0)
          fprintf(stdout, "precipitation\n");
        else
          fprintf(stdout, "no precipitation\n");
      }
  }
    
  if ((data_type >= 1 && data_type <= 5)    ||
       (data_type >= 10 && data_type <= 14) ||
       (data_type >= 18 && data_type <= 22))
  {
    if (data_type == 1)
      arrays_data_type = yajl_tree_get (main_node, PATH_PRECIP_10MIN, yajl_t_array);
    if (data_type == 2)
      arrays_data_type = yajl_tree_get (main_node, PATH_PRECIP_HOURLY, yajl_t_array);
    if (data_type == 3)
      arrays_data_type = yajl_tree_get (main_node, PATH_PRECIP_DAILY, yajl_t_array);
      
    if (data_type == 4)
      arrays_data_type = yajl_tree_get (main_node, PATH_TEMP_AUTO, yajl_t_array);
    if (data_type == 5)
      arrays_data_type = yajl_tree_get (main_node, PATH_TEMP_OBS, yajl_t_array);
    
    if (data_type == 10)
      arrays_data_type = yajl_tree_get (main_node, PATH_WIND_DIR_TEL, yajl_t_array);
    if (data_type == 11)
      arrays_data_type = yajl_tree_get (main_node, PATH_WIND_DIR_OBS, yajl_t_array);
    if (data_type == 12)
      arrays_data_type = yajl_tree_get (main_node, PATH_WIND_VEL_TEL, yajl_t_array);
    if (data_type == 13)
      arrays_data_type = yajl_tree_get (main_node, PATH_WIND_VEL_OBS, yajl_t_array);
    if (data_type == 14)
      arrays_data_type = yajl_tree_get (main_node, PATH_WIND_VEL_MAX, yajl_t_array);
      
    if (data_type == 18)
      arrays_data_type = yajl_tree_get (main_node, PATH_WATER_STATE_AUTO, yajl_t_array);
    if (data_type == 19)
      arrays_data_type = yajl_tree_get (main_node, PATH_WATER_STATE_OBS, yajl_t_array);
    if (data_type == 20)
      arrays_data_type = yajl_tree_get (main_node, PATH_WATER_DISCHARGE, yajl_t_array);
    if (data_type == 21)
      arrays_data_type = yajl_tree_get (main_node, PATH_WATER_TEMP_AUTO, yajl_t_array);
    if (data_type == 22)
      arrays_data_type = yajl_tree_get (main_node, PATH_WATER_TEMP_OBS, yajl_t_array);
      
    if (!arrays_data_type)
    {
      fprintf(stderr, "Failed to parse data!\nArray is empty!\n");
    }
    else
    {
      array_length = arrays_data_type -> u.array.len;
      // go through all the array
      for (i=0; i < array_length; i++)
      {
        keys_data_type = arrays_data_type -> u.array.values[i];
        
        // waterState has different fields than in METEO json
        if (data_type == 18 || data_type == 19)
          values_data_type = keys_data_type -> u.object.values[1];
        else
          values_data_type = keys_data_type -> u.object.values[0];
        
        // match the date
        if (g_strcmp0(target_time, (gchar *)values_data_type -> u.object.keys) == 0)
        {
          values_data_type = keys_data_type -> u.object.values[1];
          // print main value
          fprintf(stdout, "%.1lf ", values_data_type -> u.number.d);
          
          // print corresponding measure
          if (data_type >= 0  && data_type <= 3)
            fprintf(stdout, "mm\n");
          if (data_type == 4  || data_type == 5 || data_type == 21 || data_type == 22)
            fprintf(stdout, "°C\n");
          if (data_type == 10 || data_type == 11)
            fprintf(stdout, "°\n");
          if (data_type >= 12 && data_type <= 14)
            fprintf(stdout, "m/s\n");
          if (data_type == 18 || data_type == 19)
            fprintf(stdout, "cm\n");
          if (data_type == 20)
            fprintf(stdout, "m³/s\n");
        }
      }
    }
  }

  // data_type = {temp.auto.min, temp.auto.max, temp.obs.min, temp.obs.max
  //              wind.vel.tel.max, wind.vel.obs.max}
  if ((data_type >= 6 && data_type <= 9) || (data_type == 15 || data_type == 16))
  {
    // data_type = temp.auto.*
    if (data_type == 6 || data_type == 7)
      arrays_data_type = yajl_tree_get (main_node, PATH_TEMP_AUTO, yajl_t_array);
    // data_type = temp.obs.*
    if (data_type == 8 || data_type == 9)
      arrays_data_type = yajl_tree_get (main_node, PATH_TEMP_OBS, yajl_t_array);
    // data_type = wind.vel.tel.*
    if (data_type == 15)
      arrays_data_type = yajl_tree_get (main_node, PATH_WIND_VEL_TEL, yajl_t_array);
    // data_type = wind.vel.obs.*
    if (data_type == 16)
      arrays_data_type = yajl_tree_get (main_node, PATH_WIND_VEL_OBS, yajl_t_array);
      
    if (!arrays_data_type)
    {
      fprintf(stderr, "Failed to parse data!\nArray is empty!\n");
    }
    else
    {
      array_length = arrays_data_type -> u.array.len;
      if (array_length)
      {
        keys_data_type    = arrays_data_type->u.array.values[0];
        values_data_type  = keys_data_type->u.object.values[1];
        search_result     = values_data_type->u.number.d;

        values_data_type    = keys_data_type->u.object.values[0];
        search_result_time   = g_strndup((gchar*)values_data_type->u.object.keys, 21);
        
        for (i=0; i < array_length; i++)
        {
          keys_data_type    = arrays_data_type->u.array.values[i];
          values_data_type  = keys_data_type->u.object.values[1];
          
          // if *.min (temp)
          if (search_result >= values_data_type->u.number.d &&
            (data_type == 6 || data_type == 8))
          {
            search_result   = values_data_type->u.number.d;
            values_data_type  = keys_data_type->u.object.values[0];
            g_free (search_result_time);
            search_result_time     = g_strndup((gchar*)values_data_type->u.object.keys, 21);
          }
          
          // if *.max (temp+wind)
          if (search_result <= values_data_type->u.number.d &&
            (data_type == 7 || data_type == 9 || data_type == 15 || data_type == 16))
          {
            search_result   = values_data_type->u.number.d;
            values_data_type  = keys_data_type->u.object.values[0];
            g_free (search_result_time);
            search_result_time     = g_strndup((gchar*)values_data_type->u.object.keys, 21);
          }
        }
      if (data_type >= 6 && data_type <= 9)
        fprintf(stdout, "%.1f °C, %s\n", search_result, search_result_time);
      if (data_type == 15 || data_type == 16)
        fprintf(stdout, "%.1f m/s, %s\n", search_result, search_result_time);

      g_free(search_result_time);
      }
    }
  }
  
  // data_type = water.info
  if (data_type == 17)
  {
    object_data_type = yajl_tree_get(main_node, PATH_WATER_INFO_RIVER, yajl_t_string);
    if (object_data_type)
      fprintf(stdout, "river: %s, ", (gchar *) object_data_type -> u.object.keys);
    object_data_type = yajl_tree_get(main_node, PATH_WATER_INFO_STATE, yajl_t_string);
    if (object_data_type)
      fprintf(stdout, "state: %s, ", (gchar *) object_data_type -> u.object.keys);
    object_data_type = yajl_tree_get(main_node, PATH_WATER_INFO_TREND, yajl_t_string);
    if (object_data_type)
      fprintf(stdout, "trend: %s\n", (gchar *) object_data_type -> u.object.keys);
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
