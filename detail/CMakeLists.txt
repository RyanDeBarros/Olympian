cmake_minimum_required(VERSION 3.20)
project(OlympianDetail VERSION 1.0.0 LANGUAGES CXX)

include(FetchContent)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()


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

target_link_libraries(OlympianDetail PRIVATE
    tomlplusplus::tomlplusplus
)

target_include_directories(OlympianDetail PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Add subdirectories
add_subdirectory(assets)
add_subdirectory(definitions)
