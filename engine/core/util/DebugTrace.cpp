#include "DebugTrace.h"

#include "core/util/Logger.h"

namespace oly
{
    internal::DebugTraceScope::DebugTraceScope(
        const DebugTrace& trace,
        const StringParam& name_space,
        const StringParam& action)
        : trace_(trace)
        , name_space_(name_space.transfer())
        , action_(action.transfer())
    {
        _OLY_ENGINE_LOG_DEBUG(name_space_.c_str())
            << "Starting \"" << action_ << "\" [" << trace.source_ << "]..." << kLog.nl;
    }

    internal::DebugTraceScope::~DebugTraceScope()
    {
        _OLY_ENGINE_LOG_DEBUG(name_space_.c_str())
            << "...Ending \"" << action_ << "\" [" << trace_.source_ << "]" << kLog.nl;
    }

    DebugTrace::DebugTrace(const std::string_view source)
        : source_(source)
    {
    }

    internal::DebugTraceScope DebugTrace::scope(const StringParam& name_space, const StringParam& action) const
    {
        return internal::DebugTraceScope(*this, name_space, action);
    }
}
