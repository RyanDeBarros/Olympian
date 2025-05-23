#include "Sprites.h"

#include "core/base/Context.h"
#include "core/util/Time.h"
#include "graphics/resources/Shaders.h"

namespace oly::rendering
{
	UVRect& UVRect::from_rect(const math::Rect2D& rect)
	{
		auto rect_uvs = rect.uvs();
		uvs[0] = rect_uvs[0];
		uvs[1] = rect_uvs[1];
		uvs[2] = rect_uvs[2];
		uvs[3] = rect_uvs[3];
		return *this;
	}

	glm::vec4 oly::rendering::ModulationRect::mix(glm::vec2 uv) const
	{
		return glm::mix(glm::mix(colors[0], colors[1], uv.x), glm::mix(colors[3], colors[2], uv.x), uv.y);
	}

	const SpriteBatch::QuadInfo& SpriteBatch::get_quad_info(GLuint vb_pos) const
	{
		return quad_ssbo_block.get<INFO>(vb_pos);
	}

	SpriteBatch::QuadInfo& SpriteBatch::set_quad_info(GLuint vb_pos)
	{
		return quad_ssbo_block.set<INFO>(vb_pos);
	}

	SpriteBatch::SpriteBatch(Capacity capacity)
		: ebo(vao, capacity.sprites), tex_data_ssbo(capacity.textures * sizeof(TexData)), quad_ssbo_block(capacity.sprites), ubo(capacity.uvs, capacity.modulations, capacity.anims)
	{
		glm::ivec2 size = context::get_platform().window().get_size();
		projection_bounds = 0.5f * glm::vec4{ -size.x, size.x, -size.y, size.y };

		shader_locations.projection = glGetUniformLocation(graphics::internal_shaders::sprite_batch, "uProjection");
		shader_locations.modulation = glGetUniformLocation(graphics::internal_shaders::sprite_batch, "uGlobalModulation");
		shader_locations.time = glGetUniformLocation(graphics::internal_shaders::sprite_batch, "uTime");

		ubo.tex_coords.send<UVRect>(0, {});
		ubo.modulation.send<ModulationRect>(0, {});
		ubo.anim.send<graphics::AnimFrameFormat>(0, {});
	}

	void SpriteBatch::render() const
	{
		quad_ssbo_block.pre_draw_all();

		glBindVertexArray(vao);
		glUseProgram(graphics::internal_shaders::sprite_batch);
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
		if (auto texture = quad_info_store.textures.decrement_usage(quad_info.tex_slot))
		{
			auto& slots = quad_info_store.dimensionless_texture_slot_map.find(texture.value().texture)->second;
			slots.erase(quad_info.tex_slot);
			if (slots.empty())
				quad_info_store.dimensionless_texture_slot_map.erase(texture.value().texture);
		}
		quad_info_store.tex_coords.decrement_usage(quad_info.tex_coord_slot);
		quad_info_store.modulations.decrement_usage(quad_info.color_slot);
	}

	void SpriteBatch::set_texture(GLuint vb_pos, const graphics::BindlessTextureRes& texture, glm::vec2 dimensions)
	{
		auto& tex_slot = quad_ssbo_block.buf.at<INFO>(vb_pos).tex_slot;
		if (quad_info_store.textures.set_object<TexData>(tex_data_ssbo, tex_slot, vb_pos, QuadInfoStore::SizedTexture{ texture, dimensions }, TexData{ texture ? texture->get_handle() : 0, dimensions }))
		{
			quad_ssbo_block.flag<INFO>(vb_pos);
			quad_info_store.dimensionless_texture_slot_map[texture].insert(tex_slot);
		}
	}

	void SpriteBatch::set_tex_coords(GLuint vb_pos, const UVRect& uvs)
	{
		if (quad_info_store.tex_coords.set_object(ubo.tex_coords, quad_ssbo_block.buf.at<INFO>(vb_pos).tex_coord_slot, vb_pos, uvs))
			quad_ssbo_block.flag<INFO>(vb_pos);
	}

	void SpriteBatch::set_modulation(GLuint vb_pos, const ModulationRect& modulation)
	{
		if (quad_info_store.modulations.set_object(ubo.modulation, quad_ssbo_block.buf.at<INFO>(vb_pos).color_slot, vb_pos, modulation))
			quad_ssbo_block.flag<INFO>(vb_pos);
	}

