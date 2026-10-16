#include "NineSlice.h"

#include "core/context/rendering/Textures.h"

#include "core/util/Parser.h"

#include "definitions/Keys.h"

namespace oly::rendering
{
    NineSlice::NineSlice()
        : sprites_(9, Sprite())
    {
        init();
    }

    NineSlice::NineSlice(Unbatched)
        : sprites_(9, Sprite(kUnbatched))
    {
        init();
    }

    NineSlice::NineSlice(SpriteBatch& batch)
        : sprites_(9, Sprite(batch))
    {
        init();
    }

    static NineSlice load_sprite_nonant(TOMLNode node, const DebugTrace* trace)
    {
        if (!node)
            return {};

        assets::Parser parser(node);

        NineSlice nonant;

        glm::vec2 nsize_{};
        parser.optional(detail::Key::NSize)(nsize_);
        math::Padding offsets_ = math::Padding::load(parser.field(detail::Key::Offsets));

        if (auto sprite = parser.optional<TOMLNode>(detail::Key::Sprite)())
            nonant.setup_nonant(trace ? Sprite::load(*sprite, *trace) : Sprite::load(*sprite), nsize_, offsets_);
        else
            nonant.setup_nonant(nsize_, offsets_);

        nonant.set_camera_invariant(parser.defaulted(detail::Key::CameraInvariant)(false));

        return nonant;
    }

    NineSlice NineSlice::load(TOMLNode node)
    {
        return load_sprite_nonant(node, nullptr);
    }

    NineSlice NineSlice::load(TOMLNode node, const DebugTrace& trace)
    {
        auto scope = trace.scope("ASSETS", "oly::rendering::NineSlice::load()");
        return load_sprite_nonant(node, &trace);
    }

    // TODO make accessor class accessible directly in issuer_instance, and maybe rename to issuer_accessor or something clearer
    internal::SpriteReference::accessor NineSlice::get_batch() const
    {
        return sprite(0, 0).get_batch();
    }

    void NineSlice::set_batch(Unbatched)
    {
        for (unsigned char x = 0; x < 3; ++x)
        {
            for (unsigned char y = 0; y < 3; ++y)
                sprite(x, y).set_batch(kUnbatched);
        }
    }

    void NineSlice::set_batch(SpriteBatch& batch)
    {
        for (unsigned char x = 0; x < 3; ++x)
        {
            for (unsigned char y = 0; y < 3; ++y)
                sprite(x, y).set_batch(batch);
        }
    }

    void NineSlice::draw() const
    {
        if (dirty_.grid)
        {
            sync_grid();
            dirty_.grid = false;
        }
        if (dirty_.modulation)
        {
            sync_modulation();
            dirty_.modulation = false;
        }
        if (dirty_.mod_grid)
        {
            sync_mod_grid();
            dirty_.mod_grid = false;
        }
        for (unsigned char x = 0; x < 3; ++x)
            for (unsigned char y = 0; y < 3; ++y)
                sprite(x, y).draw();
    }

    const Transform2D& NineSlice::get_local() const
    {
        return transformer_.get_local();
    }

    Transform2D& NineSlice::set_local()
    {
        return transformer_.set_local();
    }

    Transformer2DExposure<> NineSlice::get_transformer() const
    {
        return transformer_;
    }
    
    Transformer2DExposure<ex::AttachOnly> NineSlice::set_transformer()
    {
        return transformer_;
    }

    void NineSlice::copy_sprite_attributes(const Sprite& sprite)
    {
        set_local() = sprite.get_local();
        set_frame_format(sprite.get_frame_format());

        // set texture
        glm::vec2 dimensions;
        auto texture = sprite.get_texture(dimensions);
        for (unsigned char x = 0; x < 3; ++x)
            for (unsigned char y = 0; y < 3; ++y)
                this->sprite(x, y).set_texture(texture, dimensions);
        regular_dimensions_ = dimensions;

        // set mod texture
        texture = sprite.get_mod_texture(dimensions);
        for (unsigned char x = 0; x < 3; ++x)
            for (unsigned char y = 0; y < 3; ++y)
                this->sprite(x, y).set_mod_texture(texture, dimensions);
        regular_mod_dimensions_ = dimensions;

        // set other attributes
        nsize_ = regular_dimensions_;
        regular_uvs_ = sprite.get_tex_coords();
        regular_modulation_ = sprite.get_modulation();
        regular_mod_uvs_ = sprite.get_mod_tex_coords();

        dirty_.grid = true;
        dirty_.modulation = true;
        dirty_.mod_grid = true;
    }

    void NineSlice::set_texture(const detail::ResourcePath& texture_file, unsigned int texture_index)
    {
        for (unsigned char x = 0; x < 3; ++x)
            for (unsigned char y = 0; y < 3; ++y)
                sprite(x, y).set_texture(texture_file, texture_index);
        regular_dimensions_ = context::get_texture_size(texture_file, texture_index);
        nsize_ = regular_dimensions_;
        dirty_.grid = true;
        dirty_.mod_grid = true;
    }

