#include "PathInfo.h"

#include "core/editor/Logger.h"
#include "core/Macros.h"
#include "assets/MetaSplitter.h"

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
