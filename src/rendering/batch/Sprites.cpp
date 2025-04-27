#include "Sprites.h"

#include <glm/gtc/type_ptr.hpp>

#include "../Resources.h"
#include "../../Olympian.h"

namespace oly
{
	namespace rendering
	{
		const SpriteBatch::QuadInfo& SpriteBatch::get_quad_info(GLuint vb_pos) const
		{
			return quad_ssbo_block.get<INFO>(vb_pos);
		}

		SpriteBatch::QuadInfo& SpriteBatch::set_quad_info(GLuint vb_pos)
		{
			return quad_ssbo_block.set<INFO>(vb_pos);
		}

		SpriteBatch::SpriteBatch(Capacity capacity, const glm::vec4& projection_bounds)
			: ebo(vao, capacity.sprites), tex_data_ssbo(capacity.textures * sizeof(TexData)), quad_ssbo_block(capacity.sprites), ubo(capacity.uvs, capacity.modulations, capacity.anims), projection_bounds(projection_bounds)
		{
			shader_locations.projection = shaders::location(shaders::sprite_batch, "uProjection");
			shader_locations.modulation = shaders::location(shaders::sprite_batch, "uGlobalModulation");
			shader_locations.time = shaders::location(shaders::sprite_batch, "uTime");

			ubo.tex_coords.send<TexUVRect>(0, {});
			ubo.modulation.send<Modulation>(0, {});
			ubo.anim.send<AnimFrameFormat>(0, {});
		}

