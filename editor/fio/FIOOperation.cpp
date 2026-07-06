#include "FIOOperation.h"

#include "core/editor/Logger.h"
#include "fio/Trashcan.h"

namespace oly::editor::fio
{
	bool RenamePathAction::Forward()
	{
		std::error_code ec;
		std::filesystem::rename(old_path, new_path, ec);

		std::stringstream ss;
		ss << "fio::RenameFile::Forward() " << (ec ? "fail" : "success") << ": \"" << old_path.generic_string() << "\" to \"" << new_path.generic_string() << "\"";

		if (ec)
			ss << ": " << ec.message();

		Logger::Log(ec ? LogLevel::Error : LogLevel::Success, ss.str());
		return !ec;
	}

	bool RenamePathAction::Backward()
	{
		std::error_code ec;
		std::filesystem::rename(new_path, old_path, ec);

		std::stringstream ss;
		ss << "fio::RenameFile::Backward() " << (ec ? "fail" : "success") << ": \"" << new_path.generic_string() << "\" to \"" << old_path.generic_string() << "\"";

		if (ec)
			ss << ": " << ec.message();

		Logger::Log(ec ? LogLevel::Error : LogLevel::Success, ss.str());
		return !ec;
	}

	size_t RenamePathAction::EmpiricalSize() const
	{
		return sizeof(*this);
	}
}
