#include "DebugTrace.h"

#include "core/util/Logger.h"

namespace oly::internal
{
	DebugTraceScope::DebugTraceScope(const DebugTrace& trace, const StringParam& name_space, const StringParam& action)
		: trace(trace), name_space(name_space.transfer()), action(action.transfer())
	{
		_OLY_ENGINE_LOG_DEBUG(this->name_space.c_str()) << "Starting \"" << this->action << "\" [" << trace.source << "]..." << LOG.nl;
	}

	DebugTraceScope::~DebugTraceScope()
	{
		_OLY_ENGINE_LOG_DEBUG(name_space.c_str()) << "...Ending \"" << action << "\" [" << trace.source << "]" << LOG.nl;
	}
}
