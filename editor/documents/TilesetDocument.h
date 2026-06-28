#pragma once

#include "documents/IDocument.h"

#include "desc/impl/TilesetDesc.h"
#include "desc/DoubleDescriptor.h"

#include "gui/scopes/Form.h"
#include "gui/graphics/Texture.h"

#include "assets/MetaSplitter.h"

namespace oly::editor
{
	struct IndividualEditorState
	{
		detail::TileConfigGrid grid;
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
		std::array<std::array<std::optional<detail::TileConfigGrid>, M>, N> subgrids;
		
		GroupEditorGrid(std::array<std::array<bool, M>, N> selectable)
		{
			for (size_t y = 0; y < N; ++y)
			{
				for (size_t x = 0; x < M; ++x)
				{
					if (selectable[y][x])
						subgrids[y][x] = TileConfigSubgrid(selectable, x, y);
					else
						subgrids[y][x] = std::nullopt;
				}
			}
		}

		const detail::TileConfigGrid* At(int x, int y) const
		{
			if (subgrids[y][x].has_value())
				return &subgrids[y][x].value();
			else
				return nullptr;
		}

		detail::TileConfigGrid* At(int x, int y)
		{
			if (subgrids[y][x].has_value())
				return &subgrids[y][x].value();
			else
				return nullptr;
		}
	};

	struct GridEditorStateBase
	{
		std::optional<std::pair<int, int>> selected_cell;

		virtual ~GridEditorStateBase() = default;

		virtual const detail::TileConfigGrid* At(int x, int y) const = 0;
		virtual detail::TileConfigGrid* At(int x, int y) = 0;
		virtual size_t Rows() const = 0;
		virtual size_t Cols() const = 0;
	};

	enum class GroupEditorType
	{
		T4x4 = 0,
		T5x5
	};

	struct Grid4x4EditorState : public GridEditorStateBase
	{
		GroupEditorGrid<3, 3> grid_3x3;
		GroupEditorGrid<3, 1> grid_3x1;
		GroupEditorGrid<1, 3> grid_1x3;
		GroupEditorGrid<1, 1> grid_1x1;

		Grid4x4EditorState();

		const detail::TileConfigGrid* At(int x, int y) const override;
		detail::TileConfigGrid* At(int x, int y) override;
		size_t Rows() const override;
		size_t Cols() const override;
	};

	struct Grid5x5EditorState : public GridEditorStateBase
	{
		GroupEditorGrid<5, 5> grid_5x5;

		Grid5x5EditorState();

		const detail::TileConfigGrid* At(int x, int y) const override;
		detail::TileConfigGrid* At(int x, int y) override;
		size_t Rows() const override;
		size_t Cols() const override;
	};

	struct GroupEditorsState
	{
		Grid4x4EditorState state_4x4;
		Grid5x5EditorState state_5x5;
		
		GroupEditorType current_type = GroupEditorType::T4x4;
	};

	class TilesetDocument : public IDocument
	{
		DoubleDescriptor<TilesetDesc> _desc;
		detail::MetaMap _meta;
		IndividualEditorState _individual_editor;
		GroupEditorsState _group_editors;

		enum class TextureError
		{
			None,
			NotAFile,
			NotAResource,
			MissingImport,
			NotATexture,
			CorruptedAsset,
			BadSlot,
			CannotLoad,
		};

		struct ActiveTexture
		{
			Texture texture;
			TextureError error = TextureError::None;
			bool stale = true;
		};

		std::unordered_map<detail::TileConfig, ActiveTexture> _textures;

	public:
		using IDocument::IDocument;

		static const char* GetVersion();

		void InitImpl() override;
		void Draw() override;
		void LoadImpl() override;
		void DumpImpl() override;
		const IDoubleDescriptor& GetDoubleDescriptor() const override;
		IDoubleDescriptor& GetDoubleDescriptor() override;

	private:
		void DrawGroupEditor();
		void DrawIndividualEditor();
		void DrawToggleCell(ImVec2 rect_start, ImVec2 rect_end, bool& on, const bool available);
		void Draw(const detail::TileConfigGrid grid);

		void Load(TOMLNode node, TilesetDesc& desc);
		void Load(TOMLNode node, TilesetAssignmentDesc& desc);
		
		void Dump(toml::table& table, TilesetDesc& desc);
		void Dump(toml::table& table, TilesetAssignmentDesc& desc);

		TilesetAssignmentDesc& GetAssignment(const detail::TileConfigGrid grid);
		TilesetAssignmentDesc& GetAssignment(const detail::TileConfig config);
		DataPathSource GetAssignmentPath(const detail::TileConfigGrid grid);
		DataPathSource GetAssignmentPath(const detail::TileConfig config);
		void OnActiveTextureChanged(const detail::TileConfigGrid grid);
		void UpdateActiveTextures();

		static bool TextureErrorIsWarning(TilesetDocument::TextureError error);
		void DrawActiveTexture(ImVec2 rect_start, ImVec2 rect_end, const detail::TileConfigGrid grid, unsigned char empty_gray_value);
		void DrawActiveTextureDirect(const detail::TileConfigGrid grid, ImVec2 rect_start, ImVec2 rect_end);
		void TextureErrorTooltip(TextureError error);
		ActiveTexture& GetActiveTexture(const detail::TileConfigGrid grid);
		static detail::TileConfig GetResolvedTileConfig(const detail::TileConfigGrid grid);
	};
}
