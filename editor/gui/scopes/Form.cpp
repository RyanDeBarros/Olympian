#include "Form.h"

#include "core/Errors.h"
#include "gui/properties/PropertyGrid.h"

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
		other._valid = false;

		if (&other == ACTIVE_FORM)
			ACTIVE_FORM = this;
	}

	Form::~Form()
	{
		if (_valid)
			EndTable();
	}

	Form* Form::ActiveForm()
	{
		return ACTIVE_FORM;
	}
	
	bool Form::ValidActiveForm()
	{
		return ACTIVE_FORM && *ACTIVE_FORM;
	}

	Form::operator bool() const
	{
		return _valid && _draw_content;
	}

	void Form::BeginTable()
	{
		ACTIVE_FORM = this;
		_scope.Push(&ACTIVE_FORM).Push(_id_counter++);
		_draw_content = gui::PropertyGrid::BeginForm(ImGui::GetID("##FormID"));
	}

	void Form::EndTable()
	{
		gui::PropertyGrid::EndForm(_draw_content);
		_draw_content = false;

		_scope.PopAll();
		ACTIVE_FORM = nullptr;
	}

	FormPause::FormPause()
		: _form(ACTIVE_FORM)
	{
		if (_form)
			_was_drawing_content = _form->_draw_content;

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
		return _form && _form->_valid && _was_drawing_content;
	}
}
