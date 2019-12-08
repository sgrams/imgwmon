/*
 * imgwmoncpp 0.1-git
 * Stanislaw Grams <sjg@fmdx.pl>
 *
 * src/station.cc
 * See ../LICENSE for license information
 */
#include <stdexcept>
#include "station.hh"

std::string
station_c::get_name (
  )
{
  return this->name;
}

void
station_c::set_name (
  std::string name
  )
{
  if (name.empty ()) {
    throw std::invalid_argument ("name set incorrectly");
    return;
  }
  this->name = name;
  return;
}

uint64_t
station_c::get_number (
  )
{
  return this->number;
}

void
station_c::set_number (
  uint64_t number
  )
{
  if (number > 0) {
    this->number = number;
  }
}

dataflavour_t
station_c::get_flavour (
  )
{
  return this->flavour;
}
