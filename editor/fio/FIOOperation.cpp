#include "FIOOperation.h"

#include "core/PathInfo.h"
#include "core/editor/Logger.h"
#include "fio/Trashcan.h"
#include "documents/DocumentManager.h"

namespace oly::editor::fio
{
	static std::optional<detail::ResourcePath> GetAuxPath(const detail::ResourcePath& path)
	{
		if (!path.is_oly_path())
		{
			detail::ResourcePath import = path.get_import_path();
			if (PathInfo::IsImportFile(import.get_absolute()))
				return import;
		}

		return std::nullopt;
	}

	bool RenamePathAction::Forward()
	{
		std::error_code ec;
		auto aux_old_path = GetAuxPath(old_path);
		std::filesystem::rename(old_path, new_path, ec);

		std::stringstream ss;
		ss << "fio::RenameFile::Forward() " << (ec ? "fail" : "success") << ": \"" << old_path.generic_string() << "\" to \"" << new_path.generic_string() << "\"";

		if (ec)
		{
			ss << ": " << ec.message();
			Logger::LogError(ss.str());
			return false;
		}

		DocumentManager::Instance().NotifyRename(old_path, new_path);

		if (aux_old_path)
		{
			auto aux_new_path = detail::ResourcePath(new_path).get_import_path();
			std::error_code ec;
			std::filesystem::rename(aux_old_path->get_absolute(), aux_new_path.get_absolute(), ec);

			std::stringstream ss;
			ss << "fio::RenameFile::Forward() " << (ec ? "fail" : "success") << ": \"" << aux_old_path->string() << "\" to \"" << aux_new_path.string() << "\"";

			if (ec)
			{
				ss << ": " << ec.message();
				Logger::LogError(ss.str());
				return false;
			}

			DocumentManager::Instance().NotifyRename(*aux_old_path, aux_new_path);
		}

		Logger::LogSuccess(ss.str());
		return true;
	}

	bool RenamePathAction::Backward()
	{
		std::error_code ec;
		auto aux_new_path = GetAuxPath(new_path);
		std::filesystem::rename(new_path, old_path, ec);

		std::stringstream ss;
		ss << "fio::RenameFile::Backward() " << (ec ? "fail" : "success") << ": \"" << new_path.generic_string() << "\" to \"" << old_path.generic_string() << "\"";

		if (ec)
		{
			ss << ": " << ec.message();
			Logger::LogError(ss.str());
			return false;
		}

		DocumentManager::Instance().NotifyRename(new_path, old_path);

		if (aux_new_path)
		{
			auto aux_old_path = detail::ResourcePath(old_path).get_import_path();
			std::error_code ec;
			std::filesystem::rename(aux_new_path->get_absolute(), aux_old_path.get_absolute(), ec);

			std::stringstream ss;
			ss << "fio::RenameFile::Forward() " << (ec ? "fail" : "success") << ": \"" << aux_new_path->string() << "\" to \"" << aux_old_path.string() << "\"";

			if (ec)
			{
				ss << ": " << ec.message();
				Logger::LogError(ss.str());
				return false;
			}

			DocumentManager::Instance().NotifyRename(*aux_new_path, aux_old_path);
		}

		Logger::LogSuccess(ss.str());
		return true;
	}

	size_t RenamePathAction::EmpiricalSize() const
	{
		return sizeof(*this);
	}

	void DeletePathAction::Init()
	{
		_aux_path = GetAuxPath(del_path);
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

		if (_aux_path)
		{
			if (!Trashcan::Delete(*_aux_path))
			{
				std::stringstream ss;
				ss << "fio::DeletePathAction::Forward() failed to delete \"" << _aux_path->get_resource_shorthand() << "\"";
				Logger::LogError(ss.str());
				return false;
			}

			std::stringstream ss;
			ss << "fio::DeletePathAction::Forward() success: deleted \"" << _aux_path->get_resource_shorthand() << "\"";
			Logger::LogSuccess(ss.str());
		}

		return true;
	}

	bool DeletePathAction::Backward()
	{
		if (_aux_path)
		{
			if (!Trashcan::Restore(*_aux_path))
			{
				std::stringstream ss;
				ss << "fio::DeletePathAction::Backward() failed to restore \"" << _aux_path->get_resource_shorthand() << "\"";
				Logger::LogError(ss.str());
				return false;
			}

			std::stringstream ss;
			ss << "fio::DeletePathAction::Backward() success: restored \"" << _aux_path->get_resource_shorthand() << "\"";
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
