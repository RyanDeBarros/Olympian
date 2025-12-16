#include "DebugTrace.h"

#include "core/util/Logger.h"

namespace oly::internal
{
	DebugTraceScope::DebugTraceScope(const DebugTrace& trace, std::string&& name_space, std::string&& action)
		: trace(trace), name_space(std::move(name_space)), action(std::move(action))
	{
		_OLY_ENGINE_LOG_DEBUG(this->name_space.c_str()) << "Starting \"" << this->action << "\" [" << trace.source << "]..." << LOG.nl;
	}

	DebugTraceScope::~DebugTraceScope()
	{
		_OLY_ENGINE_LOG_DEBUG(this->name_space.c_str()) << "...Ending \"" << action << "\" [" << trace.source << "]" << LOG.nl;
	}
}
