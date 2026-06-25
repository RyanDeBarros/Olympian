cmake_minimum_required(VERSION 3.20)
project(OlympianEditor VERSION 1.0.0 LANGUAGES CXX)

include(FetchContent)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()


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
#        ImGui
# ====================
FetchContent_Declare(
	imgui
	GIT_REPOSITORY https://github.com/ocornut/imgui.git
	GIT_TAG        v1.92.7-docking
)

FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
	${imgui_SOURCE_DIR}/imgui.cpp
	${imgui_SOURCE_DIR}/imgui_draw.cpp
	${imgui_SOURCE_DIR}/imgui_tables.cpp
	${imgui_SOURCE_DIR}/imgui_widgets.cpp
)

target_include_directories(imgui PUBLIC
	${imgui_SOURCE_DIR}
)


# ====================
#  ImGui File Dialog
# ====================

FetchContent_Declare(
    imgui_file_dialog
    GIT_REPOSITORY https://github.com/aiekick/ImGuiFileDialog
    GIT_TAG        v0.6.8
)

FetchContent_MakeAvailable(imgui_file_dialog)

add_library(imgui_file_dialog STATIC
    ${imgui_file_dialog_SOURCE_DIR}/ImGuiFileDialog.cpp
)

target_include_directories(imgui_file_dialog PUBLIC
    ${imgui_file_dialog_SOURCE_DIR}
)

target_link_libraries(imgui_file_dialog PUBLIC
    imgui
)


# ====================
#    ImGui backends
# ====================
add_library(imgui_backend STATIC
	${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
	${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui_backend PUBLIC
	${imgui_SOURCE_DIR}
	${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui_backend PUBLIC
	imgui
	glfw
	OpenGL::GL
)


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
#     Build Editor
# ====================

add_executable(OlympianEditor)

# Editor compile options (MSVC specific)
if (MSVC)
	target_compile_options(OlympianEditor PUBLIC /Zc:__cplusplus)
endif()

# Editor compile definitions
target_compile_definitions(OlympianEditor PUBLIC
	GLEW_STATIC
	IMGUI_DEFINE_MATH_OPERATORS
)

target_link_libraries(OlympianEditor PRIVATE
    imgui
    imgui_backend
    imgui_file_dialog
    glm::glm
	libglew_static
    tomlplusplus::tomlplusplus
)

target_link_libraries(OlympianEditor PUBLIC OlympianDetail)

target_include_directories(OlympianEditor PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${stb_SOURCE_DIR}
	${nanosvg_SOURCE_DIR}/src
)

target_sources(OlympianEditor PRIVATE
    Main.cpp
)

# Add subdirectories
add_subdirectory(core)
add_subdirectory(desc)
add_subdirectory(documents)
add_subdirectory(external)
add_subdirectory(gui)
add_subdirectory(panels)

add_custom_command(TARGET OlympianEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/res
    $<TARGET_FILE_DIR:OlympianEditor>/res
)
