#include "utils.h"
#include "cpu.h"

#include <cstdio>
#include <cstdlib>
#include <getopt.h>

static char *core_name = nullptr;
static char *log_file = nullptr;
static char *img_file = nullptr;

static bool gdb_remote = false;

static void parse_args(int argc, char *argv[]) {
  const struct option table[] = {
    {"core"   , required_argument, nullptr, 'c'},
    {"gdb"    , no_argument      , nullptr, 'g'},
    {"log"    , required_argument, nullptr, 'l'},
    {"help"   , no_argument      , nullptr, 'h'},
    {nullptr  , 0                , nullptr,  0 },
  };
  int o;
  while ((o = getopt_long(argc, argv, "-cgl:h", table, nullptr)) != -1) {
    switch (o) {
      case 'c': core_name = optarg; break;
      case 'g': gdb_remote = true; break;
      case 'l': log_file = optarg; break;
      case 1: img_file = optarg; return;
      default:
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-c,--core               specify core name\n");
        printf("\t-g,--gdb                run gdb remote server\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf("\n");
        exit(0);
    }
  }
}

int main(int argc, char *argv[]) {
  parse_args(argc, argv);

  init_log(log_file);
  Assert(core_name, "No core specified!");
  dut = std::make_unique<Core>(core_name);
  
  dut->init(img_file, false);

  return cpu_exec();
}
