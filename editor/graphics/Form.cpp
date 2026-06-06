#include "Form.h"

namespace oly::editor
{
	Form::Form(ImGuiTableFlags table_flags, ImGuiTableColumnFlags value_column_flags, ImGuiTableColumnFlags key_column_flags)
		: _table_flags(table_flags), _value_column_flags(value_column_flags), _key_column_flags(key_column_flags)
	{
		ImGui::PushID(this);
		_draw_content = BeginTable();
	}

	Form::Form(const Form& other)
		: _table_flags(other._table_flags), _value_column_flags(other._value_column_flags), _key_column_flags(other._key_column_flags)
	{
		ImGui::PushID(this);
		_draw_content = BeginTable();
	}

	Form::~Form()
	{
		if (_draw_content)
			ImGui::EndTable();

		ImGui::PopID();
	}

	Form::operator bool() const
	{
		return _draw_content;
	}

	bool Form::BeginTable() const
	{
		if (ImGui::BeginTable("", 2, _table_flags))
		{
			ImGui::TableSetupColumn("", _key_column_flags);
			ImGui::TableSetupColumn("", _value_column_flags);
			return true;
		}
		else
			return false;
	}

	Form::PauseImpl::PauseImpl(const Form& form)
		: _form(form)
	{
		ImGui::EndTable();
		ImGui::PopID();
	}

	Form::PauseImpl::~PauseImpl()
	{
		ImGui::PushID(&_form);
		_form.BeginTable();
	}

	Form::PauseImpl::operator bool() const
	{
		return _form;
	}

	Form::PauseImpl Form::Pause() const
	{
		return PauseImpl(*this);
	}

	// TODO v8 fix flicker in collapsing section

	Form::CollapsingSection::CollapsingSection(const Form& form, const char* label, bool start_open)
		: _pause(form.Pause())
	{
		if (_pause)
		{
			if (start_open)
				ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			_visible = ImGui::TreeNode(label);
			if (_visible)
				_form = std::make_unique<Form>(form);
		}
	}

	Form::CollapsingSection::~CollapsingSection()
	{
		if (_visible)
		{
			_form.reset();
			ImGui::TreePop();
		}
	}

	Form::CollapsingSection::operator bool() const
	{
		return _visible;
	}

	Form::CollapsingSection Form::Collapse(const char* label, bool start_open) const
	{
		return CollapsingSection(*this, label, start_open);
	}
}
