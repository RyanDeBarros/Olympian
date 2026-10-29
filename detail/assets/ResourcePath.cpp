#include "ResourcePath.h"

#include "assets/MetaSplitter.h"

namespace oly::detail
{
    static const char* kOlyExt = ".oly";
    static std::filesystem::path resource_root;

    ResourcePath::ResourcePath()
    {
        absolute_ = resource_root;
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

    ResourcePath ResourcePath::operator/(const std::string_view subpath) const
    {
        return ResourcePath(subpath, *this);
    }

    ResourcePath ResourcePath::operator/(const std::string& subpath) const
    {
        return ResourcePath(subpath, *this);
    }

    ResourcePath ResourcePath::operator/(const char* subpath) const
    {
        return ResourcePath(subpath, *this);
    }

    ResourcePath ResourcePath::operator/(const std::filesystem::path& subpath) const
    {
        return ResourcePath(subpath, *this);
    }

    bool ResourcePath::operator==(const ResourcePath& o) const
    {
        return absolute_ == o.absolute_;
    }

    bool ResourcePath::operator<(const ResourcePath& o) const
    {
        return absolute_ < o.absolute_;
    }

    ResourcePath ResourcePath::_cmake_ported(const std::string_view path)
    {
        return ResourcePath(path, "..");
    }

    void ResourcePath::set_resource_root(const std::filesystem::path& root)
    {
        resource_root = std::filesystem::weakly_canonical(root);
    }

    bool ResourcePath::is_resource_root() const
    {
        return absolute_ == resource_root;
    }

    std::string ResourcePath::string() const
    {
        return absolute_.generic_string();
    }

    std::string ResourcePath::get_resource_shorthand() const
    {
        if (is_resource())
            return "@/" + std::filesystem::relative(absolute_, resource_root).generic_string();
        else
            return absolute_.generic_string();
    }

    std::filesystem::path ResourcePath::get_absolute() const
    {
        return absolute_;
    }

    bool ResourcePath::has_extension() const
    {
        return absolute_.has_extension();
    }

    std::string ResourcePath::extension() const
    {
        return absolute_.extension().generic_string();
    }

    void ResourcePath::create_parents() const
    {
        std::filesystem::create_directories(absolute_.parent_path());
    }

    ResourcePath ResourcePath::get_import_path() const
    {
        if (is_oly_path())
            return *this;
        else
        {
            ResourcePath p = *this;
            p.absolute_ += kOlyExt;
            return p;
        }
    }

    ResourcePath ResourcePath::get_source_path() const
    {
        if (is_oly_path())
        {
            ResourcePath p = *this;
            p.absolute_.replace_extension();
            return p;
        }
        else
            return *this;
    }

    bool ResourcePath::is_oly_path() const
    {
        return extension_matches(kOlyExt);
    }

    bool ResourcePath::exists() const
    {
        return std::filesystem::exists(absolute_);
    }

    bool ResourcePath::is_file() const
    {
        return std::filesystem::is_regular_file(absolute_);
    }

    bool ResourcePath::is_directory() const
    {
        return std::filesystem::is_directory(absolute_);
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
        return path_is_relative_to(absolute_, resource_root);
    }

    bool ResourcePath::is_relative_to(const ResourcePath& base) const
    {
        return path_is_relative_to(absolute_, base.absolute_);
    }

    std::string ResourcePath::tabname() const
    {
        if (is_oly_path())
            return absolute_.filename().replace_extension().generic_string();
        else
            return absolute_.filename().generic_string();
    }

    bool ResourcePath::resource_relative_path(std::filesystem::path& out) const
    {
        if (is_resource())
        {
            out = std::filesystem::relative(absolute_, resource_root);
            return true;
        }
        else
            return false;
    }

    bool ResourcePath::resource_parents(std::vector<std::string>& parents) const
    {
        if (!is_resource())
            return false;

        auto relative = std::filesystem::relative(absolute_, resource_root);
        for (const auto& part : relative.parent_path())
            parents.push_back(part.generic_string());

        return true;
    }

    std::string ResourcePath::filename() const
    {
        if (absolute_.empty() || absolute_ == absolute_.root_path() || absolute_.filename() == ".")
            return absolute_.filename().generic_string();

        if (absolute_.filename().empty())
            return absolute_.parent_path().filename().generic_string();

        return absolute_.filename().generic_string();
    }

    std::ifstream ResourcePath::get_ifstream(std::ios_base::openmode mode) const
    {
        return std::ifstream(absolute_, mode);
    }

    std::ofstream ResourcePath::get_ofstream(std::ios_base::openmode mode) const
    {
        return std::ofstream(absolute_, mode);
    }

    std::fstream ResourcePath::get_fstream(std::ios_base::openmode mode) const
    {
        return std::fstream(absolute_, mode);
    }

    std::string ResourcePath::load_toml(toml::table& table) const
    {
        try
        {
            table = toml::parse_file(absolute_.c_str());
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
        return absolute_.empty();
    }

    size_t ResourcePath::hash() const
    {
        return std::hash<std::filesystem::path>{}(absolute_);
    }

    void ResourcePath::set(const std::filesystem::path& path, const ResourcePath& relative_to)
    {
        if (path.is_absolute())
            absolute_ = std::filesystem::weakly_canonical(path);
        else
        {
            std::string s = std::move(path.generic_string());
            if (s.starts_with("@/"))
                absolute_ = std::filesystem::weakly_canonical(resource_root / s.substr(2));
            else
            {
                if (relative_to.empty())
                    absolute_ = std::filesystem::weakly_canonical(resource_root / s);
                else
                {
                    absolute_ = std::filesystem::weakly_canonical(
                        (std::filesystem::is_directory(relative_to.absolute_)
                            ? relative_to.absolute_
                            : relative_to.absolute_.parent_path()) / s);
                }
            }
        }
    }
}

size_t std::hash<oly::detail::ResourcePath>::operator()(const oly::detail::ResourcePath& f) const
{
    return f.hash();
}