    void NineSlice::set_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions)
    {
        for (unsigned char x = 0; x < 3; ++x)
            for (unsigned char y = 0; y < 3; ++y)
                sprite(x, y).set_texture(texture, dimensions);
        regular_dimensions_ = dimensions;
        nsize_ = regular_dimensions_;
        dirty_.grid = true;
        dirty_.mod_grid = true;
    }

    void NineSlice::set_texture(const graphics::BindlessTextureRef& texture)
    {
        glm::vec2 dimensions = context::get_texture_size(texture);
        for (unsigned char x = 0; x < 3; ++x)
            for (unsigned char y = 0; y < 3; ++y)
                sprite(x, y).set_texture(texture, dimensions);
        regular_dimensions_ = dimensions;
        nsize_ = regular_dimensions_;
        dirty_.grid = true;
        dirty_.mod_grid = true;
    }

    void NineSlice::set_tex_coords(math::UVRect rect)
    {
        regular_uvs_ = rect;
        dirty_.grid = true;
    }

    void NineSlice::set_modulation(glm::vec4 modulation)
    {
        regular_modulation_ = modulation;
        dirty_.modulation = true;
    }

    void NineSlice::set_frame_format(const graphics::AnimFrameFormat& anim) const
    {
        for (unsigned char x = 0; x < 3; ++x)
            for (unsigned char y = 0; y < 3; ++y)
                sprite(x, y).set_frame_format(anim);
    }

    void NineSlice::set_camera_invariant(bool is_camera_invariant) const
    {
        for (unsigned char x = 0; x < 3; ++x)
            for (unsigned char y = 0; y < 3; ++y)
                sprite(x, y).set_camera_invariant(is_camera_invariant);
    }

    void NineSlice::set_mod_texture(const detail::ResourcePath& texture_file, unsigned int texture_index)
    {
        for (unsigned char x = 0; x < 3; ++x)
            for (unsigned char y = 0; y < 3; ++y)
                sprite(x, y).set_mod_texture(texture_file, texture_index);
        regular_mod_dimensions_ = context::get_texture_size(texture_file, texture_index);
    }

    void NineSlice::set_mod_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions)
    {
        for (unsigned char x = 0; x < 3; ++x)
            for (unsigned char y = 0; y < 3; ++y)
                sprite(x, y).set_mod_texture(texture, dimensions);
        regular_mod_dimensions_ = dimensions;
    }

    void NineSlice::set_mod_texture(const graphics::BindlessTextureRef& texture)
    {
        glm::vec2 dimensions = context::get_texture_size(texture);
        for (unsigned char x = 0; x < 3; ++x)
            for (unsigned char y = 0; y < 3; ++y)
                sprite(x, y).set_mod_texture(texture, dimensions);
        regular_mod_dimensions_ = dimensions;
    }

    void NineSlice::set_mod_tex_coords(math::UVRect rect)
    {
        regular_mod_uvs_ = rect;
        dirty_.mod_grid = true;
    }

    graphics::BindlessTextureRef NineSlice::get_texture() const
    {
        return sprite(0, 0).get_texture();
    }

    graphics::BindlessTextureRef NineSlice::get_texture(glm::vec2& dimensions) const
    {
        dimensions = regular_dimensions_;
        return sprite(0, 0).get_texture();
    }

    math::UVRect NineSlice::get_tex_coords() const
    {
        return regular_uvs_;
    }

    glm::vec4 NineSlice::get_modulation() const
    {
        return regular_modulation_;
    }

    graphics::AnimFrameFormat NineSlice::get_frame_format() const
    {
        return sprite(0, 0).get_frame_format();
    }

    bool NineSlice::is_camera_invariant() const
    {
        return sprite(0, 0).is_camera_invariant();
    }

    graphics::BindlessTextureRef NineSlice::get_mod_texture() const
    {
        return sprite(0, 0).get_mod_texture();
    }

    graphics::BindlessTextureRef NineSlice::get_mod_texture(glm::vec2& dimensions) const
    {
        dimensions = regular_mod_dimensions_;
        return sprite(0, 0).get_mod_texture();
    }

    math::UVRect NineSlice::get_mod_tex_coords() const
    {
        return regular_mod_uvs_;
    }

    math::Padding NineSlice::get_offsets() const
    {
        return offsets_;
    }

    math::Padding& NineSlice::set_offsets()
    {
        dirty_.grid = true;
        dirty_.mod_grid = true;
        return offsets_;
    }

    float NineSlice::width() const
    {
        return nsize_.x;
    }

    float NineSlice::height() const
    {
        return nsize_.y;
    }

    glm::vec2 NineSlice::size() const
    {
        return nsize_;
    }

    void NineSlice::set_width(float w)
    {
        clamp_nsize({ w, nsize_.y });
    }

    void NineSlice::set_height(float h)
    {
        clamp_nsize({ nsize_.x, h });
    }

    void NineSlice::set_size(glm::vec2 size)
    {
        clamp_nsize(size);
    }

    void NineSlice::setup_nonant(glm::vec2 size, math::Padding offs)
    {
        offsets_ = offs;
        clamp_nsize(size);
    }

    void NineSlice::setup_nonant(const Sprite& copy, glm::vec2 size, math::Padding offs)
    {
        copy_sprite_attributes(copy);
        setup_nonant(size, offs);
    }

    glm::vec2 NineSlice::get_dimensions() const
    {
        return regular_dimensions_;
    }

    void NineSlice::init()
    {
        for (unsigned char x = 0; x < 3; ++x)
        {
            for (unsigned char y = 0; y < 3; ++y)
            {
                sprite(x, y).transformer.attach_parent(&transformer_);
                imp::poly<PivotTransformModifier2D> modifier = imp::poly_default;
                modifier->pivot.x = -0.5f * x + 1.0f;
                modifier->pivot.y = -0.5f * y + 1.0f;
                sprite(x, y).transformer.set_modifier() = std::move(modifier);
            }
        }
    }

    Sprite& NineSlice::sprite(unsigned char x, unsigned char y) const
    {
        return sprites_[size_t(3 * y + x)];
    }

    void NineSlice::clamp_nsize(glm::vec2 size)
    {
        nsize_.x = std::max(size.x, offsets_.left + offsets_.right);
        nsize_.y = std::max(size.y, offsets_.bottom + offsets_.top);
        dirty_.grid = true;
        dirty_.mod_grid = true;
    }

    void NineSlice::sync_grid() const
    {
        // [2][0] [2][1] [2][2]
        // [1][0] [1][1] [1][2]
        // [0][0] [0][1] [0][2]

        float xuvs[4]{ 0.0f, offsets_.left / regular_dimensions_.x, 1.0f - offsets_.right / regular_dimensions_.x, 1.0f };
        float yuvs[4]{ 0.0f, offsets_.bottom / regular_dimensions_.y, 1.0f - offsets_.top / regular_dimensions_.y, 1.0f };

        for (size_t n = 0; n < 4; ++n)
        {
            xuvs[n] = regular_uvs_.interp_x(xuvs[n]);
            yuvs[n] = regular_uvs_.interp_y(yuvs[n]);
        }

        float widths[3]{ offsets_.left, nsize_.x - offsets_.left - offsets_.right, offsets_.right };
        float heights[3]{ offsets_.bottom, nsize_.y - offsets_.bottom - offsets_.top, offsets_.top };

        float xpos[3]{ -0.5f * widths[1], 0.0f, 0.5f * widths[1] };
        float ypos[3]{ -0.5f * heights[1], 0.0f, 0.5f * heights[1] };

        for (unsigned char x = 0; x < 3; ++x)
            for (unsigned char y = 0; y < 3; ++y)
            {
                sprite(x, y).set_tex_coords({ .x1 = xuvs[x], .x2 = xuvs[x + 1], .y1 = yuvs[y], .y2 = yuvs[y + 1] });
                glm::vec2 size = { widths[x], heights[y] };
                sprite(x, y).transformer.ref_modifier<PivotTransformModifier2D>().size = size;
                Transform2D& local = sprite(x, y).set_local();
                local.scale = size / regular_dimensions_;
                local.position = glm::vec2{ xpos[x], ypos[y] };
            }
    }

    void NineSlice::sync_modulation() const
    {
        for (unsigned char x = 0; x < 3; ++x)
            for (unsigned char y = 0; y < 3; ++y)
                sprite(x, y).set_modulation(regular_modulation_);
    }

    void NineSlice::sync_mod_grid() const
    {
        // [2][0] [2][1] [2][2]
        // [1][0] [1][1] [1][2]
        // [0][0] [0][1] [0][2]

        float xuvs[4]{ 0.0f, offsets_.left / nsize_.x, 1.0f - offsets_.right / nsize_.x, 1.0f };
        float yuvs[4]{ 0.0f, offsets_.bottom / nsize_.y, 1.0f - offsets_.top / nsize_.y, 1.0f };

        for (size_t n = 0; n < 4; ++n)
        {
            xuvs[n] = regular_mod_uvs_.interp_x(xuvs[n]);
            yuvs[n] = regular_mod_uvs_.interp_y(yuvs[n]);
        }

        for (unsigned char x = 0; x < 3; ++x)
            for (unsigned char y = 0; y < 3; ++y)
                sprite(x, y).set_mod_tex_coords({ .x1 = xuvs[x], .x2 = xuvs[x + 1], .y1 = yuvs[y], .y2 = yuvs[y + 1] });
    }
}
