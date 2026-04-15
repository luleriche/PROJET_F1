cmake_minimum_required(VERSION 3.18)
project(monExecutable)

add_compile_options(-Wall)

find_package(SFML COMPONENTS graphics window system)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(${PROJECT_NAME}.out main.cpp)

target_link_libraries(${PROJECT_NAME}.out PRIVATE sfml-graphics sfml-window sfml-system)