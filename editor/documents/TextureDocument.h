#pragma once

#include "documents/IDocument.h"

#include "desc/TextureDesc.h"
#include "TOML.h"

namespace oly::editor
{
	class TextureDocument : public IDocument
	{
		TextureArrayDesc _desc;
		bool _gif = false;
		bool _svg = false;

	public:
		using IDocument::IDocument;

		void Init() override;
		void Draw() override;
		void Load();
		void Dump();

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

		// TODO v7 Dump(*)
	};
}
