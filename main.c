/* 
 *  imgwmon 0.1-git
 *  (C) 2016-2017 Stanisław J. Grams <sjg@fmdx.pl>
 * 
 *	main.c
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

#include <glib.h>

#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <time.h>

#include <errno.h>

#include "conf.h"
#include "data.h"

void	info_print (void);

int main (gint argc, gchar **argv)
{
	static	gint verbose_flag = 0;
	static struct option long_options[] =
	{
		{"help", no_argument, 0, 'h'},
		{"set", no_argument, 0, 's'},
		{"verbose", no_argument, 0, 'v'},
		{"date", required_argument, 0, 'd'},
		{"id", required_argument, 0, 'i'},
		{"type", required_argument, 0, 't'},
		{0, 0, 0, 0},
	};
	
	static char *data_types[] =
	{
		"precip.cur",
		"precip.10min",
		"precip.hourly",
		"precip.daily",
		
		"temp.auto",
		"temp.obs",
		
		"temp.auto.min",
		"temp.auto.max",
		"temp.obs.min",
		"temp.obs.max",
		
		"wind.dir.tel",
		"wind.dir.obs",
		"wind.vel.tel",
		"wind.vel.obs",
		"wind.vel.max",
		
		"wind.vel.tel.min",
		"wind.vel.tel.max",
		"wind.vel.obs.min",
		"wind.vel.obs.max",
		
		"water.info",
	
		"water.state.auto",
		"water.state.obs",
		"water.discharge",
	
		"water.temp.auto",
		"water.temp.obs",
		
		NULL
	};
	
	memory_object *mem = g_malloc(1*sizeof(memory_object));
	struct tm *real_time_struct;
	time_t temp_time = time(NULL);
	
	gchar time_target[21];
	
	gshort data_type;
	gshort data_index;
	gshort data_index_target;
	gshort c, i, t;
	
	gint id;
	gint id_target;
	gint id_meteo;
	gint id_hydro;
	
	gint option_index;

	conf_init (NULL);
	
	id			= 0;
	id_target	= 0;
	id_meteo	= conf_get_main_id_meteo();
	id_hydro	= conf_get_main_id_hydro();
	
	data_index			= 0;
	data_index_target	= 0;
	data_type			= 0;
	
	option_index		= 0;
	
	real_time_struct = gmtime(&temp_time);
	strftime(time_target, 21, "%Y-%m-%dT%H:00:00Z", real_time_struct);
	
	mem->memory = g_malloc(1);
	mem->size = 0;
	

	while ((c = getopt_long (argc, argv, "hsvd:i:t:", long_options, &option_index)) != -1)
	{
		switch (c)
		{
			case 's':
				//conf_set_main_id_meteo(id_meteo);
				//conf_set_main_id_hydro(id_hydro);

				//conf_save();
				//return EXIT_SUCCESS;
				break;
				
			case 'v':
				verbose_flag = 1;
				break;
				
			case 'h':
				info_print();
				g_free (mem->memory);
				g_free (mem);
				return EXIT_SUCCESS;
				
			case 'd':
				if (optind == argc)
				{
					printf("Date is required to be specified before data type!\n");
					g_free (mem->memory);
					g_free (mem);
					return EXIT_FAILURE;
				}
				
				if (!atoi(optarg) == 0)
				{
					if (strlen(optarg) == 5)
					{
						// if HH:MM is specified
						real_time_struct = gmtime(&temp_time);
						memset(time_target, 0, sizeof(time_target));
						strftime(time_target, 12, "%Y-%m-%dT", real_time_struct);
						strncat(time_target, optarg, 7);
						strncat(time_target, ":00Z", 5);
					}

					else if (strlen(optarg) == 10)
					{
				  	// if YYYY-MM-DD is specified
						real_time_struct = gmtime(&temp_time);
						memset(time_target, 0, sizeof(time_target));
						strcpy(time_target, optarg);
						strncat(time_target, "T06:00:00Z", 9);
					}

					else if (strlen(optarg) == 16)
					{
						// if YYYY-MM-DD HH:MM is specified
						real_time_struct = gmtime(&temp_time);
						memset(time_target, 0, sizeof(time_target));
						strcpy(time_target, optarg);
						strncat(time_target, ":00Z", 5);
						time_target[10] = 'T';
					}

					else
					{
						// if the date is invalid
						real_time_struct = gmtime(&temp_time);
						memset(time_target, 0, sizeof(time_target));
					}
				}
				else
				{
					if(verbose_flag)
						fprintf(stderr, "Date and time unspecified. Using defaults!\n");
				}
				break;
				
			case 'i':
				if (optarg == NULL || atoi(optarg)==0 || strlen(optarg) != 9)
				{
					if (verbose_flag)
						fprintf(stderr, "The specified ID is invalid, using defaults.\n");
				}
				else
					id = atoi(optarg);
				break;
				
			case 't':
				t = 1;

				for (i=0; *(data_types+i) != NULL; i++)
				{
					if (strcmp(optarg, *(data_types+i)) == 0)
						break;
				}
						
				data_type = i; //set data_type

				if (*(data_types+i) == NULL)
				{
					fprintf(stderr, "The specified type of data is invalid.\n");
					return EXIT_FAILURE;
				}
				
				else
				{	
					// set data_index and id_target
					if (!id)
					{
						for (i=19; (*(data_types+i) != NULL); i++)
							if (strncmp(*(data_types+i), optarg, 4) == 0)
								break;
						if (*(data_types+i) == NULL)
						{
							
							id_target	= id_meteo;
							data_index	= 0;
						}
						else
						{
							id_target	= id_hydro;
							data_index	= 1;
						}
					}
					else
						id_target = id;
				}
				
				if (verbose_flag)
				{
					fprintf(stderr, "data_index\t = %i\n", data_index);
					fprintf(stderr, "data_type\t = %s\n", *(data_types+data_type));
					fprintf(stderr, "id_target\t = %d\n", id_target);
					fprintf(stderr, "time_target = %s\n", time_target);
				}
				
				if (!mem->size || data_index != data_index_target)
				{
					if (verbose_flag)
						fprintf(stderr, "...retrieving data\n");
					if (mem->size)
					{
						g_free (mem->memory);
						mem->memory = g_malloc(1);
					}
					data_index_target = data_index;
					data_get(mem, data_index_target, id_target);
				}
				data_process(mem, data_index_target, data_type, time_target);
				break;
				
			case '?':
				fprintf(stderr, "Syntax: imgwmon [OPTIONS] ...\nTry `imgwmon --help` for more information.\n");
				g_free (mem->memory);
				g_free (mem);
				return EXIT_FAILURE;
				
			default:
				break;
		}
	}
	if (optind < 2)
	{
		fprintf(stderr, "Syntax: imgwmon [OPTIONS] ...\nTry `imgwmon --help` for more information.\n");
		g_free (mem->memory);
		g_free (mem);
		return EXIT_FAILURE;
	}
	
	if (t == -1)
	{
		fprintf(stderr, "Type of data parameter is mandatory!\n");
		fprintf(stderr, "Syntax: imgwmon [OPTIONS] ...\nTry `imgwmon --help` for more information.\n");
		g_free (mem->memory);
		g_free (mem);
		return EXIT_FAILURE;
	}

	conf_save();
	g_free (mem->memory);
	g_free (mem);
	return EXIT_SUCCESS;
}

void info_print (void)
{
  fprintf(stdout,
      "imgwmon 0.1-git (C) 2016-2017 Stanislaw J. Grams <sjg@fmdx.pl>\n"
      "Usage: imgwmon <options>\n"
      "\t-h\t\tPrint usage information\n"
	  "\t-s\t\tEdit configuration\n"
      "\t-i <id>\t\tSet the station id number\n"
      "\t-d <date>\tSet the date of fetching data (\"YYYY-MM-DD HH:MM\")\n"
      "\t-t <type>\tSet the type of fetching data\n\n"

			"\tList of available METEO data types:\n"
			"\tprecip.cur\t\tprecipitation at the moment\n"
			"\tprecip.10min\t\tprecipitation per 10 minutes, last 48 hours\n"
			"\tprecip.hourly\t\tprecipitation per day \n"
			"\tprecip.daily\t\tprecipitation per day\n"
			"\n"
			"\ttemp.auto\t\ttemperature per hour\n"
			"\ttemp.obs\t\ttemperature per hour, observer\n"
			"\n"
			"\ttemp.auto.min\t\tmin. temperature, last 48 hours\n"
			"\ttemp.auto.max\t\tmax. temperature, last 48 hours\n"
			"\ttemp.obs.min\t\tmin. temperature, last 48 hours, observer\n"
			"\ttemp.obs.max\t\tmax. temperature, last 48 hours, observer\n"
			"\n"
			"\twind.dir.tel\t\twind direction per 10 minutes\n"
			"\twind.dir.obs\t\twind direction per 1 hour, observer\n"
			"\twind.vel.tel\t\tavg. wind speed per 10 minutes\n"
			"\twind.vel.obs\t\tavg. wind speed per 1 hour, observer\n"
			"\twind.vel.max\t\tmax. wind speed per 10 minutes\n"
			"\n"
			"\twind.vel.tel.min\tmin. wind speed per 10 minutes\n"
			"\twind.vel.tel.max\tmax. wind speed per 10 minutes\n"
			"\twind.vel.obs.min\tmin. wind speed per 1 hour, observer\n"
			"\twind.vel.obs.max\tmax. wind speed per 1 hour, observer\n"
			"\n"
			"\tList of available HYDRO data types:\n"
			"\twater.info\t\tstate of water station and trend at the moment\n"
			"\twater.state.auto\twater states per hour\n"
			"\twater.state.obs\t\twater states, observer\n"
			"\twater.discharge\t\twater discharge per hour\n"
			"\twater.temp.auto\t\twater temperature per hour\n"
			"\twater.temp.obs\t\twater temperature, observer\n"
			"\n"
			"\tThe data is available up to the last 48 hours.\n"
			"\tThe date is required to be set before data type\n"
			"\tDates presented in UTC time.\n"
			"\n"
			"The source of data is Instytut Meteorologii i Gospodarki Wodnej - Panstwowy Instytut Badawczy.\n"
			"Zrodlem pochodzenia danych jest Instytut Meteorologii i Gospodarki Wodnej - Panstwowy Instytut Badawczy\n"
			);
}

