#pragma once

#include <imtk.hpp>

namespace oly::editor
{
    class DrawDockedWindowImpl
    {
        std::unique_ptr<imtk::window> window_;
        bool request_close_;

    public:
        DrawDockedWindowImpl(std::unique_ptr<imtk::window>&& window, bool request_close);

        bool IsVisible() const;
        bool RequestsClose() const;
    };

    class IPanel
    {
        inline static IPanel* gain_next_ = nullptr;

        bool open_ = false;

    public:
        virtual ~IPanel() = default;

        virtual void InitImpl() = 0;
        virtual void TerminateImpl() {}

        virtual const char* GetTitle() const = 0;
        virtual void Draw() = 0;

    public:
        void Init();
        void Terminate();

        void Open();
        void Close();
        bool IsOpen() const;
        void ToggleOpen();
        void GainFocus();

    protected:
        DrawDockedWindowImpl DrawDockedWindow(ImGuiWindowFlags flags = 0);
    };
}
