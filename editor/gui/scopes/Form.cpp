#include "Form.h"

namespace oly::editor
{
	static Form* ACTIVE_FORM = nullptr;

	Form::Form()
	{
		BeginTable();
	}

	Form::Form(Form&& other) noexcept
		: _draw_content(other._draw_content), _id_counter(other._id_counter), _scope(std::move(other._scope))
	{
		other._draw_content = false;
		other._id_counter = 0;

		if (&other == ACTIVE_FORM)
			ACTIVE_FORM = this;
	}

	Form::~Form()
	{
		if (_draw_content)
			EndTable();
	}

	Form* Form::ActiveForm()
	{
		return ACTIVE_FORM;
	}

	Form::operator bool() const
	{
		return _draw_content;
	}

	void Form::BeginTable()
	{
		ACTIVE_FORM = this;

		_scope.Push(&ACTIVE_FORM).Push(_id_counter++);

		// TODO v9.1 these settings should be set up by PropertyGrid
		if (ImGui::BeginTable("", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingStretchProp))
		{
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFrameHeight());
			_draw_content = true;
		}
		else
			_draw_content = false;
	}

	void Form::EndTable()
	{
		if (_draw_content)
			ImGui::EndTable();

		_draw_content = false;
		_scope.PopAll();

		ACTIVE_FORM = nullptr;
	}

	FormPause::FormPause()
		: _form(ACTIVE_FORM)
	{
		if (_form)
			_form->EndTable();
	}

	FormPause::~FormPause()
	{
		if (_form)
			_form->BeginTable();
	}

	FormPause::operator bool() const
	{
		return _form;
	}
}
