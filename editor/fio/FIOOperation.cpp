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

	bool DeletePathAction::Forward()
	{
		if (!Trashcan::Delete(del_path))
		{
			std::stringstream ss;
			ss << "fio::DeletePathAction::Forward() failed to delete \"" << del_path.get_resource_shorthand() << "\"";
			Logger::LogError(ss.str());
			return false;
		}

		std::stringstream ss;
		ss << "fio::DeletePathAction::Forward() success: deleted \"" << del_path.get_resource_shorthand() << "\"";
		Logger::LogSuccess(ss.str());

		if (aux_path)
		{
			if (!Trashcan::Delete(*aux_path))
			{
				std::stringstream ss;
				ss << "fio::DeletePathAction::Forward() failed to delete \"" << aux_path->get_resource_shorthand() << "\"";
				Logger::LogError(ss.str());
				return false;
			}

			std::stringstream ss;
			ss << "fio::DeletePathAction::Forward() success: deleted \"" << aux_path->get_resource_shorthand() << "\"";
			Logger::LogSuccess(ss.str());
		}

		return true;
	}

	bool DeletePathAction::Backward()
	{
		if (aux_path)
		{
			if (!Trashcan::Restore(*aux_path))
			{
				std::stringstream ss;
				ss << "fio::DeletePathAction::Backward() failed to restore \"" << aux_path->get_resource_shorthand() << "\"";
				Logger::LogError(ss.str());
				return false;
			}

			std::stringstream ss;
			ss << "fio::DeletePathAction::Backward() success: restored \"" << aux_path->get_resource_shorthand() << "\"";
			Logger::LogSuccess(ss.str());
		}

		if (!Trashcan::Restore(del_path))
		{
			std::stringstream ss;
			ss << "fio::DeletePathAction::Backward() failed to restore \"" << del_path.get_resource_shorthand() << "\"";
			Logger::LogError(ss.str());
			return false;
		}

		std::stringstream ss;
		ss << "fio::DeletePathAction::Backward() success: restored \"" << del_path.get_resource_shorthand() << "\"";
		Logger::LogSuccess(ss.str());

		return true;
	}

	size_t DeletePathAction::EmpiricalSize() const
	{
		return sizeof(*this);
	}

	bool CreateAssetAction::Forward()
	{
		if (Trashcan::Restore(asset_path))
		{
			std::stringstream ss;
			ss << "fio::CreateAssetAction::Forward() success: restored \"" << asset_path.get_resource_shorthand() << "\"";
			Logger::LogSuccess(ss.str());

			return true;
		}

		std::stringstream ss;
		ss << "fio::CreateAssetAction::Forward() failed to restore \"" << asset_path.get_resource_shorthand() << "\"";
		Logger::LogError(ss.str());
		return false;
	}

	bool CreateAssetAction::Backward()
	{
		if (Trashcan::Delete(asset_path))
		{
			std::stringstream ss;
			ss << "fio::CreateAssetAction::Backward() success: deleted \"" << asset_path.get_resource_shorthand() << "\"";
			Logger::LogSuccess(ss.str());

			return true;
		}

		std::stringstream ss;
		ss << "fio::CreateAssetAction::Backward() failed to delete \"" << asset_path.get_resource_shorthand() << "\"";
		Logger::LogError(ss.str());
		return false;
	}

	size_t CreateAssetAction::EmpiricalSize() const
	{
		return sizeof(*this);
	}
}
