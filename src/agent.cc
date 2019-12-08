/*
 * imgwmoncpp 0.1-git
 * Stanislaw Grams <sjg@fmdx.pl>
 *
 * src/agent.cc
 * See ../LICENSE for license information
 */
#include <iostream> // debug
#include <string>
#include <memory>
#include <stdexcept>
#include <curl/curl.h>
#include "agent.hh"
#include "station.hh"
#include "data.hh"


agent_c::agent_c ()
{
  curl_global_init (CURL_GLOBAL_DEFAULT);
}


agent_c::~agent_c () {
  curl_global_cleanup ();
}

size_t
agent_c::curl_callback (
  char *in,
  size_t size,
  size_t nmemb,
  std::string *out
  )
{
  size_t realsize = size * nmemb;
  if (in == NULL || out == NULL) {
    throw std::invalid_argument ("parameters set incorrectly");
    return 0;
  }
  out->append (in, realsize);
  return realsize;
}

int
agent_c::curl_retrieve (
  std::string url,
  std::string *buffer
  )
{
  long status = 0;

  CURL *conn = curl_easy_init ();
  if (conn == NULL) {
    status = -1;
  }
  curl_easy_setopt (conn, CURLOPT_URL, url.c_str ());
  curl_easy_setopt (conn, CURLOPT_TIMEOUT, AGENT_TIMEOUT);
  curl_easy_setopt (conn, CURLOPT_WRITEFUNCTION, curl_callback);
  curl_easy_setopt (conn, CURLOPT_WRITEDATA, buffer);
  curl_easy_setopt (conn, CURLOPT_USERAGENT, AGENT_USERAGENT);

  curl_easy_perform (conn); // fetch data
  curl_easy_getinfo (conn, CURLINFO_RESPONSE_CODE, &status); // fetch status code
  curl_easy_cleanup (conn); // cleanup

  return status == HTTP_OK ? 0 : status;
}

int
agent_c::idfetcher_execute (
  station_c station,
  data_c *data
  )
{
  int status = 0;
  std::string url;

  if (data == NULL) {
    return -1;
  }

  // build an url
  url = std::string (AGENT_IDFETCHER_URI_PREFIX + station.get_name() +
    AGENT_IDFETCHER_URI_SUFFIX);

  status = agent_c::curl_retrieve (url, &data->buffer);
  return status;
}

int
agent_c::datafetcher_execute (
  station_c station,
  data_c *data
  )
{
  long        status = 0;
  std::string url;

  if (data == NULL || station.get_number () == 0) {
    return -1;
  }

  // build an url
  url = std::string (AGENT_DATAFETCHER_METEO_URI);
  if (station.get_flavour () == hydro) {
    url = std::string (AGENT_DATAFETCHER_HYDRO_URI);
  }
  url += std::to_string (station.get_number ());

  status = agent_c::curl_retrieve (url, &data->buffer);
  return status;
}
