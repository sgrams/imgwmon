/* 
 *  imgwmon 0.1-git
 *  (C) 2016-2017 Stanisław J. Grams <sjg@fmdx.pl>
 * 
 *	conf.c
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
#include <string.h>
#include "conf.h"

#define CONF_DIR    "imgwmon"
#define CONF_FILE   "default.conf"

#define CONF_DEFAULT_MAIN_ID_METEO 253190220
#define CONF_DEFAULT_MAIN_ID_HYDRO 153190150

typedef struct
conf
{
    gchar *path;
    GKeyFile *keyfile;

    gint main_id_meteo;
    gint main_id_hydro;
} conf_t;

static conf_t conf;
static void conf_read();

void
conf_init (const gchar *custom_path)
{
    gchar *directory;
    if (custom_path)
        conf.path = g_strdup(custom_path);
    else
    {
        directory = g_build_filename(g_get_user_config_dir(), CONF_DIR, NULL);
        g_mkdir(directory, 0700);
        conf.path = g_build_filename(directory, CONF_FILE, NULL);
        g_free(directory);
    }
    conf.keyfile = g_key_file_new();
    conf_read();
}

static
void
conf_read()
{
    gboolean file_exists = TRUE;
    if (!g_key_file_load_from_file(conf.keyfile, conf.path, G_KEY_FILE_KEEP_COMMENTS, NULL))
    {
        file_exists = FALSE;
        fprintf(stderr, "Unable to read configuration, using default settings.\n");
    }

    conf.main_id_meteo = conf_read_integer("main", "id_meteo", CONF_DEFAULT_MAIN_ID_METEO);
    conf.main_id_hydro = conf_read_integer("main", "id_hydro", CONF_DEFAULT_MAIN_ID_HYDRO);

    if (!file_exists)
        conf_save();
}

gint
conf_read_integer (const gchar *group_name,
                const gchar *key,
                gint default_value)
{
    gint value;
    GError *err = NULL;

    value = g_key_file_get_integer(conf.keyfile, group_name, key, &err);
    if (err)
    {
        value = default_value;
        g_error_free(err);
    }
    return value;
}

void
conf_save ()
{
    GError *err = NULL;
    gchar *configuration;
    gsize length, out;
    FILE *fp;

    g_key_file_set_integer(conf.keyfile, "main", "id_meteo", conf.main_id_meteo);
    g_key_file_set_integer(conf.keyfile, "main", "id_hydro", conf.main_id_hydro);

    if (!(configuration = g_key_file_to_data(conf.keyfile, &length, &err)))
    {
        fprintf(stderr, "Unable to generate the configuration file.\n%s\n", err->message);
        g_error_free(err);
        err = NULL;
    }

    if ((fp = fopen(conf.path, "w")) == NULL)
    {
        fprintf(stderr, "Unable to save the configuration to a file: %s\n",
                conf.path);
    }

    else
    {
        out = fwrite(configuration, sizeof(char), length, fp);
        if (out != length)
        {
        fprintf (stderr, "Failed to save the configuration to a file: %s\n(wrote only %lu of %lu bytes)\n",
                conf.path, out, length);
        }
        fclose(fp);
    }
    g_free(configuration);
    g_free(err);
}
void
conf_destroy ()
{
  g_free(conf.path);
  g_key_file_free(conf.keyfile);
}

gint
conf_get_main_id_meteo ()
{
    return conf.main_id_meteo;
}

gint
conf_get_main_id_hydro ()
{
    return conf.main_id_hydro;
}

void
conf_set_main_id_meteo (gint id_meteo)
{
    conf.main_id_meteo = id_meteo;
}

void
conf_set_main_id_hydro (gint id_hydro)
{
    conf.main_id_hydro = id_hydro;
}
