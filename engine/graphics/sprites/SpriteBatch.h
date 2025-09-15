#pragma once

#include "graphics/backend/specialized/UsageSlotTracker.h"
#include "graphics/backend/specialized/ElementBuffers.h"
#include "graphics/backend/basic/Textures.h"

#include "core/math/Shapes.h"
#include "core/base/Constants.h"

namespace oly::rendering
{
	class SpriteBatch;

	namespace internal
	{
		struct SpriteReference;

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
	}

	class SpriteBatch
	{
		friend struct internal::SpriteReference;

		graphics::VertexArray vao;
		graphics::PersistentEBO<6> ebo;

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

		struct
		{
			GLuint projection, modulation, time;
		} shader_locations;

	public:
		struct AnimHash
		{
			size_t operator()(const graphics::AnimFrameFormat& anim) const {
				return std::hash<GLuint>{}(anim.starting_frame) ^ (std::hash<GLuint>{}(anim.num_frames) << 1)
					^ (std::hash<float>{}(anim.starting_time) << 2) ^ (std::hash<float>{}(anim.delay_seconds) << 3);
			}
		};

	private:
		// TODO v4 once these are template variables in shader, make these max variables configurable to match.
		static const GLuint max_modulations = 1000;
		static const GLuint max_anims = 1000;

		struct UBO
		{
			graphics::LightweightUBO<graphics::Mutability::MUTABLE> modulation, anim;

			UBO() : modulation(max_modulations * sizeof(glm::vec4), sizeof(glm::vec4)), anim(max_anims * sizeof(graphics::AnimFrameFormat), sizeof(graphics::AnimFrameFormat)) {}
		} ubo;

	public:
		glm::mat3 projection = 1.0f;
		glm::vec4 global_modulation = glm::vec4(1.0f);

		SpriteBatch();
		SpriteBatch(const SpriteBatch&) = delete;
		SpriteBatch(SpriteBatch&&) = delete;
		~SpriteBatch();

		void render() const;

	private:
		typedef StrictIDGenerator<GLuint>::ID SpriteID;
		StrictIDGenerator<GLuint> id_generator;
		SpriteID gen_sprite_id();
		void erase_sprite_id(const SpriteID& id);

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

	namespace internal
	{
		// TODO v4 support setting different batch
		struct SpriteReference
		{
			SpriteBatch& batch;
			const bool in_context;
			SpriteBatch::SpriteID id;

			SpriteReference(SpriteBatch* batch = nullptr);
			SpriteReference(const SpriteReference&);
			SpriteReference(SpriteReference&&) noexcept;
			~SpriteReference();
			SpriteReference& operator=(const SpriteReference&);
			SpriteReference& operator=(SpriteReference&&) noexcept;

			bool is_in_context() const { return in_context; }

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

			std::invoke_result_t<decltype(&decltype(SpriteBatch::ebo)::draw_primitive), decltype(SpriteBatch::ebo)> draw_primitive() const;
			void draw_quad() const;
		};
	}
}
