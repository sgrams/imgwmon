/*
 * imgwmoncpp 0.1-git
 * Stanislaw Grams <sjg@fmdx.pl>
 *
 * src/types.hh
 * See ../LICENSE for license information
 */
#ifndef IMGWMONCPP_TYPES_H
#define IMGWMONCPP_TYPES_H

typedef enum datatype {
  // general
  info             = 0x00,

  /* meteo flavour */
  // precipitation
  precip_cur       = 0x10,
  precip_10min     = 0x11,
  precip_hourly    = 0x12,
  precip_daily     = 0x13,

  // temperature
  temp_auto        = 0x20,
  temp_obs         = 0x21,

  temp_auto_min    = 0x22,
  temp_auto_max    = 0x23,
  temp_obs_min     = 0x24,
  temp_obs_max     = 0x25,

  // wind
  wind_dir_tel     = 0x30,
  wind_dir_obs     = 0x31,
  wind_vel_tel     = 0x32,
  wind_vel_obs     = 0x33,
  wind_vel_max     = 0x34,

  wind_vel_tel_max = 0x35,
  wind_vel_obs_max = 0x36,

  /* hydro flavour */
  // water state
  state_auto       = 0xF0,
  state_obs        = 0xF1,

  // water discharge
  discharge_auto   = 0xF2,
  discharge_obs    = 0xF3
} datatype_t;

typedef enum dataflavour {
  // only meteo & hydro supported by pogodynka
  meteo = 0x00,
  hydro = 0x01
} dataflavour_t;

typedef enum command {
  empty   = 0x00,
  help    = 0x01,
  verbose = 0x02,
  station = 0x03,
  date    = 0x04,
  type    = 0x05,
  json    = 0x06
} command_t;
#endif // IMGWMONCPP_TYPES_H
