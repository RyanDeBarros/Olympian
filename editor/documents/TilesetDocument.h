#pragma once

#include "documents/IDocument.h"

#include "desc/TilesetDesc.h"

#include "gui/Form.h"

#include "assets/MetaSplitter.h"

namespace oly::editor
{
	class TilesetDocument : public IDocument
	{
		TilesetDesc _scratch;
		TilesetDesc _disk;
		detail::MetaMap _meta;

	public:
		using IDocument::IDocument;

		static const char* GetVersion();

		void Init() override;
		void Draw() override;
		void Load() override;
		void Dump() override;

	private:
		void Draw(Form& form, TilesetDesc& desc);
		void Draw(Form& form, TilesetAssignmentDesc& desc);

		void Load(TOMLNode node, TilesetDesc& desc);
		void Load(TOMLNode node, TilesetAssignmentDesc& desc);
		
		void Dump(toml::table& table, TilesetDesc& desc);
		void Dump(toml::table& table, TilesetAssignmentDesc& desc);
	};
}
