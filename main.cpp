#include <array>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <istream>
#include <iterator>
#include <memory>
#include <ostream>
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
  int stdout_fd[2]; // read form 0, write to 1.
  pid_t pid;

public:
  UnixProcess() {
    if (pipe(stdout_fd) == -1) {
      perror("pipe");
      exit(EXIT_FAILURE);
    }
  }

  ~UnixProcess() {
    close(stdout_fd[0]);
    close(stdout_fd[1]);
    wait(NULL); // Wait for child
  }

  bool create(std::string command, std::vector<std::string> args) {
    pid = fork();
    if (pid == -1) {
      perror("fork");
      exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Child process
      dup2(stdout_fd[1], STDOUT_FILENO);
      close(stdout_fd[1]);
      close(stdout_fd[0]);
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
      close(stdout_fd[1]);
      return true;
    }
  }

  std::string readFromStdOut() {
    char buffer[4096];
    ssize_t readBytes = read(get_stdout_fd(), buffer, sizeof(buffer));
    if (readBytes > 0) {
      return std::string(buffer, readBytes);
    }
    return "";
  }

  int get_stdout_fd() { return stdout_fd[0]; }
};

class MyStreambuf : public std::streambuf {
public:
  MyStreambuf() {
    setp(buf_.begin(), buf_.end()); // Set the output buffer
  }

protected:
  // This function is called when the buffer is full
  int_type overflow(int_type ch) override {
    if (ch != traits_type::eof()) {
      auto size = this->epptr() - this->pbase();
      auto str = std::string{pbase(), epptr()};
      memset(this->pbase(), 0, size);
      setp(pbase(), epptr());
      return 0;
    }
    return traits_type::eof();
  }

  //   // This function is used to write a sequence of characters
  //   std::streamsize xsputn(const char_type *s, std::streamsize n) override {
  //     // Implement your custom logic to write 'n' characters from 's'
  //     // to your output device or storage
  //     // You can use traits_type::copy or any other method to copy the data
  //     // Make sure to update the buffer pointers accordingly
  //     return n; // Return the number of characters written
  //   }

private:
  std::array<char, 64> buf_{};
};
void stream_test() {
  char cbuf[4]{};
  MyStreambuf buf{};
  std::ostream os{&buf};
  os << 'a';
  os << "helo wod";
  for (int i = 0; i < 10; i++) {
    os << (char)('0' + i);
  }
  os.flush();
}
int main() {
  stream_test();
  std::string str;
  {
    UnixProcess git{};
    git.create("git", {"status"});
    auto fd = git.get_stdout_fd();

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
