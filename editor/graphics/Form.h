#pragma once

#include <imgui.h>

#include <memory>

namespace oly::editor
{
	class Form
	{
		bool _draw_content = false;
		ImGuiTableFlags _table_flags;
		ImGuiTableColumnFlags _value_column_flags;
		ImGuiTableColumnFlags _key_column_flags;

	public:
		Form(ImGuiTableFlags table_flags = ImGuiTableFlags_SizingFixedFit,
			ImGuiTableColumnFlags value_column_flags = ImGuiTableColumnFlags_WidthStretch,
			ImGuiTableColumnFlags key_column_flags = ImGuiTableColumnFlags_None);
		Form(const Form&);
		Form(Form&&) = delete;
		~Form();
		Form& operator=(const Form&) = delete;

		operator bool() const;

	private:
		bool BeginTable() const;

		friend class PauseImpl;
		class PauseImpl
		{
			const Form& _form;

		public:
			PauseImpl(const Form& form);
			PauseImpl(const PauseImpl&) = delete;
			PauseImpl(PauseImpl&&) = delete;
			~PauseImpl();

			operator bool() const;
		};

	public:
		PauseImpl Pause() const;

	private:
		class CollapsingSection
		{
			bool _visible = false;
			PauseImpl _pause;
			std::unique_ptr<Form> _form;

		public:
			CollapsingSection(const Form& form, const char* label, bool start_open);
			CollapsingSection(const CollapsingSection&) = delete;
			CollapsingSection(CollapsingSection&&) = delete;
			~CollapsingSection();

			operator bool() const;
		};

	public:
		CollapsingSection Collapse(const char* label, bool start_open = false) const;
	};
}
