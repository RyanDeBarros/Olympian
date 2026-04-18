#include "Loader.h"

#include "core/algorithms/STLUtils.h"
#include "core/base/Definitions.h"
#include "core/util/LoggerOperators.h"
#include "core/util/Parser.h"

#include ".gen/keys/Transform.inl"

// TODO v7 use CTOMLNode throughout

namespace oly::io
{
	toml::v3::parse_result load_toml(const ResourcePath& file)
	{
		try
		{
			_OLY_ENGINE_LOG_DEBUG("ASSETS") << "Loading TOML file " << file << LOG.nl;
			return toml::parse_file(file.get_absolute().c_str());
		}
		catch (const toml::parse_error& err)
		{
			_OLY_ENGINE_LOG_ERROR("ASSETS") << "Cannot load TOML file " << file << LOG.nl;
			throw Error(ErrorCode::TomlParse, err.description().data());
		}
	}
}
