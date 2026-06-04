#include "PathInfo.h"

#include "assets/MetaSplitter.h"

namespace oly::editor
{
	bool PathInfo::IsImportFile(const std::filesystem::path& path)
	{
		return path.extension() == ".oly" && detail::MetaSplitter::decode_meta(path.string().c_str()).is_import();
	}
}
