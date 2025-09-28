#include "Spritebatch.h"

#include "graphics/resources/Shaders.h"
#include "core/context/rendering/Sprites.h"
#include "core/context/rendering/Textures.h"
#include "core/util/Time.h"

namespace oly::rendering
{
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

	SpriteBatch::SpriteBatch(UBOCapacity capacity)
		: ebo(vao), shader(graphics::internal_shaders::sprite_batch(capacity.modulations(), capacity.anims())), tex_data_ssbo(graphics::SHADER_STORAGE_MAX_BUFFER_SIZE, sizeof(TexData)),
		tex_coords_ssbo(graphics::SHADER_STORAGE_MAX_BUFFER_SIZE, sizeof(math::UVRect)), ubo(capacity)
	{
		internal::SpriteBatchRegistry::instance().batches.insert(this);

		shader_locations.projection = glGetUniformLocation(shader, "uProjection");
		shader_locations.modulation = glGetUniformLocation(shader, "uGlobalModulation");
		shader_locations.time = glGetUniformLocation(shader, "uTime");

		tex_coords_ssbo.send<math::UVRect>(0, {});
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
		glUseProgram(shader);
		glUniformMatrix3fv(shader_locations.projection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniform4f(shader_locations.modulation, global_modulation[0], global_modulation[1], global_modulation[2], global_modulation[3]);
		glUniform1f(shader_locations.time, TIME.now<>());

		tex_data_ssbo.bind_base(0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, quad_ssbo_block.buf.get_buffer<INFO>());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, quad_ssbo_block.buf.get_buffer<TRANSFORM>());
		tex_coords_ssbo.bind_base(3);
		ubo.modulation.bind_base(1);
		ubo.anim.bind_base(2);
		ebo.render_elements(GL_TRIANGLES);

		quad_ssbo_block.post_draw_all();
	}

	GLuint SpriteBatch::gen_sprite_id()
	{
		GLuint id = id_generator.gen();
		if (id == NULL_ID)
			throw Error(ErrorCode::STORAGE_OVERFLOW);

		quad_ssbo_block.set<INFO>(id) = {};
		quad_ssbo_block.set<TRANSFORM>(id) = 1.0f;
		return id;
	}

	void SpriteBatch::erase_sprite_id(GLuint id)
	{
		const auto decrement_texture = [this](GLushort slot) {
			if (auto texture = quad_info_store.textures.decrement_usage(slot))
			{
				auto& map = quad_info_store.dimensionless_texture_slot_map;
				auto it = map.find(texture.value().texture);
				if (it != map.end())
				{
					auto& slots = it->second;
					slots.erase(slot);
					if (slots.empty())
						map.erase(it);
				}
			}
			};

		if (id != NULL_ID) [[likely]]
		{
			const QuadInfo& quad_info = quad_ssbo_block.buf.at<INFO>(id);
			decrement_texture(quad_info.tex_slot);
			decrement_texture(quad_info.mod_tex_slot);
			quad_info_store.tex_coords.decrement_usage(quad_info.tex_coord_slot);
			quad_info_store.modulations.decrement_usage(quad_info.color_slot);
			id_generator.yield(id);
		}
	}

	void SpriteBatch::set_texture(GLuint vb_pos, const graphics::BindlessTextureRef& texture, glm::vec2 dimensions)
	{
		if (vb_pos == NULL_ID) [[unlikely]]
			throw Error(ErrorCode::INVALID_ID);

		auto& tex_slot = quad_ssbo_block.buf.at<INFO>(vb_pos).tex_slot;
		if (quad_info_store.textures.set_object<TexData>(tex_data_ssbo, tex_slot,
			QuadInfoStore::SizedTexture{ texture, dimensions }, TexData{ texture ? texture->get_handle() : 0, dimensions }))
		{
			quad_ssbo_block.flag<INFO>(vb_pos);
			quad_info_store.dimensionless_texture_slot_map[texture].insert(tex_slot);
		}
	}

	void SpriteBatch::set_tex_coords(GLuint vb_pos, math::UVRect uvs)
	{
		if (vb_pos == NULL_ID) [[unlikely]]
			throw Error(ErrorCode::INVALID_ID);

		if (quad_info_store.tex_coords.set_object(tex_coords_ssbo, quad_ssbo_block.buf.at<INFO>(vb_pos).tex_coord_slot, uvs))
			quad_ssbo_block.flag<INFO>(vb_pos);
	}

