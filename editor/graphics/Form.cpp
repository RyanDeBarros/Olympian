#include "Form.h"

namespace oly::editor
{
	Form::Form(ImGuiTableFlags table_flags, ImGuiTableColumnFlags value_column_flags, ImGuiTableColumnFlags key_column_flags)
		: _table_flags(table_flags), _value_column_flags(value_column_flags), _key_column_flags(key_column_flags)
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
		ImGui::PushID(_form);
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

	Form::CollapsingSection::CollapsingSection(const Form& form, const char* label)
	{
		if (auto pause = form.Pause())
			_visible = ImGui::TreeNode(label);
	}

	Form::CollapsingSection::~CollapsingSection()
	{
		if (_visible)
			ImGui::TreePop();
	}

	Form::CollapsingSection::operator bool() const
	{
		return _visible;
	}

	Form::CollapsingSection Form::Collapse(const char* label) const
	{
		return CollapsingSection(*this, label);
	}
}
