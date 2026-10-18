#include "Sprite.h"

#include "core/context/rendering/Sprites.h"
#include "core/context/rendering/Textures.h"

#include "core/util/Parser.h"

#include "graphics/sprites/Definitions.h"

#include "definitions/Keys.h"

namespace oly::rendering
{
    StaticSprite::StaticSprite(Unbatched)
        : ref_(kUnbatched)
    {
    }

    StaticSprite::StaticSprite(internal::SpriteBatch& batch)
        : ref_(batch)
    {
    }

    StaticSprite::StaticSprite(SpriteBatch& batch)
        : ref_(batch)
    {
    }

    void StaticSprite::draw() const
    {
        ref_.draw_quad();
    }

    internal::SpriteReference::accessor StaticSprite::get_batch() const
    {
        return ref_.get_batch();
    }

    void StaticSprite::set_batch(Unbatched)
    {
        ref_.set_batch(kUnbatched);
    }

    void StaticSprite::set_batch(SpriteBatch& batch)
    {
        ref_.set_batch(batch);
    }

    void StaticSprite::set_texture(const detail::ResourcePath& texture_file, unsigned int texture_index) const
    {
        ref_.set_texture(texture_file, texture_index);
    }

    void StaticSprite::set_texture(const graphics::BindlessTextureRef& texture) const
    {
        ref_.set_texture(texture);
    }

