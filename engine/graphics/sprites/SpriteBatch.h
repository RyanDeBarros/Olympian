#pragma once

#include "graphics/backend/specialized/UsageSlotTracker.h"
#include "graphics/backend/specialized/ElementBuffers.h"
#include "graphics/backend/basic/Textures.h"
#include "graphics/Tags.h"
#include "graphics/Camera.h"

#include "core/math/Shapes.h"
#include "core/base/Constants.h"
#include "core/types/Issuer.h"

namespace oly::rendering
{
	namespace internal
	{
		class SpriteBatch;

		class SpriteBatchRegistry
		{
			friend class SpriteBatch;
			std::unordered_set<SpriteBatch*> batches;

			SpriteBatchRegistry() = default;
			SpriteBatchRegistry(const SpriteBatchRegistry&) = delete;
			SpriteBatchRegistry(SpriteBatchRegistry&&) = delete;

		public:
			void update_texture_handle(const graphics::BindlessTextureRef& texture);

			static SpriteBatchRegistry& instance() { static SpriteBatchRegistry reg; return reg; }
		};

		class SpriteReference;

		class SpriteBatch : public oly::internal::Issuer<SpriteBatch>
		{
			friend class SpriteReference;

			graphics::VertexArray vao;
			graphics::PersistentEBO<6> ebo;
			GLuint shader;

			struct
			{
				GLuint projection, modulation, time;
			} shader_locations;

			struct TexData
			{
				GLuint64 handle = 0;
				glm::vec2 dimensions = {};
			};

			graphics::LightweightSSBO<graphics::Mutability::MUTABLE> tex_data_ssbo;

			struct QuadInfo
			{
				GLushort tex_slot = 0;
				GLushort tex_coord_slot = 0;
				GLushort color_slot = 0;
				GLushort frame_slot = 0;
				GLushort is_text_glyph = 0;
				GLushort mod_tex_slot = 0;
				GLushort mod_tex_coord_slot = 0;
			};

			enum
			{
				INFO,
				TRANSFORM
			};
			graphics::LazyPersistentGPUBufferBlock<QuadInfo, glm::mat3> quad_ssbo_block;

			const QuadInfo& get_quad_info(GLuint vb_pos) const;
			QuadInfo& set_quad_info(GLuint vb_pos);

			graphics::LightweightSSBO<graphics::Mutability::MUTABLE> tex_coords_ssbo;

			struct AnimHash
			{
				size_t operator()(const graphics::AnimFrameFormat& anim) const {
					return std::hash<GLuint>{}(anim.starting_frame) ^ (std::hash<GLuint>{}(anim.num_frames) << 1)
						^ (std::hash<float>{}(anim.starting_time) << 2) ^ (std::hash<float>{}(anim.delay_seconds) << 3);
				}
			};

		public:
			class UBOCapacity
			{
				static const GLuint max_modulations = 1000; // guaranteed 16KB / 16B = #1000
				static const GLuint max_anims = 1000; // guaranteed 16KB / 16B = #1000

				GLushort _modulations;
				GLushort _anims;

			public:
				UBOCapacity(GLushort modulations = max_modulations, GLushort anims = max_anims)
					: _modulations(glm::min(modulations, (GLushort)max_modulations)), _anims(glm::min(anims, (GLushort)max_anims))
				{
				}

				GLushort modulations() const { return _modulations; }
				GLushort anims() const { return _anims; }
			};

		private:
			struct UBO
			{
				graphics::LightweightUBO<graphics::Mutability::MUTABLE> modulation, anim;

				UBO(UBOCapacity capacity)
					: modulation(capacity.modulations() * sizeof(glm::vec4), sizeof(glm::vec4)),
					anim(capacity.anims() * sizeof(graphics::AnimFrameFormat), sizeof(graphics::AnimFrameFormat))
				{
				}
			} ubo;

		public:
			Camera2DRef camera = REF_DEFAULT;
			glm::vec4 global_modulation = glm::vec4(1.0f);

			SpriteBatch(UBOCapacity = {});
			SpriteBatch(const SpriteBatch&) = delete;
			SpriteBatch(SpriteBatch&&) = delete;
			~SpriteBatch();

			void render() const;
			void render(const glm::mat3& projection) const;

