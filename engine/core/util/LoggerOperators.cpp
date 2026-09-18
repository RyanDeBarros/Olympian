#include "LoggerOperators.h"

#include "assets/ResourcePath.h"

namespace oly
{
	Logger::Impl operator<<(Logger::Impl impl, const imp::utf::string& str)
	{
		return impl << std::string(str.encoding().begin(), str.encoding().end());
	}

	Logger::Impl operator<<(Logger::Impl impl, const detail::ResourcePath& file)
	{
		return impl.stream(file.get_absolute());
	}
}
