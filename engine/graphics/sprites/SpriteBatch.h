#pragma once

#include "core/base/Constants.h"

#include "core/math/Shapes.h"

#include "graphics/Camera.h"
#include "graphics/Tags.h"

#include "graphics/backend/basic/Shader.h"
#include "graphics/backend/basic/Textures.h"

#include "graphics/backend/specialized/ElementBuffers.h"
#include "graphics/backend/specialized/UsageSlotTracker.h"

#include <imp/auto_tracker.hpp>
#include <imp/id_generator.hpp>

namespace oly::rendering
{
    namespace internal
    {
        extern void update_texture_handle(const graphics::BindlessTextureRef& texture);

        class SpriteReference;

        class SpriteBatch
            : public imp::auto_trackable<SpriteBatch>
            , public imp::issuer<SpriteBatch>
        {
            friend class SpriteReference;

        public:
            class UBOCapacity
            {
                static const GLuint kMaxModulations = 1000; // guaranteed 16KB / 16B = #1000
                static const GLuint kMaxAnims = 1000; // guaranteed 16KB / 16B = #1000

                GLushort modulations_;
                GLushort anims_;

            public:
                UBOCapacity(GLushort modulations = kMaxModulations, GLushort anims = kMaxAnims);

                GLushort modulations() const;
                GLushort anims() const;
            };

        private:
            struct UBO
            {
                graphics::LightweightUBO<graphics::Mutability::Mutable> modulation;
                graphics::LightweightUBO<graphics::Mutability::Mutable> anim;

                UBO(UBOCapacity capacity);
            };

            struct TexData
            {
                GLuint64 handle = 0;
                glm::vec2 size = {};
            };

            struct AnimHash
            {
                size_t operator()(const graphics::AnimFrameFormat& anim) const;
            };

            struct QuadInfo
            {
                static const GLushort kGlyphFlag = 1 << 0;
                static const GLushort kCamInvFlag = 1 << 1;

                GLushort tex_slot = 0;
                GLushort tex_coord_slot = 0;
                GLushort color_slot = 0;
                GLushort frame_slot = 0;
                GLushort flags = 0;
                GLushort mod_tex_slot = 0;
                GLushort mod_tex_coord_slot = 0;
            };

            struct QuadInfoStore
            {
                struct SizedTexture
                {
                    graphics::BindlessTextureRef texture;
                    glm::vec2 size = {};

                    bool operator==(const SizedTexture& t) const = default;
                };

                struct SizedTextureHash
                {
                    size_t operator()(const SizedTexture& t) const;
                };

                graphics::UsageSlotTracker<SizedTexture, GLushort, SizedTextureHash> textures;
                graphics::UsageSlotTracker<math::UVRect, GLushort> tex_coords;
                graphics::UsageSlotTracker<glm::vec4, GLushort> modulations;
                graphics::UsageSlotTracker<graphics::AnimFrameFormat, GLushort, AnimHash> anims;

                std::unordered_map<graphics::BindlessTextureRef, std::unordered_set<GLuint>> dimensionless_texture_slot_map;
            };

        private:
            static const GLuint kNullId = GLuint(-1);

            graphics::PersistentEBO<6> ebo_;
            graphics::VertexArray vao_;
            GLuint shader_;
            SmartReference<graphics::Shader> shader_ref_;

            struct
            {
                GLuint projection;
                GLuint invariant_projection;
                GLuint modulation;
                GLuint time;
            } shader_locations_;

            graphics::LightweightSSBO<graphics::Mutability::Mutable> tex_data_ssbo_;

            struct SSBOSlot
            {
                enum
                {
                    Info,
                    Transform
                };
            };

            graphics::LazyPersistentGPUBufferBlock<QuadInfo, glm::mat3> quad_ssbo_block_;
            graphics::LightweightSSBO<graphics::Mutability::Mutable> tex_coords_ssbo_;
            UBO ubo_;
            imp::soft_id_generator<GLuint> id_generator_;
            QuadInfoStore quad_info_store_;

        public:
            Camera2DRef camera = kRefDefault;
            glm::vec4 global_modulation = glm::vec4(1.0f);

        public:
            SpriteBatch(UBOCapacity = {});
            SpriteBatch(const SpriteBatch&) = delete;
            SpriteBatch(SpriteBatch&&) = delete;

        public:
            void render() const;
            void update_texture_handle(const graphics::BindlessTextureRef& texture);

        private:
            static void assert_valid_id(GLuint id);

            const QuadInfo& get_quad_info(GLuint vb_pos) const;
            QuadInfo& set_quad_info(GLuint vb_pos);

            GLuint gen_sprite_id();
            void erase_sprite_id(GLuint id);

            void set_texture(GLuint vb_pos, const graphics::BindlessTextureRef& texture, glm::vec2 size);
            void set_tex_coords(GLuint vb_pos, math::UVRect uvs);
            void set_modulation(GLuint vb_pos, glm::vec4 modulation);
            void set_frame_format(GLuint vb_pos, const graphics::AnimFrameFormat& anim);
            void set_text_glyph(GLuint vb_pos, bool is_text_glyph);
            void set_camera_invariant(GLuint vb_pos, bool is_camera_invariant);
            void set_mod_texture(GLuint vb_pos, const graphics::BindlessTextureRef& texture, glm::vec2 size);
            void set_mod_tex_coords(GLuint vb_pos, math::UVRect uvs);

            graphics::BindlessTextureRef get_texture(GLuint vb_pos, glm::vec2& size) const;
            math::UVRect get_tex_coords(GLuint vb_pos) const;
            glm::vec4 get_modulation(GLuint vb_pos) const;
            graphics::AnimFrameFormat get_frame_format(GLuint vb_pos) const;
            bool is_text_glyph(GLuint vb_pos) const;
            bool is_camera_invariant(GLuint vb_pos) const;
            graphics::BindlessTextureRef get_mod_texture(GLuint vb_pos, glm::vec2& size) const;
            math::UVRect get_mod_tex_coords(GLuint vb_pos) const;

            void update_texture_slot(
                GLuint vb_pos,
                GLushort& tex_slot,
                const graphics::BindlessTextureRef& texture,
                glm::vec2 size);
        };
    }

