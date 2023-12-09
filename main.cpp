#include <cstdint>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class File_id_finder {
public:
  std::optional<std::string_view> get_path(uint64_t id) {}
  // auto create new id, when it is a new file.
  uint64_t get_id(std::string_view file_name) {}
};

class File_path {
  uint64_t file_id_{};

public:
  auto operator<=>(const File_path &rhs) { return file_id_ <=> rhs.file_id_; }
};
class Reverse_search {
public:
  bool task_a_file(std::string file_path) { return false; }

private:
  std::unordered_map<std::string, std::set<File_path>> words_to_files_;
};

int main() { return 0; }
