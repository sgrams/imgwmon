/*
 * imgwmoncpp 0.1-git
 * Stanislaw Grams <sjg@fmdx.pl>
 *
 * src/commandparser.hh
 * See ../LICENSE for license information
 */
#ifndef IMGWMONCPP_COMMANDPARSER_H
#define IMGWMONCPP_COMMANDPARSER_H
#include <stdexcept>
#include <cstdint>
#include <vector>
#include "types.hh"
#include "station.hh"

class commandparser_c {
  private:
  int               argc;
  char              **argv;
  std::vector<command_t> commands;

  public:
  // constructors
  commandparser_c (int argc, char **argv);

  void set_argc   (int argc);
  int  get_argc   (void);
  void set_argv   (char **argv);
  char **get_argv (void);

  // parser methods
  std::vector<command_t> get_commands (void);
};
#endif // IMGWMONCPP_COMMANDPARSER_H
