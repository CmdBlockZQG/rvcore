#include "common.h"
#include "cpu.h"

#include <cstdlib>
#include <readline/readline.h>
#include <readline/history.h>

static char *rl_gets() {
  static char *line_read = nullptr;

  if (line_read) {
    free(line_read);
    line_read = nullptr;
  }

  line_read = readline("(rvcore) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_help(char *args);

static int cmd_q(char *args) {
  return -1;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static struct {
  const char *name;
  const char *desc;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "q", "Exit rvcore", cmd_q },
  { "c", "Continue the execution of the program", cmd_c },
};

static constexpr int nr_cmd = ARRLEN(cmd_table);

static int cmd_help(char *args) {
  for (int i = 0; i < nr_cmd; i ++) {
    printf("%s - %s\n", cmd_table[i].name, cmd_table[i].desc);
  }
  return 0;
}

void sdb_mainloop(bool batch) {
  if (batch) {
    cmd_c(nullptr);
    return;
  }

  for (char *str; (str = rl_gets()) != nullptr; ) {
    char *str_end = str + strlen(str);

    char *cmd = strtok(str, " ");
    if (cmd == nullptr) continue;

    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) args = nullptr;

    int i;
    for (i = 0; i < nr_cmd; ++i) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) return;
        break;
      }
    }

    if (i == nr_cmd) printf("Unknown command: '%s'\n", cmd);
  }
}
