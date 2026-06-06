#include "Form.h"

namespace oly::editor
{
	Form::Form(ImGuiTableFlags table_flags, ImGuiTableColumnFlags value_column_flags, ImGuiTableColumnFlags key_column_flags)
		: _table_flags(table_flags), _value_column_flags(value_column_flags), _key_column_flags(key_column_flags)
	{
		BeginTable();
	}

	Form::Form(Form&& other) noexcept
		: _draw_content(other._draw_content), _table_flags(other._table_flags), _value_column_flags(other._value_column_flags), _key_column_flags(other._key_column_flags)
	{
		other._draw_content = false;
	}

	Form::~Form()
	{
		if (_draw_content)
			EndTable();
	}

	Form::operator bool() const
	{
		return _draw_content;
	}

	void Form::BeginTable()
	{
		ImGui::PushID(this);
		ImGui::PushID(_id_counter++);
		if (ImGui::BeginTable("", 2, _table_flags))
		{
			ImGui::TableSetupColumn("", _key_column_flags);
			ImGui::TableSetupColumn("", _value_column_flags);
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
		ImGui::PopID();
		ImGui::PopID();
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
