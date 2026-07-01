#pragma once

#include "gui/scopes/IDScope.h"

#include <imgui.h>

#include <memory>

namespace oly::editor
{
	class Form
	{
		bool _valid = true;
		bool _draw_content = false;
		int _id_counter = 0;
		gui::IDScope _scope;

	public:
		Form();
		Form(const Form&) = delete;
		Form(Form&&) noexcept;
		~Form();
		Form& operator=(Form&&) noexcept = delete;

		static Form* ActiveForm();
		static bool ValidActiveForm();

		operator bool() const;

	private:
		friend class FormPause;
		void BeginTable();
		void EndTable();
	};

	// NOTE: After pausing a form, make sure to check ValidActiveForm() before continuing to draw properties
	class FormPause
	{
		Form* _form = nullptr;
		bool _was_drawing_content = false;

	public:
		FormPause();
		FormPause(const FormPause&) = delete;
		FormPause(FormPause&&) = delete;
		~FormPause();

		operator bool() const;
	};
}
