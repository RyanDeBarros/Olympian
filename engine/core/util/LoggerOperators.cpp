#include "LoggerOperators.h"

#include "core/util/UTF.h"
#include "core/util/ResourcePath.h"

namespace oly
{
	Logger::Impl operator<<(Logger::Impl impl, const utf::String& str)
	{
		return impl << std::string(str.encoding().begin(), str.encoding().end());
	}

	Logger::Impl operator<<(Logger::Impl impl, const ResourcePath& file)
	{
		return impl.stream(file.get_absolute());
	}
}
