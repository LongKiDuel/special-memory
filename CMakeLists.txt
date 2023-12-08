cmake_minimum_required(VERSION 3.23.0)
project(main VERSION 0.1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

add_executable(main main.cpp)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(main PRIVATE fmt::fmt)

find_path(SQLITE_MODERN_CPP_INCLUDE_DIRS "sqlite_modern_cpp.h")
target_include_directories(main PRIVATE ${SQLITE_MODERN_CPP_INCLUDE_DIRS})
target_include_directories(main PRIVATE sqlite::sqlite)
find_package(xxHash CONFIG REQUIRED)
target_link_libraries(main PRIVATE xxHash::xxhash)

find_package(unofficial-sqlite3 CONFIG REQUIRED)
target_link_libraries(main PRIVATE unofficial::sqlite3::sqlite3)

enable_testing()
add_subdirectory(test)

add_subdirectory(benchmark)

install(TARGETS main DESTINATION bin)
# install(TARGETS $<TARGET_RUNTIME_DLLS:main> DESTINATION bin)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_FILE_NAME update_server_package)
include(CPack)

include(GNUInstallDirs)

