#include "Ellipses.h"

#include <algorithm>

#include "core/context/rendering/Rendering.h"
#include "graphics/resources/Shaders.h"

namespace oly::rendering
{
	EllipseBatch::EllipseBatch(Capacity capacity)
		: ebo(vao, capacity.ellipses), ssbo_block(capacity.ellipses)
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

	EllipseBatch::EllipseReference::EllipseReference(EllipseBatch& batch)
		: batch(batch)
	{
		pos = this->batch.generate_id();
		set_dimension() = {};
		set_color() = {};
		set_transform() = 1.0f;
	}

	EllipseBatch::EllipseReference::EllipseReference(const EllipseReference& other)
		: batch(other.batch)
	{
		pos = batch.generate_id();
		set_dimension() = other.get_dimension();
		set_color() = other.get_color();
		set_transform() = other.get_transform();
	}

	EllipseBatch::EllipseReference::EllipseReference(EllipseReference&& other) noexcept
		: batch(other.batch)
	{
		EllipseDimension dimension = other.get_dimension();
		ColorGradient color = other.get_color();
		glm::mat3 transform = other.get_transform();
		pos = std::move(other.pos);
		set_dimension() = dimension;
		set_color() = color;
		set_transform() = transform;
	}

	EllipseBatch::EllipseReference& EllipseBatch::EllipseReference::operator=(const EllipseReference& other)
	{
		if (this != &other)
		{
			set_dimension() = other.get_dimension();
			set_color() = other.get_color();
			set_transform() = other.get_transform();
		}
		return *this;
	}

	EllipseBatch::EllipseReference& EllipseBatch::EllipseReference::operator=(EllipseReference&& other) noexcept
	{
		if (this != &other)
		{
			set_dimension() = other.get_dimension();
			set_color() = other.get_color();
			set_transform() = other.get_transform();
		}
		return *this;
	}

	const EllipseBatch::EllipseDimension& EllipseBatch::EllipseReference::get_dimension() const
	{
		return batch.ssbo_block.get<DIMENSION>(pos.get());
	}

	EllipseBatch::EllipseDimension& EllipseBatch::EllipseReference::set_dimension()
	{
		return batch.ssbo_block.set<DIMENSION>(pos.get());
	}

	const EllipseBatch::ColorGradient& EllipseBatch::EllipseReference::get_color() const
	{
		return batch.ssbo_block.get<COLOR>(pos.get());
	}

	EllipseBatch::ColorGradient& EllipseBatch::EllipseReference::set_color()
	{
		return batch.ssbo_block.set<COLOR>(pos.get());
	}

	const glm::mat3& EllipseBatch::EllipseReference::get_transform() const
	{
		return batch.ssbo_block.get<TRANSFORM>(pos.get());
	}

	glm::mat3& EllipseBatch::EllipseReference::set_transform()
	{
		return batch.ssbo_block.set<TRANSFORM>(pos.get());
	}

	void EllipseBatch::EllipseReference::draw() const
	{
		graphics::quad_indices(batch.ebo.draw_primitive().data(), pos.get());
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
			const_cast<EllipseBatch::EllipseReference&>(ellipse).set_transform() = transformer.global();
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
