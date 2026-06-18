#pragma once

#include "gui/StyleStack.h"

namespace oly::editor
{
	struct GUIState
	{
		static inline std::vector<gui::StyleCtorVariant> input_data_styles;

		class InputDataStyleStack
		{
			size_t _count = 0;

		public:
			InputDataStyleStack() = default;
			InputDataStyleStack(const InputDataStyleStack&) = delete;
			InputDataStyleStack(InputDataStyleStack&&) noexcept;
			~InputDataStyleStack();
			InputDataStyleStack& operator=(InputDataStyleStack&&) noexcept = delete;

			void PushStyle(gui::StyleCtorVariant ctor);
			void PopStyles();
		};
	};
}
