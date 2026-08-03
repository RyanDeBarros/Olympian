#pragma once

#include "core/UndoHistory.h"

#include "assets/ResourcePath.h"

namespace oly::editor::fio
{
	struct RenamePathAction : public UndoAction
	{
		std::filesystem::path old_path;
		std::filesystem::path new_path;

		bool Forward() override;
		bool Backward() override;
		size_t EmpiricalSize() const override;
	};

	struct DeletePathAction : public UndoAction
	{
		detail::ResourcePath del_path;

	private:
		std::optional<detail::ResourcePath> _aux_path;

	public:
		bool Forward() override;
		bool Backward() override;
		size_t EmpiricalSize() const override;
	};

	struct CreateAssetAction : public UndoAction
	{
		detail::ResourcePath asset_path;

		bool Forward() override;
		bool Backward() override;
		size_t EmpiricalSize() const override;
	};
}