		void SpriteBatch::render() const
		{
			quad_ssbo_block.pre_draw_all();

			glBindVertexArray(vao);
			glUseProgram(shaders::sprite_batch);
			glUniformMatrix3fv(shader_locations.projection, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]))));
			glUniform4f(shader_locations.modulation, global_modulation[0], global_modulation[1], global_modulation[2], global_modulation[3]);
			glUniform1f(shader_locations.time, TIME.now<float>());

			tex_data_ssbo.bind_base(0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, quad_ssbo_block.buf.get_buffer<INFO>());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, quad_ssbo_block.buf.get_buffer<TRANSFORM>());
			ubo.tex_coords.bind_base(0);
			ubo.modulation.bind_base(1);
			ubo.anim.bind_base(2);
			ebo.render_elements(GL_TRIANGLES);

			quad_ssbo_block.post_draw_all();
		}

		SpriteBatch::VBID SpriteBatch::gen_sprite_id()
		{
			VBID id = vbid_generator.generate();
			quad_ssbo_block.set<INFO>(id.get()) = {};
			quad_ssbo_block.set<TRANSFORM>(id.get()) = 1.0f;
			return id;
		}

		void SpriteBatch::erase_sprite_id(GLuint id)
		{
			const QuadInfo& quad_info = quad_ssbo_block.buf.at<INFO>(id);
			quad_info_store.textures.decrement_usage(quad_info.tex_slot);
			quad_info_store.tex_coords.decrement_usage(quad_info.tex_coord_slot);
			quad_info_store.modulations.decrement_usage(quad_info.color_slot);
		}

		void SpriteBatch::set_texture(GLuint vb_pos, const BindlessTextureRes& texture, glm::vec2 dimensions)
		{
			if (quad_info_store.textures.set_object<TexData>(tex_data_ssbo, quad_ssbo_block.buf.at<INFO>(vb_pos).tex_slot, vb_pos,
				QuadInfoStore::DimensionlessTexture{ texture, dimensions }, TexData{ texture->get_handle(), dimensions }))
				quad_ssbo_block.flag<INFO>(vb_pos);
		}

		void SpriteBatch::set_tex_coords(GLuint vb_pos, const TexUVRect& uvs)
		{
			if (quad_info_store.tex_coords.set_object(ubo.tex_coords, quad_ssbo_block.buf.at<INFO>(vb_pos).tex_coord_slot, vb_pos, uvs))
				quad_ssbo_block.flag<INFO>(vb_pos);
		}

		void SpriteBatch::set_modulation(GLuint vb_pos, const Modulation& modulation)
		{
			if (quad_info_store.modulations.set_object(ubo.modulation, quad_ssbo_block.buf.at<INFO>(vb_pos).color_slot, vb_pos, modulation))
				quad_ssbo_block.flag<INFO>(vb_pos);
		}

		void SpriteBatch::set_frame_format(GLuint vb_pos, const AnimFrameFormat& anim)
		{
			if (quad_info_store.anims.set_object(ubo.anim, quad_ssbo_block.buf.at<INFO>(vb_pos).frame_slot, vb_pos, anim))
				quad_ssbo_block.flag<INFO>(vb_pos);
		}

		BindlessTextureRes SpriteBatch::get_texture(GLuint vb_pos, glm::vec2& dimensions) const
		{
			GLuint slot = get_quad_info(vb_pos).tex_slot;
			if (slot == 0)
				return nullptr;
			QuadInfoStore::DimensionlessTexture tex = quad_info_store.textures.get_object(slot);
			dimensions = tex.dimensions;
			return tex.texture;
		}

		SpriteBatch::TexUVRect SpriteBatch::get_tex_coords(GLuint vb_pos) const
		{
			GLuint slot = get_quad_info(vb_pos).tex_coord_slot;
			return slot != 0 ? quad_info_store.tex_coords.get_object(slot) : TexUVRect{};
		}

		SpriteBatch::Modulation SpriteBatch::get_modulation(GLuint vb_pos) const
		{
			GLuint slot = get_quad_info(vb_pos).color_slot;
			return slot != 0 ? quad_info_store.modulations.get_object(slot) : Modulation{};
		}

		AnimFrameFormat SpriteBatch::get_frame_format(GLuint vb_pos) const
		{
			GLuint slot = get_quad_info(vb_pos).frame_slot;
			return slot != 0 ? quad_info_store.anims.get_object(slot) : AnimFrameFormat{};
		}

		void SpriteBatch::update_texture_handle(const BindlessTextureRes& texture)
		{
			GLuint slot;
			if (quad_info_store.textures.get_slot({ texture }, slot))
				tex_data_ssbo.send<TexData>(slot, &TexData::handle, texture->get_handle());
		}

		void SpriteBatch::update_texture_handle(const BindlessTextureRes& texture, glm::vec2 dimensions)
		{
			GLuint slot;
			if (quad_info_store.textures.get_slot({ texture, dimensions }, slot))
			{
				tex_data_ssbo.send<TexData>(slot, { texture->get_handle(), dimensions });
				quad_info_store.textures.get_object(slot).dimensions = dimensions;
			}
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
			set_frame_format(other.get_frame_format());
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
				set_frame_format(other.get_frame_format());
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
			if (transformer.flush())
			{
				transformer.pre_get();
				batch->quad_ssbo_block.set<SpriteBatch::TRANSFORM>(vbid.get()) = transformer.global();
			}
			quad_indices(batch->ebo.draw_primitive().data(), vbid.get());
		}
		
		void Sprite::set_texture(const TextureRegistry* texture_registry, const std::string& texture_name) const
		{
			switch (texture_registry->get_type(texture_name))
			{
			case TextureRegistry::TextureType::IMAGE:
			case TextureRegistry::TextureType::NSVG:
				set_texture(texture_registry->get_texture(texture_name), texture_registry->get_image_dimensions(texture_name).dimensions());
				break;
			case TextureRegistry::TextureType::ANIM:
				if (auto sp = texture_registry->get_anim_dimensions(texture_name).lock())
					set_texture(texture_registry->get_texture(texture_name), sp->dimensions());
				break;
			}
		}

		void Sprite::set_texture(const Context* context, const std::string& texture_name) const
		{
			set_texture(&context->texture_registry(), texture_name);
		}

		void Sprite::set_texture(const BindlessTextureRes& texture, glm::vec2 dimensions) const
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

		void Sprite::set_frame_format(const AnimFrameFormat& anim) const
		{
			batch->set_frame_format(vbid.get(), anim);
		}

		BindlessTextureRes Sprite::get_texture() const
		{
			glm::vec2 _;
			return get_texture(_);
		}

		BindlessTextureRes Sprite::get_texture(glm::vec2& dimensions) const
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

		AnimFrameFormat Sprite::get_frame_format() const
		{
			return batch->get_frame_format(vbid.get());
		}

		void Sprite::post_set() const
		{
			transformer.post_set();
		}
		
		void Sprite::pre_get() const
		{
			transformer.pre_get();
		}
	}
}
