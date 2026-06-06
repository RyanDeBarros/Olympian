#pragma once

namespace oly::editor
{
	class DisabledSection
	{
	public:
		DisabledSection(bool disabled);
		DisabledSection(const DisabledSection&) = delete;
		DisabledSection(DisabledSection&&) = delete;
		~DisabledSection();

		operator bool() const;
	};
}
