cmake_minimum_required(VERSION 3.20)
project(Olympian)

# Set C++ standard globally
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add subdirectories
add_subdirectory(engine)
# TODO v9 remove Tester - this is only since Tester is in same project. When project is in other repo, remove this
add_subdirectory(Tester)
