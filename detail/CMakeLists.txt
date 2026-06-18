cmake_minimum_required(VERSION 3.20)
project(OlympianDetail VERSION 1.0.0 LANGUAGES CXX)

include(FetchContent)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()


# ====================
#        GLFW
# ====================

set(GLFW_BUILD_DOCS off CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS off CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES off CACHE BOOL "" FORCE)

FetchContent_Declare(
	glfw
	GIT_REPOSITORY https://github.com/glfw/glfw.git
	GIT_TAG        3.4
)

FetchContent_MakeAvailable(glfw)


# ====================
#        GLEW
# ====================

FetchContent_Declare(
    glew
    GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
    GIT_TAG        glew-cmake-2.2.0
)

FetchContent_MakeAvailable(glew)


# ====================
#        TOML++
# ====================

FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG        v3.4.0
)

FetchContent_MakeAvailable(tomlplusplus)


# ====================
#     Build Detail
# ====================

add_library(OlympianDetail STATIC)

if (MSVC)
	target_compile_options(OlympianDetail PUBLIC /Zc:__cplusplus)
endif()

target_compile_definitions(OlympianDetail PUBLIC
	GLEW_STATIC
)

target_link_libraries(OlympianDetail PRIVATE
	libglew_static
    glfw
    tomlplusplus::tomlplusplus
)

target_include_directories(OlympianDetail PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Add subdirectories
add_subdirectory(assets)
add_subdirectory(definitions)
add_subdirectory(util)
