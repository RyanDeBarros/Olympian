#include "Spritebatch.h"

#include "core/context/rendering/Sprites.h"
#include "core/context/rendering/Textures.h"

#include "core/util/Time.h"

#include "graphics/resources/Shaders.h"

#include <imp/hash.hpp>

namespace oly::rendering::internal
{
    void update_texture_handle(const graphics::BindlessTextureRef& texture)
    {
        for (SpriteBatch* batch : SpriteBatch::tracked())
            batch->update_texture_handle(texture);
    }

    SpriteBatch::UBOCapacity::UBOCapacity(GLushort modulations, GLushort anims)
        : modulations_(glm::min(modulations, (GLushort)kMaxModulations))
        , anims_(glm::min(anims, (GLushort)kMaxAnims))
    {
    }

    GLushort SpriteBatch::UBOCapacity::modulations() const
    {
        return modulations_;
    }

    GLushort SpriteBatch::UBOCapacity::anims() const
    {
        return anims_;
    }

    SpriteBatch::UBO::UBO(UBOCapacity capacity)
        : modulation(capacity.modulations() * sizeof(glm::vec4), sizeof(glm::vec4))
        , anim(capacity.anims() * sizeof(graphics::AnimFrameFormat), sizeof(graphics::AnimFrameFormat))
    {
    }

    size_t SpriteBatch::AnimHash::operator()(const graphics::AnimFrameFormat& anim) const
    {
        return imp::hasher()
            .with(anim.starting_frame)
            .with(anim.num_frames)
            .with(anim.starting_time)
            .with(anim.delay_seconds);
    }

    size_t SpriteBatch::QuadInfoStore::SizedTextureHash::operator()(const SizedTexture& t) const
    {
        return imp::hasher().with(t.texture).with(t.size);
    }

    SpriteBatch::SpriteBatch(UBOCapacity capacity)
        : ebo_(vao_)
        , shader_ref_(graphics::internal_shaders::sprite_batch(capacity.modulations(), capacity.anims()))
        , tex_data_ssbo_(graphics::SHADER_STORAGE_MAX_BUFFER_SIZE, sizeof(TexData))
        , tex_coords_ssbo_(graphics::SHADER_STORAGE_MAX_BUFFER_SIZE, sizeof(math::UVRect))
        , ubo_(capacity)
    {
        shader_ = *shader_ref_;

        shader_locations_.projection = glGetUniformLocation(shader_, "uProjection");
        shader_locations_.invariant_projection = glGetUniformLocation(shader_, "uInvariantProjection");
        shader_locations_.modulation = glGetUniformLocation(shader_, "uGlobalModulation");
        shader_locations_.time = glGetUniformLocation(shader_, "uTime");

        tex_coords_ssbo_.send<math::UVRect>(0, {});
        ubo_.modulation.send<glm::vec4>(0, glm::vec4(1.0f));
        ubo_.anim.send<graphics::AnimFrameFormat>(0, {});
    }

    void SpriteBatch::render() const
    {
        if (ebo_.empty() || !camera)
            return;

        quad_ssbo_block_.pre_draw_all();

        glBindVertexArray(vao_);
        glUseProgram(shader_);

        glUniformMatrix3fv(
            shader_locations_.projection,
            1,
            GL_FALSE,
            glm::value_ptr(camera->projection_matrix()));

        glUniformMatrix3fv(
            shader_locations_.invariant_projection,
            1,
            GL_FALSE,
            glm::value_ptr(camera->invariant_projection_matrix()));

        glUniform4f(
            shader_locations_.modulation,
            global_modulation[0],
            global_modulation[1],
            global_modulation[2],
            global_modulation[3]);
        
        glUniform1f(shader_locations_.time, TIME.now<>());

        tex_data_ssbo_.bind_base(0);
        quad_ssbo_block_.buf.bind_ssbo_base<SSBOSlot::Info>(1);
        quad_ssbo_block_.buf.bind_ssbo_base<SSBOSlot::Transform>(2);
        tex_coords_ssbo_.bind_base(3);
        ubo_.modulation.bind_base(0);
        ubo_.anim.bind_base(1);
        ebo_.render_elements(GL_TRIANGLES);

        quad_ssbo_block_.post_draw_all();
    }

