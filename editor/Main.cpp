#include "core/editor/Editor.h"
#include "core/editor/Logger.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <sstream>

static void glfw_error_callback(int error, const char* description)
{
    std::stringstream ss;
    ss << "GLFW code " << error << ": " << description;
    oly::editor::Logger::LogError(ss.str());
}

int main()
{
    imtk::os_window w(1, 1, "Olympian Editor");
    oly::editor::Editor::Instance().Init(&w);

    while (!glfwWindowShouldClose(w.get()))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        oly::editor::Editor::Instance().Tick();

        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(w.get());
    }

    oly::editor::Editor::Instance().Terminate();
}
