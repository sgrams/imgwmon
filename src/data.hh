/*
 * imgwmoncpp 0.1-git
 * Stanislaw Grams <sjg@fmdx.pl>
 *
 * src/data.hh
 * See ../LICENSE for license information
 */
#ifndef IMGWMONCPP_DATA_H
#define IMGWMONCPP_DATA_H
#include <stdexcept>
#include <cstdint>
#include <vector>
#include <memory>
#include "types.hh"
#include "station.hh"

class data_c {
  public:
  std::string buffer;
  size_t      size;
};
#endif // IMGWMONCPP_DATA_H
