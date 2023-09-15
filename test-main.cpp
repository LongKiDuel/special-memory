#include "include/find-all-git-repos.h"
#include <cstdlib>
#include <format>
#include <iostream>
int main()
{
    for(auto p: find_all_git_repo(getenv("HOME"))){
        std::cout << std::format("git repo: {} \n",p.string());
    }
}