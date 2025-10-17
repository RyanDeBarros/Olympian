#include "LoggerOperators.h"

namespace oly
{
	Logger::Impl operator<<(Logger::Impl impl, const utf::String& str)
	{
		return impl << std::string(str.encoding().begin(), str.encoding().end());
	}
}