	void SpriteBatch::set_modulation(GLuint vb_pos, glm::vec4 modulation)
	{
		if (vb_pos == NULL_ID) [[unlikely]]
			throw Error(ErrorCode::INVALID_ID);

		if (quad_info_store.modulations.set_object(ubo.modulation, quad_ssbo_block.buf.at<INFO>(vb_pos).color_slot, modulation))
			quad_ssbo_block.flag<INFO>(vb_pos);
	}

	void SpriteBatch::set_frame_format(GLuint vb_pos, const graphics::AnimFrameFormat& anim)
	{
		if (vb_pos == NULL_ID) [[unlikely]]
			throw Error(ErrorCode::INVALID_ID);

		if (quad_info_store.anims.set_object(ubo.anim, quad_ssbo_block.buf.at<INFO>(vb_pos).frame_slot, anim))
			quad_ssbo_block.flag<INFO>(vb_pos);
	}

	void SpriteBatch::set_text_glyph(GLuint vb_pos, bool is_text_glyph)
	{
		if (vb_pos == NULL_ID) [[unlikely]]
			throw Error(ErrorCode::INVALID_ID);

		set_quad_info(vb_pos).is_text_glyph = is_text_glyph;
	}

	void SpriteBatch::set_mod_texture(GLuint vb_pos, const graphics::BindlessTextureRef& texture, glm::vec2 dimensions)
	{
		if (vb_pos == NULL_ID) [[unlikely]]
			throw Error(ErrorCode::INVALID_ID);

		auto& mod_tex_slot = quad_ssbo_block.buf.at<INFO>(vb_pos).mod_tex_slot;
		if (quad_info_store.textures.set_object<TexData>(tex_data_ssbo, mod_tex_slot, QuadInfoStore::SizedTexture{ texture, texture ? dimensions : glm::vec2(0.0f) },
			TexData{texture ? texture->get_handle() : 0, texture ? dimensions : glm::vec2(0.0f) }))
		{
			quad_ssbo_block.flag<INFO>(vb_pos);
			quad_info_store.dimensionless_texture_slot_map[texture].insert(mod_tex_slot);
		}
	}

	void SpriteBatch::set_mod_tex_coords(GLuint vb_pos, math::UVRect uvs)
	{
		if (vb_pos == NULL_ID) [[unlikely]]
			throw Error(ErrorCode::INVALID_ID);

		if (quad_info_store.tex_coords.set_object(tex_coords_ssbo, quad_ssbo_block.buf.at<INFO>(vb_pos).mod_tex_coord_slot, uvs))
			quad_ssbo_block.flag<INFO>(vb_pos);
	}

	graphics::BindlessTextureRef SpriteBatch::get_texture(GLuint vb_pos, glm::vec2& dimensions) const
	{
		if (vb_pos == NULL_ID) [[unlikely]]
			throw Error(ErrorCode::INVALID_ID);

		GLuint slot = get_quad_info(vb_pos).tex_slot;
		if (slot == 0)
			return nullptr;
		QuadInfoStore::SizedTexture tex = quad_info_store.textures.get_object(slot);
		dimensions = tex.dimensions;
		return tex.texture;
	}

	math::UVRect SpriteBatch::get_tex_coords(GLuint vb_pos) const
	{
		if (vb_pos == NULL_ID) [[unlikely]]
			throw Error(ErrorCode::INVALID_ID);

		GLuint slot = get_quad_info(vb_pos).tex_coord_slot;
		return slot != 0 ? quad_info_store.tex_coords.get_object(slot) : math::UVRect{};
	}

	glm::vec4 SpriteBatch::get_modulation(GLuint vb_pos) const
	{
		if (vb_pos == NULL_ID) [[unlikely]]
			throw Error(ErrorCode::INVALID_ID);

		GLuint slot = get_quad_info(vb_pos).color_slot;
		return slot != 0 ? quad_info_store.modulations.get_object(slot) : glm::vec4(1.0f);
	}