    void SpriteBatch::update_texture_handle(const graphics::BindlessTextureRef& texture)
    {
        auto iter = quad_info_store_.dimensionless_texture_slot_map.find(texture);
        if (iter != quad_info_store_.dimensionless_texture_slot_map.end())
        {
            GLuint64 handle = texture->get_handle();
            const auto& slots = iter->second;
            for (auto it = slots.begin(); it != slots.end(); ++it)
                tex_data_ssbo_.send<TexData>(*it, &TexData::handle, handle);
        }
    }

    void SpriteBatch::assert_valid_id(GLuint id)
    {
        if (id == kNullId) [[unlikely]]
            throw Error(ErrorCode::InvalidID);
    }

    const SpriteBatch::QuadInfo& SpriteBatch::get_quad_info(GLuint vb_pos) const
    {
        return quad_ssbo_block_.get<SSBOSlot::Info>(vb_pos);
    }

    SpriteBatch::QuadInfo& SpriteBatch::set_quad_info(GLuint vb_pos)
    {
        return quad_ssbo_block_.set<SSBOSlot::Info>(vb_pos);
    }

    GLuint SpriteBatch::gen_sprite_id()
    {
        GLuint id = id_generator_.gen();
        if (id == kNullId)
            throw Error(ErrorCode::StorageOverflow);

        quad_ssbo_block_.set<SSBOSlot::Info>(id) = {};
        quad_ssbo_block_.set<SSBOSlot::Transform>(id) = 1.0f;
        return id;
    }

    void SpriteBatch::erase_sprite_id(GLuint id)
    {
        const auto decrement_texture = [this](GLushort slot) {
            if (auto texture = quad_info_store_.textures.decrement_usage(slot))
            {
                auto& map = quad_info_store_.dimensionless_texture_slot_map;
                auto it = map.find(texture.value().texture);
                if (it != map.end())
                {
                    auto& slots = it->second;
                    slots.erase(slot);
                    if (slots.empty())
                        map.erase(it);
                }
            }
        };

        if (id != kNullId) [[likely]]
        {
            const QuadInfo& quad_info = quad_ssbo_block_.buf.at<SSBOSlot::Info>(id);
            decrement_texture(quad_info.tex_slot);
            decrement_texture(quad_info.mod_tex_slot);
            quad_info_store_.tex_coords.decrement_usage(quad_info.tex_coord_slot);
            quad_info_store_.modulations.decrement_usage(quad_info.color_slot);
            id_generator_.yield(id);
        }
    }

    void SpriteBatch::set_texture(GLuint vb_pos, const graphics::BindlessTextureRef& texture, glm::vec2 size)
    {
        SpriteBatch::assert_valid_id(vb_pos);
        update_texture_slot(vb_pos, quad_ssbo_block_.buf.at<SSBOSlot::Info>(vb_pos).tex_slot, texture, size);
    }

    void SpriteBatch::set_tex_coords(GLuint vb_pos, math::UVRect uvs)
    {
        SpriteBatch::assert_valid_id(vb_pos);
        if (quad_info_store_.tex_coords.set_object(
                tex_coords_ssbo_,
                quad_ssbo_block_.buf.at<SSBOSlot::Info>(vb_pos).tex_coord_slot, uvs))
        {
            quad_ssbo_block_.flag<SSBOSlot::Info>(vb_pos);
        }
    }

    void SpriteBatch::set_modulation(GLuint vb_pos, glm::vec4 modulation)
    {
        SpriteBatch::assert_valid_id(vb_pos);
        if (quad_info_store_.modulations.set_object(
                ubo_.modulation,
                quad_ssbo_block_.buf.at<SSBOSlot::Info>(vb_pos).color_slot, modulation))
        {
            quad_ssbo_block_.flag<SSBOSlot::Info>(vb_pos);
        }
    }