		private:
			SoftIDGenerator<GLuint> id_generator;
			static const GLuint NULL_ID = GLuint(-1);
			static void assert_valid_id(GLuint id);
			GLuint gen_sprite_id();
			void erase_sprite_id(GLuint id);

			struct QuadInfoStore
			{
				struct SizedTexture
				{
					graphics::BindlessTextureRef texture;
					glm::vec2 dimensions = {};

					bool operator==(const SizedTexture& t) const = default;
				};

				struct SizedTextureHash
				{
					size_t operator()(const SizedTexture& t) const { return std::hash<graphics::BindlessTextureRef>{}(t.texture) ^ std::hash<glm::vec2>{}(t.dimensions); }
				};

				graphics::UsageSlotTracker<SizedTexture, GLushort, SizedTextureHash> textures;
				graphics::UsageSlotTracker<math::UVRect, GLushort> tex_coords;
				graphics::UsageSlotTracker<glm::vec4, GLushort> modulations;
				graphics::UsageSlotTracker<graphics::AnimFrameFormat, GLushort, AnimHash> anims;

				std::unordered_map<graphics::BindlessTextureRef, std::unordered_set<GLuint>> dimensionless_texture_slot_map;
			} quad_info_store;

			void set_texture(GLuint vb_pos, const graphics::BindlessTextureRef& texture, glm::vec2 dimensions);
			void set_tex_coords(GLuint vb_pos, math::UVRect uvs);
			void set_modulation(GLuint vb_pos, glm::vec4 modulation);
			void set_frame_format(GLuint vb_pos, const graphics::AnimFrameFormat& anim);
			void set_text_glyph(GLuint vb_pos, bool is_text_glyph);
			void set_mod_texture(GLuint vb_pos, const graphics::BindlessTextureRef& texture, glm::vec2 dimensions);
			void set_mod_tex_coords(GLuint vb_pos, math::UVRect uvs);

			graphics::BindlessTextureRef get_texture(GLuint vb_pos, glm::vec2& dimensions) const;
			math::UVRect get_tex_coords(GLuint vb_pos) const;
			glm::vec4 get_modulation(GLuint vb_pos) const;
			graphics::AnimFrameFormat get_frame_format(GLuint vb_pos) const;
			bool is_text_glyph(GLuint vb_pos) const;
			graphics::BindlessTextureRef get_mod_texture(GLuint vb_pos, glm::vec2& dimensions) const;
			math::UVRect get_mod_tex_coords(GLuint vb_pos) const;

		public:
			void update_texture_handle(const graphics::BindlessTextureRef& texture);
		};
	}

	using SpriteBatch = PublicIssuer<internal::SpriteBatch>;

	namespace internal
	{
		class SpriteReference : public PublicIssuerHandle<SpriteBatch>
		{
			using Super = PublicIssuerHandle<SpriteBatch>;
			GLuint id = SpriteBatch::NULL_ID;

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

			bool is_in_context() const;
			auto get_batch() const { return lock(); }
			void set_batch(Unbatched);
			void set_batch(rendering::SpriteBatch& batch);

			void set_texture(const std::string& texture_file, unsigned int texture_index = 0) const;
			void set_texture(const graphics::BindlessTextureRef& texture) const;
			void set_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions) const;
			void set_tex_coords(math::UVRect uvs) const;
			void set_modulation(glm::vec4 modulation) const;
			void set_frame_format(const graphics::AnimFrameFormat& anim) const;
			void set_text_glyph(bool is_text_glyph) const;
			void set_mod_texture(const std::string& texture_file, unsigned int texture_index = 0) const;
			void set_mod_texture(const graphics::BindlessTextureRef& texture) const;
			void set_mod_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions) const;
			void set_mod_tex_coords(math::UVRect uvs) const;
			void set_transform(const glm::mat3& transform) const;

			graphics::BindlessTextureRef get_texture() const;
			graphics::BindlessTextureRef get_texture(glm::vec2& dimensions) const;
			math::UVRect get_tex_coords() const;
			glm::vec4 get_modulation() const;
			graphics::AnimFrameFormat get_frame_format() const;
			bool is_text_glyph() const;
			graphics::BindlessTextureRef get_mod_texture() const;
			graphics::BindlessTextureRef get_mod_texture(glm::vec2& dimensions) const;
			math::UVRect get_mod_tex_coords() const;
			glm::mat3 get_transform() const;

			void draw_quad() const;
		};
	}
}
