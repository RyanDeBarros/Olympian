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

	EllipseBatch::EllipseID EllipseBatch::generate_id()
	{
		return pos_generator.generate();
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

	EllipseReference::EllipseReference(EllipseBatch* batch)
		: batch(batch)
	{
		if (this->batch)
		{
			pos = this->batch->generate_id();
			set_attributes(*this, Attributes{});
		}
	}

	EllipseReference::EllipseReference(const EllipseReference& other)
		: batch(other.batch)
	{
		if (batch)
		{
			pos = batch->generate_id();
			set_attributes(*this, get_attributes_ref(other));
		}
	}

	EllipseReference::EllipseReference(EllipseReference&& other) noexcept
		: batch(other.batch), pos(std::move(other.pos))
	{
	}

	EllipseReference& EllipseReference::operator=(const EllipseReference& other)
	{
		if (this != &other)
		{
			if (batch != other.batch)
			{
				pos.yield();
				batch = other.batch;
				if (batch)
					pos = batch->generate_id();
			}
			if (batch)
				set_attributes(*this, get_attributes_ref(other));
		}
		return *this;
	}

	EllipseReference& EllipseReference::operator=(EllipseReference&& other) noexcept
	{
		if (this != &other)
		{
			if (batch != other.batch)
			{
				batch = other.batch;
				pos = std::move(other.pos);
			}
			else if (batch)
				set_attributes(*this, get_attributes_ref(other));
		}
		return *this;
	}

	void EllipseReference::set_batch(EllipseBatch* batch)
	{
		if (this->batch == batch)
			return;

		if (this->batch)
		{
			if (batch)
			{
				Attributes attr = get_attributes(*this);
				pos.yield();
				this->batch = batch;
				pos = this->batch->generate_id();
				set_attributes(*this, attr);
			}
			else
			{
				pos.yield();
				this->batch = batch;
			}
		}
		else
		{
			this->batch = batch;
			pos = this->batch->generate_id();
			set_attributes(*this, Attributes{});
		}
	}

	EllipseBatch::EllipseDimension EllipseReference::get_dimension() const
	{
		if (batch) [[likely]]
			return batch->ssbo_block.get<EllipseBatch::DIMENSION>(pos.get());
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	EllipseBatch::EllipseDimension& EllipseReference::set_dimension()
	{
		if (batch) [[likely]]
			return batch->ssbo_block.set<EllipseBatch::DIMENSION>(pos.get());
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	const EllipseBatch::ColorGradient& EllipseReference::get_color() const
	{
		if (batch) [[likely]]
			return batch->ssbo_block.get<EllipseBatch::COLOR>(pos.get());
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	EllipseBatch::ColorGradient& EllipseReference::set_color()
	{
		if (batch) [[likely]]
			return batch->ssbo_block.set<EllipseBatch::COLOR>(pos.get());
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	const glm::mat3& EllipseReference::get_transform() const
	{
		if (batch) [[likely]]
			return batch->ssbo_block.get<EllipseBatch::TRANSFORM>(pos.get());
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	glm::mat3& EllipseReference::set_transform()
	{
		if (batch) [[likely]]
			return batch->ssbo_block.set<EllipseBatch::TRANSFORM>(pos.get());
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void EllipseReference::draw() const
	{
		if (batch) [[likely]]
			graphics::quad_indices(batch->ebo.draw_primitive().data(), pos.get());
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	Ellipse::Ellipse(EllipseBatch& batch, float r, glm::vec4 color)
		: ellipse(&batch)
	{
		ellipse.set_dimension().rx = r;
		ellipse.set_dimension().ry = r;
		set_color(color);
	}

	Ellipse::Ellipse(EllipseBatch& batch, float rx, float ry, glm::vec4 color)
		: ellipse(&batch)
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
