#include "DocumentManager.h"

#include "documents/IDocument.h"

#include "core/MainWindow.h"
#include "panels/AssetEditorPanel.h"

#include "assets/MetaSplitter.h"
#include "definitions/Keys.h"

#include "documents/FontDocument.h"
#include "documents/ProjectDocument.h"
#include "documents/SignalDocument.h"
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
				if (meta.get_version() == FontDocument::GetVersion())
					Add<FontDocument>(std::move(oly_file));
				else
					return OpenAssetCode::UnsupportedAssetVersion;
				break;

			case detail::Key::Meta_Project:
				if (meta.get_version() == ProjectDocument::GetVersion())
					Add<ProjectDocument>(std::move(oly_file));
				else
					return OpenAssetCode::UnsupportedAssetVersion;
				break;

			case detail::Key::Meta_Signal:
				if (meta.get_version() == SignalDocument::GetVersion())
					Add<SignalDocument>(std::move(oly_file));
				else
					return OpenAssetCode::UnsupportedAssetVersion;
				break;

			case detail::Key::Meta_Texture:
				if (meta.get_version() == TextureDocument::GetVersion())
					Add<TextureDocument>(std::move(oly_file));
				else
					return OpenAssetCode::UnsupportedAssetVersion;
				break;

			default:
				return OpenAssetCode::UnsupportedAssetType;
			}
		}
		else if (path.exists() && path.is_file())
		{
			std::string ext = path.extension();
			for (size_t i = 0; i < ext.size(); ++i)
				ext[i] = tolower(ext[i]);

			if (ext == ".ttf" || ext == ".otf")
				Add<FontDocument>(std::move(oly_file));
			else if (ext == ".png" || ext == ".jpg" || ext == ".gif" || ext == ".svg")
				Add<TextureDocument>(std::move(oly_file));
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

	size_t DocumentManager::GetDocumentIndex(const detail::ResourcePath& oly_path) const
	{
		for (size_t i = 0; i < _documents.size(); ++i)
		{
			if (_documents[i]->GetOlyPath() == oly_path)
				return i;
		}

		return _documents.size();
	}

	size_t DocumentManager::GetDocumentIndex(const IDocument* doc) const
	{
		for (size_t i = 0; i < _documents.size(); ++i)
		{
			if (_documents[i].get() == doc)
				return i;
		}

		return _documents.size();
	}

	bool DocumentManager::DocumentExists(const detail::ResourcePath& oly_path) const
	{
		return GetDocumentIndex(oly_path) < _documents.size();
	}

	void DocumentManager::Add(std::unique_ptr<IDocument>&& doc)
	{
		size_t idx = GetDocumentIndex(doc->GetOlyPath());
		if (idx < _documents.size())
			AssetEditorPanel::Instance().FocusTab(_documents[idx].get());
		else
		{
			AssetEditorPanel::Instance().FocusTab(doc.get());
			doc->Init();
			_documents.push_back(std::move(doc));
		}
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

	void DocumentManager::Remove(const detail::ResourcePath& oly_path)
	{
		for (auto it = _documents.begin(); it != _documents.end(); ++it)
		{
			if ((*it)->GetOlyPath() == oly_path)
			{
				_documents.erase(it);
				break;
			}
		}
	}
}
