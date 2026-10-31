#include "DocumentManager.h"

#include "core/windows/MainWindow.h"

#include "documents/FontDocument.h"
#include "documents/FontFamilyDocument.h"
#include "documents/IDocument.h"
#include "documents/ProjectDocument.h"
#include "documents/RasterFontDocument.h"
#include "documents/SignalDocument.h"
#include "documents/TextureDocument.h"
#include "documents/TilesetDocument.h"

#include "panels/AssetEditorPanel.h"

#include "assets/MetaSplitter.h"

#include "definitions/Keys.h"

#define DOCUMENT_GENERATOR(M) \
        M(Font) \
        M(FontFamily) \
        M(Project) \
        M(RasterFont) \
        M(Signal) \
        M(Texture) \
        M(Tileset)

namespace oly::editor
{
    DocumentManager& DocumentManager::Instance()
    {
        return MainWindow::Instance().GetDocumentManager();
    }

    void DocumentManager::Draw()
    {
        for (const auto& doc : documents_)
            doc->Draw();
    }

    template<typename T>
    struct ImportableTypeTag
    {
        using type = T;
    };

    static OpenAssetCode ProcessUnimportedAsset(const detail::ResourcePath& source_asset, auto&& visitor)
    {
        std::string ext = source_asset.extension();
        for (size_t i = 0; i < ext.size(); ++i)
            ext[i] = tolower(ext[i]);

        if (ext == ".ttf" || ext == ".otf")
            visitor(ImportableTypeTag<FontDocument>{}, source_asset.get_import_path());
        else if (ext == ".png" || ext == ".jpg" || ext == ".gif" || ext == ".svg")
            visitor(ImportableTypeTag<TextureDocument>{}, source_asset.get_import_path());
        else
            return OpenAssetCode::UnsupportedExtension;

        return OpenAssetCode::Success;
    }

    OpenAssetCode DocumentManager::OpenAsset(const detail::ResourcePath& path)
    {
        detail::ResourcePath oly_file = path.get_import_path();
        if (oly_file.exists())
        {
            const auto meta = detail::MetaSplitter::decode_meta(oly_file);
            switch (meta.get_type())
            {
#define SWITCH_CASE(Doc) \
            case detail::Key::Meta_##Doc: \
                if (meta.get_version() == Doc##Document::GetVersion()) \
                    Add<Doc##Document>(std::move(oly_file)); \
                else \
                    return OpenAssetCode::UnsupportedAssetVersion; \
                break;

                DOCUMENT_GENERATOR(SWITCH_CASE);

#undef SWITCH_CASE

            default:
                return OpenAssetCode::UnsupportedAssetType;
            }
        }
        else if (path.exists() && path.is_file())
        {
            return ProcessUnimportedAsset(
                path,
                [this]<typename T>(ImportableTypeTag<T>, detail::ResourcePath oly_file) {
                    Add<T>(std::move(oly_file));
                });
        }
        else
            return OpenAssetCode::DoesNotExist;

        return OpenAssetCode::Success;
    }

    template<std::derived_from<IDocument> D>
    static bool InitNewAssetImpl(detail::ResourcePath path)
    {
        try
        {
            D doc(std::move(path));
            doc.Init();
            doc.DumpAsset();
            return true;
        }
        catch (const imtk::breakout_error& e)
        {
            e.log();
            return false;
        }
    }

    bool DocumentManager::InitNewAsset(detail::ResourcePath path, detail::Key meta_type)
    {
        switch (meta_type)
        {
#define SWITCH_CASE(Doc) case detail::Key::Meta_##Doc: return InitNewAssetImpl<Doc##Document>(std::move(path));

            DOCUMENT_GENERATOR(SWITCH_CASE);

#undef SWITCH_CASE

        case detail::Key::Meta_Folder:
            return std::filesystem::create_directory(path.get_absolute());

        default:
            return false;
        }
    }

    bool DocumentManager::ImportAsset(const detail::ResourcePath& source_asset)
    {
        if (source_asset.is_oly_path())
            return false;

        try
        {
            return ProcessUnimportedAsset(
                source_asset,
                []<typename T>(ImportableTypeTag<T>, detail::ResourcePath import_path) {
                    T doc(std::move(import_path));
                    doc.Init();
                    doc.DumpAsset();
                }) == OpenAssetCode::Success;
        }
        catch (const imtk::breakout_error& e)
        {
            e.log();
            return false;
        }
    }

    size_t DocumentManager::DocumentCount() const
    {
        return documents_.size();
    }

    const IDocument& DocumentManager::GetDocument(size_t i) const
    {
        return *documents_[i];
    }

    IDocument& DocumentManager::GetDocument(size_t i)
    {
        return *documents_[i];
    }

    size_t DocumentManager::GetDocumentIndex(const detail::ResourcePath& oly_path) const
    {
        for (size_t i = 0; i < documents_.size(); ++i)
        {
            if (documents_[i]->GetOlyPath() == oly_path)
                return i;
        }

        return documents_.size();
    }

    size_t DocumentManager::GetDocumentIndex(const IDocument* doc) const
    {
        for (size_t i = 0; i < documents_.size(); ++i)
        {
            if (documents_[i].get() == doc)
                return i;
        }

        return documents_.size();
    }

    bool DocumentManager::DocumentExists(const detail::ResourcePath& oly_path) const
    {
        return GetDocumentIndex(oly_path) < documents_.size();
    }

    void DocumentManager::NotifyRename(const detail::ResourcePath& old_path, const detail::ResourcePath& new_path)
    {
        for (auto& doc : documents_)
        {
            if (doc->GetOlyPath() == old_path)
                doc->Rename(new_path);
        }
    }

    void DocumentManager::Add(std::unique_ptr<IDocument>&& doc)
    {
        size_t idx = GetDocumentIndex(doc->GetOlyPath());
        if (idx < documents_.size())
            AssetEditorPanel::Instance().FocusTab(documents_[idx].get());
        else
        {
            AssetEditorPanel::Instance().FocusTab(doc.get());
            doc->Init();
            documents_.push_back(std::move(doc));
        }
    }

    void DocumentManager::Remove(IDocument& document)
    {
        for (auto it = documents_.begin(); it != documents_.end(); ++it)
        {
            if (it->get() == &document)
            {
                documents_.erase(it);
                break;
            }
        }
    }

    void DocumentManager::Remove(size_t i)
    {
        documents_.erase(documents_.begin() + i);
    }

    void DocumentManager::Remove(const detail::ResourcePath& oly_path)
    {
        for (auto it = documents_.begin(); it != documents_.end(); ++it)
        {
            if ((*it)->GetOlyPath() == oly_path)
            {
                documents_.erase(it);
                break;
            }
        }
    }
}