    void SpriteBatch::set_frame_format(GLuint vb_pos, const graphics::AnimFrameFormat& anim)
    {
        SpriteBatch::assert_valid_id(vb_pos);
        if (quad_info_store_.anims.set_object(
                ubo_.anim,
                quad_ssbo_block_.buf.at<SSBOSlot::Info>(vb_pos).frame_slot, anim))
        {
            quad_ssbo_block_.flag<SSBOSlot::Info>(vb_pos);
        }
    }

    void SpriteBatch::set_text_glyph(GLuint vb_pos, bool is_text_glyph)
    {
        SpriteBatch::assert_valid_id(vb_pos);
        if (is_text_glyph)
            set_quad_info(vb_pos).flags |= QuadInfo::kGlyphFlag;
        else
            set_quad_info(vb_pos).flags &= ~QuadInfo::kGlyphFlag;
    }

    void SpriteBatch::set_camera_invariant(GLuint vb_pos, bool is_camera_invariant)
    {
        SpriteBatch::assert_valid_id(vb_pos);
        if (is_camera_invariant)
            set_quad_info(vb_pos).flags |= QuadInfo::kCamInvFlag;
        else
            set_quad_info(vb_pos).flags &= ~QuadInfo::kCamInvFlag;
    }

    void SpriteBatch::set_mod_texture(GLuint vb_pos, const graphics::BindlessTextureRef& texture, glm::vec2 size)
    {
        SpriteBatch::assert_valid_id(vb_pos);
        update_texture_slot(vb_pos, quad_ssbo_block_.buf.at<SSBOSlot::Info>(vb_pos).mod_tex_slot, texture, size);
    }

    void SpriteBatch::set_mod_tex_coords(GLuint vb_pos, math::UVRect uvs)
    {
        SpriteBatch::assert_valid_id(vb_pos);
        if (quad_info_store_.tex_coords.set_object(
                tex_coords_ssbo_,
                quad_ssbo_block_.buf.at<SSBOSlot::Info>(vb_pos).mod_tex_coord_slot, uvs))
        {
            quad_ssbo_block_.flag<SSBOSlot::Info>(vb_pos);
        }
    }

    graphics::BindlessTextureRef SpriteBatch::get_texture(GLuint vb_pos, glm::vec2& size) const
    {
        SpriteBatch::assert_valid_id(vb_pos);
        GLuint slot = get_quad_info(vb_pos).tex_slot;
        if (slot == 0)
            return nullptr;
        QuadInfoStore::SizedTexture tex = quad_info_store_.textures.get_object(slot);
        size = tex.size;
        return tex.texture;
    }

    math::UVRect SpriteBatch::get_tex_coords(GLuint vb_pos) const
    {
        SpriteBatch::assert_valid_id(vb_pos);
        GLuint slot = get_quad_info(vb_pos).tex_coord_slot;
        return slot != 0 ? quad_info_store_.tex_coords.get_object(slot) : math::UVRect{};
    }

    glm::vec4 SpriteBatch::get_modulation(GLuint vb_pos) const
    {
        SpriteBatch::assert_valid_id(vb_pos);
        GLuint slot = get_quad_info(vb_pos).color_slot;
        return slot != 0 ? quad_info_store_.modulations.get_object(slot) : glm::vec4(1.0f);
    }

    graphics::AnimFrameFormat SpriteBatch::get_frame_format(GLuint vb_pos) const
    {
        SpriteBatch::assert_valid_id(vb_pos);
        GLuint slot = get_quad_info(vb_pos).frame_slot;
        return slot != 0 ? quad_info_store_.anims.get_object(slot) : graphics::AnimFrameFormat{};
    }

    bool SpriteBatch::is_text_glyph(GLuint vb_pos) const
    {
        SpriteBatch::assert_valid_id(vb_pos);
        return get_quad_info(vb_pos).flags & QuadInfo::kGlyphFlag;
    }

    bool SpriteBatch::is_camera_invariant(GLuint vb_pos) const
    {
        SpriteBatch::assert_valid_id(vb_pos);
        return get_quad_info(vb_pos).flags & QuadInfo::kCamInvFlag;
    }