	void SpriteBatch::set_frame_format(GLuint vb_pos, const graphics::AnimFrameFormat& anim)
	{
		if (quad_info_store.anims.set_object(ubo.anim, quad_ssbo_block.buf.at<INFO>(vb_pos).frame_slot, vb_pos, anim))
			quad_ssbo_block.flag<INFO>(vb_pos);
	}

	graphics::BindlessTextureRes SpriteBatch::get_texture(GLuint vb_pos, glm::vec2& dimensions) const
	{
		GLuint slot = get_quad_info(vb_pos).tex_slot;
		if (slot == 0)
			return nullptr;
		QuadInfoStore::SizedTexture tex = quad_info_store.textures.get_object(slot);
		dimensions = tex.dimensions;
		return tex.texture;
	}

	UVRect SpriteBatch::get_tex_coords(GLuint vb_pos) const
	{
		GLuint slot = get_quad_info(vb_pos).tex_coord_slot;
		return slot != 0 ? quad_info_store.tex_coords.get_object(slot) : UVRect{};
	}

	ModulationRect SpriteBatch::get_modulation(GLuint vb_pos) const
	{
		GLuint slot = get_quad_info(vb_pos).color_slot;
		return slot != 0 ? quad_info_store.modulations.get_object(slot) : ModulationRect{};
	}

	graphics::AnimFrameFormat SpriteBatch::get_frame_format(GLuint vb_pos) const
	{
		GLuint slot = get_quad_info(vb_pos).frame_slot;
		return slot != 0 ? quad_info_store.anims.get_object(slot) : graphics::AnimFrameFormat{};
	}

	void SpriteBatch::update_texture_handle(const graphics::BindlessTextureRes& texture)
	{
		auto iter = quad_info_store.dimensionless_texture_slot_map.find(texture);
		if (iter != quad_info_store.dimensionless_texture_slot_map.end())
		{
			GLuint64 handle = texture->get_handle();
			const auto& slots = iter->second;
			for (auto it = slots.begin(); it != slots.end(); ++it)
				tex_data_ssbo.send<TexData>(*it, &TexData::handle, handle);
		}
	}

	Sprite::Sprite()
		: batch(&context::sprite_batch())
	{
		vbid = batch->gen_sprite_id();
	}

	Sprite::Sprite(SpriteBatch& sprite_batch)
		: batch(&sprite_batch)
	{
		vbid = batch->gen_sprite_id();
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
		graphics::quad_indices(batch->ebo.draw_primitive().data(), vbid.get());
	}

	void Sprite::set_texture(const std::string& texture_file, unsigned int texture_index) const
	{
		auto texture = context::load_texture(texture_file, texture_index);
		set_texture(texture, context::get_texture_dimensions(texture_file, texture_index));
	}

	void Sprite::set_texture(const std::string& texture_file, float svg_scale, unsigned int texture_index) const
	{
		auto texture = context::load_svg_texture(texture_file, svg_scale, texture_index);
		set_texture(texture, context::get_texture_dimensions(texture_file, texture_index));
	}

	void Sprite::set_texture(const graphics::BindlessTextureRes& texture, glm::vec2 dimensions) const
	{
		batch->set_texture(vbid.get(), texture, dimensions);
	}

	void Sprite::set_tex_coords(const UVRect& tex_coords) const
	{
		batch->set_tex_coords(vbid.get(), tex_coords);
	}

	void Sprite::set_tex_coords(const math::Rect2D& rect) const
	{
		batch->set_tex_coords(vbid.get(), UVRect{}.from_rect(rect));
	}

	void Sprite::set_modulation(const ModulationRect& modulation) const
	{
		batch->set_modulation(vbid.get(), modulation);
	}

	void Sprite::set_modulation(glm::vec4 modulation) const
	{
		batch->set_modulation(vbid.get(), { modulation, modulation, modulation, modulation });
	}

	void Sprite::set_frame_format(const graphics::AnimFrameFormat& anim) const
	{
		batch->set_frame_format(vbid.get(), anim);
	}

	graphics::BindlessTextureRes Sprite::get_texture() const
	{
		glm::vec2 _;
		return batch->get_texture(vbid.get(), _);
	}

	graphics::BindlessTextureRes Sprite::get_texture(glm::vec2& dimensions) const
	{
		return batch->get_texture(vbid.get(), dimensions);
	}

	UVRect Sprite::get_tex_coords() const
	{
		return batch->get_tex_coords(vbid.get());
	}

	ModulationRect Sprite::get_modulation() const
	{
		return batch->get_modulation(vbid.get());
	}

	graphics::AnimFrameFormat Sprite::get_frame_format() const
	{
		return batch->get_frame_format(vbid.get());
	}
}
