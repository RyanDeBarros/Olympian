#pragma once

#include "../SpecializedBuffers.h"
#include "math/Transforms.h"
#include "util/FreeSpaceTracker.h"
#include "util/IDGenerator.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace oly
{
	namespace mut
	{
		struct Sprite;

		class SpriteBatch
		{
			friend struct Sprite;
			
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

			void render();
			void flush() const;

		private:
			mutable GLuint sprites_to_draw = 0;
			mutable bool resize_ebo = false;
			mutable bool resize_sprites = false;

			std::unordered_map<Sprite*, GLuint> sprites;
			IDGenerator<GLuint> vb_pos_generator;
			GLuint gen_sprite_pos();
			void erase_sprite_pos(GLuint vb_pos);

			template<typename StoredObjectType, typename StoredObjectTypeHash>
			class BOStore
			{
				struct UsageHolder
				{
					StoredObjectType obj;
					GLuint usage = 0;
				};

				std::unordered_map<GLuint, UsageHolder> usages;
				std::unordered_map<StoredObjectType, GLuint, StoredObjectTypeHash> slot_lookup;
				IDGenerator<GLuint> pos_generator;

				void _decrement_usage(GLuint i)
				{
					auto it = usages.find(i);
					--it->second.usage;
					if (it->second.usage == 0)
						erase_slot(it);
				}
				
				void erase_slot(const typename decltype(usages)::iterator& it) { pos_generator.yield(it->first); slot_lookup.erase(it->second.obj); usages.erase(it); }

			public:
				BOStore() { pos_generator.gen(); /* waste 0th slot */ }
				
				void decrement_usage(GLuint i) { if (i != 0) _decrement_usage(i); }
				
				void set_object(rendering::LightweightBuffer<rendering::Mutability::MUTABLE>& buffer, SpriteBatch& sprite_batch, GLuint& slot, GLuint pos,
					const StoredObjectType& stored_obj, const std::function<bool(const StoredObjectType&)>& is_default)
				{ set_object<StoredObjectType>(buffer, sprite_batch, slot, pos, stored_obj, stored_obj, is_default); }
				
				template<typename BufferObjectType>
				void set_object(rendering::LightweightBuffer<rendering::Mutability::MUTABLE>& buffer, SpriteBatch& sprite_batch, GLuint& slot, GLuint pos,
					const StoredObjectType& stored_obj, const BufferObjectType& buffer_obj, const std::function<bool(const StoredObjectType&)>& is_default)
				{
					if (is_default(stored_obj)) // remove object from sprite
					{
						if (slot != 0)
						{
							_decrement_usage(slot);
							slot = 0;
							sprite_batch.ssbo.quad_info.lazy_send(pos);
						}
						return;
					}
					if (slot != 0) // sprite has existing object -> decrement its usage
					{
						auto it = usages.find(slot);
						if (stored_obj == it->second.obj) // same object that exists -> do nothing
							return;
						--it->second.usage;
						if (it->second.usage == 0)
							erase_slot(it);
					}
					auto newit = slot_lookup.find(stored_obj);
					if (newit != slot_lookup.end()) // object already exists -> increment its usage
					{
						++usages.find(newit->second)->second.usage;
						slot = newit->second;
						sprite_batch.ssbo.quad_info.lazy_send(pos);
					}
					else // create new object slot
					{
						slot = pos_generator.gen();
						usages[slot] = { stored_obj, 1 };
						slot_lookup[stored_obj] = slot;
						OLY_ASSERT(slot * sizeof(BufferObjectType) <= (GLuint)buffer.get_size());
						if (slot * sizeof(BufferObjectType) == buffer.get_size())
							buffer.grow(buffer.get_size() + sizeof(BufferObjectType));
						buffer.send<BufferObjectType>(slot, buffer_obj);
						sprite_batch.ssbo.quad_info.lazy_send(pos);
					}
				}

				const StoredObjectType& get_object(GLuint slot) const { return usages.find(slot)->second.obj; }
			};

			struct QuadInfoStore
			{
				struct Texture
				{
					rendering::BindlessTextureRes texture;
					glm::vec2 dimensions = {};

					bool operator==(const Texture&) const = default;
				};
				struct TextureHash
				{
					size_t operator()(const Texture& t) const { return std::hash<rendering::BindlessTextureRes>{}(t.texture) ^ std::hash<glm::vec2>{}(t.dimensions); }
				};
				BOStore<Texture, TextureHash> textures;
				BOStore<TexUVRect, TexUVRectHash> tex_coords;
				BOStore<Modulation, ModulationHash> modulations;
			} quad_info_store;

			void set_texture(GLuint vb_pos, const rendering::BindlessTextureRes& texture, glm::vec2 dimensions);
			void set_tex_coords(GLuint vb_pos, const TexUVRect& uvs);
			void set_modulation(GLuint vb_pos, const Modulation& modulation);

			rendering::BindlessTextureRes get_texture(GLuint vb_pos, glm::vec2& dimensions) const;
			TexUVRect get_tex_coords(GLuint vb_pos) const;
			Modulation get_modulation(GLuint vb_pos) const;

			void draw_sprite(GLuint vb_pos);
		};

		struct Sprite
		{
		private:
			friend class SpriteBatch;
			SpriteBatch* batch;
			GLuint vb_pos;

		public:
			Transformer2D transformer;

			Sprite(SpriteBatch* sprite_batch);
			Sprite(const Sprite&);
			Sprite(Sprite&&) noexcept;
			Sprite& operator=(const Sprite&);
			Sprite& operator=(Sprite&&) noexcept;
			~Sprite();

			void draw() const;

			void set_texture(const rendering::BindlessTextureRes& texture, glm::vec2 dimensions) const;
			void set_tex_coords(const SpriteBatch::TexUVRect& tex_coords) const;
			void set_modulation(const SpriteBatch::Modulation& modulation) const;

			rendering::BindlessTextureRes get_texture() const;
			rendering::BindlessTextureRes get_texture(glm::vec2& dimensions) const;
			SpriteBatch::TexUVRect get_tex_coords() const;
			SpriteBatch::Modulation get_modulation() const;

			const SpriteBatch& get_batch() const { return *batch; }
			SpriteBatch& get_batch() { return *batch; }
			const Transform2D& local() const { return transformer.local; }
			Transform2D& local() { return transformer.local; }
			void post_set() const; // call after modifying local
			void pre_get() const; // call before reading global

		private:
			void flush() const;
		};
	}
}
