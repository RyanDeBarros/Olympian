#include "ResourcePath.h"

#include "assets/MetaSplitter.h"

namespace oly::detail
{
	static const char* OLY_EXT = ".oly";
	std::filesystem::path ResourcePath::resource_root; // TODO v9.4 resource_root should just be the local res/ folder -> copy it to build folder in cmake. likewise, copy shader folder in engine build. no need for macros

	ResourcePath::ResourcePath()
	{
		absolute = resource_root;
	}

	void ResourcePath::set(const std::filesystem::path& path, const ResourcePath& relative_to)
	{
		if (path.is_absolute())
			absolute = std::filesystem::weakly_canonical(path);
		else
		{
			std::string s = std::move(path.generic_string());
			if (s.starts_with("@/"))
				absolute = std::filesystem::weakly_canonical(resource_root / s.substr(2));
			else
			{
				if (relative_to.empty())
					absolute = std::filesystem::weakly_canonical(resource_root / s);
				else
					absolute = std::filesystem::weakly_canonical((std::filesystem::is_directory(relative_to.absolute) ? relative_to.absolute : relative_to.absolute.parent_path()) / s);
			}
		}
	}

	void ResourcePath::set_resource_root(const std::filesystem::path& root)
	{
		resource_root = std::filesystem::weakly_canonical(root);
	}

	bool ResourcePath::is_resource_root() const
	{
		return absolute == resource_root;
	}

	std::string ResourcePath::string() const
	{
		return absolute.generic_string();
	}

	std::string ResourcePath::get_resource_shorthand() const
	{
		if (is_resource())
			return "@/" + std::filesystem::relative(absolute, resource_root).generic_string();
		else
			return absolute.generic_string();
	}

	std::filesystem::path ResourcePath::get_absolute() const
	{
		return absolute;
	}

	ResourcePath ResourcePath::get_import_path() const
	{
		if (is_oly_path())
			return *this;
		else
		{
			ResourcePath p = *this;
			p.absolute += OLY_EXT;
			return p;
		}
	}

	ResourcePath ResourcePath::get_source_path() const
	{
		if (is_oly_path())
		{
			ResourcePath p = *this;
			p.absolute.replace_extension();
			return p;
		}
		else
			return *this;
	}

	bool ResourcePath::is_oly_path() const
	{
		return extension_matches(OLY_EXT);
	}

	bool ResourcePath::exists() const
	{
		return std::filesystem::exists(absolute);
	}

	bool ResourcePath::is_file() const
	{
		return std::filesystem::is_regular_file(absolute);
	}

	bool ResourcePath::is_directory() const
	{
		return std::filesystem::is_directory(absolute);
	}

	static bool path_is_relative_to(const std::filesystem::path& path, const std::filesystem::path& base)
	{
		std::error_code ec;

		auto p = std::filesystem::weakly_canonical(path, ec);
		if (ec)
			return false;

		ec.clear();
		auto b = std::filesystem::weakly_canonical(base, ec);
		if (ec)
			return false;

		auto pit = p.begin();
		auto bit = b.begin();

		for (; bit != b.end(); ++bit, ++pit)
		{
			if (pit == p.end() || *pit != *bit)
				return false;
		}

		return true;
	}

	bool ResourcePath::is_resource() const
	{
		return path_is_relative_to(absolute, resource_root);
	}

	bool ResourcePath::is_relative_to(const ResourcePath& base) const
	{
		return path_is_relative_to(absolute, base.absolute);
	}

	std::string ResourcePath::tabname() const
	{
		if (is_oly_path())
			return absolute.filename().replace_extension().generic_string();
		else
			return absolute.filename().generic_string();
	}

	bool ResourcePath::resource_relative_path(std::filesystem::path& out) const
	{
		if (is_resource())
		{
			out = std::filesystem::relative(absolute, resource_root);
			return true;
		}
		else
			return false;
	}

	bool ResourcePath::resource_parents(std::vector<std::string>& parents) const
	{
		if (!is_resource())
			return false;

		auto relative = std::filesystem::relative(absolute, resource_root);
		for (const auto& part : relative.parent_path())
			parents.push_back(part.generic_string());

		return true;
	}

	std::string ResourcePath::filename() const
	{
		if (absolute.empty() || absolute == absolute.root_path() || absolute.filename() == ".")
			return absolute.filename().generic_string();
		
		if (absolute.filename().empty())
			return absolute.parent_path().filename().generic_string();

		return absolute.filename().generic_string();
	}

	std::string ResourcePath::load_toml(toml::table& table) const
	{
		try
		{
			table = toml::parse_file(absolute.c_str());
			return "";
		}
		catch (const toml::parse_error& err)
		{
			return std::string(err.description());
		}
	}

	void ResourcePath::dump_toml(toml::table& table, const MetaMap& meta) const
	{
		std::stringstream ss;
		ss << MetaSplitter::encode_meta(meta);
		ss << table;
		get_ofstream() << ss.str();
	}

	bool ResourcePath::operator==(const ResourcePath& o) const
	{
		return absolute == o.absolute;
	}

	bool ResourcePath::operator<(const ResourcePath& o) const
	{
		return absolute < o.absolute;
	}
}
