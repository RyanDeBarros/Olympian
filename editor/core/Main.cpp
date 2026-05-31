#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "core/Editor.h"
#include "core/Logger.h"
#include "core/ShortcutManager.h"

#include <sstream>

static void glfw_error_callback(int error, const char* description)
{
    std::stringstream ss;
    ss << "GLFW code " << error << ": " << description;
    oly::editor::Logger::Instance().LogError(ss.str().c_str());
}

static void glfw_drop_callback(GLFWwindow* window, int count, const char** paths)
{
    oly::editor::ShortcutManager::Instance().HandlePathDrop(count, paths);
}

int main()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(1, 1, "Olympian Editor", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetDropCallback(window, glfw_drop_callback);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    int monitor_x, monitor_y;
    glfwGetMonitorPos(monitor, &monitor_x, &monitor_y);
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwSetWindowPos(window, monitor_x + mode->width / 2, monitor_y + mode->height / 2);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    float monitor_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(monitor);
    ImGui::StyleColorsDark();
    ImGui::GetStyle().ScaleAllSizes(monitor_scale);

    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = monitor_scale;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    oly::editor::Editor::Instance().Init(window);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        oly::editor::Editor::Instance().Tick();

        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}
