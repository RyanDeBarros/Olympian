#pragma once

#include "core/base/TransformerExposure.h"

#include "graphics/sprites/Sprite.h"

namespace oly::rendering
{
    class NineSlice
    {
        mutable imp::fixed_vector<Sprite> sprites_;

        glm::vec2 nsize_{};
        glm::vec2 regular_dimensions_{};
        math::UVRect regular_uvs_;
        glm::vec4 regular_modulation_ = glm::vec4(1.0f);
        glm::vec2 regular_mod_dimensions_{};
        math::UVRect regular_mod_uvs_;
        math::Padding offsets_;

        struct
        {
            bool grid = false;
            bool modulation = false;
            bool mod_grid = false;
        } mutable dirty_;

        Transformer2D transformer_;

    public:
        NineSlice();
        NineSlice(Unbatched);
        NineSlice(SpriteBatch& batch);

    public:
        static NineSlice load(TOMLNode node);
        static NineSlice load(TOMLNode node, const DebugTrace& trace);

        internal::SpriteReference::accessor get_batch() const;
        void set_batch(Unbatched);
        void set_batch(SpriteBatch& batch);

        void draw() const;

        const Transform2D& get_local() const;
        Transform2D& set_local();

        Transformer2DExposure<> get_transformer() const;
        Transformer2DExposure<ex::AttachOnly> set_transformer();

        void copy_sprite_attributes(const Sprite& sprite);
        void set_texture(const detail::ResourcePath& texture_file, unsigned int texture_index = 0);
        void set_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions);
        void set_texture(const graphics::BindlessTextureRef& texture);
        void set_tex_coords(math::UVRect rect);
        void set_modulation(glm::vec4 modulation);
        void set_frame_format(const graphics::AnimFrameFormat& anim) const;
        void set_camera_invariant(bool is_camera_invariant) const;
        void set_mod_texture(const detail::ResourcePath& texture_file, unsigned int texture_index = 0);
        void set_mod_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions);
        void set_mod_texture(const graphics::BindlessTextureRef& texture);
        void set_mod_tex_coords(math::UVRect rect);

        graphics::BindlessTextureRef get_texture() const;
        graphics::BindlessTextureRef get_texture(glm::vec2& dimensions) const;
        math::UVRect get_tex_coords() const;
        glm::vec4 get_modulation() const;
        graphics::AnimFrameFormat get_frame_format() const;
        bool is_camera_invariant() const;
        graphics::BindlessTextureRef get_mod_texture() const;
        graphics::BindlessTextureRef get_mod_texture(glm::vec2& dimensions) const;
        math::UVRect get_mod_tex_coords() const;

        math::Padding get_offsets() const;
        math::Padding& set_offsets();

        float width() const;
        float height() const;
        glm::vec2 size() const;
        void set_width(float w);
        void set_height(float h);
        void set_size(glm::vec2 size);

        void setup_nonant(glm::vec2 nsize, math::Padding offsets);
        void setup_nonant(const Sprite& copy, glm::vec2 nsize, math::Padding offsets);

        glm::vec2 get_dimensions() const;

    private:
        void init();
        Sprite& sprite(unsigned char x, unsigned char y) const;

        void clamp_nsize(glm::vec2 nsize);
        void sync_grid() const;
        void sync_modulation() const;
        void sync_mod_grid() const;
    };

    using NineSliceRef = SmartReference<NineSlice>;
}
