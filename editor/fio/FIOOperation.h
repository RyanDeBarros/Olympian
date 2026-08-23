#pragma once

#include "assets/ResourcePath.h"

#include <imp/undo_history.hpp>

namespace oly::editor::fio
{
	struct RenamePathAction : public imp::undo_action
	{
		std::filesystem::path old_path;
		std::filesystem::path new_path;

		bool forward() override;
		bool backward() override;
		size_t empirical_size() const override;
	};

	struct DeletePathAction : public imp::undo_action
	{
		detail::ResourcePath del_path;

	private:
		std::optional<detail::ResourcePath> _aux_path;

	public:
		bool forward() override;
		bool backward() override;
		size_t empirical_size() const override;
	};

	struct CreateAssetAction : public imp::undo_action
	{
		detail::ResourcePath asset_path;

		bool forward() override;
		bool backward() override;
		size_t empirical_size() const override;
	};
}
