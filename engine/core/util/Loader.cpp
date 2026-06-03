#include "Loader.h"

#include "core/util/LoggerOperators.h"

namespace oly::io
{
	toml::table load_toml(const detail::ResourcePath& file)
	{
		_OLY_ENGINE_LOG_DEBUG("ASSETS") << "Loading TOML file " << file << LOG.nl;
		toml::table table;
		std::string err = file.load_toml(table);
		if (!err.empty())
		{
			_OLY_ENGINE_LOG_ERROR("ASSETS") << "Cannot load TOML file " << file << LOG.nl;
			throw Error(ErrorCode::TomlParse, err);
		}
		return table;
	}
}
