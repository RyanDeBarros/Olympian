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

	EllipseReference::EllipseReference(EllipseBatch& batch)
		: batch(batch)
	{
		pos = this->batch.generate_id();
		set_dimension() = {};
		set_color() = {};
		set_transform() = 1.0f;
	}

	EllipseReference::EllipseReference(const EllipseReference& other)
		: batch(other.batch)
	{
		pos = batch.generate_id();
		set_dimension() = other.get_dimension();
		set_color() = other.get_color();
		set_transform() = other.get_transform();
	}

	EllipseReference::EllipseReference(EllipseReference&& other) noexcept
		: batch(other.batch)
	{
		EllipseBatch::EllipseDimension dimension = other.get_dimension();
		EllipseBatch::ColorGradient color = other.get_color();
		glm::mat3 transform = other.get_transform();
		pos = std::move(other.pos);
		set_dimension() = dimension;
		set_color() = color;
		set_transform() = transform;
	}

	EllipseReference& EllipseReference::operator=(const EllipseReference& other)
	{
		if (this != &other)
		{
			set_dimension() = other.get_dimension();
			set_color() = other.get_color();
			set_transform() = other.get_transform();
		}
		return *this;
	}

	EllipseReference& EllipseReference::operator=(EllipseReference&& other) noexcept
	{
		if (this != &other)
		{
			set_dimension() = other.get_dimension();
			set_color() = other.get_color();
			set_transform() = other.get_transform();
		}
		return *this;
	}

	const EllipseBatch::EllipseDimension& EllipseReference::get_dimension() const
	{
		return batch.ssbo_block.get<EllipseBatch::DIMENSION>(pos.get());
	}

	EllipseBatch::EllipseDimension& EllipseReference::set_dimension()
	{
		return batch.ssbo_block.set<EllipseBatch::DIMENSION>(pos.get());
	}

	const EllipseBatch::ColorGradient& EllipseReference::get_color() const
	{
		return batch.ssbo_block.get<EllipseBatch::COLOR>(pos.get());
	}

	EllipseBatch::ColorGradient& EllipseReference::set_color()
	{
		return batch.ssbo_block.set<EllipseBatch::COLOR>(pos.get());
	}

	const glm::mat3& EllipseReference::get_transform() const
	{
		return batch.ssbo_block.get<EllipseBatch::TRANSFORM>(pos.get());
	}

	glm::mat3& EllipseReference::set_transform()
	{
		return batch.ssbo_block.set<EllipseBatch::TRANSFORM>(pos.get());
	}

	void EllipseReference::draw() const
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
