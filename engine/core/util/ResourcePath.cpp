#include "ResourcePath.h"

#include "core/base/Errors.h"
#include "core/util/LoggerOperators.h"

namespace oly
{
	namespace context::internal
	{
		std::filesystem::path resource_root;

		void set_resource_root(const std::string& root)
		{
			resource_root = root;
		}
	}

	void ResourcePath::set(std::filesystem::path&& path, const ResourcePath& relative_to)
	{
		if (path.is_absolute())
			absolute = std::move(path);
		else
		{
			std::string s = std::move(path.generic_string());
			if (s.starts_with("~/"))
				absolute = context::internal::resource_root / s.substr(2);
			else
			{
				if (relative_to.empty())
					absolute = context::internal::resource_root / s;
				else
					absolute = (std::filesystem::is_directory(relative_to.absolute) ? relative_to.absolute : relative_to.absolute.parent_path()) / s;
			}
		}
	}

	std::filesystem::path ResourcePath::get_absolute() const
	{
		return absolute;
	}

	ResourcePath ResourcePath::get_import_path() const
	{
		if (is_import_path())
		{
			OLY_LOG_WARNING(true) << "Path " << *this << " is already an import path." << LOG.nl;
			return *this;
		}
		else
		{
			ResourcePath p = *this;
			p.absolute += ".oly";
			return p;
		}
	}

	bool ResourcePath::is_import_path() const
	{
		return extension_matches(".oly");
	}
}
