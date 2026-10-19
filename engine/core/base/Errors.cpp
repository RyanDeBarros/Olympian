#include "Errors.h"

namespace oly
{
    Error::Error(ErrorCode code)
        : std::runtime_error("Olympian Error (" + std::to_string((int)code) + ")")
        , code(code)
    {
    }
    
    Error::Error(ErrorCode code, const std::string_view message)
        : std::runtime_error("Olympian Error (" + std::to_string((int)code) + "): " + std::string(message))
        , code(code)
    {
    }
}
