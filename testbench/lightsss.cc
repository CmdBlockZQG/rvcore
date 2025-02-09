#include "utils.h"
#include "lightsss.h"
#include "exec.h"

#include <cstdlib>
#include <queue>
#include <unistd.h>
#include <sys/wait.h>
#include <csignal>

struct ss_t {
  int pipe_fd[2]; // 管道，0读1写
  pid_t pid; // 子进程pid
};

static std::queue<ss_t> ss_q;

static void delete_ss(const ss_t &ss, const bool do_kill = true) {
  if (do_kill) kill(ss.pid, SIGKILL);
  waitpid(ss.pid, nullptr, 0);
  // 父进程已经关闭过读端，只需关闭写端即可
  close(ss.pipe_fd[1]);
}

/* 正常仿真时，fork得到快照
父进程返回1，继续照常执行并定期fork
子进程返回0，仿真已经出错，不再fork */
int lightsss_fork() {
  ss_t ss{};
  assert(pipe(ss.pipe_fd) == 0);
  const pid_t pid = fork();
  assert(pid >= 0);

  // ---------- 父进程 ----------
  if (pid) {
    // 关闭读端
    close(ss.pipe_fd[0]);
    // 将新快照加入队列
    ss.pid = pid;
    ss_q.push(ss);
    // 保持队列大小
    while (ss_q.size() > SLOT_SIZE) {
      delete_ss(ss_q.front());
      ss_q.pop();
    }
    return 1;
  }
  // ---------- 子进程 ----------
  // 关闭写端
  close(ss.pipe_fd[1]);
  // 读取父进程的pipe_fd[0]，阻塞等待
  char buffer[1];
  read(ss.pipe_fd[0], buffer, 1);
  // 父进程唤醒，关闭读端
  close(ss.pipe_fd[0]);
  return 0;
}

/* 父进程仿真出错后，调用该函数唤醒子进程
等待子进程执行完成，回收所有子进程 */
void lightsss_wake() {
  Log("LightSSS awake");
  const ss_t x = ss_q.front(); ss_q.pop();
  write(x.pipe_fd[1], "x", 1); // 唤醒子进程
  delete_ss(x, false); // 等待子进程运行完成
  lightsss_exit(); // 回收其他子进程
}

/* 父进程仿真正常结束，回收所有子进程 */
void lightsss_exit() {
  while (!ss_q.empty()) {
    delete_ss(ss_q.front());
    ss_q.pop();
  }
}

