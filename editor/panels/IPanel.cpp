#include "IPanel.h"

namespace oly::editor
{
    DrawDockedWindowImpl::DrawDockedWindowImpl(std::unique_ptr<imtk::window>&& window, bool request_close)
        : window_(std::move(window))
        , request_close_(request_close)
    {
    }

    bool DrawDockedWindowImpl::IsVisible() const
    {
        return window_ && *window_;
    }

    bool DrawDockedWindowImpl::RequestsClose() const
    {
        return request_close_;
    }

    void IPanel::Init()
    {
        InitImpl();
    }

    void IPanel::Terminate()
    {
        TerminateImpl();
    }

    void IPanel::Open()
    {
        open_ = true;
    }

    void IPanel::Close()
    {
        open_ = false;
    }

    bool IPanel::IsOpen() const
    {
        return open_;
    }

    void IPanel::ToggleOpen()
    {
        open_ = !open_;
    }

    void IPanel::GainFocus()
    {
        gain_next_ = this;
    }

    DrawDockedWindowImpl IPanel::DrawDockedWindow(ImGuiWindowFlags flags)
    {
        if (open_)
        {
            auto window = std::make_unique<imtk::window>(GetTitle(), flags, &open_);

            if (gain_next_ == this)
            {
                if (window)
                    ImGui::SetWindowFocus();

                gain_next_ = nullptr;
            }

            return DrawDockedWindowImpl(std::move(window), !open_);
        }
        else
        {
            if (gain_next_ == this)
                gain_next_ = nullptr;

            return DrawDockedWindowImpl(nullptr, false);
        }
    }
}
