#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sqlite_modern_cpp.h>
#include <string>
#include <xxhash.h>
const int part_hash_size = 4096;

XXH64_hash_t calculate_part_hash(const std::string &filepath) {
  std::ifstream file_stream(filepath, std::ios::binary);
  XXH3_state_t *state = XXH3_createState();
  assert(state != NULL && "Out of memory!");
  // Reset the state to start a new hashing session.
  XXH3_64bits_reset(state);

  if (file_stream.is_open()) {
    std::string buffer(part_hash_size, '\0');
    file_stream.read(buffer.data(), buffer.size());

    XXH3_64bits_update(state, buffer.data(), file_stream.gcount());
  }

  // Retrieve the finalized hash. This will not change the state.
  XXH64_hash_t result = XXH3_64bits_digest(state);
  XXH3_freeState(state);
  return result;
}

// Function to calculate the hash of a file using xxHash
XXH64_hash_t calculate_hash(const std::string &filepath) {
  std::ifstream file_stream(filepath, std::ios::binary);
  XXH3_state_t *state = XXH3_createState();
  assert(state != NULL && "Out of memory!");
  // Reset the state to start a new hashing session.
  XXH3_64bits_reset(state);

  if (file_stream.is_open()) {
    std::string buffer(4096, '\0');
    while (file_stream.read(&buffer[0], buffer.size())) {
      XXH3_64bits_update(state, buffer.data(), file_stream.gcount());
    }
  }

  // Retrieve the finalized hash. This will not change the state.
  XXH64_hash_t result = XXH3_64bits_digest(state);
  XXH3_freeState(state);
  return result;
}
#include <fmt/format.h>
#include <span>
std::string to_hex_string(std::span<const uint8_t> buffer) {
  std::string hex_string;
  hex_string.reserve(buffer.size() * 2);

  for (uint8_t byte : buffer) {
    hex_string += fmt::format("{:02x}", byte);
  }

  return hex_string;
}
std::string to_hex_string(std::span<const char> buffer) {
  return to_hex_string(
      std::span<const uint8_t>((const uint8_t *)buffer.data(), buffer.size()));
}

std::string to_hex_string(uint64_t number) {
  return to_hex_string(std::span<const uint8_t>((uint8_t *)&number, 8));
}

const bool skip_symbol_link = true;
bool task_file(sqlite::database &db, std::string filepath) {
  if (std::filesystem::is_directory(filepath)) {
    std::filesystem::directory_iterator begin{
        filepath, std::filesystem::directory_options::skip_permission_denied},
        end;

    while (begin != end) {
      task_file(db, begin->path().string());
      ++begin;
    }
    return true;
  }
  if (skip_symbol_link && std::filesystem::is_symlink(filepath)) {
    return true;
  }
  if (!std::filesystem::is_regular_file(filepath)) {
    return false;
  }
  std::ifstream file(filepath, std::ios::binary);
  if (!file.is_open()) {
    return false;
  }
  // Get file size
  auto file_size = std::filesystem::file_size(filepath);
  // Calculate hashes
  auto part_hash = calculate_part_hash(filepath);
  auto part_hash_str = to_hex_string(part_hash);
  std::optional<std::string> full_hash_str;
  if (file_size <= part_hash_size) {
    full_hash_str = part_hash_str;
  } else {
    if (bool use_full_hash = false) {
      auto full_hash = calculate_hash(filepath);
      full_hash_str = to_hex_string(full_hash);
    }
  }
  auto file_write_time = std::filesystem::last_write_time(filepath);

  // Insert data into the table
  db << "INSERT INTO file_info (filepath, size, full_hash, first_block_hash, "
        "last_write_time) "
        "VALUES (?,?,?,?,?);"
     << filepath << static_cast<long long>(file_size) << full_hash_str
     << part_hash_str << file_write_time.time_since_epoch().count();
  return true;
}
int main(int argc, char **argv) {
  try {
    // Open or create the database
    sqlite::database db("file_info.db");

    // Create the table

    // Use TEXT for two hash field cost ~300 bytes for a row in 64 bits hash hex
    // string. it is same for VARCHAR(32)
    db << "CREATE TABLE IF NOT EXISTS file_info ("
          "filepath TEXT PRIMARY KEY,"
          "size INTEGER,"
          "full_hash TEXT,"
          "first_block_hash TEXT,"
          "last_write_time INTEGER);";
    db << "BEGIN TRANSACTION;";

    // clear old table in test;
    db << "DELETE FROM file_info;";

    std::string path = argc > 1 ? argv[1] : "/home";
    task_file(db, path);

    db << "COMMIT;";

    std::cout << "File information added to the database." << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  return 0;
}