    using SpriteBatch = imp::issuer_instance<internal::SpriteBatch>;

    namespace internal
    {
        class SpriteReference
            : public imp::ticket<SpriteBatch>
        {
            using Super = imp::ticket<SpriteBatch>;

            GLuint id_ = SpriteBatch::kNullId;

        public:
            SpriteReference();
            SpriteReference(Unbatched);
            SpriteReference(SpriteBatch& batch);
            SpriteReference(rendering::SpriteBatch& batch);
            SpriteReference(const SpriteReference&);
            SpriteReference(SpriteReference&&) noexcept;
            ~SpriteReference();

            SpriteReference& operator=(const SpriteReference&);
            SpriteReference& operator=(SpriteReference&&) noexcept;

        public:
            bool is_in_context() const;
            accessor get_batch() const;
            void set_batch(Unbatched);
            void set_batch(rendering::SpriteBatch& batch);

            void set_texture(const detail::ResourcePath& texture_file, unsigned int texture_index = 0) const;
            void set_texture(const graphics::BindlessTextureRef& texture) const;
            void set_texture(const graphics::BindlessTextureRef& texture, glm::vec2 size) const;
            void set_tex_coords(math::UVRect uvs) const;
            void set_modulation(glm::vec4 modulation) const;
            void set_frame_format(const graphics::AnimFrameFormat& anim) const;
            void set_text_glyph(bool is_text_glyph) const;
            void set_camera_invariant(bool is_camera_invariant) const;
            void set_mod_texture(const detail::ResourcePath& texture_file, unsigned int texture_index = 0) const;
            void set_mod_texture(const graphics::BindlessTextureRef& texture) const;
            void set_mod_texture(const graphics::BindlessTextureRef& texture, glm::vec2 size) const;
            void set_mod_tex_coords(math::UVRect uvs) const;
            void set_transform(const glm::mat3& transform) const;

            graphics::BindlessTextureRef get_texture() const;
            graphics::BindlessTextureRef get_texture(glm::vec2& size) const;
            math::UVRect get_tex_coords() const;
            glm::vec4 get_modulation() const;
            graphics::AnimFrameFormat get_frame_format() const;
            bool is_text_glyph() const;
            bool is_camera_invariant() const;
            graphics::BindlessTextureRef get_mod_texture() const;
            graphics::BindlessTextureRef get_mod_texture(glm::vec2& size) const;
            math::UVRect get_mod_tex_coords() const;
            glm::mat3 get_transform() const;

            void draw_quad() const;
        };
    }
}
