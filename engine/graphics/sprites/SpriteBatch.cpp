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

	const internal::SpriteBatch::QuadInfo& internal::SpriteBatch::get_quad_info(GLuint vb_pos) const
	{
		return quad_ssbo_block.get<INFO>(vb_pos);
	}

	internal::SpriteBatch::QuadInfo& internal::SpriteBatch::set_quad_info(GLuint vb_pos)
	{
		return quad_ssbo_block.set<INFO>(vb_pos);
	}

	internal::SpriteBatch::SpriteBatch(UBOCapacity capacity)
		: ebo(vao), shader(graphics::internal_shaders::sprite_batch(capacity.modulations(), capacity.anims())), tex_data_ssbo(graphics::SHADER_STORAGE_MAX_BUFFER_SIZE, sizeof(TexData)),
		tex_coords_ssbo(graphics::SHADER_STORAGE_MAX_BUFFER_SIZE, sizeof(math::UVRect)), ubo(capacity)
	{
		SpriteBatchRegistry::instance().batches.insert(this);

		shader_locations.projection = glGetUniformLocation(shader, "uProjection");
		shader_locations.modulation = glGetUniformLocation(shader, "uGlobalModulation");
		shader_locations.time = glGetUniformLocation(shader, "uTime");

		tex_coords_ssbo.send<math::UVRect>(0, {});
		ubo.modulation.send<glm::vec4>(0, glm::vec4(1.0f));
		ubo.anim.send<graphics::AnimFrameFormat>(0, {});
	}

	internal::SpriteBatch::~SpriteBatch()
	{
		SpriteBatchRegistry::instance().batches.erase(this);
	}

	void internal::SpriteBatch::render() const
	{
		if (ebo.empty())
			return;

		quad_ssbo_block.pre_draw_all();

		glBindVertexArray(vao);
		glUseProgram(shader);
		glUniformMatrix3fv(shader_locations.projection, 1, GL_FALSE, glm::value_ptr(camera->projection_matrix()));
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

	void internal::SpriteBatch::assert_valid_id(GLuint id)
	{
		if (id == NULL_ID) [[unlikely]]
			throw Error(ErrorCode::INVALID_ID);
	}

	GLuint internal::SpriteBatch::gen_sprite_id()
	{
		GLuint id = id_generator.gen();
		if (id == NULL_ID)
			throw Error(ErrorCode::STORAGE_OVERFLOW);

		quad_ssbo_block.set<INFO>(id) = {};
		quad_ssbo_block.set<TRANSFORM>(id) = 1.0f;
		return id;
	}

	void internal::SpriteBatch::erase_sprite_id(GLuint id)
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

	void internal::SpriteBatch::set_texture(GLuint vb_pos, const graphics::BindlessTextureRef& texture, glm::vec2 dimensions)
	{
		SpriteBatch::assert_valid_id(vb_pos);
		auto& tex_slot = quad_ssbo_block.buf.at<INFO>(vb_pos).tex_slot;
		if (quad_info_store.textures.set_object<TexData>(tex_data_ssbo, tex_slot,
			QuadInfoStore::SizedTexture{ texture, dimensions }, TexData{ texture ? texture->get_handle() : 0, dimensions }))
		{
			quad_ssbo_block.flag<INFO>(vb_pos);
			quad_info_store.dimensionless_texture_slot_map[texture].insert(tex_slot);
		}
	}

	void internal::SpriteBatch::set_tex_coords(GLuint vb_pos, math::UVRect uvs)
	{
		SpriteBatch::assert_valid_id(vb_pos);
		if (quad_info_store.tex_coords.set_object(tex_coords_ssbo, quad_ssbo_block.buf.at<INFO>(vb_pos).tex_coord_slot, uvs))
			quad_ssbo_block.flag<INFO>(vb_pos);
	}

	void internal::SpriteBatch::set_modulation(GLuint vb_pos, glm::vec4 modulation)
	{
		SpriteBatch::assert_valid_id(vb_pos);
		if (quad_info_store.modulations.set_object(ubo.modulation, quad_ssbo_block.buf.at<INFO>(vb_pos).color_slot, modulation))
			quad_ssbo_block.flag<INFO>(vb_pos);
	}

	void internal::SpriteBatch::set_frame_format(GLuint vb_pos, const graphics::AnimFrameFormat& anim)
	{
		SpriteBatch::assert_valid_id(vb_pos);
		if (quad_info_store.anims.set_object(ubo.anim, quad_ssbo_block.buf.at<INFO>(vb_pos).frame_slot, anim))
			quad_ssbo_block.flag<INFO>(vb_pos);
	}

	void internal::SpriteBatch::set_text_glyph(GLuint vb_pos, bool is_text_glyph)
	{
		SpriteBatch::assert_valid_id(vb_pos);
		set_quad_info(vb_pos).is_text_glyph = is_text_glyph;
	}

	void internal::SpriteBatch::set_mod_texture(GLuint vb_pos, const graphics::BindlessTextureRef& texture, glm::vec2 dimensions)
	{
		SpriteBatch::assert_valid_id(vb_pos);
		auto& mod_tex_slot = quad_ssbo_block.buf.at<INFO>(vb_pos).mod_tex_slot;
		if (quad_info_store.textures.set_object<TexData>(tex_data_ssbo, mod_tex_slot, QuadInfoStore::SizedTexture{ texture, texture ? dimensions : glm::vec2(0.0f) },
			TexData{texture ? texture->get_handle() : 0, texture ? dimensions : glm::vec2(0.0f) }))
		{
			quad_ssbo_block.flag<INFO>(vb_pos);
			quad_info_store.dimensionless_texture_slot_map[texture].insert(mod_tex_slot);
		}
	}

	void internal::SpriteBatch::set_mod_tex_coords(GLuint vb_pos, math::UVRect uvs)
	{
		SpriteBatch::assert_valid_id(vb_pos);
		if (quad_info_store.tex_coords.set_object(tex_coords_ssbo, quad_ssbo_block.buf.at<INFO>(vb_pos).mod_tex_coord_slot, uvs))
			quad_ssbo_block.flag<INFO>(vb_pos);
	}

	graphics::BindlessTextureRef internal::SpriteBatch::get_texture(GLuint vb_pos, glm::vec2& dimensions) const
	{
		SpriteBatch::assert_valid_id(vb_pos);
		GLuint slot = get_quad_info(vb_pos).tex_slot;
		if (slot == 0)
			return nullptr;
		QuadInfoStore::SizedTexture tex = quad_info_store.textures.get_object(slot);
		dimensions = tex.dimensions;
		return tex.texture;
	}

	math::UVRect internal::SpriteBatch::get_tex_coords(GLuint vb_pos) const
	{
		SpriteBatch::assert_valid_id(vb_pos);
		GLuint slot = get_quad_info(vb_pos).tex_coord_slot;
		return slot != 0 ? quad_info_store.tex_coords.get_object(slot) : math::UVRect{};
	}

	glm::vec4 internal::SpriteBatch::get_modulation(GLuint vb_pos) const
	{
		SpriteBatch::assert_valid_id(vb_pos);
		GLuint slot = get_quad_info(vb_pos).color_slot;
		return slot != 0 ? quad_info_store.modulations.get_object(slot) : glm::vec4(1.0f);
	}

	graphics::AnimFrameFormat internal::SpriteBatch::get_frame_format(GLuint vb_pos) const
	{
		SpriteBatch::assert_valid_id(vb_pos);
		GLuint slot = get_quad_info(vb_pos).frame_slot;
		return slot != 0 ? quad_info_store.anims.get_object(slot) : graphics::AnimFrameFormat{};
	}

	bool internal::SpriteBatch::is_text_glyph(GLuint vb_pos) const
	{
		SpriteBatch::assert_valid_id(vb_pos);
		return get_quad_info(vb_pos).is_text_glyph;
	}

	graphics::BindlessTextureRef internal::SpriteBatch::get_mod_texture(GLuint vb_pos, glm::vec2& dimensions) const
	{
		SpriteBatch::assert_valid_id(vb_pos);
		GLuint slot = get_quad_info(vb_pos).mod_tex_slot;
		if (slot == 0)
			return nullptr;
		QuadInfoStore::SizedTexture tex = quad_info_store.textures.get_object(slot);
		dimensions = tex.dimensions;
		return tex.texture;
	}

	math::UVRect internal::SpriteBatch::get_mod_tex_coords(GLuint vb_pos) const
	{
		SpriteBatch::assert_valid_id(vb_pos);
		GLuint slot = get_quad_info(vb_pos).mod_tex_coord_slot;
		return slot != 0 ? quad_info_store.tex_coords.get_object(slot) : math::UVRect{};
	}

	void internal::SpriteBatch::update_texture_handle(const graphics::BindlessTextureRef& texture)
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
		: Super(context::sprite_batch()->weak_from_this())
	{
		if (auto batch = lock())
		{
			id = batch->gen_sprite_id();
			set_attributes(*this, Attributes{});
		}
	}

	internal::SpriteReference::SpriteReference(Unbatched)
	{
	}

	internal::SpriteReference::SpriteReference(SpriteBatch& batch)
		: Super(batch.weak_from_this())
	{
		id = batch.gen_sprite_id();
		set_attributes(*this, Attributes{});
	}

	internal::SpriteReference::SpriteReference(rendering::SpriteBatch& batch)
		: Super(batch->weak_from_this())
	{
		id = batch->gen_sprite_id();
		set_attributes(*this, Attributes{});
	}

	internal::SpriteReference::SpriteReference(const SpriteReference& other)
		: Super(other)
	{
		if (other.id != SpriteBatch::NULL_ID)
		{
			if (auto batch = lock())
			{
				id = batch->gen_sprite_id();
				set_attributes(*this, get_attributes_ref(other));
			}
		}
	}

	internal::SpriteReference::SpriteReference(SpriteReference&& other) noexcept
		: Super(std::move(other)), id(other.id)
	{
		other.id = SpriteBatch::NULL_ID;
	}

	internal::SpriteReference::~SpriteReference()
	{
		if (auto batch = lock())
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
			if (auto batch = lock())
				batch->erase_sprite_id(id);
			Super::operator=(std::move(other));
			id = other.id;
			other.id = SpriteBatch::NULL_ID;
		}
		return *this;
	}

	bool internal::SpriteReference::is_in_context() const
	{
		if (auto batch = lock())
			return batch.get() == context::sprite_batch().address();
		return false;
	}

	void internal::SpriteReference::set_batch(Unbatched)
	{
		if (auto batch = lock())
			batch->erase_sprite_id(id);
		id = SpriteBatch::NULL_ID;
		reset();
	}

	void internal::SpriteReference::set_batch(rendering::SpriteBatch& new_batch)
	{
		if (auto batch = lock())
		{
			if (batch.get() == new_batch.address())
				return;

			const Attributes attr = id != SpriteBatch::NULL_ID ? get_attributes(*this) : Attributes{};
			batch->erase_sprite_id(id);
			reset(*new_batch);
			batch = lock();
			id = batch->gen_sprite_id();
			set_attributes(*this, attr);
		}
		else
		{
			reset(*new_batch);
			batch = lock();
			id = batch->gen_sprite_id();
			set_attributes(*this, Attributes{});
		}
	}

	void internal::SpriteReference::set_texture(const std::string& texture_file, unsigned int texture_index) const
	{
		if (auto batch = lock()) [[likely]]
		{
			SpriteBatch::assert_valid_id(id);
			graphics::BindlessTextureRef texture = context::load_texture(texture_file, texture_index);
			glm::vec2 dimensions = context::get_texture_dimensions(texture_file, texture_index);
			batch->set_texture(id, texture, dimensions);
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::SpriteReference::set_texture(const graphics::BindlessTextureRef& texture) const
	{
		SpriteBatch::assert_valid_id(id);
		if (auto batch = lock()) [[likely]]
			batch->set_texture(id, texture, context::get_texture_dimensions(texture));
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::SpriteReference::set_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions) const
	{
		if (auto batch = lock()) [[likely]]
			batch->set_texture(id, texture, dimensions);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::SpriteReference::set_tex_coords(math::UVRect rect) const
	{
		if (auto batch = lock()) [[likely]]
			batch->set_tex_coords(id, rect);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::SpriteReference::set_modulation(glm::vec4 modulation) const
	{
		if (auto batch = lock()) [[likely]]
			batch->set_modulation(id, modulation);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::SpriteReference::set_frame_format(const graphics::AnimFrameFormat& anim) const
	{
		if (auto batch = lock()) [[likely]]
			batch->set_frame_format(id, anim);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::SpriteReference::set_text_glyph(bool is_text_glyph) const
	{
		if (auto batch = lock()) [[likely]]
			batch->set_text_glyph(id, is_text_glyph);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::SpriteReference::set_mod_texture(const std::string& texture_file, unsigned int texture_index) const
	{
		if (auto batch = lock()) [[likely]]
		{
			SpriteBatch::assert_valid_id(id);
			auto texture = context::load_texture(texture_file, texture_index);
			batch->set_mod_texture(id, texture, context::get_texture_dimensions(texture_file, texture_index));
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::SpriteReference::set_mod_texture(const graphics::BindlessTextureRef& texture) const
	{
		SpriteBatch::assert_valid_id(id);
		if (auto batch = lock()) [[likely]]
			batch->set_mod_texture(id, texture, context::get_texture_dimensions(texture));
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::SpriteReference::set_mod_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions) const
	{
		if (auto batch = lock()) [[likely]]
			batch->set_mod_texture(id, texture, dimensions);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::SpriteReference::set_mod_tex_coords(math::UVRect rect) const
	{
		if (auto batch = lock()) [[likely]]
			batch->set_mod_tex_coords(id, rect);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::SpriteReference::set_transform(const glm::mat3& transform) const
	{
		SpriteBatch::assert_valid_id(id);
		if (auto batch = lock()) [[likely]]
			batch->quad_ssbo_block.set<SpriteBatch::TRANSFORM>(id) = transform;
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	graphics::BindlessTextureRef internal::SpriteReference::get_texture() const
	{
		if (auto batch = lock()) [[likely]]
		{
			glm::vec2 _;
			return batch->get_texture(id, _);
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	graphics::BindlessTextureRef internal::SpriteReference::get_texture(glm::vec2& dimensions) const
	{
		if (auto batch = lock()) [[likely]]
			return batch->get_texture(id, dimensions);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	math::UVRect internal::SpriteReference::get_tex_coords() const
	{
		if (auto batch = lock()) [[likely]]
			return batch->get_tex_coords(id);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	glm::vec4 internal::SpriteReference::get_modulation() const
	{
		if (auto batch = lock()) [[likely]]
			return batch->get_modulation(id);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	graphics::AnimFrameFormat internal::SpriteReference::get_frame_format() const
	{
		if (auto batch = lock()) [[likely]]
			return batch->get_frame_format(id);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	bool internal::SpriteReference::is_text_glyph() const
	{
		if (auto batch = lock()) [[likely]]
			return batch->is_text_glyph(id);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	graphics::BindlessTextureRef internal::SpriteReference::get_mod_texture() const
	{
		if (auto batch = lock()) [[likely]]
		{
			glm::vec2 _;
			return batch->get_mod_texture(id, _);
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	graphics::BindlessTextureRef internal::SpriteReference::get_mod_texture(glm::vec2& dimensions) const
	{
		if (auto batch = lock()) [[likely]]
			return batch->get_mod_texture(id, dimensions);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	math::UVRect internal::SpriteReference::get_mod_tex_coords() const
	{
		if (auto batch = lock()) [[likely]]
			return batch->get_mod_tex_coords(id);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	glm::mat3 internal::SpriteReference::get_transform() const
	{
		if (auto batch = lock()) [[likely]]
		{
			SpriteBatch::assert_valid_id(id);
			return batch->quad_ssbo_block.get<SpriteBatch::TRANSFORM>(id);
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::SpriteReference::draw_quad() const
	{
		if (auto batch = lock()) [[likely]]
		{
			SpriteBatch::assert_valid_id(id);
			graphics::quad_indices(batch->ebo.draw_primitive().data(), id);
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}
}
