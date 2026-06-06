#pragma once

#include <imgui.h>

#include <memory>

namespace oly::editor
{
	class Form
	{
		bool _draw_content = false;
		int _id_counter = 0;
		ImGuiTableFlags _table_flags;
		ImGuiTableColumnFlags _value_column_flags;
		ImGuiTableColumnFlags _key_column_flags;

	public:
		Form(ImGuiTableFlags table_flags = ImGuiTableFlags_SizingFixedFit,
			ImGuiTableColumnFlags value_column_flags = ImGuiTableColumnFlags_WidthStretch,
			ImGuiTableColumnFlags key_column_flags = ImGuiTableColumnFlags_None);
		Form(const Form&) = delete;
		Form(Form&&) noexcept;
		~Form();
		Form& operator=(Form&&) noexcept = delete;

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
