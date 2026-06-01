#pragma once

#include "documents/IDocument.h"

#include "desc/TextureDesc.h"
#include "TOML.h"

#include "assets/MetaSplitter.h"

namespace oly::editor
{
	class TextureDocument : public IDocument
	{
		TextureArrayDesc _desc;
		bool _gif = false;
		bool _svg = false;
		detail::MetaMap _meta;

	public:
		using IDocument::IDocument;

		void Init() override;
		void Draw() override;
		void Load() override;
		void Dump() override;

		detail::ResourcePath GetSourcePath() const;

	private:
		void Draw(TextureArrayDesc& desc);
		void Draw(TextureDesc& desc);
		void Draw(RasterTextureDesc& desc);
		void Draw(VectorTextureDesc& desc);
		void Draw(BaseTextureDesc& desc);
		void Draw(SpritesheetDesc& desc);

		void Load(TOMLNode node, TextureArrayDesc& desc);
		void Load(TOMLNode node, TextureDesc& desc);
		void Load(TOMLNode node, RasterTextureDesc& desc);
		void Load(TOMLNode node, VectorTextureDesc& desc);
		void Load(TOMLNode node, BaseTextureDesc& desc);
		void Load(TOMLNode node, SpritesheetDesc& desc);

		void Dump(toml::table& table, TextureArrayDesc& desc);
		void Dump(toml::table& table, TextureDesc& desc);
		void Dump(toml::table& table, RasterTextureDesc& desc);
		void Dump(toml::table& table, VectorTextureDesc& desc);
		void Dump(toml::table& table, BaseTextureDesc& desc);
		void Dump(toml::table& table, SpritesheetDesc& desc);
	};
}
