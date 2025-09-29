#include "Ellipses.h"

#include <algorithm>

#include "core/context/rendering/Rendering.h"
#include "graphics/resources/Shaders.h"

namespace oly::rendering
{
	EllipseBatch::EllipseBatch()
		: ebo(vao)
	{
		projection_location = glGetUniformLocation(graphics::internal_shaders::ellipse_batch, "uProjection");
	}

	void EllipseBatch::render() const
	{
		if (ebo.empty())
			return;

		ssbo_block.pre_draw_all();

		glBindVertexArray(vao);
		glUseProgram(graphics::internal_shaders::ellipse_batch);
		glUniformMatrix3fv(projection_location, 1, GL_FALSE, glm::value_ptr(projection));

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_block.buf.get_buffer<DIMENSION>());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_block.buf.get_buffer<COLOR>());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo_block.buf.get_buffer<TRANSFORM>());
		ebo.render_elements(GL_TRIANGLES);

		ssbo_block.post_draw_all();
	}

	void EllipseBatch::assert_valid_id(Index id)
	{
		if (id == NULL_ID) [[unlikely]]
			throw Error(ErrorCode::INVALID_ID);
	}

	EllipseBatch::Index EllipseBatch::generate_id()
	{
		GLuint id = id_generator.gen();
		if (id == NULL_ID)
			throw Error(ErrorCode::STORAGE_OVERFLOW);
		else
			return id;
	}

	void EllipseBatch::erase_id(Index id)
	{
		if (id != NULL_ID) [[likely]]
			id_generator.yield(id);
	}

	struct Attributes
	{
		EllipseBatch::EllipseDimension dimension = {};
		EllipseBatch::ColorGradient color = {};
		glm::mat3 transform = 1.0f;
	};

	struct AttributesRef
	{
		EllipseBatch::EllipseDimension dimension = {};
		const EllipseBatch::ColorGradient& color = {};
		const glm::mat3& transform = 1.0f;
	};

	static Attributes get_attributes(const EllipseReference& ref)
	{
		return {
			.dimension = ref.get_dimension(),
			.color = ref.get_color(),
			.transform = ref.get_transform()
		};
	}

	static AttributesRef get_attributes_ref(const EllipseReference& ref)
	{
		return {
			.dimension = ref.get_dimension(),
			.color = ref.get_color(),
			.transform = ref.get_transform()
		};
	}

	static void set_attributes(EllipseReference& ref, const Attributes& attr)
	{
		ref.set_dimension() = attr.dimension;
		ref.set_color() = attr.color;
		ref.set_transform() = attr.transform;
	}

	static void set_attributes(EllipseReference& ref, const AttributesRef& attr)
	{
		ref.set_dimension() = attr.dimension;
		ref.set_color() = attr.color;
		ref.set_transform() = attr.transform;
	}

	EllipseReference::EllipseReference(Unbatched)
		: batch(nullptr)
	{
	}

	EllipseReference::EllipseReference(EllipseBatch& batch)
		: batch(&batch)
	{
		id = batch.generate_id();
		set_attributes(*this, Attributes{});
	}

	EllipseReference::EllipseReference(const EllipseReference& other)
		: batch(other.batch)
	{
		if (batch && other.id != EllipseBatch::NULL_ID)
		{
			id = batch->generate_id();
			set_attributes(*this, get_attributes_ref(other));
		}
	}

	EllipseReference::EllipseReference(EllipseReference&& other) noexcept
		: batch(other.batch), id(other.id)
	{
		other.id = EllipseBatch::NULL_ID;
	}

	EllipseReference::~EllipseReference()
	{
		if (batch)
			batch->erase_id(id);
	}

	EllipseReference& EllipseReference::operator=(const EllipseReference& other)
	{
		if (this != &other)
			*this = dupl(other);
		return *this;
	}

	EllipseReference& EllipseReference::operator=(EllipseReference&& other) noexcept
	{
		if (this != &other)
		{
			if (batch)
				batch->erase_id(id);
			batch = other.batch;
			id = other.id;
			other.id = EllipseBatch::NULL_ID;
		}
		return *this;
	}

	void EllipseReference::set_batch(Unbatched)
	{
		if (batch)
		{
			batch->erase_id(id);
			batch = nullptr;
		}
	}

	void EllipseReference::set_batch(EllipseBatch& batch)
	{
		if (this->batch == &batch)
			return;

		Attributes attr{};
		if (this->batch && id != EllipseBatch::NULL_ID)
		{
			attr = get_attributes(*this);
			this->batch->erase_id(id);
		}
		this->batch = &batch;
		id = batch.generate_id();
		set_attributes(*this, attr);
	}

	EllipseBatch::EllipseDimension EllipseReference::get_dimension() const
	{
		if (batch) [[likely]]
		{
			EllipseBatch::assert_valid_id(id);
			return batch->ssbo_block.get<EllipseBatch::DIMENSION>(id);
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	EllipseBatch::EllipseDimension& EllipseReference::set_dimension()
	{
		if (batch) [[likely]]
		{
			EllipseBatch::assert_valid_id(id);
			return batch->ssbo_block.set<EllipseBatch::DIMENSION>(id);
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	const EllipseBatch::ColorGradient& EllipseReference::get_color() const
	{
		if (batch) [[likely]]
		{
			EllipseBatch::assert_valid_id(id);
			return batch->ssbo_block.get<EllipseBatch::COLOR>(id);
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	EllipseBatch::ColorGradient& EllipseReference::set_color()
	{
		if (batch) [[likely]]
		{
			EllipseBatch::assert_valid_id(id);
			return batch->ssbo_block.set<EllipseBatch::COLOR>(id);
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	const glm::mat3& EllipseReference::get_transform() const
	{
		if (batch) [[likely]]
		{
			EllipseBatch::assert_valid_id(id);
			return batch->ssbo_block.get<EllipseBatch::TRANSFORM>(id);
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	glm::mat3& EllipseReference::set_transform()
	{
		if (batch) [[likely]]
		{
			EllipseBatch::assert_valid_id(id);
			return batch->ssbo_block.set<EllipseBatch::TRANSFORM>(id);
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void EllipseReference::draw() const
	{
		if (batch) [[likely]]
		{
			EllipseBatch::assert_valid_id(id);
			graphics::quad_indices(batch->ebo.draw_primitive().data(), id);
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	Ellipse::Ellipse(EllipseBatch& batch, float r, glm::vec4 color)
		: ellipse(batch)
	{
		ellipse.set_dimension().rx = r;
		ellipse.set_dimension().ry = r;
		set_color(color);
	}

	Ellipse::Ellipse(EllipseBatch& batch, float rx, float ry, glm::vec4 color)
		: ellipse(batch)
	{
		ellipse.set_dimension().rx = rx;
		ellipse.set_dimension().ry = ry;
		set_color(color);
	}

	void Ellipse::draw() const
	{
		if (transformer.flush())
			const_cast<EllipseReference&>(ellipse).set_transform() = transformer.global();
		ellipse.draw();
	}

	void Ellipse::set_color(glm::vec4 color)
	{
		ellipse.set_color().fill_inner = color;
		ellipse.set_color().fill_outer = color;
		ellipse.set_color().border_inner = color;
		ellipse.set_color().border_outer = color;
	}
}
