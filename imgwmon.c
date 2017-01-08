#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <curl/curl.h>

void	printInfo	(void);
char	*getData	(int station_id);

static int	station_id = 253190220; /*  default ID number */


int main (int argc, char **argv)
{
	static int verbose_flag=0;
	static struct option long_options[] =
	{
		{"help",		no_argument,				0,	'h'},
		{"verbose", no_argument,				0, 	'v'},
		{"date",		required_argument,	0,	'd'},
		{"type",		required_argument,	0,	't'},
		{"id",			required_argument,	0,	'i'},
	};
	
	int option_index = 0;
	int c;
	
	if(argc!=1)
	{
		while (1)
		{
			c = getopt_long (argc, argv, "hvd:t:i:", long_options, &option_index);
			if (c == -1)
				break;
			switch (c)
			{
				case 'v':
					verbose_flag = 1;
					break;
				case 'h':
					printInfo();
					return EXIT_SUCCESS;
				case 'd':
					printf("executed %s -%c %s\n", argv[0], c, optarg);
					break;
				case 't':
					printf("executed %s -%c %s\n", argv[0], c, optarg);
					break;
				case 'i':
					if (atoi(optarg) == 0)
					{
						if(verbose_flag)
							fprintf(stderr, "Entered station id is invalid, using defaults.\n");
					}
					else
						station_id = atoi(optarg);
					break;
				case '?':
					break;
				default:
					abort();
			}
		}
		getData(station_id);
	}
	else
		printInfo();
	
	return EXIT_SUCCESS;
}

char *getData (int station_id, MemoryStruct chunk)
{ 
	CURL *curl_handle;
	CURLcode res;
	char url[] = "http://monitor.pogodynka.pl/api/station/meteo/?id=";
	char buffer[10];

	snprintf(buffer, sizeof(buffer), "%d", station_id);
	strcat(url, buffer);

	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();

	curl_easy_setopt(curl_handle, CURLOPT_URL, url);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "imgwmon-agent/0.1");
	
	res = curl_easy_perform(curl_handle);
	if (res != CURLE_OK)
	{
		fprintf(stderr, "Failed to fetch data: %s\n", curl_easy_strerror(res));
	}
	curl_easy_cleanup(curl_handle);
	curl_global_cleanup();
	return chunk;
}

void printInfo (void)
{
  fprintf(stderr,
      "imgwmon, version 2017/01/08 (C) 2016-2017 Stanislaw J. Grams <sjg@fmdx.pl>\n"
      "Usage: imgwmon <options>\n"
      "\t-h\t\tPrint usage information\n"
      "\t-i <id>\t\tSet the station id number (default=253190220)\n"
      "\t-d <date>\tSet the date of fetching data (yyyy-mm-dd hh:mm), if empty - fetching latest\n"
      "\t-t <type>\tSet the type of fetching data (temperature, precipitation, wind)\n");
}
