#pragma once

#include "core/base/Transforms.h"

#include "core/util/DebugTrace.h"

#include "graphics/sprites/SpriteBatch.h"

namespace oly::rendering
{
    // ASSET
    class StaticSprite
    {
        internal::SpriteReference ref_;

    public:
        StaticSprite() = default;
        StaticSprite(Unbatched);
        StaticSprite(internal::SpriteBatch& batch);
        StaticSprite(SpriteBatch& batch);
        StaticSprite(const StaticSprite&) = default;
        StaticSprite(StaticSprite&&) noexcept = default;

        StaticSprite& operator=(const StaticSprite&) = default;
        StaticSprite& operator=(StaticSprite&&) noexcept = default;

    public:
        void draw() const;

        internal::SpriteReference::accessor get_batch() const;
        void set_batch(Unbatched);
        void set_batch(SpriteBatch& batch);

        void set_texture(const detail::ResourcePath& texture_file, unsigned int texture_index = 0) const;
        void set_texture(const graphics::BindlessTextureRef& texture) const;
        void set_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions) const;
        void set_tex_coords(math::UVRect uvs) const;
        void set_modulation(glm::vec4 modulation) const;
        void set_frame_format(const graphics::AnimFrameFormat& anim) const;
        void set_camera_invariant(bool is_camera_invariant) const;
        void set_mod_texture(const detail::ResourcePath& texture_file, unsigned int texture_index = 0) const;
        void set_mod_texture(const graphics::BindlessTextureRef& texture) const;
        void set_mod_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions) const;
        void set_mod_tex_coords(math::UVRect uvs) const;
        void set_transform(const glm::mat3& transform);

        graphics::BindlessTextureRef get_texture() const;
        graphics::BindlessTextureRef get_texture(glm::vec2& dimensions) const;
        math::UVRect get_tex_coords() const;
        glm::vec4 get_modulation() const;
        graphics::AnimFrameFormat get_frame_format() const;
        bool is_camera_invariant() const;
        graphics::BindlessTextureRef get_mod_texture() const;
        graphics::BindlessTextureRef get_mod_texture(glm::vec2& dimensions) const;
        math::UVRect get_mod_tex_coords() const;
        glm::mat3 get_transform() const;
    };

    class Sprite
    {
        internal::SpriteReference ref_;

    public:
        Transformer2D transformer;

    public:
        Sprite() = default;
        Sprite(Unbatched);
        Sprite(internal::SpriteBatch& batch);
        Sprite(SpriteBatch& batch);
        Sprite(const Sprite&) = default;
        Sprite(Sprite&&) noexcept = default;

        Sprite& operator=(const Sprite&) = default;
        Sprite& operator=(Sprite&&) noexcept = default;

    public:
        static Sprite load(TOMLNode node);
        static Sprite load(TOMLNode node, const DebugTrace& trace);

        void draw() const;

        internal::SpriteReference::accessor get_batch() const;
        void set_batch(Unbatched);
        void set_batch(SpriteBatch& batch);

        void set_texture(const detail::ResourcePath& texture_file, unsigned int texture_index = 0) const;
        void set_texture(const graphics::BindlessTextureRef& texture) const;
        void set_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions) const;
        void set_tex_coords(math::UVRect uvs) const;
        void set_modulation(glm::vec4 modulation) const;
        void set_frame_format(const graphics::AnimFrameFormat& anim) const;
        void set_camera_invariant(bool is_camera_invariant) const;
        void set_mod_texture(const detail::ResourcePath& texture_file, unsigned int texture_index = 0) const;
        void set_mod_texture(const graphics::BindlessTextureRef& texture) const;
        void set_mod_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions) const;
        void set_mod_tex_coords(math::UVRect uvs) const;

        graphics::BindlessTextureRef get_texture() const;
        graphics::BindlessTextureRef get_texture(glm::vec2& dimensions) const;
        math::UVRect get_tex_coords() const;
        glm::vec4 get_modulation() const;
        graphics::AnimFrameFormat get_frame_format() const;
        bool is_camera_invariant() const;
        graphics::BindlessTextureRef get_mod_texture() const;
        graphics::BindlessTextureRef get_mod_texture(glm::vec2& dimensions) const;
        math::UVRect get_mod_tex_coords() const;

        const Transform2D& get_local() const;
        Transform2D& set_local();
    };

    using SpriteRef = SmartReference<Sprite>;
}
