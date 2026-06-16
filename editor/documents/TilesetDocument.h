#pragma once

#include "documents/IDocument.h"

#include "desc/TilesetDesc.h"

#include "gui/Form.h"
#include "gui/Texture.h"

#include "assets/MetaSplitter.h"

namespace oly::editor
{
	struct ActiveTexture
	{
		Texture texture;
		detail::TileConfigGrid grid;
		bool error = false;
		bool stale = true;
	};

	struct IndividualEditorState
	{
		ActiveTexture active;
	};

	template<size_t M, size_t N>
	detail::TileConfigGrid TileConfigSubgrid(const std::array<std::array<bool, M>, N>& grid, int x, int y)
	{
		detail::TileConfigGrid subgrid{};
		const auto ActiveCell = [&grid](int x, int y) { return x >= 0 && x < M && y >= 0 && y < N && grid[y][x]; };
		for (int dy = -1; dy <= 1; ++dy)
			for (int dx = -1; dx <= 1; ++dx)
				subgrid[static_cast<size_t>(1 + dy)][static_cast<size_t>(1 + dx)] = ActiveCell(x + dx, y + dy);
		return subgrid;
	}

	template<size_t M, size_t N>
	struct GroupEditorGrid
	{
		std::array<std::array<std::optional<ActiveTexture>, M>, N> actives;
		std::optional<std::pair<int, int>> selected_index;
		
		GroupEditorGrid(std::array<std::array<bool, M>, N> selectable)
		{
			for (size_t y = 0; y < N; ++y)
			{
				for (size_t x = 0; x < M; ++x)
				{
					if (selectable[y][x])
					{
						ActiveTexture active;
						active.grid = TileConfigSubgrid(selectable, x, y);
						actives[y][x] = active;
					}
					else
						actives[y][x] = std::nullopt;
				}
			}
		}

		void SyncActive(const ActiveTexture& active)
		{
			for (size_t y = 0; y < N; ++y)
			{
				for (size_t x = 0; x < M; ++x)
				{
					if (actives[y][x] && actives[y][x]->grid == active.grid)
						actives[y][x] = active;
				}
			}
		}
	};

	struct GroupEditorState
	{
		GroupEditorGrid<1, 1> grid_1x1;
		GroupEditorGrid<3, 1> grid_3x1;
		GroupEditorGrid<1, 3> grid_1x3;
		GroupEditorGrid<3, 3> grid_3x3;
		GroupEditorGrid<5, 5> grid_5x5_standard;

		GroupEditorState();
	};

	class TilesetDocument : public IDocument
	{
		TilesetDesc _scratch;
		TilesetDesc _disk;
		detail::MetaMap _meta;
		IndividualEditorState _individual_editor;

	public:
		using IDocument::IDocument;

		static const char* GetVersion();

		void Init() override;
		void Draw() override;
		void Load() override;
		void Dump() override;

	private:
		void DrawGroupEditor();
		void DrawIndividualEditor();
		bool DrawToggleCell(ImVec2 rect_start, ImVec2 rect_end, bool& on, const bool available);
		void Draw(TilesetAssignmentDesc& desc);

		void Load(TOMLNode node, TilesetDesc& desc);
		void Load(TOMLNode node, TilesetAssignmentDesc& desc);
		
		void Dump(toml::table& table, TilesetDesc& desc);
		void Dump(toml::table& table, TilesetAssignmentDesc& desc);

		TilesetAssignmentDesc& GetAssignment(const detail::TileConfigGrid grid);
		void OnActiveTextureChanged();

		void UpdateActiveTexture(ActiveTexture& active);
		void DrawActiveTexture(ImVec2 rect_start, ImVec2 rect_end, ActiveTexture& active);
	};
}