    graphics::BindlessTextureRef SpriteBatch::get_mod_texture(GLuint vb_pos, glm::vec2& size) const
    {
        SpriteBatch::assert_valid_id(vb_pos);
        GLuint slot = get_quad_info(vb_pos).mod_tex_slot;
        if (slot == 0)
            return nullptr;
        QuadInfoStore::SizedTexture tex = quad_info_store_.textures.get_object(slot);
        size = tex.size;
        return tex.texture;
    }

    math::UVRect SpriteBatch::get_mod_tex_coords(GLuint vb_pos) const
    {
        SpriteBatch::assert_valid_id(vb_pos);
        GLuint slot = get_quad_info(vb_pos).mod_tex_coord_slot;
        return slot != 0 ? quad_info_store_.tex_coords.get_object(slot) : math::UVRect{};
    }

    void SpriteBatch::update_texture_slot(
        GLuint vb_pos,
        GLushort& tex_slot,
        const graphics::BindlessTextureRef& texture,
        glm::vec2 size)
    {
        GLushort old_tex_slot = tex_slot;
        std::optional<QuadInfoStore::SizedTexture> old_texture;
        if (quad_info_store_.textures.set_object<TexData>(
                tex_data_ssbo_,
                tex_slot,
                QuadInfoStore::SizedTexture{ texture, texture ? size : glm::vec2(0.0f) },
                TexData{ texture ? texture->get_handle() : 0, size }, old_texture))
        {
            if (old_texture)
            {
                auto it = quad_info_store_.dimensionless_texture_slot_map.find(old_texture->texture);
                it->second.erase(old_tex_slot);
                if (it->second.empty())
                    quad_info_store_.dimensionless_texture_slot_map.erase(it);
            }

            quad_ssbo_block_.flag<SSBOSlot::Info>(vb_pos);
            quad_info_store_.dimensionless_texture_slot_map[texture].insert(tex_slot);
        }
    }

    struct Attributes
    {
        graphics::BindlessTextureRef texture = nullptr;
        glm::vec2 texture_dimensions = {};
        math::UVRect tex_coords = {};
        glm::vec4 modulation = glm::vec4(1.0f);
        graphics::AnimFrameFormat frame_format = {};
        graphics::BindlessTextureRef mod_texture = nullptr;
        glm::vec2 mod_texture_dimensions = {};
        math::UVRect mod_tex_coords = {};
        glm::mat3 transform = 1.0f;
        bool is_text_glyph = false;
        bool camera_invariant = false;
    };

    struct AttributesRef
    {
        graphics::BindlessTextureRef texture;
        glm::vec2 texture_dimensions;
        math::UVRect tex_coords;
        glm::vec4 modulation;
        graphics::AnimFrameFormat frame_format;
        graphics::BindlessTextureRef mod_texture;
        glm::vec2 mod_texture_dimensions;
        math::UVRect mod_tex_coords;
        const glm::mat3& transform;
        bool is_text_glyph;
        bool camera_invariant;
    };

    static Attributes get_attributes(const SpriteReference& ref)
    {
        glm::vec2 texture_dimensions{}, mod_texture_dimensions{};
        Attributes attr{
            .texture = ref.get_texture(texture_dimensions),
            .tex_coords = ref.get_tex_coords(),
            .modulation = ref.get_modulation(),
            .frame_format = ref.get_frame_format(),
            .mod_texture = ref.get_mod_texture(mod_texture_dimensions),
            .mod_tex_coords = ref.get_mod_tex_coords(),
            .transform = ref.get_transform(),
            .is_text_glyph = ref.is_text_glyph(),
            .camera_invariant = ref.is_camera_invariant()
        };
        attr.texture_dimensions = texture_dimensions;
        attr.mod_texture_dimensions = mod_texture_dimensions;
        return attr;
    }

