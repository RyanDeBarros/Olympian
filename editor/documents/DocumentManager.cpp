#include "DocumentManager.h"

#include "documents/IDocument.h"

#include "core/MainWindow.h"

#include "assets/MetaSplitter.h"
#include "definitions/Keys.h"

#include "documents/FontDocument.h"
#include "documents/TextureDocument.h"

namespace oly::editor
{
	DocumentManager& DocumentManager::Instance()
	{
		return MainWindow::Instance().GetDocumentManager();
	}

	void DocumentManager::Draw()
	{
		for (const auto& doc : _documents)
			doc->Draw();
	}

	OpenAssetCode DocumentManager::OpenAsset(const detail::ResourcePath& path)
	{
		if (!path.is_resource())
			return OpenAssetCode::NotResource;

		detail::ResourcePath oly_file = path.get_import_path();
		if (oly_file.exists())
		{
			const auto meta = detail::MetaSplitter::decode_meta(oly_file);
			detail::Key type = detail::Key::_;
			if (auto t = meta.get_type())
				type = *t;

			switch (type)
			{
			case detail::Key::Meta_Font:
				Add<FontDocument>();
				break;
			case detail::Key::Meta_Texture:
				Add<TextureDocument>();
				break;
			default:
				return OpenAssetCode::BadMeta;
			}
		}
		else if (path.exists() && path.is_file())
		{
			std::string ext = path.extension();
			for (size_t i = 0; i < ext.size(); ++i)
				ext[i] = tolower(ext[i]);

			if (ext == ".ttf" || ext == ".otf")
				Add<FontDocument>();
			else if (ext == ".png" || ext == ".jpg" || ext == ".gif" || ext == ".svg")
				Add<TextureDocument>();
			else
				return OpenAssetCode::UnsupportedExtension;
		}
		else
			return OpenAssetCode::DoesNotExist;

		return OpenAssetCode::Success;
	}

	size_t DocumentManager::DocumentCount() const
	{
		return _documents.size();
	}

	const IDocument& DocumentManager::GetDocument(size_t i) const
	{
		return *_documents[i];
	}

	IDocument& DocumentManager::GetDocument(size_t i)
	{
		return *_documents[i];
	}

	void DocumentManager::Add(std::unique_ptr<IDocument>&& doc)
	{
		_documents.push_back(std::move(doc));
	}

	void DocumentManager::Remove(IDocument& document)
	{
		for (auto it = _documents.begin(); it != _documents.end(); ++it)
		{
			if (it->get() == &document)
			{
				_documents.erase(it);
				break;
			}
		}
	}

	void DocumentManager::Remove(size_t i)
	{
		_documents.erase(_documents.begin() + i);
	}
}
