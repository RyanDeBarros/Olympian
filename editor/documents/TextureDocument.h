#pragma once

#include "documents/IDocument.h"

#include "desc/TextureDesc.h"

#include "assets/MetaSplitter.h"

namespace oly::editor
{
	class TextureDocument : public IDocument
	{
		TextureDesc _scratch;
		TextureDesc _disk;
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
		void Draw(TextureDesc& desc);
		void Draw(TextureSlotDesc& desc);
		void Draw(RasterTextureDesc& desc);
		void Draw(VectorTextureDesc& desc);
		void Draw(BaseTextureDesc& desc);
		void Draw(SpritesheetDesc& desc);

		void Load(TOMLNode node, TextureDesc& desc);
		void Load(TOMLNode node, TextureSlotDesc& desc);
		void Load(TOMLNode node, RasterTextureDesc& desc);
		void Load(TOMLNode node, VectorTextureDesc& desc);
		void Load(TOMLNode node, BaseTextureDesc& desc);
		void Load(TOMLNode node, SpritesheetDesc& desc);

		void Dump(toml::table& table, TextureDesc& desc);
		void Dump(toml::table& table, TextureSlotDesc& desc);
		void Dump(toml::table& table, RasterTextureDesc& desc);
		void Dump(toml::table& table, VectorTextureDesc& desc);
		void Dump(toml::table& table, BaseTextureDesc& desc);
		void Dump(toml::table& table, SpritesheetDesc& desc);
	};
}
