#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <istream>
#include <memory>
#include <streambuf>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

class fdstreambuf : public std::streambuf {
public:
  fdstreambuf(int fd) : fd(fd) {
    // Set up the get area
    setg(buffer, buffer, buffer);
  }

protected:
  virtual int underflow() override {
    if (gptr() < egptr()) {
      return std::char_traits<char>::to_int_type(*gptr());
    }

    int numRead = read(fd, buffer, bufferSize);
    if (numRead <= 0) {
      return EOF;
    }

    setg(buffer, buffer, buffer + numRead);
    return std::char_traits<char>::to_int_type(*gptr());
  }

private:
  static const int bufferSize = 1024;
  char buffer[bufferSize]{};
  int fd{};
};

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
      close(pipefd[0]);
      {
        std::vector<char *> argv;
        argv.push_back(command.data());
        for (auto &arg : args) {
          argv.push_back(arg.data());
        }
        argv.push_back(nullptr);
        execvp(command.data(), argv.data()); // v for array, p for $PATH
      }

      _exit(EXIT_FAILURE);
    } else { // Parent process
      close(pipefd[1]);
      return true;
    }
  }

  // Methods for reading/writing to the process's stdin, stdout, and stderr
  // ...
  // bool writeToStdIn(const std::string &data) {
  //   return write(pipefd[1], data.c_str(), data.size()) != -1;
  // }

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
  int get_stdout_fd() { return pipefd[0]; }
};

int main() {
  std::string str;
  {
    UnixProcess git{};
    git.create("git", {"status"});
    auto fd = git.get_stdout_fd();
    // file.ptr = fdopen(fd, "r");
    // if (!file.get()) {
    //   std::cerr << "Failed to open file descriptor" << std::endl;
    //   return 1;
    // }
    // char bufa[2455]{};
    // fread(bufa, 1, sizeof bufa, file.get());
    // std::cout << bufa << std::endl;

    fdstreambuf buf{fd};
    std::istream is{&buf};
    std::string line;
    while (std::getline(is, line)) {
      str += line;
      str += '\n';
    }
  }
  std::cout << "\033[1;32m" << str << "\033[0m" << std::endl;
}
