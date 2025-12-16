#include "Ellipses.h"

#include <algorithm>

#include "core/context/rendering/Rendering.h"
#include "graphics/resources/Shaders.h"

#include "physics/collision/elements/OBB.h"

namespace oly::rendering
{
	internal::EllipseBatch::EllipseBatch()
		: ebo(vao)
	{
		shader_locations.projection = glGetUniformLocation(graphics::internal_shaders::ellipse_batch, "uProjection");
		shader_locations.invariant_projection = glGetUniformLocation(graphics::internal_shaders::ellipse_batch, "uInvariantProjection");
	}

	void internal::EllipseBatch::render() const
	{
		if (ebo.empty() || !camera)
			return;

		ssbo_block.pre_draw_all();

		glBindVertexArray(vao);
		glUseProgram(graphics::internal_shaders::ellipse_batch);
		glUniformMatrix3fv(shader_locations.projection, 1, GL_FALSE, glm::value_ptr(camera->projection_matrix()));
		glUniformMatrix3fv(shader_locations.invariant_projection, 1, GL_FALSE, glm::value_ptr(camera->invariant_projection_matrix()));

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_block.buf.get_buffer<DIMENSION>());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_block.buf.get_buffer<COLOR>());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo_block.buf.get_buffer<TRANSFORM>());
		ebo.render_elements(GL_TRIANGLES);

		ssbo_block.post_draw_all();
	}

	void internal::EllipseBatch::assert_valid_id(GLuint id)
	{
		if (id == NULL_ID) [[unlikely]]
			throw Error(ErrorCode::INVALID_ID);
	}

	GLuint internal::EllipseBatch::generate_id()
	{
		GLuint id = id_generator.gen();
		if (id == NULL_ID)
			throw Error(ErrorCode::STORAGE_OVERFLOW);
		else
			return id;
	}

	void internal::EllipseBatch::erase_id(GLuint id)
	{
		if (id != NULL_ID) [[likely]]
			id_generator.yield(id);
	}

	struct Attributes
	{
		EllipseDimension dimension = {};
		EllipseColorGradient color = {};
		glm::mat3 transform = 1.0f;
	};

	struct AttributesRef
	{
		EllipseDimension dimension = {};
		const EllipseColorGradient& color = {};
		const glm::mat3& transform = 1.0f;
	};

	static Attributes get_attributes(const internal::EllipseReference& ref)
	{
		return {
			.dimension = ref.get_dimension(),
			.color = ref.get_color(),
			.transform = ref.get_transform()
		};
	}

	static AttributesRef get_attributes_ref(const internal::EllipseReference& ref)
	{
		return {
			.dimension = ref.get_dimension(),
			.color = ref.get_color(),
			.transform = ref.get_transform()
		};
	}

	static void set_attributes(internal::EllipseReference& ref, const Attributes& attr)
	{
		ref.set_dimension() = attr.dimension;
		ref.set_color() = attr.color;
		ref.set_transform() = attr.transform;
	}

	static void set_attributes(internal::EllipseReference& ref, const AttributesRef& attr)
	{
		ref.set_dimension() = attr.dimension;
		ref.set_color() = attr.color;
		ref.set_transform() = attr.transform;
	}

	internal::EllipseReference::EllipseReference(Unbatched)
	{
	}

	internal::EllipseReference::EllipseReference(rendering::EllipseBatch& batch)
		: Super(batch->weak_from_this())
	{
		id = batch->generate_id();
		set_attributes(*this, Attributes{});
	}

	internal::EllipseReference::EllipseReference(const EllipseReference& other)
		: Super(other)
	{
		if (other.id != internal::EllipseBatch::NULL_ID)
		{
			if (auto batch = lock())
			{
				id = batch->generate_id();
				set_attributes(*this, get_attributes_ref(other));
			}
		}
	}

	internal::EllipseReference::EllipseReference(EllipseReference&& other) noexcept
		: Super(std::move(other)), id(other.id)
	{
		other.id = internal::EllipseBatch::NULL_ID;
	}

	internal::EllipseReference::~EllipseReference()
	{
		if (auto batch = lock())
			batch->erase_id(id);
	}

	internal::EllipseReference& internal::EllipseReference::operator=(const EllipseReference& other)
	{
		if (this != &other)
			*this = dupl(other);
		return *this;
	}

	internal::EllipseReference& internal::EllipseReference::operator=(EllipseReference&& other) noexcept
	{
		if (this != &other)
		{
			if (auto batch = lock())
				batch->erase_id(id);
			Super::operator=(std::move(other));
			id = other.id;
			other.id = internal::EllipseBatch::NULL_ID;
		}
		return *this;
	}

	void internal::EllipseReference::set_batch(Unbatched)
	{
		if (auto batch = lock())
			batch->erase_id(id);
		id = internal::EllipseBatch::NULL_ID;
		reset();
	}

	void internal::EllipseReference::set_batch(rendering::EllipseBatch& new_batch)
	{
		if (auto batch = lock())
		{
			if (batch.get() == new_batch.address())
				return;

			const Attributes attr = id != internal::EllipseBatch::NULL_ID ? get_attributes(*this) : Attributes{};
			batch->erase_id(id);
			reset(*new_batch);
			batch = lock();
			id = batch->generate_id();
			set_attributes(*this, attr);
		}
		else
		{
			reset(*new_batch);
			batch = lock();
			id = batch->generate_id();
			set_attributes(*this, Attributes{});
		}
	}

	EllipseDimension internal::EllipseReference::get_dimension() const
	{
		if (auto batch = lock()) [[likely]]
		{
			internal::EllipseBatch::assert_valid_id(id);
			return batch->ssbo_block.get<internal::EllipseBatch::DIMENSION>(id);
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	EllipseDimension& internal::EllipseReference::set_dimension() const
	{
		if (auto batch = lock()) [[likely]]
		{
			internal::EllipseBatch::assert_valid_id(id);
			return batch->ssbo_block.set<internal::EllipseBatch::DIMENSION>(id);
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	const EllipseColorGradient& internal::EllipseReference::get_color() const
	{
		if (auto batch = lock()) [[likely]]
		{
			internal::EllipseBatch::assert_valid_id(id);
			return batch->ssbo_block.get<internal::EllipseBatch::COLOR>(id);
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	EllipseColorGradient& internal::EllipseReference::set_color() const
	{
		if (auto batch = lock()) [[likely]]
		{
			internal::EllipseBatch::assert_valid_id(id);
			return batch->ssbo_block.set<internal::EllipseBatch::COLOR>(id);
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::EllipseReference::set_color(glm::vec4 color) const
	{
		set_color().set_uniform(color);
	}

	const glm::mat3& internal::EllipseReference::get_transform() const
	{
		if (auto batch = lock()) [[likely]]
		{
			internal::EllipseBatch::assert_valid_id(id);
			return batch->ssbo_block.get<internal::EllipseBatch::TRANSFORM>(id);
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	glm::mat3& internal::EllipseReference::set_transform() const
	{
		if (auto batch = lock()) [[likely]]
		{
			internal::EllipseBatch::assert_valid_id(id);
			return batch->ssbo_block.set<internal::EllipseBatch::TRANSFORM>(id);
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::EllipseReference::draw() const
	{
		if (auto batch = lock()) [[likely]]
		{
			internal::EllipseBatch::assert_valid_id(id);
			graphics::quad_indices(batch->ebo.draw_primitive().data(), id);
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	StaticEllipse::StaticEllipse(EllipseBatch& batch, float r, glm::vec4 color)
		: ref(batch)
	{
		ref.set_dimension().rx = r;
		ref.set_dimension().ry = r;
		ref.set_color(color);
	}
	
	StaticEllipse::StaticEllipse(EllipseBatch& batch, float rx, float ry, glm::vec4 color)
		: ref(batch)
	{
		ref.set_dimension().rx = rx;
		ref.set_dimension().ry = ry;
		ref.set_color(color);
	}

	void StaticEllipse::set_batch(EllipseBatch& batch)
	{
		ref.set_batch(batch);
		ref.set_dimension() = dimension;
		ref.set_color() = color;
		ref.set_transform() = transform;
	}

	void StaticEllipse::set_dimension(EllipseDimension dimension)
	{
		this->dimension = dimension;
		if (ref.get_batch())
			ref.set_dimension() = dimension;
	}

	void StaticEllipse::set_color(const EllipseColorGradient& color)
	{
		this->color = color;
		if (ref.get_batch())
			ref.set_color() = color;
	}

	void StaticEllipse::set_color(glm::vec4 color)
	{
		this->color.set_uniform(color);
		if (ref.get_batch())
			ref.set_color(color);
	}

	void StaticEllipse::set_transform(const glm::mat3& transform)
	{
		this->transform = transform;
		if (ref.get_batch())
			ref.set_transform() = transform;
	}

	math::Rect2D StaticEllipse::bounds() const
	{
		const EllipseDimension d = ref.get_dimension();
		std::array<glm::vec2, 4> pts = math::Rect2D{ .x1 = -d.rx, .x2 = d.rx, .y1 = -d.ry, .y2 = d.ry }.uvs();

		for (size_t i = 0; i < 4; ++i)
			pts[i] = transform_point(transform, pts[i]);

		math::Rect2D b{ .x1 = nmax<float>(), .x2 = -nmax<float>(), .y1 = nmax<float>(), .y2 = -nmax<float>() };
		for (size_t i = 0; i < 4; ++i)
		{
			b.x1 = min(b.x1, pts[i].x);
			b.x2 = max(b.x2, pts[i].x);
			b.y1 = min(b.y1, pts[i].y);
			b.y2 = max(b.y2, pts[i].y);
		}
		return b;
	}

	math::RotatedRect2D StaticEllipse::rotated_bounds() const
	{
		const EllipseDimension d = ref.get_dimension();
		std::array<glm::vec2, 4> pts = math::Rect2D{ .x1 = -d.rx, .x2 = d.rx, .y1 = -d.ry, .y2 = d.ry }.uvs();

		for (size_t i = 0; i < 4; ++i)
			pts[i] = transform_point(transform, pts[i]);

		return col2d::OBB::fast_wrap(pts.data(), 4).rect();
	}

	Ellipse::Ellipse(EllipseBatch& batch, float r, glm::vec4 color)
		: ref(batch)
	{
		ref.set_dimension().rx = r;
		ref.set_dimension().ry = r;
		ref.set_color(color);
	}

	Ellipse::Ellipse(EllipseBatch& batch, float rx, float ry, glm::vec4 color)
		: ref(batch)
	{
		ref.set_dimension().rx = rx;
		ref.set_dimension().ry = ry;
		ref.set_color(color);
	}

	void Ellipse::draw() const
	{
		if (transformer.flush())
			ref.set_transform() = transformer.global();
		ref.draw();
	}
}
