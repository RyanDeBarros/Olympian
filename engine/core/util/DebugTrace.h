#pragma once

#include "core/util/StringParam.h"

namespace oly
{
	class DebugTrace;

	namespace internal
	{
		class DebugTraceScope
		{
			const DebugTrace& trace;
			std::string name_space;
			std::string action;

		public:
			DebugTraceScope(const DebugTrace& trace, const StringParam& name_space, const StringParam& action);
			DebugTraceScope(const DebugTraceScope&) = delete;
			DebugTraceScope(DebugTraceScope&&) = delete;
			~DebugTraceScope();
		};
	}

	class DebugTrace
	{
		friend class internal::DebugTraceScope;
		const std::string_view source;

	public:
		DebugTrace(const std::string_view source = "") : source(source) {}

		internal::DebugTraceScope scope(const StringParam& name_space, const StringParam& action) const { return internal::DebugTraceScope(*this, name_space, action); }
	};
}
