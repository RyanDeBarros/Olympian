#include "Sprites.h"

#include <glm/gtc/type_ptr.hpp>

#include "../Resources.h"

namespace oly
{
	namespace mut
	{
		SpriteBatch::SpriteBatch(Capacity capacity, const glm::vec4& projection_bounds)
			: ebo(capacity.sprites), ssbo(capacity.textures, capacity.sprites), ubo(capacity.uvs, capacity.modulations), projection_bounds(projection_bounds)
		{
			shader_locations.projection = shaders::location(shaders::sprite_batch, "uProjection");
			shader_locations.modulation = shaders::location(shaders::sprite_batch, "uGlobalModulation");

			ubo.tex_coords.send<TexUVRect>(0, {});
			ubo.modulation.send<Modulation>(0, {});

			glBindVertexArray(vao);
			ebo.bind();
			ebo.init(GL_STREAM_DRAW);
			glBindVertexArray(0);
		}

		void SpriteBatch::render()
		{
			glBindVertexArray(vao);
			glUseProgram(shaders::sprite_batch);
			glUniformMatrix3fv(shader_locations.projection, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]))));
			glUniform4f(shader_locations.modulation, global_modulation[0], global_modulation[1], global_modulation[2], global_modulation[3]);

			ssbo.tex_data.bind_base(0);
			ssbo.quad_info.bind_base(1);
			ssbo.quad_transform.bind_base(2);
			ubo.tex_coords.bind_base(0);
			ubo.modulation.bind_base(1);

			if (resize_ebo)
			{
				resize_ebo = false;
				ebo.init(GL_STREAM_DRAW);
				ebo.clear_dirty();
			}
			else
				ebo.flush();
			ebo.set_draw_spec((GLuint)0, sprites_to_draw);
			sprites_to_draw = 0;
			ebo.draw(GL_TRIANGLES, GL_UNSIGNED_INT);
		}

		void SpriteBatch::flush() const
		{
			if (resize_sprites)
			{
				resize_sprites = false;
				ssbo.quad_info.init();
				ssbo.quad_info.clear_dirty();
				ssbo.quad_transform.init();
				ssbo.quad_transform.clear_dirty();
			}
			else
			{
				ssbo.quad_info.flush();
				ssbo.quad_transform.flush();
			}
		}

		SpriteBatch::VBID SpriteBatch::gen_sprite_id()
		{
			VBID id = vbid_generator.generate();
			OLY_ASSERT(id.get() <= ssbo.quad_info.vector().size());
			if (id.get() == ssbo.quad_info.vector().size())
			{
				ssbo.quad_info.vector().push_back({});
				ssbo.quad_transform.vector().push_back({});
				resize_sprites = true;
			}
			else if (!resize_sprites)
			{
				ssbo.quad_info.lazy_send(id.get());
				ssbo.quad_transform.lazy_send(id.get());
			}
			ssbo.quad_info.vector()[id.get()] = {};
			ssbo.quad_transform.vector()[id.get()] = glm::mat3(1.0f);
			return id;
		}

		void SpriteBatch::erase_sprite_id(GLuint id)
		{
			const SSBO::QuadInfo& quad_info = ssbo.quad_info.vector()[id];
			quad_info_store.textures.decrement_usage(quad_info.tex_slot);
			quad_info_store.tex_coords.decrement_usage(quad_info.tex_coord_slot);
			quad_info_store.modulations.decrement_usage(quad_info.color_slot);
		}

		void SpriteBatch::set_texture(GLuint vb_pos, const rendering::BindlessTextureRes& texture, glm::vec2 dimensions)
		{
			quad_info_store.textures.set_object<SSBO::TexData>(ssbo.tex_data, *this, ssbo.quad_info.vector()[vb_pos].tex_slot, vb_pos, { texture, dimensions }, { texture->get_handle(), dimensions },
				[](const QuadInfoStore::Texture& tex) { return tex.texture == nullptr; });
		}

		void SpriteBatch::set_tex_coords(GLuint vb_pos, const TexUVRect& uvs)
		{
			quad_info_store.tex_coords.set_object(ubo.tex_coords, *this, ssbo.quad_info.vector()[vb_pos].tex_coord_slot, vb_pos, uvs, [](const TexUVRect& uvs) { return uvs == TexUVRect{}; });
		}

		void SpriteBatch::set_modulation(GLuint vb_pos, const Modulation& modulation)
		{
			quad_info_store.modulations.set_object(ubo.modulation, *this, ssbo.quad_info.vector()[vb_pos].color_slot, vb_pos, modulation, [](const Modulation& modulation) { return modulation == Modulation{}; });
		}

		rendering::BindlessTextureRes SpriteBatch::get_texture(GLuint vb_pos, glm::vec2& dimensions) const
		{
			GLuint slot = ssbo.quad_info.vector()[vb_pos].tex_slot;
			if (slot == 0)
				return nullptr;
			QuadInfoStore::Texture tex = quad_info_store.textures.get_object(slot);
			dimensions = tex.dimensions;
			return tex.texture;
		}

		SpriteBatch::TexUVRect SpriteBatch::get_tex_coords(GLuint vb_pos) const
		{
			GLuint slot = ssbo.quad_info.vector()[vb_pos].tex_coord_slot;
			return slot != 0 ? quad_info_store.tex_coords.get_object(slot) : TexUVRect{};
		}

		SpriteBatch::Modulation SpriteBatch::get_modulation(GLuint vb_pos) const
		{
			GLuint slot = ssbo.quad_info.vector()[vb_pos].color_slot;
			return slot != 0 ? quad_info_store.modulations.get_object(slot) : Modulation{};
		}

		void SpriteBatch::draw_sprite(GLuint vb_pos)
		{
			OLY_ASSERT(sprites_to_draw <= ebo.vector().size());
			if (sprites_to_draw == ebo.vector().size())
			{
				resize_ebo = true;
				ebo.vector().push_back({});
			}
			else if (!resize_ebo)
				ebo.lazy_send(sprites_to_draw);
			rendering::quad_indices(ebo.vector()[sprites_to_draw].data, vb_pos);
			++sprites_to_draw;
		}

		Sprite::Sprite(SpriteBatch* sprite_batch)
			: batch(sprite_batch)
		{
			if (batch)
				vbid = batch->gen_sprite_id();
			else
				throw Error(ErrorCode::NULL_POINTER);
		}

		Sprite::Sprite(const Sprite& other)
			: batch(other.batch), transformer(other.transformer)
		{
			if (batch)
				vbid = batch->gen_sprite_id();
			else
				throw Error(ErrorCode::NULL_POINTER);

			glm::vec2 dim;
			auto tex = other.get_texture(dim);
			set_texture(tex, dim);
			set_tex_coords(other.get_tex_coords());
			set_modulation(other.get_modulation());
		}

		Sprite::Sprite(Sprite&& other) noexcept
			: batch(other.batch), vbid(std::move(other.vbid)), transformer(std::move(other.transformer))
		{
			other.batch = nullptr;
		}

		Sprite& Sprite::operator=(const Sprite& other)
		{
			if (this != &other)
			{
				if (batch != other.batch)
				{
					if (batch)
						batch->erase_sprite_id(vbid.get());
					batch = other.batch;
					if (batch)
						vbid = batch->gen_sprite_id();
					else
						throw Error(ErrorCode::NULL_POINTER);
				}
				transformer = other.transformer;

				glm::vec2 dim;
				auto tex = other.get_texture(dim);
				set_texture(tex, dim);
				set_tex_coords(other.get_tex_coords());
				set_modulation(other.get_modulation());
			}
			return *this;
		}

		Sprite& Sprite::operator=(Sprite&& other) noexcept
		{
			if (this != &other)
			{
				if (batch)
					batch->erase_sprite_id(vbid.get());
				batch = other.batch;
				vbid = std::move(other.vbid);
				transformer = std::move(other.transformer);
				other.batch = nullptr;
			}
			return *this;
		}

		Sprite::~Sprite()
		{
			if (batch)
				batch->erase_sprite_id(vbid.get());
		}

		void Sprite::draw() const
		{
			flush();
			batch->draw_sprite(vbid.get());
		}
		
		void Sprite::set_texture(const rendering::BindlessTextureRes& texture, glm::vec2 dimensions) const
		{
			batch->set_texture(vbid.get(), texture, dimensions);
		}
		
		void Sprite::set_tex_coords(const SpriteBatch::TexUVRect& tex_coords) const
		{
			batch->set_tex_coords(vbid.get(), tex_coords);
		}

		void Sprite::set_modulation(const SpriteBatch::Modulation& modulation) const
		{
			batch->set_modulation(vbid.get(), modulation);
		}

		rendering::BindlessTextureRes Sprite::get_texture() const
		{
			glm::vec2 _;
			return get_texture(_);
		}

		rendering::BindlessTextureRes Sprite::get_texture(glm::vec2& dimensions) const
		{
			return batch->get_texture(vbid.get(), dimensions);
		}

		SpriteBatch::TexUVRect Sprite::get_tex_coords() const
		{
			return batch->get_tex_coords(vbid.get());
		}

		SpriteBatch::Modulation Sprite::get_modulation() const
		{
			return batch->get_modulation(vbid.get());
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
				transform_buffer.vector()[vbid.get()] = transformer.global();
				transform_buffer.lazy_send(vbid.get());
			}
		}
	}
}
