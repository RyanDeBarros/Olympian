cmake_minimum_required(VERSION 3.20)
project(OlympianEngine VERSION 1.0.0 LANGUAGES CXX)

include(FetchContent)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()


# ====================
#      Variables
# ====================

set(ASSET_KEY_SIZE 8)


# ====================
# Run automatic tools
# ====================

find_package(Python3 REQUIRED COMPONENTS Interpreter)

function(prebuild script)
    set(options)
    set(one_value_args)
    set(multi_value_args ARGS)

    cmake_parse_arguments(PB "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    execute_process(
        COMMAND ${Python3_EXECUTABLE} -m definitions.${script} ${PB_ARGS}
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULT_VARIABLE result
    )

    if (NOT result EQUAL 0)
        message(FATAL_ERROR "definitions/${script} failed with code ${result}")
    endif()
endfunction()

prebuild("EnumCodeGen")
prebuild("KeysCodeGen" ARGS "--key-size=${ASSET_KEY_SIZE}")


# ====================
#       OpenGL
# ====================

find_package(OpenGL REQUIRED)


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
#         GLM
# ====================

set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG	       1.0.1
)

FetchContent_MakeAvailable(glm)


# ====================
#         STB
# ====================

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        31c1ad37456438565541f4919958214b6e762fb4
)

FetchContent_MakeAvailable(stb)


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
#       Nano SVG
# ====================

FetchContent_Declare(
    nanosvg
    GIT_REPOSITORY https://github.com/memononen/nanosvg.git
    GIT_TAG        48120e91e64b2f409ed600cdfd6d790a49ba11ab
)

FetchContent_MakeAvailable(nanosvg)


# ====================
#       Quartic
# ====================

FetchContent_Declare(
	quartic
	GIT_REPOSITORY https://github.com/sasamil/Quartic.git
	GIT_TAG 45bf9bb8a28b3e214bc2a9c5a59f3db61c8b6b17
)

FetchContent_MakeAvailable(quartic)
add_library(quartic STATIC ${quartic_SOURCE_DIR}/quartic.cpp)


# ====================
#     Build Engine
# ====================

# Add engine sources
file(GLOB_RECURSE ENGINE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.inl")

# TODO v10 licensing may require engine to be dynamically linked?
# Create the engine as a static library
add_library(OlympianEngine STATIC ${ENGINE_FILES})

# Engine compile definitions
target_compile_definitions(OlympianEngine PUBLIC
	GLEW_STATIC
    OLYMPIAN_ENGINE_ABS_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}\"
    OLYMPIAN_ENGINE_ASSET_KEY_SIZE=${ASSET_KEY_SIZE}
)

# Engine compile options (MSVC specific)
if (MSVC)
	target_compile_options(OlympianEngine PUBLIC /Zc:__cplusplus)
endif()


# Include directories
target_include_directories(OlympianEngine PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
    ${stb_SOURCE_DIR}
	${nanosvg_SOURCE_DIR}/src
    ${quartic_SOURCE_DIR}
)

# Link directories
target_link_directories(OlympianEngine PUBLIC
	${stb_SOURCE_DIR}
	${nanosvg_SOURCE_DIR}/src
)

# Link engine dependencies
target_link_libraries(OlympianEngine PUBLIC
	OpenGL::GL
	libglew_static
	glfw
    glm::glm
    tomlplusplus::tomlplusplus
    quartic
)


# TODO v10 install OlympianEngine so as to use find_package(Olympian) in project cmake
