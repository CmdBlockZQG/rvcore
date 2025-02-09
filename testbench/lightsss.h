#ifndef __TESTBENCH_LIGHTSSS_H__
#define __TESTBENCH_LIGHTSSS_H__

int lightsss_fork();
void lightsss_wake();
void lightsss_exit();

constexpr int LIGHTSSS_INTERVAL = 1000000;
constexpr static int SLOT_SIZE = 2;

#endif