    static AttributesRef get_attributes_ref(const SpriteReference& ref)
    {
        glm::vec2 texture_dimensions{}, mod_texture_dimensions{};
        AttributesRef attr{
            .texture = ref.get_texture(texture_dimensions),
            .tex_coords = ref.get_tex_coords(),
            .modulation = ref.get_modulation(),
            .frame_format = ref.get_frame_format(),
            .mod_texture = ref.get_mod_texture(mod_texture_dimensions),
            .mod_tex_coords = ref.get_mod_tex_coords(),
            .transform = ref.get_transform(),
            .is_text_glyph = ref.is_text_glyph(),
            .camera_invariant = ref.is_camera_invariant()
        };
        attr.texture_dimensions = texture_dimensions;
        attr.mod_texture_dimensions = mod_texture_dimensions;
        return attr;
    }

    static void set_attributes(SpriteReference& ref, const Attributes& attr)
    {
        ref.set_texture(attr.texture, attr.texture_dimensions);
        ref.set_tex_coords(attr.tex_coords);
        ref.set_modulation(attr.modulation);
        ref.set_frame_format(attr.frame_format);
        ref.set_mod_texture(attr.mod_texture, attr.mod_texture_dimensions);
        ref.set_mod_tex_coords(attr.mod_tex_coords);
        ref.set_transform(attr.transform);
        ref.set_text_glyph(attr.is_text_glyph);
        ref.set_camera_invariant(attr.camera_invariant);
    }

    static void set_attributes(SpriteReference& ref, const AttributesRef& attr)
    {
        ref.set_texture(attr.texture, attr.texture_dimensions);
        ref.set_tex_coords(attr.tex_coords);
        ref.set_modulation(attr.modulation);
        ref.set_frame_format(attr.frame_format);
        ref.set_mod_texture(attr.mod_texture, attr.mod_texture_dimensions);
        ref.set_mod_tex_coords(attr.mod_tex_coords);
        ref.set_transform(attr.transform);
        ref.set_text_glyph(attr.is_text_glyph);
        ref.set_camera_invariant(attr.camera_invariant);
    }

    SpriteReference::SpriteReference()
        : Super(context::sprite_batch()->weak_from_this())
    {
        if (auto batch = lock())
        {
            id_ = batch->gen_sprite_id();
            set_attributes(*this, Attributes{});
        }
        else
        {
            _OLY_ENGINE_LOG_WARNING("RENDERING")
                << "Cannot create default sprite - context sprite batch is null" << LOG.nl;
        }
    }

    SpriteReference::SpriteReference(Unbatched)
    {
    }

    SpriteReference::SpriteReference(SpriteBatch& batch)
        : Super(batch.weak_from_this())
    {
        id_ = batch.gen_sprite_id();
        set_attributes(*this, Attributes{});
    }

    SpriteReference::SpriteReference(rendering::SpriteBatch& batch)
        : Super(batch->weak_from_this())
    {
        id_ = batch->gen_sprite_id();
        set_attributes(*this, Attributes{});
    }

    SpriteReference::SpriteReference(const SpriteReference& other)
        : Super(other)
    {
        if (other.id_ != SpriteBatch::kNullId)
        {
            if (auto batch = lock())
            {
                id_ = batch->gen_sprite_id();
                set_attributes(*this, get_attributes_ref(other));
            }
        }
    }

    SpriteReference::SpriteReference(SpriteReference&& other) noexcept
        : Super(std::move(other)), id_(other.id_)
    {
        other.id_ = SpriteBatch::kNullId;
    }

    SpriteReference::~SpriteReference()
    {
        if (auto batch = lock())
            batch->erase_sprite_id(id_);
    }

    SpriteReference& SpriteReference::operator=(const SpriteReference& other)
    {
        if (this != &other)
            *this = dupl(other);

        return *this;
    }

    SpriteReference& SpriteReference::operator=(SpriteReference&& other) noexcept
    {
        if (this != &other)
        {
            if (auto batch = lock())
                batch->erase_sprite_id(id_);
        
            Super::operator=(std::move(other));
            id_ = other.id_;
            other.id_ = SpriteBatch::kNullId;
        }

        return *this;
    }

    bool SpriteReference::is_in_context() const
    {
        if (auto batch = lock())
            return batch.get() == context::sprite_batch().address();

        return false;
    }

