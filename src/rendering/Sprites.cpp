#include "Sprites.h"

#include <glm/gtc/type_ptr.hpp>

#include "Resources.h"

namespace oly
{
	namespace batch
	{
		SpriteBatch::SpriteBatch(Capacity capacity, const glm::vec4& projection_bounds)
			: ebo(capacity.sprites), ssbo(capacity.textures, capacity.sprites), ubo(capacity.uvs, capacity.modulations), projection_bounds(projection_bounds)
		{
			shader_locations.projection = glGetUniformLocation(shaders::sprite_batch, "uProjection");
			shader_locations.modulation = glGetUniformLocation(shaders::sprite_batch, "uGlobalModulation");

			ubo.tex_coords.send<TexUVRect>(0, {});
			ubo.modulation.send<Modulation>(0, {});

			glBindVertexArray(vao);
			ebo.init();
			glBindVertexArray(0);
		}

		void SpriteBatch::render() const
		{
			oly::check_errors();
			glBindVertexArray(vao);
			glUseProgram(shaders::sprite_batch);
			glUniformMatrix3fv(shader_locations.projection, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]))));
			glUniform4f(shader_locations.modulation, global_modulation[0], global_modulation[1], global_modulation[2], global_modulation[3]);

			ebo.flush();
			ssbo.quad_info.flush();
			ssbo.quad_transform.flush();

			ssbo.tex_data.bind_base(0);
			ssbo.quad_info.bind_base(1);
			ssbo.quad_transform.bind_base(2);
			ubo.tex_coords.bind_base(0);
			ubo.modulation.bind_base(1);
			glDrawElements(GL_TRIANGLES, (GLsizei)(sprites_to_draw * 6), GL_UNSIGNED_INT, (void*)0);
			sprites_to_draw = 0;

			oly::check_errors();
		}

		GLuint SpriteBatch::gen_sprite_pos()
		{
			GLuint vb_pos = vb_pos_generator.gen();
			OLY_ASSERT(vb_pos <= ssbo.quad_info.vector().size());
			if (vb_pos == ssbo.quad_info.vector().size())
			{
				// TODO batch these gl resizes?
				ssbo.quad_info.vector().push_back({});
				glNamedBufferData(ssbo.quad_info.buffer(), ssbo.quad_info.vector().size() * sizeof(decltype(ssbo.quad_info)::StructAlias), ssbo.quad_info.vector().data(), GL_DYNAMIC_DRAW);
				ssbo.quad_transform.vector().push_back({});
				glNamedBufferData(ssbo.quad_transform.buffer(), ssbo.quad_transform.vector().size() * sizeof(decltype(ssbo.quad_transform)::StructAlias), ssbo.quad_transform.vector().data(), GL_DYNAMIC_DRAW);
			}
			ssbo.quad_info.vector()[vb_pos] = {};
			ssbo.quad_info.lazy_send(vb_pos);
			ssbo.quad_transform.vector()[vb_pos] = glm::mat3(1.0f);
			ssbo.quad_transform.lazy_send(vb_pos);
			return vb_pos;
		}

		void SpriteBatch::erase_sprite_pos(GLuint vb_pos)
		{
			vb_pos_generator.yield(vb_pos);
			const SSBO::QuadInfo& quad_info = ssbo.quad_info.vector()[vb_pos];
			if (quad_info.tex_slot != 0)
			{
				auto it = quad_info_store.textures.find(quad_info.tex_slot);
				--it->second.usage;
				if (it->second.usage == 0)
					quad_info_store.textures.erase(it);
			}
			if (quad_info.tex_coord_slot != 0)
			{
				auto it = quad_info_store.tex_coords.find(quad_info.tex_coord_slot);
				--it->second.usage;
				if (it->second.usage == 0)
					quad_info_store.tex_coords.erase(it);
			}
			if (quad_info.color_slot != 0)
			{
				auto it = quad_info_store.modulations.find(quad_info.color_slot);
				--it->second.usage;
				if (it->second.usage == 0)
					quad_info_store.modulations.erase(it);
			}
		}

		// TODO create custom data structures in quad_info_store that are more efficient

		void SpriteBatch::set_texture(GLuint vb_pos, const rendering::BindlessTextureRes& texture, glm::vec2 dimensions)
		{
			SSBO::QuadInfo& quad_info = ssbo.quad_info.vector()[vb_pos];
			if (texture == nullptr) // remove texture from sprite
			{
				if (quad_info.tex_slot != 0) // sprite has existing texture -> decrement its usage and remove it
				{
					auto it = quad_info_store.textures.find(quad_info.tex_slot);
					--it->second.usage;
					if (it->second.usage == 0)
						quad_info_store.textures.erase(it);
					quad_info.tex_slot = 0;
					ssbo.quad_info.lazy_send(vb_pos);
				}
				return;
			}
			if (quad_info.tex_slot != 0) // sprite has existing texture -> decrement its usage
			{
				auto it = quad_info_store.textures.find(quad_info.tex_slot);
				if (texture == it->second.prop.texture && dimensions == it->second.prop.dimensions) // same texture that exists -> do nothing
					return;
				--it->second.usage;
				if (it->second.usage == 0)
					quad_info_store.textures.erase(it);
			}
			for (auto newit = quad_info_store.textures.begin(); newit != quad_info_store.textures.end(); ++newit)
			{
				if (newit->second.prop.texture == texture && newit->second.prop.dimensions == dimensions) // texture exists already -> increment its usage and set it
				{
					++newit->second.usage;
					quad_info.tex_slot = newit->first;
					ssbo.quad_info.lazy_send(vb_pos);
					return;
				}
			}
			// create new texture slot (cannot be 0)
			quad_info.tex_slot = 1;
			while (true)
			{
				if (quad_info_store.textures.count(quad_info.tex_slot))
					++quad_info.tex_slot;
				else
				{
					quad_info_store.textures[quad_info.tex_slot] = { { texture, dimensions }, 1 };
					OLY_ASSERT(quad_info.tex_slot * sizeof(SSBO::TexData) <= (GLuint)ssbo.tex_data.get_size());
					if (quad_info.tex_slot * sizeof(SSBO::TexData) == ssbo.tex_data.get_size())
					{
						// TODO batch this resize when calling flush() ? probably not very frequent though
						ssbo.tex_data.resize(ssbo.tex_data.get_size() + sizeof(SSBO::TexData));
					}
					ssbo.tex_data.send<SSBO::TexData>(quad_info.tex_slot, { texture->get_handle(), dimensions });
					ssbo.quad_info.lazy_send(vb_pos);
					return;
				}
			}
		}

		void SpriteBatch::set_tex_coords(GLuint vb_pos, const TexUVRect& uvs)
		{
			SSBO::QuadInfo& quad_info = ssbo.quad_info.vector()[vb_pos];
			if (uvs == TexUVRect{}) // remove uvs from sprite
			{
				if (quad_info.tex_coord_slot != 0) // sprite has existing uvs -> decrement its usage and remove it
				{
					auto it = quad_info_store.tex_coords.find(quad_info.tex_coord_slot);
					--it->second.usage;
					if (it->second.usage == 0)
						quad_info_store.tex_coords.erase(it);
					quad_info.tex_coord_slot = 0;
					ssbo.quad_info.lazy_send(vb_pos);
				}
				return;
			}
			if (quad_info.tex_coord_slot != 0) // sprite has existing uvs -> decrement its usage
			{
				auto it = quad_info_store.tex_coords.find(quad_info.tex_coord_slot);
				if (uvs == it->second.prop) // same uvs that exist -> do nothing
					return;
				--it->second.usage;
				if (it->second.usage == 0)
					quad_info_store.tex_coords.erase(it);
			}
			for (auto newit = quad_info_store.tex_coords.begin(); newit != quad_info_store.tex_coords.end(); ++newit)
			{
				if (newit->second.prop == uvs) // uvs exist already -> increment its usage and set it
				{
					++newit->second.usage;
					quad_info.tex_coord_slot = newit->first;
					ssbo.quad_info.lazy_send(vb_pos);
					return;
				}
			}
			// create new tex coords slot (cannot be 0)
			quad_info.tex_coord_slot = 1;
			while (true)
			{
				if (quad_info_store.tex_coords.count(quad_info.tex_coord_slot))
					++quad_info.tex_coord_slot;
				else
				{
					quad_info_store.tex_coords[quad_info.tex_coord_slot] = { uvs, 1 };
					OLY_ASSERT(quad_info.tex_coord_slot * sizeof(TexUVRect) <= (GLuint)ubo.tex_coords.get_size());
					if (quad_info.tex_coord_slot * sizeof(TexUVRect) == ubo.tex_coords.get_size())
					{
						// TODO batch this resize when calling flush() ? probably not very frequent though
						ubo.tex_coords.resize(ubo.tex_coords.get_size() + sizeof(TexUVRect));
					}
					ubo.tex_coords.send<TexUVRect>(quad_info.tex_coord_slot, uvs);
					ssbo.quad_info.lazy_send(vb_pos);
					return;
				}
			}
		}

		void SpriteBatch::set_modulation(GLuint vb_pos, const Modulation& modulation)
		{
			SSBO::QuadInfo& quad_info = ssbo.quad_info.vector()[vb_pos];
			if (modulation == Modulation{}) // remove modulation from sprite
			{
				if (quad_info.color_slot != 0) // sprite has existing modulation -> decrement its usage and remove it
				{
					auto it = quad_info_store.modulations.find(quad_info.color_slot);
					--it->second.usage;
					if (it->second.usage == 0)
						quad_info_store.modulations.erase(it);
					quad_info.color_slot = 0;
					ssbo.quad_info.lazy_send(vb_pos);
				}
				return;
			}
			if (quad_info.color_slot != 0) // sprite has existing modulation -> decrement its usage
			{
				auto it = quad_info_store.modulations.find(quad_info.color_slot);
				if (modulation == it->second.prop) // same modulation that exists -> do nothing
					return;
				--it->second.usage;
				if (it->second.usage == 0)
					quad_info_store.modulations.erase(it);
			}
			for (auto newit = quad_info_store.modulations.begin(); newit != quad_info_store.modulations.end(); ++newit)
			{
				if (newit->second.prop == modulation) // modulation exists already -> increment its usage and set it
				{
					++newit->second.usage;
					quad_info.color_slot = newit->first;
					ssbo.quad_info.lazy_send(vb_pos);
					return;
				}
			}
			// create new modulation slot (cannot be 0)
			quad_info.color_slot = 1;
			while (true)
			{
				if (quad_info_store.modulations.count(quad_info.color_slot))
					++quad_info.color_slot;
				else
				{
					quad_info_store.modulations[quad_info.color_slot] = { modulation, 1 };
					OLY_ASSERT(quad_info.color_slot * sizeof(Modulation) <= (GLuint)ubo.modulation.get_size());
					if (quad_info.color_slot * sizeof(Modulation) == ubo.modulation.get_size())
					{
						// TODO batch this resize when calling flush() ? probably not very frequent though
						ubo.modulation.resize(ubo.modulation.get_size() + sizeof(Modulation));
					}
					ubo.modulation.send<Modulation>(quad_info.color_slot, modulation);
					ssbo.quad_info.lazy_send(vb_pos);
					return;
				}
			}
		}

		void SpriteBatch::draw_sprite(GLuint vb_pos)
		{
			OLY_ASSERT(sprites_to_draw <= ebo.vector().size());
			if (sprites_to_draw == ebo.vector().size())
			{
				// resize
				// TODO batch this resize in render()
				ebo.vector().push_back({});
				glNamedBufferData(ebo.buffer(), ebo.vector().size() * sizeof(decltype(ebo)::StructAlias), ebo.vector().data(), GL_DYNAMIC_DRAW);
			}
			rendering::quad_indices(ebo.vector()[sprites_to_draw].data, vb_pos);
			ebo.lazy_send(sprites_to_draw);
			++sprites_to_draw;
		}
	}

	namespace renderable
	{
		Sprite::Sprite(batch::SpriteBatch* sprite_batch)
			: batch(sprite_batch)
		{
			vb_pos = batch->gen_sprite_pos();
		}

		Sprite::Sprite(Sprite&& other) noexcept
			: batch(other.batch), vb_pos(other.vb_pos), transformer(std::move(other.transformer))
		{
			other.batch = nullptr;
		}

		Sprite& Sprite::operator=(Sprite&& other) noexcept
		{
			if (this != &other)
			{
				if (batch)
					batch->erase_sprite_pos(vb_pos);
				batch = other.batch;
				vb_pos = other.vb_pos;
				transformer = std::move(other.transformer);
				other.batch = nullptr;
			}
			return *this;
		}

		Sprite::~Sprite()
		{
			if (batch)
				batch->erase_sprite_pos(vb_pos);
		}

		void Sprite::draw() const
		{
			flush();
			batch->draw_sprite(vb_pos);
		}
		
		void Sprite::set_texture(const rendering::BindlessTextureRes& texture, glm::vec2 dimensions) const
		{
			batch->set_texture(vb_pos, texture, dimensions);
		}
		
		void Sprite::set_tex_coords(const batch::SpriteBatch::TexUVRect& tex_coords) const
		{
			batch->set_tex_coords(vb_pos, tex_coords);
		}

		void Sprite::set_modulation(const batch::SpriteBatch::Modulation& modulation) const
		{
			batch->set_modulation(vb_pos, modulation);
		}

		void Sprite::post_set() const
		{
			transformer.post_set();
		}
		
		void Sprite::pre_get() const
		{
			transformer.pre_get();
		}
		
		void Sprite::flush() const
		{
			if (transformer.flush())
			{
				transformer.pre_get();
				auto& transform_buffer = batch->ssbo.quad_transform;
				transform_buffer.vector()[vb_pos] = transformer.global();
				transform_buffer.lazy_send(vb_pos);
			}
		}
	}
}