    void StaticSprite::set_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions) const
    {
        ref_.set_texture(texture, dimensions);
    }

    void StaticSprite::set_tex_coords(math::UVRect uvs) const
    {
        ref_.set_tex_coords(uvs);
    }

    void StaticSprite::set_modulation(glm::vec4 modulation) const
    {
        ref_.set_modulation(modulation);
    }

    void StaticSprite::set_frame_format(const graphics::AnimFrameFormat& anim) const
    {
        ref_.set_frame_format(anim);
    }

    void StaticSprite::set_camera_invariant(bool is_camera_invariant) const
    {
        ref_.set_camera_invariant(is_camera_invariant);
    }

    void StaticSprite::set_mod_texture(const detail::ResourcePath& texture_file, unsigned int texture_index) const
    {
        ref_.set_mod_texture(texture_file, texture_index);
    }

    void StaticSprite::set_mod_texture(const graphics::BindlessTextureRef& texture) const
    {
        ref_.set_mod_texture(texture);
    }

    void StaticSprite::set_mod_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions) const
    {
        ref_.set_mod_texture(texture, dimensions);
    }

    void StaticSprite::set_mod_tex_coords(math::UVRect uvs) const
    {
        ref_.set_mod_tex_coords(uvs);
    }

    void StaticSprite::set_transform(const glm::mat3& transform)
    {
        ref_.set_transform(transform);
    }

    graphics::BindlessTextureRef StaticSprite::get_texture() const
    {
        return ref_.get_texture();
    }

    graphics::BindlessTextureRef StaticSprite::get_texture(glm::vec2& dimensions) const
    {
        return ref_.get_texture(dimensions);
    }

    math::UVRect StaticSprite::get_tex_coords() const
    {
        return ref_.get_tex_coords();
    }

    glm::vec4 StaticSprite::get_modulation() const
    {
        return ref_.get_modulation();
    }

    graphics::AnimFrameFormat StaticSprite::get_frame_format() const
    {
        return ref_.get_frame_format();
    }

    bool StaticSprite::is_camera_invariant() const
    {
        return ref_.is_camera_invariant();
    }

    graphics::BindlessTextureRef StaticSprite::get_mod_texture() const
    {
        return ref_.get_mod_texture();
    }

    graphics::BindlessTextureRef StaticSprite::get_mod_texture(glm::vec2& dimensions) const
    {
        return ref_.get_mod_texture(dimensions);
    }

    math::UVRect StaticSprite::get_mod_tex_coords() const
    {
        return ref_.get_mod_tex_coords();
    }

    glm::mat3 StaticSprite::get_transform() const
    {
        return ref_.get_transform();
    }

    Sprite::Sprite(Unbatched)
        : ref_(kUnbatched)
    {
    }
    
    Sprite::Sprite(internal::SpriteBatch& batch)
        : ref_(batch)
    {
    }
    
    Sprite::Sprite(SpriteBatch& batch)
        : ref_(batch)
    {
    }

    Sprite Sprite::load(TOMLNode node)
    {
        if (!node)
            return {};

        assets::Parser parser(node);

        Sprite sprite;
        sprite.transformer = Transformer2D::load(parser.field(detail::Key::Transformer));

        auto texture = parser.optional<std::string>(detail::Key::Texture)();
        if (texture)
        {
            graphics::BindlessTextureRef btex = context::load_texture(*texture, parser.defaulted(detail::Key::TextureIndex)(0u));
            sprite.set_texture(btex, context::get_texture_size(btex));
        }

        if (auto modulation = parser.optional<glm::vec4>(detail::Key::Modulation)())
            sprite.set_modulation(*modulation);

        if (auto uvs = parser.optional<glm::vec4>(detail::Key::TextureCoordinates)())
            sprite.set_tex_coords({ .x1 = (*uvs)[0], .x2 = (*uvs)[1], .y1 = (*uvs)[2], .y2 = (*uvs)[3] });

        if (auto ff_parser = parser.optional(detail::Key::FrameFormat).subparser())
        {
            if (auto mode = ff_parser->optional<FrameFormat>(detail::Key::Mode)())
            {
                switch (*mode)
                {
                case FrameFormat::Single:
                    if (texture)
                        sprite.set_frame_format(graphics::setup_anim_frame_format_single(*texture, ff_parser->defaulted(detail::Key::Frame)(0u)));
                    else
                        _OLY_ENGINE_LOG_WARNING("ASSETS") << "No texture was set for (single) frame format." << kLog.nl;
                    break;
                case FrameFormat::Auto:
                    if (texture)
                        sprite.set_frame_format(graphics::setup_anim_frame_format(*texture, ff_parser->defaulted(detail::Key::Speed)(1.f),
                            ff_parser->defaulted(detail::Key::StartingFrame)(0u)));
                    else
                        _OLY_ENGINE_LOG_WARNING("ASSETS") << "No texture was set for (auto) frame format." << kLog.nl;
                    break;
                }
            }
            else
            {
                sprite.set_frame_format({
                    .starting_frame = ff_parser->defaulted(detail::Key::StartingFrame)(0u),
                    .num_frames = ff_parser->defaulted(detail::Key::NumFrames)(0u),
                    .starting_time = ff_parser->defaulted(detail::Key::StartingTime)(0.f),
                    .delay_seconds = ff_parser->defaulted(detail::Key::DelaySeconds)(0.f)
                    });
            };
        }

        sprite.set_camera_invariant(parser.defaulted(detail::Key::CameraInvariant)(false));

        return sprite;
    }

    Sprite Sprite::load(TOMLNode node, const DebugTrace& trace)
    {
        auto scope = trace.scope("ASSETS", "oly::rendering::Sprite::load()");
        return load(node);
    }

    void Sprite::draw() const
    {
        if (transformer.flush())
            ref_.set_transform(transformer.global());
        ref_.draw_quad();
    }

    internal::SpriteReference::accessor Sprite::get_batch() const
    {
        return ref_.get_batch();
    }
    
    void Sprite::set_batch(Unbatched)
    {
        ref_.set_batch(kUnbatched);
    }
    
    void Sprite::set_batch(SpriteBatch& batch)
    {
        ref_.set_batch(batch);
    }

    void Sprite::set_texture(const detail::ResourcePath& texture_file, unsigned int texture_index) const
    {
        ref_.set_texture(texture_file, texture_index);
    }

    void Sprite::set_texture(const graphics::BindlessTextureRef& texture) const
    {
        ref_.set_texture(texture);
    }

    void Sprite::set_texture(const graphics::BindlessTextureRef& texture, glm::vec2 size) const
    {
        ref_.set_texture(texture, size);
    }

    void Sprite::set_tex_coords(math::UVRect uvs) const
    {
        ref_.set_tex_coords(uvs);
    }

    void Sprite::set_modulation(glm::vec4 modulation) const
    {
        ref_.set_modulation(modulation);
    }

    void Sprite::set_frame_format(const graphics::AnimFrameFormat& anim) const
    {
        ref_.set_frame_format(anim);
    }

    void Sprite::set_camera_invariant(bool is_camera_invariant) const
    {
        ref_.set_camera_invariant(is_camera_invariant);
    }

    void Sprite::set_mod_texture(const detail::ResourcePath& texture_file, unsigned int texture_index) const
    {
        ref_.set_mod_texture(texture_file, texture_index);
    }

    void Sprite::set_mod_texture(const graphics::BindlessTextureRef& texture) const
    {
        ref_.set_mod_texture(texture);
    }

    void Sprite::set_mod_texture(const graphics::BindlessTextureRef& texture, glm::vec2 size) const
    {
        ref_.set_mod_texture(texture, size);
    }

    void Sprite::set_mod_tex_coords(math::UVRect uvs) const
    {
        ref_.set_mod_tex_coords(uvs);
    }

    graphics::BindlessTextureRef Sprite::get_texture() const
    {
        return ref_.get_texture();
    }

    graphics::BindlessTextureRef Sprite::get_texture(glm::vec2& size) const
    {
        return ref_.get_texture(size);
    }

    math::UVRect Sprite::get_tex_coords() const
    {
        return ref_.get_tex_coords();
    }

    glm::vec4 Sprite::get_modulation() const
    {
        return ref_.get_modulation();
    }

    graphics::AnimFrameFormat Sprite::get_frame_format() const
    {
        return ref_.get_frame_format();
    }

    bool Sprite::is_camera_invariant() const
    {
        return ref_.is_camera_invariant();
    }

    graphics::BindlessTextureRef Sprite::get_mod_texture() const
    {
        return ref_.get_mod_texture();
    }

    graphics::BindlessTextureRef Sprite::get_mod_texture(glm::vec2& dimensions) const
    {
        return ref_.get_mod_texture(dimensions);
    }

    math::UVRect Sprite::get_mod_tex_coords() const
    {
        return ref_.get_mod_tex_coords();
    }

    const Transform2D& Sprite::get_local() const
    {
        return transformer.get_local();
    }
    
    Transform2D& Sprite::set_local()
    {
        return transformer.set_local();
    }
}
