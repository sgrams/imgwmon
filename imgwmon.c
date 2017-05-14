/* 
 *  imgwmon 0.1-git
 *  (C) 2016-2017 Stanisław J. Grams <sjg@fmdx.pl>
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <curl/curl.h>
#include <errno.h>
#include <yajl/yajl_tree.h>  

#define METEO_STATION_ID 253190220
#define HYDRO_STATION_ID 153190150

void printInfo (void);
char *getData (short object_of_data, size_t target_id);
void processData (char *data, short object_of_data, char *target_type_of_data, char *target_time);

static size_t
WriteMemoryCallback (void *contents, size_t size, size_t nmemb, void *userp);

struct MemoryStruct
{
	char *memory;
	size_t size;
};

static int verbose_flag = 0;

int main (int argc, char **argv)
{
	static struct option long_options[] =
	{
		{"help", no_argument, 0, 'h'},
		{"verbose", no_argument, 0, 'v'},
		{"date", required_argument, 0, 'd'},
		{"type", required_argument, 0, 't'},
		{"id", required_argument, 0, 'i'},
		{0, 0, 0, 0},
	};

	static char *types_of_data[] =
	{
		"currentPrecipRecords",
		"currentWaterStateRecords",
		"hourlyPrecipRecords",
		"dailyPrecipRecords",
		"tenMinutesPrecipRecords",
		"temperatureAutoRecords",
		"temperatureObsRecords",
		"maxTemperatureAutoRecords",
		"minTemperatureAutoRecords",
		"maxTemperatureObsRecords",
		"minTemperatureObsRecords",
		"windDirectionTelRecords",
		"windDirectionObsRecords",
		"windVelocityTelRecords",
		"windVelocityObsRecords",
		"windMaxVelocityRecords",
		"waterStateRecords",
		"waterStateObserverRecords",
		"dischargeRecords",
		"waterTemperatureAutoRecords",
		"waterTemperatureObsRecords",
		NULL
	};

	char *target_type_of_data;
	char *temp_target_type_of_data;
	char target_time[21];
	char *data = NULL;

	int target_id = 0;
	int id = 0;
	int option_index = 0;
	int c, i;
	
	time_t temp_time = time(NULL);
	struct tm *real_time_struct;

	short t = -1;
	short object_of_data = 0;
	short target_object_of_data = 0;

	real_time_struct = gmtime(&temp_time);
	strftime(target_time, 21, "%Y-%m-%dT%H:00:00Z", real_time_struct);

	while ((c = getopt_long (argc, argv, "hvd:i:t:", long_options, &option_index)) != -1)
	{
		switch (c)
		{
			case 'v':
				verbose_flag = 1;
				break;
			case 'h':
				printInfo();
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
						memset(target_time, 0, sizeof(target_time));
						strftime(target_time, 12, "%Y-%m-%dT", real_time_struct);
						strncat(target_time, optarg, 7);
						strncat(target_time, ":00Z", 5);
					}

					else if (strlen(optarg) == 10)
					{
				  	// if YYYY-MM-DD is specified
						real_time_struct = gmtime(&temp_time);
						memset(target_time, 0, sizeof(target_time));
						strcpy(target_time, optarg);
						strncat(target_time, "T06:00:00Z", 9);
					}

					else if (strlen(optarg) == 16)
					{
						// if YYYY-MM-DD HH:MM is specified
						real_time_struct = gmtime(&temp_time);
						memset(target_time, 0, sizeof(target_time));
						strcpy(target_time, optarg);
						strncat(target_time, ":00Z", 5);
						target_time[10] = 'T';
					}

					else
					{
						// if the date is invalid
						real_time_struct = gmtime(&temp_time);
						memset(target_time, 0, sizeof(target_time));
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
					target_id = atoi(optarg);
				break;
			case 't':
				t = 1;
				target_type_of_data = malloc(1);
				temp_target_type_of_data = malloc(strlen(optarg) + strlen("Records") + 1);

				strncpy(temp_target_type_of_data, optarg, strlen(optarg)+1);
				strncat(temp_target_type_of_data, "Records", strlen("Records") + 1);

				for (i=0; *(types_of_data+i) != NULL; i++)
					if (strcmp(temp_target_type_of_data, *(types_of_data+i)) == 0)							
						break;
				
				if (*(types_of_data+i) == NULL)
				{
					fprintf(stderr, "The specified type of data is invalid.\n");
					return EXIT_FAILURE;
				}

				else
				{
					strncpy(target_type_of_data, temp_target_type_of_data, strlen(temp_target_type_of_data)+1);
					target_type_of_data = strndup(temp_target_type_of_data, strlen(temp_target_type_of_data)+1);

					if (!strcmp(target_time, ""))
					{
						if (strcmp(target_type_of_data, "windDirectionTelRecords") == 0 ||
								strcmp(target_type_of_data, "windVelocityTelRecords") == 0 ||
								strcmp(target_type_of_data, "windMaxVelocityRecords") == 0)
						{
							real_time_struct = gmtime(&temp_time);
							strftime(target_time, 21, "%Y-%m-%dT%H:%M:00Z", real_time_struct);
							if (verbose_flag)
								fprintf(stderr, "The specified date is invalid, using latest possible time (%s).\n", target_time);
						}
						else if (strcmp(target_type_of_data, "dailyPrecipRecords") == 0)
						{
							real_time_struct = gmtime(&temp_time);
							real_time_struct->tm_min = real_time_struct->tm_min - (real_time_struct->tm_min)%10;
							strftime(target_time, 21, "%Y-%m-%dT06:00:00Z", real_time_struct);
							if (verbose_flag)
								fprintf(stderr, "The specified date is invalid, using latest possible time (%s).\n", target_time);
						}
						else if (strcmp(target_type_of_data, "tenMinutesPrecipRecords") == 0 &&
										strlen(target_time)!=5)
						{
							real_time_struct = gmtime(&temp_time);
							real_time_struct->tm_min = real_time_struct->tm_min - (real_time_struct->tm_min)%10;
							strftime(target_time, 21, "%Y-%m-%dT%H:%M:00Z", real_time_struct);
							if (verbose_flag)
								fprintf(stderr, "The specified date is invalid, using latest possible time (%s).\n", target_time);
						}
					}

					// forcing dailyPrecip HH:MM to 06:00
					if (strcmp(target_type_of_data, "dailyPrecipRecords") == 0)
					{
							target_time[11]='0'; target_time[12]='6';
							target_time[13]=':'; target_time[14]='0';
							target_time[15]='0'; target_time[16]=':';
							target_time[17]=':'; target_time[18]='0';
							target_time[19]='Z';
					}

					if (strcmp(target_type_of_data, "waterStateRecords") == 0 ||
							strcmp(target_type_of_data, "currentWaterStateRecords") == 0 ||
							strcmp(target_type_of_data, "waterStateObserverRecords") == 0 ||
							strcmp(target_type_of_data, "dischargeRecords") == 0 ||
							strcmp(target_type_of_data, "waterTemperatureAutoRecords") == 0 ||
							strcmp(target_type_of_data, "waterTemperatureObsRecords") == 0)
					{
						if(!target_id)
							target_id = HYDRO_STATION_ID;
						object_of_data = 1; 
					}
					else						
					{
						if(!target_id)
							target_id = METEO_STATION_ID;
						object_of_data = 0;
					}
				}
				if (verbose_flag)
				{
					fprintf(stderr, "target_type_of_data\t = %s\n", target_type_of_data);
					fprintf(stderr, "target_time\t\t = %s\n", target_time);
					fprintf(stderr, "target_id\t\t = %d\n", target_id);
				}

				if (data == NULL || object_of_data != target_object_of_data || id != target_id)
				{
					data = malloc(1);
					if (verbose_flag)
						fprintf(stderr, "...retrieving data\n");

					target_object_of_data = object_of_data;
					id = target_id;
					data = getData(target_object_of_data, target_id);
				}
				processData(data, object_of_data, target_type_of_data, target_time);
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

	free(data);
	free(target_type_of_data);
	free(temp_target_type_of_data);
	return EXIT_SUCCESS;
}
char *getData (short object_of_data, size_t target_id)
{ 
	CURL *curl_handle;
	CURLcode res;

	char url[] = "http://monitor.pogodynka.pl/api/station/meteo/?id=";
	char buffer[11];
	struct MemoryStruct chunk;

	chunk.memory = malloc(1);
	chunk.size = 0;
	
	if (object_of_data)
	{
		url[0]='\0';
		strncat(url, "http://monitor.pogodynka.pl/api/station/hydro/?id=", 50);
	}

	snprintf(buffer, 10, "%zd", target_id);
	strncat(url, buffer, strlen(buffer)+1);

	if(verbose_flag)
		printf("target_url\t\t = %s\n", url);

	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();

	curl_easy_setopt(curl_handle, CURLOPT_URL, url);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "imgwmon-agent/0.1");
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
	res = curl_easy_perform(curl_handle);
	if (res != CURLE_OK)
	{
		if(verbose_flag)
			fprintf(stderr, "Failed to fetch data: %s\n", curl_easy_strerror(res));
	}
	curl_easy_cleanup(curl_handle);
	curl_global_cleanup();

	return chunk.memory;
}

void processData (char *data, short object_of_data, char *target_type_of_data, char *target_time)
{
	const char *main_path[] =
	{target_type_of_data, NULL};
	const char *temperatureAutoPath[] =
	{"temperatureAutoRecords", NULL};
	const char *temperatureObsPath[] =
	{"temperatureObsRecords", NULL};
	const char *current_precip_path[] =
	{"status", "precip", "value", NULL};
	const char *current_trend_path[] =
	{"trend", NULL};
	const char *current_state_path[] =
	{"state", NULL};
	const char *current_river_path[] =
	{"status", "river", NULL};

	char errbuf[1024];

	int i, array_length;
	float target_result;

	yajl_val main_node;
	yajl_val object_target_type_of_data;
	yajl_val arrays_target_type_of_data;
	yajl_val keys_target_type_of_data;
	yajl_val values_target_type_of_data;

	errbuf[0] = 0;

	main_node = yajl_tree_parse((const char *) data, errbuf, sizeof(errbuf));
	if (!main_node)
	{
		if(verbose_flag)
			fprintf(stderr, "Unable to parse data. Parser returned %s.\n", errbuf);
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


					if(strcmp(target_time, (char *)values_target_type_of_data->u.object.keys) == 0)
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
			fprintf(stdout, "river: %s, ", (char *)object_target_type_of_data->u.object.keys);
		object_target_type_of_data = yajl_tree_get(main_node, current_state_path, yajl_t_string);
		if (object_target_type_of_data)
			fprintf(stdout, "state: %s, ", (char *)object_target_type_of_data->u.object.keys);
		object_target_type_of_data = yajl_tree_get(main_node, current_trend_path, yajl_t_string);
		if (object_target_type_of_data)
			fprintf(stdout, "trend: %s\n", (char *)object_target_type_of_data->u.object.keys);
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
				target_time = strndup((char*)values_target_type_of_data->u.object.keys, 21);

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
						target_time = strndup((char*)values_target_type_of_data->u.object.keys, 21);
					}
								
					else if (target_result <= values_target_type_of_data->u.number.d &&
							(strcmp(target_type_of_data, "maxTemperatureAutoRecords") == 0 ||
							strcmp(target_type_of_data, "maxTemperatureObsRecords") == 0))
					{
						target_result = values_target_type_of_data->u.number.d;
						values_target_type_of_data = keys_target_type_of_data->u.object.values[0];
						target_time = strndup((char*)values_target_type_of_data->u.object.keys, 21);
					}
				}
			fprintf(stdout, "%.1f °C, %s\n", target_result, target_time);
			}
		}
	}
	yajl_tree_free(main_node);
}
void printInfo (void)
{
  fprintf(stdout,
      "imgwmon 0.1-git (C) 2016-2017 Stanislaw J. Grams <sjg@fmdx.pl>\n"
      "Usage: imgwmon <options>\n"
      "\t-h\t\tPrint usage information\n"
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

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;

	mem->memory = realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory == NULL)
	{
		fprintf(stderr, "Not enough memory, returned %s\n", strerror(errno));
		return 0;
	}
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;
	return realsize;
}
