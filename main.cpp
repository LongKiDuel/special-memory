#include <cstdint>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class File_id_finder {
public:
  std::optional<std::string_view> get_path(uint64_t id) const {
    if (id >= storage_.size()) {
      return storage_[id];
    }
    return {};
  }
  // auto create new id, when it is a new file.
  uint64_t get_id(std::string_view file_name) {
    if (auto iter = dictionary_.find(file_name); iter == dictionary_.end()) {
      auto id = storage_.size();
      storage_.push_back(std::string{file_name});
      return id;
    } else {
      return iter->second;
    }
  }
  void build(std::vector<std::string> file_list) {
    storage_ = std::move(file_list);
    dictionary_.clear();

    uint64_t i{};
    for (auto &file : storage_) {
      dictionary_[file] = i;
      i++;
    }
  }

private:
  std::vector<std::string> storage_;
  std::unordered_map<std::string_view, uint64_t> dictionary_;
};

class File_path {
  uint64_t file_id_{};

  static File_id_finder finder;
  explicit File_path(uint64_t id) { file_id_ = id; }

public:
  File_path(std::string_view file_path) { file_id_ = finder.get_id(file_path); }

  uint64_t get_id() const { return file_id_; }
  std::string_view get_file_name() { return finder.get_path(file_id_).value(); }

  auto operator<=>(const File_path &rhs) { return file_id_ <=> rhs.file_id_; }
};
class Reverse_search {
public:
  bool task_a_file(std::string file_path) { return false; }

private:
  std::unordered_map<std::string, std::set<File_path>> words_to_files_;
};

int main() { return 0; }
