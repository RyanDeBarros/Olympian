#include "PathInfo.h"

#include "core/editor/ResourceLoader.h"
#include "core/editor/Logger.h"

#include "core/Macros.h"

#include "assets/MetaSplitter.h"
#include "definitions/Keys.h"

#ifdef OLY_OS_WINDOWS
#include <windows.h>
#include <shlobj.h>
#elif OLY_OS_APPLE
#include <cstdlib>
#elif OLY_OS_LINUX
#include <cstdlib>
#endif

namespace oly::editor
{
	bool PathInfo::IsImportFile(const std::filesystem::path& path)
	{
		return path.extension() == ".oly" && detail::MetaSplitter::decode_meta(path.string().c_str()).is_import();
	}

    static IconResource GetIconResource(const std::filesystem::path& path)
    {
        if (std::filesystem::is_directory(path))
            return IconResource::Folder;

        return PathInfo::GetAssetIcon(detail::MetaSplitter::decode_meta(path.string().c_str()).get_type());
    }

    imtk::texture PathInfo::GetIcon(const std::filesystem::path& path)
    {
        return ResourceLoader::GetTexture(GetIconResource(path));
    }

    IconResource PathInfo::GetAssetIcon(detail::Key meta_type)
    {
        switch (meta_type)
        {
        case detail::Key::Meta_Folder:
            return IconResource::Folder;

        case detail::Key::Meta_Font:
            return IconResource::Font;

        case detail::Key::Meta_FontFamily:
            return IconResource::FontFamily;

        case detail::Key::Meta_Project:
            return IconResource::Settings;

        case detail::Key::Meta_RasterFont:
            return IconResource::RasterFont;

        case detail::Key::Meta_Signal:
            return IconResource::Controller;

        case detail::Key::Meta_Texture:
            return IconResource::Texture;

        case detail::Key::Meta_Tileset:
            return IconResource::Tileset;

        default:
            return IconResource::File;
        }
    }

	void PathInfo::RevealInExplorer(const std::filesystem::path& path, bool open_folder_contents)
	{
#ifdef OLY_OS_WINDOWS
        std::filesystem::path abs = std::filesystem::absolute(path);

        if (open_folder_contents && std::filesystem::is_directory(abs))
        {
            ShellExecute(nullptr, "open", abs.string().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
            return;
        }

        PIDLIST_ABSOLUTE pidl = ILCreateFromPath(abs.string().c_str());
        if (pidl)
        {
            SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);
            ILFree(pidl);
        }
        else
            Logger::LogError("Failed to reveal path " + path.generic_string());

#elif OLY_OS_APPLE
        std::string cmd;

        if (open_folder_contents && std::filesystem::is_directory(path))
            cmd = "open \"" + path.string() + "\"";
        else
            cmd = "open -R \"" + path.string() + "\"";

        std::system(cmd.c_str());

#elif OLY_OS_LINUX
        std::string cmd;

        if (open_folder_contents && std::filesystem::is_directory(path))
            cmd = "xdg-open \"" + path.string() + "\"";
        else
            cmd = "xdg-open \"" + path.parent_path().string() + "\"";

        std::system(cmd.c_str());
#endif
	}

    std::string PathInfo::NameOf(const std::filesystem::path& path)
    {
        if (path.empty() || path == path.root_path() || path.filename() == ".")
            return path.filename().generic_string();

        if (path.filename().empty())
            return path.parent_path().filename().generic_string();

        return path.filename().generic_string();
    }
}
