#include "utils.h"
#include "exec.h"

#include <cstdio>
#include <cstdlib>
#include <getopt.h>

static char *core_name = nullptr;
static char *diff_core_name = nullptr;
static char *log_file = nullptr;
static char *img_file = nullptr;

static bool gdb_remote = false;

static void parse_args(const int argc, char *argv[]) {
  const option table[] = {
    {"core"   , required_argument, nullptr, 'c'},
    {"diff"   , required_argument, nullptr, 'd'},
    {"gdb"    , no_argument      , nullptr, 'g'},
    {"log"    , required_argument, nullptr, 'l'},
    {"help"   , no_argument      , nullptr, 'h'},
    {nullptr  , 0                , nullptr,  0 },
  };
  int o;
  while ((o = getopt_long(argc, argv, "-cdgl:h", table, nullptr)) != -1) {
    switch (o) {
      case 'c': core_name = optarg; break;
      case 'd': diff_core_name = optarg; break;
      case 'g': gdb_remote = true; break;
      case 'l': log_file = optarg; break;
      case 1: img_file = optarg; return;
      default:
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-c,--core               specify core name\n");
        printf("\t-d,--diff               specify diff core name\n");
        printf("\t-g,--gdb                run gdb remote server\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf("\n");
        exit(0);
    }
  }
}

int main(const int argc, char *argv[]) {
  parse_args(argc, argv);

  init_log(log_file);
  Assert(core_name, "No core specified!");
  dut = new Core(core_name);
  Log("Core loaded: " ANSI_FMT("%s", ANSI_FG_CYAN), core_name);

  dut->core_init(img_file, false);
  if (diff_core_name) difftest_init(diff_core_name, img_file);

  return cpu_exec();
}
