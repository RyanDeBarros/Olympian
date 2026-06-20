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
		if (ImGui::BeginTable("", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingStretchProp))
		{
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
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

	Form::PauseImpl::PauseImpl(Form& form)
		: _form(form)
	{
		_form.EndTable();
	}

	Form::PauseImpl::~PauseImpl()
	{
		_form.BeginTable();
	}

	Form::PauseImpl::operator bool() const
	{
		return true;
	}

	Form::PauseImpl Form::Pause()
	{
		return PauseImpl(*this);
	}
}
