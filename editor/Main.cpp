#include "core/editor/Editor.h"

int main()
{
    oly::editor::Editor editor;
    while (!editor.ShouldClose())
        editor.Tick();
}
