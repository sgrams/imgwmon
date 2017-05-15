#ifndef IMGWMON_CONF_H_
#define IMGWMON_CONF_H_

void conf_init(const gchar*);
void conf_save();

static gint conf_read_integer(const gchar*, const gchar*, gint);

gint conf_get_main_id_meteo();
void conf_set_main_id_meteo(gint);

gint conf_get_main_id_hydro();
void conf_set_main_id_hydro(gint);

#endif