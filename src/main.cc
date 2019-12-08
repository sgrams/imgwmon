/*
 * imgwmoncpp 0.1-git
 * Stanislaw Grams <sjg@fmdx.pl>
 *
 * src/main.cc
 * See ../LICENSE for license information
 */

#include <iostream>
#include <string>
#include "agent.hh"
#include "data.hh"
#include "station.hh"

int
main () {
  int status = 0;

  data_c    data;
  station_c station;
  agent_c   agent;

  station.set_name (std::string("Lidzbark"));
  agent.idfetcher_execute (station, &data);
  std::cout << data.buffer << "\n";

  return status;
}
