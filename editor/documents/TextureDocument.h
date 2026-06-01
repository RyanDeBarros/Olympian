#pragma once

#include "documents/IDocument.h"

#include "desc/TextureDesc.h"

namespace oly::editor
{
	class TextureDocument : public IDocument
	{
		TextureArrayDesc _desc;

	public:
		using IDocument::IDocument;

		void Init() override;
		void Draw() override;

		detail::ResourcePath GetSourcePath() const;

	private:
		void Draw(TextureArrayDesc& desc);
		void Draw(TextureDesc& desc);
		void Draw(RasterTextureDesc& desc);
		void Draw(VectorTextureDesc& desc);
		void Draw(BaseTextureDesc& desc);
		void Draw(SpritesheetDesc& desc);
	};
}
