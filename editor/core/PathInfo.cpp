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

        switch (detail::MetaSplitter::decode_meta(path.string().c_str()).get_type())
        {
        case detail::Key::Meta_Signal:
            return IconResource::Controller;

        default:
            return IconResource::File;
        }
    }

    Texture PathInfo::GetIcon(const std::filesystem::path& path)
    {
        return ResourceLoader::GetTexture(GetIconResource(path));
    }

	void PathInfo::RevealInExplorer(const std::filesystem::path& path)
	{
#ifdef OLY_OS_WINDOWS
        std::filesystem::path abs = std::filesystem::absolute(path);
        std::wstring wpath = abs.wstring();

        PIDLIST_ABSOLUTE pidl = ILCreateFromPathW(wpath.c_str());
        if (pidl)
        {
            SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);
            ILFree(pidl);
        }
        else
            Logger::Instance().Log(LogLevel::Error, ("Failed to reveal path " + path.generic_string()).c_str());
#elif OLY_OS_APPLE
        std::string cmd = "open -R \"" + path.string() + "\"";
        std::system(cmd.c_str());
#elif OLY_OS_LINUX
        std::string cmd = "xdg-open \"" + path.parent_path().string() + "\"";
        std::system(cmd.c_str());
#endif
	}
}
