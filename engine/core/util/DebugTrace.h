#pragma once

#include <string>

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
			DebugTraceScope(const DebugTrace& trace, std::string&& name_space, std::string&& action);
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

		internal::DebugTraceScope scope(std::string&& name_space, std::string&& action) const { return internal::DebugTraceScope(*this, std::move(name_space), std::move(action)); }
	};
}
