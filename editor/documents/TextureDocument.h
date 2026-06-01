#pragma once

#include "documents/IDocument.h"

#include "desc/TextureDesc.h"
#include "TOML.h"

#include "assets/MetaSplitter.h"

namespace oly::editor
{
	class TextureDocument : public IDocument
	{
		TextureDesc _disk;
		TextureDesc _desc;
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
		void Draw(TextureDesc& desc, const TextureDesc* disk);
		void Draw(TextureSlotDesc& desc, const TextureSlotDesc* disk);
		void Draw(RasterTextureDesc& desc, const RasterTextureDesc* disk);
		void Draw(VectorTextureDesc& desc, const VectorTextureDesc* disk);
		void Draw(BaseTextureDesc& desc, const BaseTextureDesc* disk);
		void Draw(SpritesheetDesc& desc, const SpritesheetDesc* disk);

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
