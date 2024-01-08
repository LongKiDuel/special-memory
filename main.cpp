#include <iostream>
#include <random>
#include <string>
// return 32 sized string of [a-z][A-Z][0-9]
std::string random_str() {
  const char charset[] = "abcdefghijklmnopqrstuvwxyz"
                         "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                         "0123456789";
  const size_t charset_size = sizeof(charset) - 1;
  const size_t string_length = 32; // Desired length of the string

  std::string result;
  result.reserve(string_length);

  // Create a random device and a generator
  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_int_distribution<> distribution(0, charset_size - 1);

  // Generate the random string
  for (size_t i = 0; i < string_length; ++i) {
    result += charset[distribution(generator)];
  }

  return result;
}

int main() {
  std::string randomString = random_str();
  std::cout << "Random String: " << randomString << std::endl;
  return 0;
}