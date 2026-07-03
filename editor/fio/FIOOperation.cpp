#include "FIOOperation.h"

#include "core/editor/Logger.h"
#include "core/editor/Notifier.h"

namespace oly::editor::fio
{
	bool RenamePathAction::Forward()
	{
		std::error_code ec;
		std::filesystem::rename(old_path, new_path, ec);
		if (!ec)
		{
			// TODO v9.2 Logger::Log() instead of going through Instance(). Also, add LogSuccess(), LogError(), etc.
			Logger::Instance().Log(LogLevel::Success, "fio::RenameFile::Forward() success: \"" + old_path.generic_string() + "\" to \"" + new_path.generic_string() + "\"");
			return true;
		}

		// TODO v9.2 instead of using main window to push notifications, put notification system into its own thing. Under the hood it can use MainWindow::Instance(), but it would be easy to extend if main window is not open.
		Notifier::NotifyError("fio::RenameFile::Forward() fail: \"" + old_path.generic_string() + "\" to \"" + new_path.generic_string() + "\"" + ec.message());
		return false;
	}

	bool RenamePathAction::Backward()
	{
		std::error_code ec;
		std::filesystem::rename(new_path, old_path, ec);
		if (!ec)
		{
			Logger::Instance().Log(LogLevel::Success, "fio::RenameFile::Backward() success: \"" + new_path.generic_string() + "\" to \"" + old_path.generic_string() + "\"");
			return true;
		}

		Notifier::NotifyError("fio::RenameFile::Backward() fail: \"" + new_path.generic_string() + "\" to \"" + old_path.generic_string() + "\"" + ec.message());
		return false;
	}

	size_t RenamePathAction::EmpiricalSize() const
	{
		return sizeof(*this);
	}
}
