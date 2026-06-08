#pragma once

#include <imgui.h>

#include <memory>

namespace oly::editor
{
	class Form
	{
		bool _draw_content = false;
		int _id_counter = 0;

	public:
		Form();
		Form(const Form&) = delete;
		Form(Form&&) noexcept;
		~Form();
		Form& operator=(Form&&) noexcept = delete;

		static Form* ActiveForm();

		operator bool() const;

	private:
		void BeginTable();
		void EndTable();

	public:
		friend class PauseImpl;
		class PauseImpl
		{
			Form& _form;

		public:
			PauseImpl(Form& form);
			PauseImpl(const PauseImpl&) = delete;
			PauseImpl(PauseImpl&&) = delete;
			~PauseImpl();

			operator bool() const;
		};

		PauseImpl Pause();
	};
}
