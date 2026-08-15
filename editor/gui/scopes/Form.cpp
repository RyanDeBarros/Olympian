#include "Form.h"

#include "core/Errors.h"

namespace oly::editor
{
	static Form* ACTIVE_FORM = nullptr;

	// TODO v9.3 move to imtk::prop::form
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
		_scope.push(&ACTIVE_FORM).push(_id_counter++);
		_draw_content = imtk::prop::grid::begin_form(ImGui::GetID("##FormID"));
	}

	void Form::EndTable()
	{
		imtk::prop::grid::end_form(_draw_content);
		_draw_content = false;

		_scope.pop_all();
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
