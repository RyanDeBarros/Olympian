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
    oly::editor::Editor editor;
    while (!editor.ShouldClose())
        editor.Tick();
}
