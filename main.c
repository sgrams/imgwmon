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

int main (gint argc, char **argv)
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
		"precip.current",
		"precip.hourly",
		"precip.daily",
		"precip.10min",
		
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
		
		"water.state.current",
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
	gshort data_type_target;
	gshort data_index;
	gshort data_index_target;

	gint c, i, t;
	
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
				return EXIT_SUCCESS;
				
			case 'd':
				if (optind == argc)
				{
					printf("Date is required to be specified before data type!\n");
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
					if (strcmp(optarg, *(data_types+i)) == 0)
						break;
						
				if (*(data_types+i) == NULL)
				{
					fprintf(stderr, "The specified type of data is invalid.\n");
					return EXIT_FAILURE;
				}

				else
				{	
					// check for id parameter if it's HYDRO type
					if (!id)
					{
						for (i=15; (*(data_types+i) != NULL); i++)
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
				
				/*if (verbose_flag)
				{
					fprintf(stderr, "data_index_target\t = %i\n", data_index_target);
					fprintf(stderr, "data_type_target\t = %s\n", data_type_target);
					fprintf(stderr, "id_target\t\t = %d\n", id_target);
				}*/
				
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
				data_process(mem, data_index_target, data_type_target, time_target);
				break;
				
			case '?':
				fprintf(stderr, "Syntax: imgwmon [OPTIONS] ...\nTry `imgwmon --help` for more information.\n");
				return EXIT_FAILURE;
				
			default:
				break;
		}
	}
	if (optind < 2)
	{
		fprintf(stderr, "Syntax: imgwmon [OPTIONS] ...\nTry `imgwmon --help` for more information.\n");
		return EXIT_FAILURE;
	}
	
	if (t == -1)
	{
		fprintf(stderr, "Type of data parameter is mandatory!\n");
		fprintf(stderr, "Syntax: imgwmon [OPTIONS] ...\nTry `imgwmon --help` for more information.\n");
		return EXIT_FAILURE;
	}

	conf_save();
	g_free (data_type_target);
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
      "\t-d <date>\tSet the date of fetching data (date format=\"YYYY-MM-DD HH:MM\" UTC),\n\t\t\tif empty - fetching latest\n"
			"\t! The date is required to be set before data type !\n"
      "\t-t <type>\tSet the type of fetching data\n\n"
			"\tList of available METEO data types:\n"
			"\tcurrentPrecip\t\t - precipitation at the moment\n"
			"\thourlyPrecip\t\t - precipitation per hour\n"
			"\tdailyPrecip\t\t - precipitation per day (date format=\"YYYY-MM-DD\")\n"
			"\ttenMinutesPrecip\t - precipitation per 10 minutes (date format=\"HH:MM\"),\n\t\t\t\t data available up to the last hour\n"
			"\ttemperatureAuto\t\t - temperature per hour\n"
			"\ttemperatureObs\t\t - temperature per hour, measured by an observer\n"
			"\tminTemperatureAuto\t - minimum temperature during the last 48 hours\n"
			"\tmaxTemperatureAuto\t - maximum temperature during the last 48 hours\n"
			"\tminTemperatureObs\t - minimum temperature during the last 48 hours, measured by an observer\n"
			"\tmaxTemperatureObs\t - maximum temperature during the last 48 hours, measured by an observer\n"
			"\twindDirectionTel\t - wind direction per 10 minutes\n"
			"\twindDirectionObs\t - wind direction per 1 hour, measured by an observer\n"
			"\twindVelocityTel\t\t - average wind speed per 10 minutes\n"
			"\twindVelocityObs\t\t - average wind speed per 1 hour, measured by an observer\n"
			"\twindMaxVelocity\t\t - maximum wind speed per 10 minutes\n"
			"\n"
			"\tList of available HYDRO data types:\n"
			"\tcurrentWaterState\t - state of water and trend at the moment\n"
			"\twaterState\t\t - water states per hour\n"
			"\twaterStateObserver\t - water states (frequency depends on the station),\n\t\t\t\t measured by an observer\n"
			"\tdischarge\t\t - water discharge per hour\n"
			"\twaterTemperatureAuto\t - water temperature per hour\n"
			"\twaterTemperatureObs\t - water temperature (frequency depends on the station),\n\t\t\t\t measured by an observer\n"
			"\n"
			"\tThe data is available up to the last 48 hours.\n"
			"\n"
			"The source of data is Instytut Meteorologii i Gospodarki Wodnej - Panstwowy Instytut Badawczy.\n"
			"Zrodlem pochodzenia danych jest Instytut Meteorologii i Gospodarki Wodnej - Panstwowy Instytut Badawczy\n"
			);
}