    SpriteReference::accessor SpriteReference::get_batch() const
    {
        return lock();
    }

    void SpriteReference::set_batch(Unbatched)
    {
        if (auto batch = lock())
            batch->erase_sprite_id(id_);

        id_ = SpriteBatch::kNullId;
        reset();
    }

    void SpriteReference::set_batch(rendering::SpriteBatch& new_batch)
    {
        if (auto batch = lock())
        {
            if (batch.get() == new_batch.address())
                return;

            const Attributes attr = id_ != SpriteBatch::kNullId ? get_attributes(*this) : Attributes{};
            batch->erase_sprite_id(id_);
            reset(*new_batch);
            batch = lock();
            id_ = batch->gen_sprite_id();
            set_attributes(*this, attr);
        }
        else
        {
            reset(*new_batch);
            batch = lock();
            id_ = batch->gen_sprite_id();
            set_attributes(*this, Attributes{});
        }
    }

    void SpriteReference::set_texture(const detail::ResourcePath& texture_file, unsigned int texture_index) const
    {
        if (auto batch = lock()) [[likely]]
        {
            SpriteBatch::assert_valid_id(id_);
            graphics::BindlessTextureRef texture = context::load_texture(texture_file, texture_index);
            glm::vec2 size = context::get_texture_size(texture_file, texture_index);
            batch->set_texture(id_, texture, size);
        }
        else
            throw Error(ErrorCode::NullPointer);
    }

    void SpriteReference::set_texture(const graphics::BindlessTextureRef& texture) const
    {
        SpriteBatch::assert_valid_id(id_);
        if (auto batch = lock()) [[likely]]
            batch->set_texture(id_, texture, texture ? context::get_texture_size(texture) : glm::vec2{});
        else
            throw Error(ErrorCode::NullPointer);
    }

    void SpriteReference::set_texture(const graphics::BindlessTextureRef& texture, glm::vec2 size) const
    {
        if (auto batch = lock()) [[likely]]
            batch->set_texture(id_, texture, size);
        else
            throw Error(ErrorCode::NullPointer);
    }

    void SpriteReference::set_tex_coords(math::UVRect rect) const
    {
        if (auto batch = lock()) [[likely]]
            batch->set_tex_coords(id_, rect);
        else
            throw Error(ErrorCode::NullPointer);
    }

    void SpriteReference::set_modulation(glm::vec4 modulation) const
    {
        if (auto batch = lock()) [[likely]]
            batch->set_modulation(id_, modulation);
        else
            throw Error(ErrorCode::NullPointer);
    }

    void SpriteReference::set_frame_format(const graphics::AnimFrameFormat& anim) const
    {
        if (auto batch = lock()) [[likely]]
            batch->set_frame_format(id_, anim);
        else
            throw Error(ErrorCode::NullPointer);
    }

    void SpriteReference::set_text_glyph(bool is_text_glyph) const
    {
        if (auto batch = lock()) [[likely]]
            batch->set_text_glyph(id_, is_text_glyph);
        else
            throw Error(ErrorCode::NullPointer);
    }

    void SpriteReference::set_camera_invariant(bool is_camera_invariant) const
    {
        if (auto batch = lock()) [[likely]]
            batch->set_camera_invariant(id_, is_camera_invariant);
        else
            throw Error(ErrorCode::NullPointer);
    }

    void SpriteReference::set_mod_texture(const detail::ResourcePath& texture_file, unsigned int texture_index) const
    {
        if (auto batch = lock()) [[likely]]
        {
            SpriteBatch::assert_valid_id(id_);
            auto texture = context::load_texture(texture_file, texture_index);
            batch->set_mod_texture(id_, texture, context::get_texture_size(texture_file, texture_index));
        }
        else
            throw Error(ErrorCode::NullPointer);
    }

    void SpriteReference::set_mod_texture(const graphics::BindlessTextureRef& texture) const
    {
        SpriteBatch::assert_valid_id(id_);
        if (auto batch = lock()) [[likely]]
            batch->set_mod_texture(id_, texture, context::get_texture_size(texture));
        else
            throw Error(ErrorCode::NullPointer);
    }

