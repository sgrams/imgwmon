#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <curl/curl.h>
#include <yajl/yajl_parse.h>

static int station_id = 253190220;
void printInfo	(void);
char *getData	(int station_id);
static size_t	
WriteMemoryCallback	(void *contents, size_t size, size_t nmemb, void *userp);

struct MemoryStruct
{
	char *memory;
	size_t size;
};

int main (int argc, char **argv)
{
	static int verbose_flag = 0;
	static struct option long_options[] =
	{
		{"help",	no_argument,	0,	'h'	},
		{"verbose",	no_argument,	0, 	'v'	},
		{"date",	required_argument,	0,	'd'	},
		{"type",	required_argument,	0,	't'	},
		{"id",	required_argument,	0,	'i'	},
		{0,	0,	0,	0	},
	};
	char *types_of_data[] =
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
	char *data;
	int option_index = 0;
	int c, i = 0;

	data = malloc(1);

	if(argc>1)
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
					exit(0);
				case 'd':
					if (optarg == NULL || strlen(optarg)!=17)
						if (verbose_flag)
							fprintf(stderr, "The specified date is invalid, using latest UTC full-hour.\n");
					break;
				case 't':
					strcat(optarg, "Records");
					for (i=0; *(types_of_data+i) != NULL; i++)
						if (strcmp(optarg, *(types_of_data+i)) == 0)
							break;
					if (*(types_of_data+i) == NULL)
					{
						if (verbose_flag)
							fprintf(stderr, "The specified type of data is invalid.\n");
						return EXIT_FAILURE;
					}
					break;
				case 'i':
					if (optarg == NULL || atoi(optarg)==0 || strlen(optarg) != 6)
					{
						if(verbose_flag)
							fprintf(stderr, "The specified ID is invalid, using defaults.\n");
					}
					else
						station_id = atoi(optarg);
					break;
				case '?':
					return EXIT_FAILURE;
				default:
					return EXIT_SUCCESS;
			}
		}
		data = getData(station_id);
		if(verbose_flag)
			printf("%s\n", data);
	}
	else
		printInfo();

	free(data);
	return EXIT_SUCCESS;
}

char *getData (int station_id)
{ 
	CURL *curl_handle;
	CURLcode res;
	char url[] = "http://monitor.pogodynka.pl/api/station/meteo/?id=";
	char buffer[10];
	struct MemoryStruct chunk;

	chunk.memory = malloc(1);
	chunk.size = 0;

	snprintf(buffer, sizeof(buffer), "%d", station_id);
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

