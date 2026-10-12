cmake_minimum_required(VERSION 3.20)
project(Olympian)

# Set C++ standard globally
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(detail)
add_subdirectory(engine)
add_subdirectory(editor)
add_subdirectory(Tester)
