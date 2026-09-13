cmake_minimum_required(VERSION 3.10)

project(chip8)

find_package(SDL3 REQUIRED CONFIG)

add_executable(chip8 src/chip8.c)

target_link_libraries(chip8 PRIVATE SDL3::SDL3 m)
