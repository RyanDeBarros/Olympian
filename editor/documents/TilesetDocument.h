#pragma once

#include "documents/IDocument.h"

#include "desc/TilesetDesc.h"

#include "gui/Form.h"
#include "gui/Texture.h"

#include "assets/MetaSplitter.h"

namespace oly::editor
{
	struct IndividualEditorState
	{
		detail::TileConfigGrid grid;
		Texture active_texture;
		bool texture_error = false;
		bool stale_texture = true;

		IndividualEditorState();
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
	};
}
