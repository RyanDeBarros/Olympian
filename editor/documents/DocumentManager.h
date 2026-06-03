#pragma once

#include <memory>
#include <vector>

#include "assets/ResourcePath.h"

namespace oly::editor
{
	class IDocument;

	enum class OpenAssetCode
	{
		Success = 0,
		NotResource,
		UnsupportedAssetType,
		UnsupportedAssetVersion,
		UnsupportedExtension,
		DoesNotExist
	};

	class DocumentManager
	{
		std::vector<std::unique_ptr<IDocument>> _documents;

	public:
		static DocumentManager& Instance();

		void Draw();

		OpenAssetCode OpenAsset(const detail::ResourcePath& path);

		size_t DocumentCount() const;
		const IDocument& GetDocument(size_t i) const;
		IDocument& GetDocument(size_t i);
		size_t GetDocumentIndex(const detail::ResourcePath& oly_path) const;
		size_t GetDocumentIndex(const IDocument* doc) const;
		bool DocumentExists(const detail::ResourcePath& oly_path) const;

		template<typename T, typename... Args>
		void Add(Args&&... args)
		{
			Add(std::make_unique<T>(std::forward<Args>(args)...));
		}

		void Add(std::unique_ptr<IDocument>&& doc);
		void Remove(IDocument& document);
		void Remove(size_t i);
		void Remove(const detail::ResourcePath& oly_path);
	};
}
