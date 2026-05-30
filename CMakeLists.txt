cmake_minimum_required(VERSION 3.20)
project(Olympian)

# Set C++ standard globally
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Variables
set(ASSET_KEY_SIZE 8)

add_subdirectory(engine)
add_subdirectory(definitions)
# TODO v10 remove Tester - this is only since Tester is in same project. When project is in other repo, remove this
add_subdirectory(Tester)
