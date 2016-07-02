#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <llvm-c/Core.h>
#include <llvm-c/BitWriter.h>

enum { PIPE_OUT, PIPE_IN, PIPE_LEN };

int
exec_llc(LLVMModuleRef mod)
{
  int llcin[PIPE_LEN], llcout[PIPE_LEN];

  if (pipe(llcin) < 0) perror("pipe failed");
  if (pipe(llcout) < 0) perror("pipe failed");

  pid_t pid = fork();
  if (pid < 0) perror("fork failed");

  if (pid == 0) { // child
    close(llcin[PIPE_IN]); // never use
    close(llcout[PIPE_OUT]); // never use

    close(STDIN_FILENO);
    close(STDOUT_FILENO);

    dup2(llcin[PIPE_OUT], STDIN_FILENO);
    dup2(llcout[PIPE_IN], STDOUT_FILENO);
    execlp("llc", "llc", NULL);
    perror("execlp failed");
    exit(1);
  } else { // parent
    close(llcin[PIPE_OUT]); // never use
    close(llcout[PIPE_IN]); // never use

    LLVMWriteBitcodeToFD(mod, llcin[PIPE_IN], 0, 0);
    close(llcin[PIPE_IN]);

    int status;
    waitpid(pid, &status, 0);
    if (status < 0)
      perror("llc failed");

    return llcout[PIPE_OUT];
  }
}

void
exec_as(int in_fd, char *outfile)
{
  pid_t pid = fork();
  if (pid < 0) perror("fork failed");

  if (pid == 0) { // child
    close(STDIN_FILENO);
    dup2(in_fd, STDIN_FILENO);

    execlp("as", "as", "-o", outfile, NULL);
    perror("execlp failed");
  } else { // parent
    close(in_fd); // never use

    int status;
    waitpid(pid, &status, 0);
    if (status < 0)
      perror("as failed");
  }
}

void
assemble(LLVMModuleRef mod, char *outfile)
{
  int llc_outfd = exec_llc(mod);
  exec_as(llc_outfd, outfile);
}
