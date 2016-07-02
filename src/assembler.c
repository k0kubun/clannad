#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <llvm-c/Core.h>
#include <llvm-c/BitWriter.h>

enum { FD_READ, FD_WRITE };

void
assemble(LLVMModuleRef mod, char *outfile)
{
  int fds[2];
  LLVMWriteBitcodeToFile(mod, "main.bc");

  if (pipe(fds) < 0) perror("pipe failed");

  pid_t pid = fork();
  if (pid < 0) perror("fork failed");
  if (pid == 0) { // child
    close(fds[FD_WRITE]); // never use

    close(STDIN_FILENO);
    dup2(fds[FD_READ], STDIN_FILENO);
    execlp("llc", "llc", NULL);
    perror("execlp failed");
  } else { // parent
    close(fds[FD_READ]); // never use

    LLVMWriteBitcodeToFD(mod, fds[FD_WRITE], 0, 0);
    close(fds[FD_WRITE]);

    int status;
    waitpid(pid, &status, 0);
    if (status < 0)
      perror("llc failed");
  }
}
