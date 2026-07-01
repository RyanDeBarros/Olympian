#pragma once

namespace oly::editor
{
	class CollapsingSection
	{
		bool _valid = true;
		bool _visible = false;

	public:
		CollapsingSection(const char* label, bool start_open = false);
		CollapsingSection(const CollapsingSection&) = delete;
		CollapsingSection(CollapsingSection&&) noexcept;
		~CollapsingSection();
		CollapsingSection& operator=(CollapsingSection&&) noexcept = delete;

		operator bool() const;
	};
}
