#pragma once

namespace oly::editor
{
	class DisabledSection
	{
		bool _alive = true;
		bool _disabled = false;

	public:
		DisabledSection(bool disabled);
		DisabledSection(const DisabledSection&) = delete;
		DisabledSection(DisabledSection&&) noexcept;
		~DisabledSection();
		DisabledSection& operator=(DisabledSection&&) = delete;

		operator bool() const;
		bool Disabled() const;
	};
}
