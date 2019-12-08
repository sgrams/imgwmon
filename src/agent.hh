/*
 * imgwmoncpp 0.1-git
 * Stanislaw Grams <sjg@fmdx.pl>
 *
 * src/agent.hh
 * See ../LICENSE for license information
 */
#ifndef IMGWMONCPP_AGENT_H
#define IMGWMONCPP_AGENT_H
#include <cstdint>
#include <memory>
#include <curl/curl.h>
#include "types.hh"
#include "station.hh"
#include "data.hh"

#define AGENT_IDFETCHER_URI_PREFIX  "http://monitor.pogodynka.pl/api/search/?query="
#define AGENT_IDFETCHER_URI_SUFFIX  "&limit=10"
#define AGENT_DATAFETCHER_METEO_URI "http://monitor.pogodynka.pl/api/station/meteo/?id="
#define AGENT_DATAFETCHER_HYDRO_URI "http://monitor.pogodynka.pl/api/station/hydro/?id="
#define AGENT_USERAGENT             "imgwmoncpp-agent/0.1-git"
#define AGENT_TIMEOUT 10

#define HTTP_OK 200

class agent_c {
  private:
  static size_t curl_callback (char *in, size_t size, size_t nmemb, std::string *out);
  int           curl_retrieve (std::string url, std::string *buffer);

  public:
  agent_c ();
  ~agent_c ();
  // fetchers
  int idfetcher_execute   (station_c station, data_c *data);
  int datafetcher_execute (station_c station, data_c *data);
};

#endif // IMGWMONCPP_AGENT_H
