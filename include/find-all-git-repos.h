#pragma once

#include <filesystem>
#include <set>
#include <unordered_set>
#include <vector>
inline std::vector<std::filesystem::path>
find_all_git_repo(std::filesystem::path root) {
  std::set<std::filesystem::path> pathes;

  std::filesystem::recursive_directory_iterator start{
      root, std::filesystem::directory_options::skip_permission_denied},
      end;

  while (start != end) {
    const auto &path = start->path();
    if (std::filesystem::is_directory(path)) {
      if (path.filename() == ".git") {
        auto absolute_path = std::filesystem::absolute(start->path());

        pathes.insert(absolute_path.parent_path());
      }
    }
    start++;
  }
  return {pathes.begin(), pathes.end()};
}
