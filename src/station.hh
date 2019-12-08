/*
 * imgwmoncpp 0.1-git
 * Stanislaw Grams <sjg@fmdx.pl>
 *
 * src/station.hh
 * See ../LICENSE for license information
 */
#ifndef IMGWMONCPP_STATION_H
#define IMGWMONCPP_STATION_H
#include <cstdint>
#include <string>
#include "types.hh"

class station_c {
  private:
  std::string   name;
  uint64_t      number  = 0;
  dataflavour_t flavour = meteo;

  public:
  std::string   get_name ();
  void          set_name (std::string name);

  uint64_t      get_number ();
  void          set_number (uint64_t number);

  dataflavour_t get_flavour (void);
  void          set_flavour (dataflavour_t flavour);
};

#endif // IMGWMONCPP_STATION_H
