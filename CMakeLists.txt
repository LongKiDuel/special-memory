cmake_minimum_required(VERSION 3.23.0)
project(main VERSION 0.1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

find_package(imgui CONFIG REQUIRED)


add_library(imguix)

# Fix it
target_include_directories(imguix PUBLIC include)
target_sources(imguix PRIVATE src/app.cpp src/window.cpp)
target_link_libraries(imguix PUBLIC imgui::imgui)

add_executable(main main.cpp)

find_package(fmt CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

target_link_libraries(imguix PRIVATE Vulkan::Vulkan)

target_link_libraries(main PRIVATE fmt::fmt)
target_link_libraries(main PRIVATE imguix)
find_package(glm CONFIG REQUIRED)
target_link_libraries(main PRIVATE glm::glm)
find_package(imguizmo CONFIG REQUIRED)
target_link_libraries(main PRIVATE imguizmo::imguizmo)

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

