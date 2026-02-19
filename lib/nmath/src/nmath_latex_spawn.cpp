//
// Created by felix on 2/19/26.
//

#include "nmath_internals.hpp"
#include <fcntl.h>
#include <spawn.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

extern "C" char **environ;

bool nmath_spawn_pdflatex(const std::vector<std::string> &args)
{
  pid_t pid;
  posix_spawn_file_actions_t actions;
  posix_spawn_file_actions_init(&actions);

  posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO, "/dev/null",
                                   O_WRONLY, 0644);
  posix_spawn_file_actions_adddup2(&actions, STDOUT_FILENO, STDERR_FILENO);

  std::vector<char *> argv;
  for (const auto &arg : args)
  {
    argv.push_back(const_cast<char *>(arg.c_str()));
  }
  argv.push_back(nullptr);

  int status =
      posix_spawnp(&pid, argv[0], &actions, nullptr, argv.data(),
        environ);

  posix_spawn_file_actions_destroy(&actions);

  if (status == 0)
  {
    if (waitpid(pid, &status, 0) != -1)
    {
      return WIFEXITED(status) && WEXITSTATUS(status) == 0;
    }
  }

  return false;
}