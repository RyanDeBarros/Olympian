#include "Sprites.h"

#include "core/context/rendering/Rendering.h"
#include "core/context/rendering/Sprites.h"
#include "core/context/rendering/Textures.h"
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

	void internal::SpriteBatchRegistry::update_texture_handle(const graphics::BindlessTextureRef& texture)
	{
		for (SpriteBatch* batch : batches)
			batch->update_texture_handle(texture);
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
		: ebo(vao, capacity.sprites), tex_data_ssbo(capacity.textures * sizeof(TexData), graphics::SHADER_STORAGE_MAX_BUFFER_SIZE),
		quad_ssbo_block(capacity.sprites), ubo(capacity.uvs, capacity.modulations, capacity.anims)
	{
		internal::SpriteBatchRegistry::instance().batches.insert(this);

		shader_locations.projection = glGetUniformLocation(graphics::internal_shaders::sprite_batch, "uProjection");
		shader_locations.modulation = glGetUniformLocation(graphics::internal_shaders::sprite_batch, "uGlobalModulation");
		shader_locations.time = glGetUniformLocation(graphics::internal_shaders::sprite_batch, "uTime");

		ubo.tex_coords.send<UVRect>(0, {});
		ubo.modulation.send<glm::vec4>(0, glm::vec4(1.0f));
		ubo.anim.send<graphics::AnimFrameFormat>(0, {});
	}

	SpriteBatch::~SpriteBatch()
	{
		internal::SpriteBatchRegistry::instance().batches.erase(this);
	}

	void SpriteBatch::render() const
	{
		if (ebo.empty())
			return;

		quad_ssbo_block.pre_draw_all();

		glBindVertexArray(vao);
		glUseProgram(graphics::internal_shaders::sprite_batch);
		glUniformMatrix3fv(shader_locations.projection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniform4f(shader_locations.modulation, global_modulation[0], global_modulation[1], global_modulation[2], global_modulation[3]);
		glUniform1f(shader_locations.time, TIME.now<>());

		tex_data_ssbo.bind_base(0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, quad_ssbo_block.buf.get_buffer<INFO>());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, quad_ssbo_block.buf.get_buffer<TRANSFORM>());
		ubo.tex_coords.bind_base(0);
		ubo.modulation.bind_base(1);
		ubo.anim.bind_base(2);
		ebo.render_elements(GL_TRIANGLES);

		quad_ssbo_block.post_draw_all();
	}

	SpriteBatch::SpriteID SpriteBatch::gen_sprite_id()
	{
		SpriteID id = id_generator.generate();
		quad_ssbo_block.set<INFO>(id.get()) = {};
		quad_ssbo_block.set<TRANSFORM>(id.get()) = 1.0f;
		return id;
	}

	void SpriteBatch::erase_sprite_id(const SpriteID& id)
	{
		if (id.is_valid())
		{
			const QuadInfo& quad_info = quad_ssbo_block.buf.at<INFO>(id.get());
			if (auto texture = quad_info_store.textures.decrement_usage(quad_info.tex_slot))
			{
				auto& map = quad_info_store.dimensionless_texture_slot_map;
				auto it = map.find(texture.value().texture);
				if (it != map.end())
				{
					auto& slots = it->second;
					slots.erase(quad_info.tex_slot);
					if (slots.empty())
						map.erase(it);
				}
			}
			quad_info_store.tex_coords.decrement_usage(quad_info.tex_coord_slot);
			quad_info_store.modulations.decrement_usage(quad_info.color_slot);
		}
	}

	void SpriteBatch::set_texture(GLuint vb_pos, const graphics::BindlessTextureRef& texture, glm::vec2 dimensions)
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

	void SpriteBatch::set_modulation(GLuint vb_pos, glm::vec4 modulation)
	{
		if (quad_info_store.modulations.set_object(ubo.modulation, quad_ssbo_block.buf.at<INFO>(vb_pos).color_slot, vb_pos, modulation))
			quad_ssbo_block.flag<INFO>(vb_pos);
	}

	void SpriteBatch::set_frame_format(GLuint vb_pos, const graphics::AnimFrameFormat& anim)
	{
		if (quad_info_store.anims.set_object(ubo.anim, quad_ssbo_block.buf.at<INFO>(vb_pos).frame_slot, vb_pos, anim))
			quad_ssbo_block.flag<INFO>(vb_pos);
	}

	graphics::BindlessTextureRef SpriteBatch::get_texture(GLuint vb_pos, glm::vec2& dimensions) const
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

	glm::vec4 SpriteBatch::get_modulation(GLuint vb_pos) const
	{
		GLuint slot = get_quad_info(vb_pos).color_slot;
		return slot != 0 ? quad_info_store.modulations.get_object(slot) : glm::vec4(1.0f);
	}

	graphics::AnimFrameFormat SpriteBatch::get_frame_format(GLuint vb_pos) const
	{
		GLuint slot = get_quad_info(vb_pos).frame_slot;
		return slot != 0 ? quad_info_store.anims.get_object(slot) : graphics::AnimFrameFormat{};
	}

	void SpriteBatch::update_texture_handle(const graphics::BindlessTextureRef& texture)
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

	internal::SpriteReference::SpriteReference(SpriteBatch* batch)
		: batch(batch ? *batch : context::sprite_batch()), in_context(!batch)
	{
		id = this->batch.gen_sprite_id();
	}

	internal::SpriteReference::SpriteReference(const SpriteReference& other)
		: batch(other.batch), in_context(other.in_context)
	{
		id = batch.gen_sprite_id();
	}

	internal::SpriteReference::SpriteReference(SpriteReference&& other) noexcept
		: batch(other.batch), in_context(other.in_context), id(std::move(other.id))
	{
	}

	internal::SpriteReference::~SpriteReference()
	{
		batch.erase_sprite_id(id);
	}

	internal::SpriteReference& internal::SpriteReference::operator=(const SpriteReference& other)
	{
		return *this;
	}

	internal::SpriteReference& internal::SpriteReference::operator=(SpriteReference&& other) noexcept
	{
		if (this != &other)
		{
			if (&batch == &other.batch)
			{
				batch.erase_sprite_id(id);
				id = std::move(other.id);
			}
		}
		return *this;
	}

	void internal::SpriteReference::set_texture(const std::string& texture_file, unsigned int texture_index) const
	{
		auto texture = context::load_texture(texture_file, texture_index);
		batch.set_texture(id.get(), texture, context::get_texture_dimensions(texture_file, texture_index));
	}

	void internal::SpriteReference::set_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions) const
	{
		batch.set_texture(id.get(), texture, dimensions);
	}

	void internal::SpriteReference::set_tex_coords(const UVRect& uvs) const
	{
		batch.set_tex_coords(id.get(), uvs);
	}

	void internal::SpriteReference::set_tex_coords(const math::Rect2D& rect) const
	{
		batch.set_tex_coords(id.get(), UVRect{}.from_rect(rect));
	}
	
	void internal::SpriteReference::set_modulation(glm::vec4 modulation) const
	{
		batch.set_modulation(id.get(), modulation);
	}

	void internal::SpriteReference::set_frame_format(const graphics::AnimFrameFormat& anim) const
	{
		batch.set_frame_format(id.get(), anim);
	}

	void internal::SpriteReference::set_transform(const glm::mat3& transform) const
	{
		batch.quad_ssbo_block.set<SpriteBatch::TRANSFORM>(id.get()) = transform;
	}

	graphics::BindlessTextureRef internal::SpriteReference::get_texture() const
	{
		glm::vec2 _;
		return batch.get_texture(id.get(), _);
	}

	graphics::BindlessTextureRef internal::SpriteReference::get_texture(glm::vec2& dimensions) const
	{
		return batch.get_texture(id.get(), dimensions);
	}

	UVRect internal::SpriteReference::get_tex_coords() const
	{
		return batch.get_tex_coords(id.get());
	}

	glm::vec4 internal::SpriteReference::get_modulation() const
	{
		return batch.get_modulation(id.get());
	}

	graphics::AnimFrameFormat internal::SpriteReference::get_frame_format() const
	{
		return batch.get_frame_format(id.get());
	}

	glm::mat3 internal::SpriteReference::get_transform() const
	{
		return batch.quad_ssbo_block.get<SpriteBatch::TRANSFORM>(id.get());
	}

	std::invoke_result_t<decltype(&decltype(SpriteBatch::ebo)::draw_primitive), decltype(SpriteBatch::ebo)> internal::SpriteReference::draw_primitive() const
	{
		return batch.ebo.draw_primitive();
	}

	void internal::SpriteReference::draw_quad() const
	{
		graphics::quad_indices(draw_primitive().data(), id.get());
	}

	StaticSprite::StaticSprite(SpriteBatch* batch)
		: ref(batch)
	{
	}

	StaticSprite::StaticSprite(const StaticSprite& other)
		: ref(other.ref)
	{
		glm::vec2 dim;
		auto tex = other.get_texture(dim);
		set_texture(tex, dim);
		set_tex_coords(other.get_tex_coords());
		set_modulation(other.get_modulation());
		set_frame_format(other.get_frame_format());
	}

	StaticSprite::StaticSprite(StaticSprite&& other) noexcept
		: ref(other.ref)
	{
	}

	StaticSprite& StaticSprite::operator=(const StaticSprite& other)
	{
		if (this != &other)
		{
			ref = other.ref;
			glm::vec2 dim;
			auto tex = other.get_texture(dim);
			set_texture(tex, dim);
			set_tex_coords(other.get_tex_coords());
			set_modulation(other.get_modulation());
			set_frame_format(other.get_frame_format());
		}
		return *this;
	}

	StaticSprite& StaticSprite::operator=(StaticSprite&& other) noexcept
	{
		if (this != &other)
		{
			ref = std::move(other.ref);
			if (&ref.batch != &other.ref.batch)
			{
				glm::vec2 dim;
				auto tex = other.get_texture(dim);
				set_texture(tex, dim);
				set_tex_coords(other.get_tex_coords());
				set_modulation(other.get_modulation());
				set_frame_format(other.get_frame_format());
			}
		}
		return *this;
	}

	StaticSprite::~StaticSprite()
	{
	}

	void StaticSprite::draw(BatchBarrier barrier) const
	{
		if (ref.in_context) [[likely]]
			if (barrier) [[likely]]
				context::internal::flush_batches_except(context::InternalBatch::SPRITE);
		ref.draw_quad();
		if (ref.in_context) [[likely]]
			context::internal::set_batch_rendering_tracker(context::InternalBatch::SPRITE, true);
	}

	Sprite::Sprite(SpriteBatch* batch)
		: ref(batch)
	{
	}

	Sprite::Sprite(const Sprite& other)
		: ref(other.ref), transformer(other.transformer)
	{
		glm::vec2 dim;
		auto tex = other.get_texture(dim);
		set_texture(tex, dim);
		set_tex_coords(other.get_tex_coords());
		set_modulation(other.get_modulation());
		set_frame_format(other.get_frame_format());
	}

	Sprite::Sprite(Sprite&& other) noexcept
		: ref(std::move(other.ref)), transformer(std::move(other.transformer))
	{
	}

	Sprite& Sprite::operator=(const Sprite& other)
	{
		if (this != &other)
		{
			ref = other.ref;
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
			ref = std::move(other.ref);
			transformer = std::move(other.transformer);
			if (&ref.batch != &other.ref.batch)
			{
				glm::vec2 dim;
				auto tex = other.get_texture(dim);
				set_texture(tex, dim);
				set_tex_coords(other.get_tex_coords());
				set_modulation(other.get_modulation());
				set_frame_format(other.get_frame_format());
			}
		}
		return *this;
	}

	Sprite::~Sprite()
	{
	}

	void Sprite::draw(BatchBarrier barrier) const
	{
		if (ref.in_context) [[likely]]
			if (barrier) [[likely]]
				context::internal::flush_batches_except(context::InternalBatch::SPRITE);
		if (transformer.flush())
			ref.set_transform(transformer.global());
		ref.draw_quad();
		if (ref.in_context) [[likely]]
			context::internal::set_batch_rendering_tracker(context::InternalBatch::SPRITE, true);
	}
}