	graphics::AnimFrameFormat SpriteBatch::get_frame_format(GLuint vb_pos) const
	{
		if (vb_pos == NULL_ID) [[unlikely]]
			throw Error(ErrorCode::INVALID_ID);

		GLuint slot = get_quad_info(vb_pos).frame_slot;
		return slot != 0 ? quad_info_store.anims.get_object(slot) : graphics::AnimFrameFormat{};
	}

	bool SpriteBatch::is_text_glyph(GLuint vb_pos) const
	{
		if (vb_pos == NULL_ID) [[unlikely]]
			throw Error(ErrorCode::INVALID_ID);

		return get_quad_info(vb_pos).is_text_glyph;
	}

	graphics::BindlessTextureRef SpriteBatch::get_mod_texture(GLuint vb_pos, glm::vec2& dimensions) const
	{
		if (vb_pos == NULL_ID) [[unlikely]]
			throw Error(ErrorCode::INVALID_ID);

		GLuint slot = get_quad_info(vb_pos).mod_tex_slot;
		if (slot == 0)
			return nullptr;
		QuadInfoStore::SizedTexture tex = quad_info_store.textures.get_object(slot);
		dimensions = tex.dimensions;
		return tex.texture;
	}

	math::UVRect SpriteBatch::get_mod_tex_coords(GLuint vb_pos) const
	{
		if (vb_pos == NULL_ID) [[unlikely]]
			throw Error(ErrorCode::INVALID_ID);

		GLuint slot = get_quad_info(vb_pos).mod_tex_coord_slot;
		return slot != 0 ? quad_info_store.tex_coords.get_object(slot) : math::UVRect{};
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

	struct Attributes
	{
		graphics::BindlessTextureRef texture = nullptr;
		glm::vec2 texture_dimensions = {};
		math::UVRect tex_coords = {};
		glm::vec4 modulation = glm::vec4(1.0f);
		graphics::AnimFrameFormat frame_format = {};
		bool is_text_glyph = false;
		graphics::BindlessTextureRef mod_texture = nullptr;
		glm::vec2 mod_texture_dimensions = {};
		math::UVRect mod_tex_coords = {};
		glm::mat3 transform = 1.0f;
	};

	struct AttributesRef
	{
		graphics::BindlessTextureRef texture;
		glm::vec2 texture_dimensions;
		math::UVRect tex_coords;
		glm::vec4 modulation;
		graphics::AnimFrameFormat frame_format;
		bool is_text_glyph;
		graphics::BindlessTextureRef mod_texture;
		glm::vec2 mod_texture_dimensions;
		math::UVRect mod_tex_coords;
		const glm::mat3& transform;
	};

	static Attributes get_attributes(const internal::SpriteReference& ref)
	{
		glm::vec2 texture_dimensions{}, mod_texture_dimensions{};
		Attributes attr{
			.texture = ref.get_texture(texture_dimensions),
			.tex_coords = ref.get_tex_coords(),
			.modulation = ref.get_modulation(),
			.frame_format = ref.get_frame_format(),
			.is_text_glyph = ref.is_text_glyph(),
			.mod_texture = ref.get_mod_texture(mod_texture_dimensions),
			.mod_tex_coords = ref.get_mod_tex_coords(),
			.transform = ref.get_transform()
		};
		attr.texture_dimensions = texture_dimensions;
		attr.mod_texture_dimensions = mod_texture_dimensions;
		return attr;
	}

	static AttributesRef get_attributes_ref(const internal::SpriteReference& ref)
	{
		glm::vec2 texture_dimensions{}, mod_texture_dimensions{};
		AttributesRef attr{
			.texture = ref.get_texture(texture_dimensions),
			.tex_coords = ref.get_tex_coords(),
			.modulation = ref.get_modulation(),
			.frame_format = ref.get_frame_format(),
			.is_text_glyph = ref.is_text_glyph(),
			.mod_texture = ref.get_mod_texture(mod_texture_dimensions),
			.mod_tex_coords = ref.get_mod_tex_coords(),
			.transform = ref.get_transform()
		};
		attr.texture_dimensions = texture_dimensions;
		attr.mod_texture_dimensions = mod_texture_dimensions;
		return attr;
	}

	static void set_attributes(internal::SpriteReference& ref, const Attributes& attr)
	{
		ref.set_texture(attr.texture, attr.texture_dimensions);
		ref.set_tex_coords(attr.tex_coords);
		ref.set_modulation(attr.modulation);
		ref.set_frame_format(attr.frame_format);
		ref.set_text_glyph(attr.is_text_glyph);
		ref.set_mod_texture(attr.mod_texture, attr.mod_texture_dimensions);
		ref.set_mod_tex_coords(attr.mod_tex_coords);
		ref.set_transform(attr.transform);
	}

	static void set_attributes(internal::SpriteReference& ref, const AttributesRef& attr)
	{
		ref.set_texture(attr.texture, attr.texture_dimensions);
		ref.set_tex_coords(attr.tex_coords);
		ref.set_modulation(attr.modulation);
		ref.set_frame_format(attr.frame_format);
		ref.set_text_glyph(attr.is_text_glyph);
		ref.set_mod_texture(attr.mod_texture, attr.mod_texture_dimensions);
		ref.set_mod_tex_coords(attr.mod_tex_coords);
		ref.set_transform(attr.transform);
	}

	internal::SpriteReference::SpriteReference()
		: batch(&context::sprite_batch())
	{
		id = this->batch->gen_sprite_id();
		set_attributes(*this, Attributes{});
	}

	internal::SpriteReference::SpriteReference(SpriteBatch* batch)
		: batch(batch)
	{
		if (this->batch)
		{
			id = this->batch->gen_sprite_id();
			set_attributes(*this, Attributes{});
		}
	}

	internal::SpriteReference::SpriteReference(const SpriteReference& other)
		: batch(other.batch)
	{
		if (batch && other.id != SpriteBatch::NULL_ID)
		{
			id = batch->gen_sprite_id();
			set_attributes(*this, get_attributes_ref(other));
		}
	}

	internal::SpriteReference::SpriteReference(SpriteReference&& other) noexcept
		: batch(other.batch), id(other.id)
	{
		other.id = SpriteBatch::NULL_ID;
	}

	internal::SpriteReference::~SpriteReference()
	{
		if (batch)
			batch->erase_sprite_id(id);
	}

	internal::SpriteReference& internal::SpriteReference::operator=(const SpriteReference& other)
	{
		if (this != &other)
			*this = dupl(other);
		return *this;
	}

	internal::SpriteReference& internal::SpriteReference::operator=(SpriteReference&& other) noexcept
	{
		if (this != &other)
		{
			if (batch != other.batch)
			{
				if (batch)
					batch->erase_sprite_id(id);
				batch = other.batch;
				id = other.id;
				other.id = SpriteBatch::NULL_ID;
			}
			else if (batch)
			{
				if (other.id != SpriteBatch::NULL_ID)
					set_attributes(*this, get_attributes_ref(other));
				else
				{
					batch->erase_sprite_id(id);
					id = SpriteBatch::NULL_ID;
				}
			}
		}
		return *this;
	}

	void internal::SpriteReference::set_batch(SpriteBatch* batch)
	{
		if (this->batch == batch)
			return;

		if (this->batch)
		{
			if (batch)
			{
				Attributes attr = get_attributes(*this);
				this->batch->erase_sprite_id(id);
				this->batch = batch;
				id = this->batch->gen_sprite_id();
				set_attributes(*this, attr);
			}
			else
			{
				this->batch->erase_sprite_id(id);
				this->batch = batch;
			}
		}
		else
		{
			this->batch = batch;
			id = this->batch->gen_sprite_id();
			set_attributes(*this, Attributes{});
		}
	}

	void internal::SpriteReference::set_texture(const std::string& texture_file, unsigned int texture_index) const
	{
		if (batch) [[likely]]
		{
			if (id != SpriteBatch::NULL_ID) [[likely]]
			{
				graphics::BindlessTextureRef texture = context::load_texture(texture_file, texture_index);
				glm::vec2 dimensions = context::get_texture_dimensions(texture_file, texture_index);
				batch->set_texture(id, texture, dimensions);
			}
			else
				throw Error(ErrorCode::INVALID_ID);
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::SpriteReference::set_texture(const graphics::BindlessTextureRef& texture) const
	{
		if (batch) [[likely]]
			batch->set_texture(id, texture, context::get_texture_dimensions(texture));
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::SpriteReference::set_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions) const
	{
		if (batch) [[likely]]
			batch->set_texture(id, texture, dimensions);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::SpriteReference::set_tex_coords(math::UVRect rect) const
	{
		if (batch) [[likely]]
			batch->set_tex_coords(id, rect);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::SpriteReference::set_modulation(glm::vec4 modulation) const
	{
		if (batch) [[likely]]
			batch->set_modulation(id, modulation);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::SpriteReference::set_frame_format(const graphics::AnimFrameFormat& anim) const
	{
		if (batch) [[likely]]
			batch->set_frame_format(id, anim);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::SpriteReference::set_text_glyph(bool is_text_glyph) const
	{
		if (batch) [[likely]]
			batch->set_text_glyph(id, is_text_glyph);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::SpriteReference::set_mod_texture(const std::string& texture_file, unsigned int texture_index) const
	{
		if (batch) [[likely]]
		{
			auto texture = context::load_texture(texture_file, texture_index);
			batch->set_mod_texture(id, texture, context::get_texture_dimensions(texture_file, texture_index));
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::SpriteReference::set_mod_texture(const graphics::BindlessTextureRef& texture) const
	{
		if (batch) [[likely]]
			batch->set_mod_texture(id, texture, context::get_texture_dimensions(texture));
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::SpriteReference::set_mod_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions) const
	{
		if (batch) [[likely]]
			batch->set_mod_texture(id, texture, dimensions);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::SpriteReference::set_mod_tex_coords(math::UVRect rect) const
	{
		if (batch) [[likely]]
			batch->set_mod_tex_coords(id, rect);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::SpriteReference::set_transform(const glm::mat3& transform) const
	{
		if (batch) [[likely]]
			batch->quad_ssbo_block.set<SpriteBatch::TRANSFORM>(id) = transform;
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	graphics::BindlessTextureRef internal::SpriteReference::get_texture() const
	{
		if (batch) [[likely]]
		{
			glm::vec2 _;
			return batch->get_texture(id, _);
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	graphics::BindlessTextureRef internal::SpriteReference::get_texture(glm::vec2& dimensions) const
	{
		if (batch) [[likely]]
			return batch->get_texture(id, dimensions);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	math::UVRect internal::SpriteReference::get_tex_coords() const
	{
		if (batch) [[likely]]
			return batch->get_tex_coords(id);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	glm::vec4 internal::SpriteReference::get_modulation() const
	{
		if (batch) [[likely]]
			return batch->get_modulation(id);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	graphics::AnimFrameFormat internal::SpriteReference::get_frame_format() const
	{
		if (batch) [[likely]]
			return batch->get_frame_format(id);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	bool internal::SpriteReference::is_text_glyph() const
	{
		if (batch) [[likely]]
			return batch->is_text_glyph(id);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	graphics::BindlessTextureRef internal::SpriteReference::get_mod_texture() const
	{
		if (batch) [[likely]]
		{
			glm::vec2 _;
			return batch->get_mod_texture(id, _);
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	graphics::BindlessTextureRef internal::SpriteReference::get_mod_texture(glm::vec2& dimensions) const
	{
		if (batch) [[likely]]
			return batch->get_mod_texture(id, dimensions);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	math::UVRect internal::SpriteReference::get_mod_tex_coords() const
	{
		if (batch) [[likely]]
			return batch->get_mod_tex_coords(id);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	glm::mat3 internal::SpriteReference::get_transform() const
	{
		if (batch) [[likely]]
		{
			if (id != SpriteBatch::NULL_ID) [[likely]]
				return batch->quad_ssbo_block.get<SpriteBatch::TRANSFORM>(id);
			else
				throw Error(ErrorCode::INVALID_ID);
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::SpriteReference::draw_quad() const
	{
		if (batch) [[likely]]
		{
			if (id != SpriteBatch::NULL_ID) [[likely]]
				graphics::quad_indices(batch->ebo.draw_primitive().data(), id);
			else
				throw Error(ErrorCode::INVALID_ID);
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}
}
