#include "ResourcePath.h"

#include "assets/MetaSplitter.h"

namespace oly::detail
{
	static const char* OLY_EXT = ".oly";
    static std::filesystem::path resource_root; // TODO v10 resource_root should just be the local res/ folder -> copy it to build folder in cmake. likewise, copy shader folder in engine build. no need for macros

	ResourcePath::ResourcePath()
	{
		_absolute = resource_root;
	}

    ResourcePath::ResourcePath(const std::string_view path, const ResourcePath& relative_to)
    {
        set(path, relative_to);
    }
    
    ResourcePath::ResourcePath(const std::string& path, const ResourcePath& relative_to)
    {
        set(path, relative_to);
    }
    
    ResourcePath::ResourcePath(const char* path, const ResourcePath& relative_to)
    {
        set(path, relative_to);
    }
    
    ResourcePath::ResourcePath(const std::filesystem::path& path, const ResourcePath& relative_to)
    {
        set(path, relative_to);
    }

    ResourcePath& ResourcePath::operator=(const std::string_view path)
    {
        set(path, {});
        return *this;
    }
    
    ResourcePath& ResourcePath::operator=(const std::string& path)
    {
        set(path, {});
        return *this;
    }
    
    ResourcePath& ResourcePath::operator=(const char* path)
    {
        set(path, {});
        return *this;
    }
    
    ResourcePath& ResourcePath::operator=(const std::filesystem::path& path)
    {
        set(path, {});
        return *this;
    }

	void ResourcePath::set(const std::filesystem::path& path, const ResourcePath& relative_to)
	{
		if (path.is_absolute())
			_absolute = std::filesystem::weakly_canonical(path);
		else
		{
			std::string s = std::move(path.generic_string());
			if (s.starts_with("@/"))
				_absolute = std::filesystem::weakly_canonical(resource_root / s.substr(2));
			else
			{
				if (relative_to.empty())
					_absolute = std::filesystem::weakly_canonical(resource_root / s);
				else
					_absolute = std::filesystem::weakly_canonical((std::filesystem::is_directory(relative_to._absolute) ? relative_to._absolute : relative_to._absolute.parent_path()) / s);
			}
		}
	}

	void ResourcePath::set_resource_root(const std::filesystem::path& root)
	{
		resource_root = std::filesystem::weakly_canonical(root);
	}

	bool ResourcePath::is_resource_root() const
	{
		return _absolute == resource_root;
	}

	std::string ResourcePath::string() const
	{
		return _absolute.generic_string();
	}

	std::string ResourcePath::get_resource_shorthand() const
	{
		if (is_resource())
			return "@/" + std::filesystem::relative(_absolute, resource_root).generic_string();
		else
			return _absolute.generic_string();
	}

	std::filesystem::path ResourcePath::get_absolute() const
	{
		return _absolute;
	}

    bool ResourcePath::has_extension() const
    {
        return _absolute.has_extension();
    }
    
    std::string ResourcePath::extension() const
    {
        return _absolute.extension().generic_string();
    }

    void ResourcePath::create_parents() const
    {
        std::filesystem::create_directories(_absolute.parent_path());
    }

	ResourcePath ResourcePath::get_import_path() const
	{
		if (is_oly_path())
			return *this;
		else
		{
			ResourcePath p = *this;
			p._absolute += OLY_EXT;
			return p;
		}
	}

	ResourcePath ResourcePath::get_source_path() const
	{
		if (is_oly_path())
		{
			ResourcePath p = *this;
			p._absolute.replace_extension();
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
		return std::filesystem::exists(_absolute);
	}

	bool ResourcePath::is_file() const
	{
		return std::filesystem::is_regular_file(_absolute);
	}

	bool ResourcePath::is_directory() const
	{
		return std::filesystem::is_directory(_absolute);
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
		return path_is_relative_to(_absolute, resource_root);
	}

	bool ResourcePath::is_relative_to(const ResourcePath& base) const
	{
		return path_is_relative_to(_absolute, base._absolute);
	}

	std::string ResourcePath::tabname() const
	{
		if (is_oly_path())
			return _absolute.filename().replace_extension().generic_string();
		else
			return _absolute.filename().generic_string();
	}

	bool ResourcePath::resource_relative_path(std::filesystem::path& out) const
	{
		if (is_resource())
		{
			out = std::filesystem::relative(_absolute, resource_root);
			return true;
		}
		else
			return false;
	}

	bool ResourcePath::resource_parents(std::vector<std::string>& parents) const
	{
		if (!is_resource())
			return false;

		auto relative = std::filesystem::relative(_absolute, resource_root);
		for (const auto& part : relative.parent_path())
			parents.push_back(part.generic_string());

		return true;
	}

	std::string ResourcePath::filename() const
	{
		if (_absolute.empty() || _absolute == _absolute.root_path() || _absolute.filename() == ".")
			return _absolute.filename().generic_string();
		
		if (_absolute.filename().empty())
			return _absolute.parent_path().filename().generic_string();

		return _absolute.filename().generic_string();
	}

    std::ifstream ResourcePath::get_ifstream(std::ios_base::openmode mode) const
    {
        return std::ifstream(_absolute, mode);
    }
    
    std::ofstream ResourcePath::get_ofstream(std::ios_base::openmode mode) const
    {
        return std::ofstream(_absolute, mode);
    }
    
    std::fstream ResourcePath::get_fstream(std::ios_base::openmode mode) const
    {
        return std::fstream(_absolute, mode);
    }

	std::string ResourcePath::load_toml(toml::table& table) const
	{
		try
		{
			table = toml::parse_file(_absolute.c_str());
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

    bool ResourcePath::empty() const
    {
        return _absolute.empty();
    }
    
    size_t ResourcePath::hash() const
    {
        return std::hash<std::filesystem::path>{}(_absolute);
    }

	bool ResourcePath::operator==(const ResourcePath& o) const
	{
		return _absolute == o._absolute;
	}

	bool ResourcePath::operator<(const ResourcePath& o) const
	{
		return _absolute < o._absolute;
	}
}