    void SpriteReference::set_mod_texture(const graphics::BindlessTextureRef& texture, glm::vec2 size) const
    {
        if (auto batch = lock()) [[likely]]
            batch->set_mod_texture(id_, texture, size);
        else
            throw Error(ErrorCode::NullPointer);
    }

    void SpriteReference::set_mod_tex_coords(math::UVRect rect) const
    {
        if (auto batch = lock()) [[likely]]
            batch->set_mod_tex_coords(id_, rect);
        else
            throw Error(ErrorCode::NullPointer);
    }

    void SpriteReference::set_transform(const glm::mat3& transform) const
    {
        SpriteBatch::assert_valid_id(id_);
        if (auto batch = lock()) [[likely]]
            batch->quad_ssbo_block_.set<SpriteBatch::SSBOSlot::Transform>(id_) = transform;
        else
            throw Error(ErrorCode::NullPointer);
    }

    graphics::BindlessTextureRef SpriteReference::get_texture() const
    {
        if (auto batch = lock()) [[likely]]
        {
            glm::vec2 _;
            return batch->get_texture(id_, _);
        }
        else
            throw Error(ErrorCode::NullPointer);
    }

    graphics::BindlessTextureRef SpriteReference::get_texture(glm::vec2& size) const
    {
        if (auto batch = lock()) [[likely]]
            return batch->get_texture(id_, size);
        else
            throw Error(ErrorCode::NullPointer);
    }

    math::UVRect SpriteReference::get_tex_coords() const
    {
        if (auto batch = lock()) [[likely]]
            return batch->get_tex_coords(id_);
        else
            throw Error(ErrorCode::NullPointer);
    }

    glm::vec4 SpriteReference::get_modulation() const
    {
        if (auto batch = lock()) [[likely]]
            return batch->get_modulation(id_);
        else
            throw Error(ErrorCode::NullPointer);
    }

    graphics::AnimFrameFormat SpriteReference::get_frame_format() const
    {
        if (auto batch = lock()) [[likely]]
            return batch->get_frame_format(id_);
        else
            throw Error(ErrorCode::NullPointer);
    }

    bool SpriteReference::is_text_glyph() const
    {
        if (auto batch = lock()) [[likely]]
            return batch->is_text_glyph(id_);
        else
            throw Error(ErrorCode::NullPointer);
    }

    bool SpriteReference::is_camera_invariant() const
    {
        if (auto batch = lock()) [[likely]]
            return batch->is_camera_invariant(id_);
        else
            throw Error(ErrorCode::NullPointer);
    }

    graphics::BindlessTextureRef SpriteReference::get_mod_texture() const
    {
        if (auto batch = lock()) [[likely]]
        {
            glm::vec2 _;
            return batch->get_mod_texture(id_, _);
        }
        else
            throw Error(ErrorCode::NullPointer);
    }

    graphics::BindlessTextureRef SpriteReference::get_mod_texture(glm::vec2& size) const
    {
        if (auto batch = lock()) [[likely]]
            return batch->get_mod_texture(id_, size);
        else
            throw Error(ErrorCode::NullPointer);
    }

    math::UVRect SpriteReference::get_mod_tex_coords() const
    {
        if (auto batch = lock()) [[likely]]
            return batch->get_mod_tex_coords(id_);
        else
            throw Error(ErrorCode::NullPointer);
    }

    glm::mat3 SpriteReference::get_transform() const
    {
        if (auto batch = lock()) [[likely]]
        {
            SpriteBatch::assert_valid_id(id_);
            return batch->quad_ssbo_block_.get<SpriteBatch::SSBOSlot::Transform>(id_);
        }
        else
            throw Error(ErrorCode::NullPointer);
    }

    void SpriteReference::draw_quad() const
    {
        if (auto batch = lock()) [[likely]]
        {
            SpriteBatch::assert_valid_id(id_);
            graphics::quad_indices(batch->ebo_.draw_primitive().data(), id_);
        }
        else
            throw Error(ErrorCode::NullPointer);
    }
}
