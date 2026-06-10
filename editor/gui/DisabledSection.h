#pragma once

namespace oly::editor
{
	class DisabledSection
	{
		bool _alive = true;

	public:
		DisabledSection(bool disabled);
		DisabledSection(const DisabledSection&) = delete;
		DisabledSection(DisabledSection&&) noexcept;
		~DisabledSection();
		DisabledSection& operator=(DisabledSection&&) = delete;

		operator bool() const;
	};
}
