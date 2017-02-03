/*  imgw-pib station list data parser
 *  copyright (c) 2017 Stanisław J. Grams
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yajl/yajl_tree.h>

void parseData (char *data, FILE *output);

int main (void)
{
	int i, licznik=0;
	char *data_target;
	char ch;
	FILE *data = fopen("data.txt", "r");
	FILE *output = fopen("output.txt", "w");


	fseek(data, 0, SEEK_END);
	licznik = ftell(data);
	rewind(data);

	data_target = malloc((licznik * sizeof(char)) + 1);
	fread(data_target, licznik, 1, data);
	data_target[licznik] = '\0';

	parseData(data_target, output);

	free(data_target);
	fclose(data);
	fclose(output);
	return 0;
}
void parseData (char *data, FILE *output)
{
	const char *path[] = {NULL};
	char errbuf[1024];
	int i, array_length;
	
	yajl_val main_node;
	yajl_val arrays_target_data;
	yajl_val keys_target_data;
	yajl_val values_target_data;

	errbuf[0]=0;
	main_node = yajl_tree_parse((const char *) data, errbuf, sizeof(errbuf));
	if(!main_node)
		fprintf(stderr, "DUPA!\n");
	arrays_target_data = yajl_tree_get(main_node, path, yajl_t_array);
	if (arrays_target_data)
	{
		array_length = arrays_target_data->u.array.len;
		if (array_length)
		{
			for (i=0; i<array_length; i++)
			{
				keys_target_data = arrays_target_data->u.array.values[i];
				values_target_data = keys_target_data->u.object.values[2];
				fprintf(output, "%s ", values_target_data->u.object.keys);
				values_target_data = keys_target_data->u.object.values[3];
				fprintf(output, "%s\n", values_target_data->u.object.keys);
			}
		}
	}
}
