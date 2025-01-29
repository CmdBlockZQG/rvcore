#ifndef __TESTBENCH_CORE_H__
#define __TESTBENCH_CORE_H__

class Core {
  public:
    void (*init)(const char *img_file, bool mute);
    int (*step)();
    void (*exit)();

    explicit Core(const char *name);
};

#endif
