#pragma once

#include "desc/TextureDesc.h"

#include "documents/IDocument.h"

#include "assets/MetaSplitter.h"

namespace oly::editor
{
    struct PreviewNav
    {
        ImVec2 pos = ImVec2(0, 0);
        float zoom = 0.f;
        float svg_scale = 1.f;
    };

    struct SpritesheetPreviewData
    {
        imtk::w::icon_button preview;
        imtk::w::icon_button playing;

        int active_index = 0;
        float timer = 0.f;

        SpritesheetPreviewData();
    };

    struct SpritesheetInfo
    {
        int rows;
        int cols;
        float cell_width;
        float cell_height;
        float full_width;
        float full_height;
        ImVec2 rect_offset;
    };

    class TextureDocument
        : public IDocument
    {
    public:
        enum class TextureSettingsLoadResult
        {
            Success,
            NotAFile,
            NotAResource,
            MissingImport,
            NotATexture,
            Corrupted,
            BadSlot
        };

    private:
        imtk::desc::doubler<TextureFullDesc> desc_;
        detail::MetaMap meta_;
        bool gif_ = false;
        bool svg_ = false;
        imtk::w::owned_list_indexer slots_;
        imtk::texture texture_;
        PreviewNav preview_nav_;

        bool stale_preview_texture_ = true;
        SpritesheetPreviewData spritesheet_preview_data_;

    public:
        TextureDocument(detail::ResourcePath oly_path);

    public:
        void InitImpl() override;
        void Draw() override;
        void LoadImpl() override;
        void DumpImpl() override;
        void ResetAssetImpl() override;
        const imtk::desc::idoubler& GetDoubleDescriptor() const override;
        imtk::desc::idoubler& GetDoubleDescriptor() override;

    public:
        static const char* GetVersion();
        static TextureSettingsLoadResult LoadTextureSettings(
            const detail::ResourcePath path,
            int slot,
            GLenum& min_filter,
            GLenum& mag_filter,
            float& scale,
            bool& generate_mipmaps);

        detail::ResourcePath GetSourcePath() const;

    private:
        void UpdatePreviewTexture();
        void DrawPreview();
        SpritesheetDesc* SpritesheetPreview();
        SpritesheetInfo CalcSpritesheetInfo(const SpritesheetDesc& desc);
        void DrawSpritesheetOverlay(const SpritesheetDesc& desc, ImVec2 rect_start, ImVec2 size);
        void PlaySpritesheetAnimation(const SpritesheetDesc& desc);

        void Draw(TextureFullDesc& desc);
        void Draw(RasterTextureDesc& desc);
        void Draw(VectorTextureDesc& desc);
        void Draw(BaseTextureDesc& desc);
        void Draw(SpritesheetDesc& desc);

        // TODO can make Load() methods in all document classes 'static', possibly even Draw()/Dump().
        static void Load(imtk::toml_node node, TextureFullDesc& desc, bool svg, bool gif);
        static void Load(imtk::toml_node node, RasterTextureDesc& desc, bool gif);
        static void Load(imtk::toml_node node, VectorTextureDesc& desc, bool gif);
        static void Load(imtk::toml_node node, BaseTextureDesc& desc, bool gif);
        static void Load(imtk::toml_node node, SpritesheetDesc& desc);

        void Dump(toml::table& table, TextureFullDesc& desc);
        void Dump(toml::table& table, RasterTextureDesc& desc);
        void Dump(toml::table& table, VectorTextureDesc& desc);
        void Dump(toml::table& table, BaseTextureDesc& desc);
        void Dump(toml::table& table, SpritesheetDesc& desc);

        void OnActiveSlotChanged();

        imtk::list_adapter ListAdapter();
        std::unique_ptr<imtk::ilist_op_adapter> ListOpAdapter();
    };
}
