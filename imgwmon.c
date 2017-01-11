#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <curl/curl.h>
#include <errno.h>
#include "jsmn.h"

void printInfo (void);
char *getData (int target_id);

static size_t
WriteMemoryCallback (void *contents, size_t size, size_t nmemb, void *userp);

struct MemoryStruct
{
	char *memory;
	size_t size;
};

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
		"hourlyPrecipRecords",
		"dailyPrecipRecords",
		"tenMinutesPrecipRecords",
		"temperatureAutoRecords",
		"temperatureObsRecords",
		"windDirectionTelRecords",
		"windDirectionObsRecords",
		"windVelocityTelRecords",
		"windVelocityObsRecords",
		"windMaxVelocityRecords",
		NULL
	};

	char *target_type_of_data = "temperatureAutoRecords";
	char target_time[21];
	char *temp_target_type_of_data;
	char *data	= malloc(1);
	
	int target_id			= 253190220;
	int verbose_flag	= 0;
	int option_index	= 0;
	int c, i;
	
	time_t temp_time = time(NULL);
	struct tm *real_time_struct;
	
	real_time_struct = gmtime(&temp_time);
	strftime(target_time, 21, "%Y-%m-%dT%H:00:00Z", real_time_struct);
	if (argc > 1)
	{
		while ((c = getopt_long (argc, argv, "hvd:t:i:", long_options, &option_index)) != -1)
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
					if ((strcmp(target_type_of_data, "dailyPrecipRecords") == 0 && strlen(optarg) != 10) ||
						(strcmp(target_type_of_data, "tenMinutesPrecipRecords") == 0 && strlen(optarg) != 5))
					{
						if (verbose_flag)
							fprintf(stderr, "The specified date is invalid, using latest possible time (%s).\n", target_time);
						printf("target_time=%s\n", target_time);
					}
					else if (strcmp(target_type_of_data, "dailyPrecipRecords") == 0 && strlen(optarg) == 10)
					{
						/* YYYY-MM-DD specified */
						strcpy(target_time, optarg);
						strncat(target_time, "T06:00:00Z", 7);
					}
					else if (strcmp(target_type_of_data, "tenMinutesPrecipRecords") == 0 && strlen(optarg) == 5)
					{
						/* HH:MM specified */
						strftime(target_time, 12, "%Y-%m-%dT", real_time_struct);
						strncat(target_time, optarg, 6);
						strncat(target_time, ":00Z", 1);
					}
					else if (strlen(optarg)!=16)
					{
						/* Invalid date */
						if (verbose_flag)
							fprintf(stderr, "The specified date is invalid, using latest possible time (%s).\n", target_time);
					}
					else
					{
						/* Correct date of common types, YYYY-MM-DD HH:MM (minutes=00)*/
						strcpy(target_time, optarg);
						strcat(target_time, ":00Z");
						target_time[10] = 'T';
					}
					break;
				case 't':
					temp_target_type_of_data = malloc(strlen(optarg)+strlen("Records")+1);
					strncpy(temp_target_type_of_data, optarg, strlen(optarg)+1);
					strcat(temp_target_type_of_data, "Records");

					for (i=0; *(types_of_data+i) != NULL; i++)
						if (strcmp(temp_target_type_of_data, *(types_of_data+i)) == 0)
							break;
					
					if (*(types_of_data+i) == NULL)
					{
						if (verbose_flag)
							fprintf(stderr, "The specified type of data is invalid, using defaults.\n");
					}
					else
					{
						target_type_of_data = malloc(strlen(temp_target_type_of_data) + 1);
						strcpy(target_type_of_data, temp_target_type_of_data+'\0');
						if (strcmp(target_type_of_data, "dailyPrecipRecords") == 0 ||
								strcmp(target_type_of_data, "windDirectionTelRecords") == 0 ||
								strcmp(target_type_of_data, "windVelocityTelRecords") == 0 ||
								strcmp(target_type_of_data, "windMaxVelocityRecords") == 0)
						{
							real_time_struct->tm_min = real_time_struct->tm_min - (real_time_struct->tm_min)%10;
							strftime(target_time, 21, "%Y-%m-%dT%H:%M:00Z", real_time_struct);
						}
					}
					break;
				case 'i':
					if (optarg == NULL || atoi(optarg)==0 || strlen(optarg) != 6)
					{
						if (verbose_flag)
							fprintf(stderr, "The specified ID is invalid, using defaults.\n");
					}
					else
						target_id = atoi(optarg);
					break;
				case '?':
					return EXIT_FAILURE;
				default:
					return EXIT_SUCCESS;
			}
		}
		data = getData(target_id);
		printf("target_time=%s\n", target_time);
	}
	else
		printInfo();
	free(data);
	return EXIT_SUCCESS;
}

char *getData (int target_id)
{ 
	CURL *curl_handle;
	CURLcode res;
	char url[] = "http://monitor.pogodynka.pl/api/station/meteo/?id=";
	char buffer[10];
	struct MemoryStruct chunk;

	chunk.memory = malloc(1);
	chunk.size = 0;

	snprintf(buffer, sizeof(buffer), "%d", target_id);
	strcat(url, buffer);

	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();

	curl_easy_setopt(curl_handle, CURLOPT_URL, url);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "imgwmon-agent/0.1");
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
	res = curl_easy_perform(curl_handle);
	if (res != CURLE_OK)
	{
		fprintf(stderr, "Failed to fetch data: %s\n", curl_easy_strerror(res));
	}
	curl_easy_cleanup(curl_handle);
	curl_global_cleanup();
	
	return chunk.memory;
}

void printInfo (void)
{
  fprintf(stderr,
      "imgwmon, version 2017/01/dupa (C) 2016-2017 Stanislaw J. Grams <sjg@fmdx.pl>\n"
      "Usage: imgwmon <options>\n"
      "\t-h\t\tPrint usage information\n"
      "\t-i <id>\t\tSet the station id number (default=\"253190220\")\n"
      "\t-d <date>\tSet the date of fetching data (date format=\"YYYY-MM-DD HH:MM\", if empty - fetching latest\n"
      "\t-t <type>\tSet the type of fetching data (default=\"temperatureAuto\")\n\n"
			"\tList of available data types:\n"
			"\tcurrentPrecip\t\t - state of the precipitation at the moment\n"
			"\thourlyPrecip\t\t - record of a precipitation per hour\n"
			"\tdailyPrecip\t\t - record of a precipitation per day (date format=\"YYYY-MM-DD\")\n"
			"\ttenMinutesPrecip\t - record of a precipitation per 10 minutes (date format=\"HH:MM\")\n"
			"\ttemperatureAuto\t\t - record of a temperature per hour, measured automatically\n"
			"\ttemperatureObs\t\t - record of a temperature per hour, measured by an observer\n"
			"\twindDirectionTel\t - record of a wind direction per 10 minutes, measured automatically\n"
			"\twindDirectionObs\t - record of a wind direction per 1 hour, measured by an observer\n"
			"\twindVelocityTel\t\t - record of an average wind speed per 10 minutes, measured automatically\n"
			"\twindVelocityObs\t\t - record of an average wind speed per 1 hour, measured by an observer\n"
			"\twindMaxVelocity\t\t - record of a maximum wind speed per 10 minutes, measured automatically\n"
			"\tThe data is available up to the last three days.\n"
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
		printf("Not enough memory!\n");
		return 0;
	}
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;
	return realsize;
}
