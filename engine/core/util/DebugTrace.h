#pragma once

#include "core/util/StringParam.h"

namespace oly
{
    class DebugTrace;

    namespace internal
    {
        class DebugTraceScope
        {
            const DebugTrace& trace_;
            std::string name_space_;
            std::string action_;

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

    private:
        const std::string_view source_;

    public:
        DebugTrace(const std::string_view source = "");

        internal::DebugTraceScope scope(const StringParam& name_space, const StringParam& action) const;
    };
}
