#pragma once

#include "documents/IDocument.h"

#include "desc/RasterFontDesc.h"

#include "gui/Form.h"

#include "assets/MetaSplitter.h"

namespace oly::editor
{
	class RasterFontDocument : public IDocument
	{
		RasterFontDesc _scratch;
		RasterFontDesc _disk;
		detail::MetaMap _meta;

	public:
		using IDocument::IDocument;

		static const char* GetVersion();

		void Init() override;
		void Draw() override;
		void Load() override;
		void Dump() override;

	private:
		void Load(TOMLNode node, RasterFontDesc& desc);

		void Dump(toml::table& table, RasterFontDesc& desc);
	};
}
