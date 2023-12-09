#include <cstring>
#include <iostream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

class UnixProcess {
private:
  int pipefd[2];
  pid_t pid;

public:
  UnixProcess() {
    if (pipe(pipefd) == -1) {
      perror("pipe");
      exit(EXIT_FAILURE);
    }
  }

  ~UnixProcess() {
    close(pipefd[0]);
    close(pipefd[1]);
    wait(NULL); // Wait for child
  }

  bool create(std::string command, std::vector<std::string> args) {
    pid = fork();
    if (pid == -1) {
      perror("fork");
      exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Child process
      dup2(pipefd[1], STDOUT_FILENO);
      close(pipefd[1]);
      dup2(pipefd[0], STDIN_FILENO);
      close(pipefd[0]);
      // execlp(command.c_str(), command.c_str(), (char *)NULL);
      {
        std::vector<char *> argv;
        argv.push_back(command.data());
        for (auto &arg : args) {
          argv.push_back(arg.data());
        }
        argv.push_back(nullptr);
        execvp(command.data(), argv.data());
      }

      _exit(EXIT_FAILURE);
    } else { // Parent process
      return true;
    }
  }

  // Methods for reading/writing to the process's stdin, stdout, and stderr
  // ...
  bool writeToStdIn(const std::string &data) {
    return write(pipefd[1], data.c_str(), data.size()) != -1;
  }

  std::string readFromStdOut() {
    char buffer[4096];
    ssize_t readBytes = read(pipefd[0], buffer, sizeof(buffer));
    if (readBytes > 0) {
      return std::string(buffer, readBytes);
    }
    return "";
  }

  std::string readFromStdErr() {
    // This method needs a separate pair of pipes for stderr
    // It's similar to readFromStdOut but using the stderr pipe
    // ...
    return "";
  }
};

int main() {
  std::string str;
  {
    UnixProcess git{};
    git.create("git", {"status"});
    str = git.readFromStdOut();
  }
  std::cout << "\033[1;32m" << str << "\033[0m" << std::endl;
}
