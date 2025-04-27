#pragma once

#include "../SpecializedBuffers.h"
#include "../UsageSlotTracker.h"
#include "util/IDGenerator.h"
#include "math/Transforms.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace oly
{
	namespace rendering
	{
		struct Sprite;

		class SpriteBatch
		{
			friend struct Sprite;

			VertexArray vao;
			PersistentEBO<> ebo;

			struct TexData
			{
				GLuint64 handle = 0;
				glm::vec2 dimensions = {};
			};

			struct QuadInfo
			{
				GLuint tex_slot = 0;
				GLuint tex_coord_slot = 0;
				GLuint color_slot = 0;
				GLuint frame_slot = 0;
			};

			LightweightSSBO<Mutability::MUTABLE> tex_data_ssbo;
			enum
			{
				INFO,
				TRANSFORM
			};
			LazyPersistentGPUBufferBlock<QuadInfo, glm::mat3> quad_ssbo_block;

			const QuadInfo& get_quad_info(GLuint vb_pos) const;
			QuadInfo& set_quad_info(GLuint vb_pos);

			struct
			{
				GLuint projection, modulation, time;
			} shader_locations;
		public:
			struct TexUVRect
			{
				glm::vec2 uvs[4] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };

				bool operator==(const TexUVRect&) const = default;
			};
			struct TexUVRectHash
			{
				size_t operator()(const TexUVRect& uvs) const {
					return std::hash<glm::vec2>{}(uvs.uvs[0]) ^ (std::hash<glm::vec2>{}(uvs.uvs[1]) << 1)
						^ (std::hash<glm::vec2>{}(uvs.uvs[2]) << 2) ^ (std::hash<glm::vec2>{}(uvs.uvs[3]) << 3);
				}
			};
			struct Modulation
			{
				glm::vec4 colors[4] = { glm::vec4(1.0f), glm::vec4(1.0f), glm::vec4(1.0f), glm::vec4(1.0f) };

				bool operator==(const Modulation&) const = default;
			};
			struct ModulationHash
			{
				size_t operator()(const Modulation& mod) const {
					return std::hash<glm::vec4>{}(mod.colors[0]) ^ (std::hash<glm::vec4>{}(mod.colors[1]) << 1)
						^ (std::hash<glm::vec4>{}(mod.colors[2]) << 2) ^ (std::hash<glm::vec4>{}(mod.colors[3]) << 3);
				}
			};
			struct AnimHash
			{
				size_t operator()(const AnimFrameFormat& anim) const {
					return std::hash<GLuint>{}(anim.starting_frame) ^ (std::hash<GLuint>{}(anim.num_frames) << 1)
						^ (std::hash<float>{}(anim.starting_time) << 2) ^ (std::hash<float>{}(anim.delay_seconds) << 3);
				}
			};
		private:
			struct UBO
			{
				LightweightUBO<Mutability::MUTABLE> tex_coords, modulation, anim;

				UBO(GLuint uvs, GLuint modulations, GLuint anims) : tex_coords(uvs * sizeof(TexUVRect)), modulation(modulations * sizeof(Modulation)), anim(anims * sizeof(AnimFrameFormat)) {}
			} ubo;

		public:
			glm::vec4 projection_bounds, global_modulation = glm::vec4(1.0f);

			struct Capacity
			{
				Capacity(GLuint initial_sprites, GLuint new_textures, GLuint new_uvs = 0, GLuint new_modulations = 0, GLuint num_anims = 0)
					: sprites(initial_sprites), textures(new_textures + 1), uvs(new_uvs + 1), modulations(new_modulations + 1), anims(num_anims + 1)
				{
					OLY_ASSERT(4 * initial_sprites <= UINT_MAX);
					OLY_ASSERT(uvs <= 500);
					OLY_ASSERT(modulations <= 250);
					OLY_ASSERT(anims <= 1000);
				}

			private:
				friend class SpriteBatch;
				GLuint sprites, textures, uvs, modulations, anims;
			};

			SpriteBatch(Capacity capacity, const glm::vec4& projection_bounds);

			void render() const;

		private:
			StrictIDGenerator<GLuint> vbid_generator;
			typedef StrictIDGenerator<GLuint>::ID VBID;
			VBID gen_sprite_id();
			void erase_sprite_id(GLuint id);

			struct QuadInfoStore
			{
				struct DimensionlessTexture
				{
					BindlessTextureRes texture;
					glm::vec2 dimensions = {};

					bool operator==(const DimensionlessTexture& t) const { return texture == t.texture; }
				};
				struct DimensionlessTextureHash
				{
					size_t operator()(const DimensionlessTexture& t) const { return std::hash<BindlessTextureRes>{}(t.texture); }
				};
				UsageSlotTracker<DimensionlessTexture, DimensionlessTextureHash> textures;
				UsageSlotTracker<TexUVRect, TexUVRectHash> tex_coords;
				UsageSlotTracker<Modulation, ModulationHash> modulations;
				UsageSlotTracker<AnimFrameFormat, AnimHash> anims;
			} quad_info_store;

			void set_texture(GLuint vb_pos, const BindlessTextureRes& texture, glm::vec2 dimensions);
			void set_tex_coords(GLuint vb_pos, const TexUVRect& uvs);
			void set_modulation(GLuint vb_pos, const Modulation& modulation);
			void set_frame_format(GLuint vb_pos, const AnimFrameFormat& anim);

			BindlessTextureRes get_texture(GLuint vb_pos, glm::vec2& dimensions) const;
			TexUVRect get_tex_coords(GLuint vb_pos) const;
			Modulation get_modulation(GLuint vb_pos) const;
			AnimFrameFormat get_frame_format(GLuint vb_pos) const;

		public:
			void update_texture_handle(const BindlessTextureRes& texture);
			void update_texture_handle(const BindlessTextureRes& texture, glm::vec2 dimensions);
		};
	}

	class TextureRegistry;
	class Context;
	namespace rendering
	{

		struct Sprite
		{
		private:
			friend class SpriteBatch;
			SpriteBatch* batch;
			SpriteBatch::VBID vbid;

		public:
			Transformer2D transformer;

			Sprite(SpriteBatch* sprite_batch);
			Sprite(const Sprite&);
			Sprite(Sprite&&) noexcept;
			Sprite& operator=(const Sprite&);
			Sprite& operator=(Sprite&&) noexcept;
			~Sprite();

			void draw() const;

			void set_texture(const TextureRegistry* texture_registry, const std::string& texture_name) const;
			void set_texture(const Context* context, const std::string& texture_name) const;
			void set_texture(const BindlessTextureRes& texture, glm::vec2 dimensions) const;
			void set_tex_coords(const SpriteBatch::TexUVRect& tex_coords) const;
			void set_modulation(const SpriteBatch::Modulation& modulation) const;
			void set_frame_format(const AnimFrameFormat& anim) const;

			BindlessTextureRes get_texture() const;
			BindlessTextureRes get_texture(glm::vec2& dimensions) const;
			SpriteBatch::TexUVRect get_tex_coords() const;
			SpriteBatch::Modulation get_modulation() const;
			AnimFrameFormat get_frame_format() const;

			const SpriteBatch& get_batch() const { return *batch; }
			SpriteBatch& get_batch() { return *batch; }
			const Transform2D& local() const { return transformer.local; }
			Transform2D& local() { return transformer.local; }
			void post_set() const; // call after modifying local
			void pre_get() const; // call before reading global
		};
	}
}
