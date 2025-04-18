#pragma once

#include "core/Core.h"
#include "math/Transforms.h"
#include "SpecializedBuffers.h"
#include "util/FreeSpaceTracker.h"
#include "util/IDGenerator.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace oly
{
	namespace renderable
	{
		struct Sprite;
	}

	namespace batch
	{
		class SpriteBatch
		{
			friend struct renderable::Sprite;
			
			rendering::VertexArray vao;
			rendering::QuadLayoutEBO<rendering::Mutability::MUTABLE, GLuint> ebo;

			struct SSBO
			{
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
				};

				rendering::LightweightSSBO<rendering::Mutability::MUTABLE> tex_data;
				rendering::IndexedSSBO<QuadInfo, GLuint, rendering::Mutability::MUTABLE> quad_info;
				rendering::IndexedSSBO<glm::mat3, GLuint, rendering::Mutability::MUTABLE> quad_transform;

				SSBO(GLuint textures, GLuint sprites) : tex_data(textures * sizeof(TexData)), quad_info(sprites), quad_transform(sprites) {}
			} ssbo;

			struct
			{
				GLuint projection, modulation;
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
		private:
			struct UBO
			{
				rendering::LightweightUBO<rendering::Mutability::MUTABLE> tex_coords, modulation;

				UBO(GLuint uvs, GLuint modulations) : tex_coords(uvs * sizeof(TexUVRect)), modulation(modulations * sizeof(Modulation)) {}
			} ubo;

		public:
			glm::vec4 projection_bounds, global_modulation = glm::vec4(1.0f);

			struct Capacity
			{
				Capacity(GLuint initial_sprites, GLuint new_textures, GLuint new_uvs = 0, GLuint new_modulations = 0)
					: sprites(initial_sprites), textures(new_textures + 1), uvs(new_uvs + 1), modulations(new_modulations + 1) {}

			private:
				friend class SpriteBatch;
				GLuint sprites, textures, uvs, modulations;
			};

			SpriteBatch(Capacity capacity, const glm::vec4& projection_bounds);

			void render() const;

		private:
			mutable GLuint sprites_to_draw = 0;

			std::unordered_map<renderable::Sprite*, GLuint> sprites;
			IDGenerator<GLuint> vb_pos_generator;
			GLuint gen_sprite_pos();
			void erase_sprite_pos(GLuint vb_pos);

			template<typename Property>
			struct SSBOIndexTracker
			{
				Property prop;
				GLuint usage = 0;
			};
			struct QuadInfoStore
			{
				struct Texture
				{
					rendering::BindlessTextureRes texture;
					glm::vec2 dimensions;
				};
				std::unordered_map<GLuint, SSBOIndexTracker<Texture>> textures;
				std::unordered_map<GLuint, SSBOIndexTracker<TexUVRect>> tex_coords;
				std::unordered_map<GLuint, SSBOIndexTracker<Modulation>> modulations;
			} quad_info_store;

			void set_texture(GLuint vb_pos, const rendering::BindlessTextureRes& texture, glm::vec2 dimensions);
			void set_tex_coords(GLuint vb_pos, const TexUVRect& uvs);
			void set_modulation(GLuint vb_pos, const Modulation& modulation);

			// TODO some kind of (expensive) prune() that gets rid of extra space in ssbos/ubos/quad_info_store?

			void draw_sprite(GLuint vb_pos);
		};
	}

	namespace renderable
	{
		struct Sprite
		{
		private:
			friend class batch::SpriteBatch;
			batch::SpriteBatch* batch;
			GLuint vb_pos;

		public:
			Transformer2D transformer;

			Sprite(batch::SpriteBatch* sprite_batch);
			Sprite(const Sprite&) = delete; // TODO implement clone() that copies over texture, tex coord, transformer, and modulation. Transformer2D::clone() should copy upper hierarchy and local, but leave children empty.
			Sprite(Sprite&&) noexcept;
			Sprite& operator=(Sprite&&) noexcept;
			~Sprite();

			void draw() const;

			void set_texture(const rendering::BindlessTextureRes& texture, glm::vec2 dimensions) const;
			void set_tex_coords(const batch::SpriteBatch::TexUVRect& tex_coords) const;
			void set_modulation(const batch::SpriteBatch::Modulation& modulation) const;

			// TODO getters

			const batch::SpriteBatch& get_batch() const { return *batch; }
			batch::SpriteBatch& get_batch() { return *batch; }
			const Transform2D& local() const { return transformer.local; }
			Transform2D& local() { return transformer.local; }
			void post_set() const; // call after modifying local
			void pre_get() const; // call before reading global

		private:
			void flush() const;
		};
	}
}
