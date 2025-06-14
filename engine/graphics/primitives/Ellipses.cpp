#include "Ellipses.h"

#include <algorithm>

#include "core/base/Context.h"
#include "graphics/resources/Shaders.h"
#include "Ellipses.h"

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

	EllipseBatch::EllipseReference::EllipseReference()
	{
		pos = context::ellipse_batch().generate_id();
		set_dimension() = {};
		set_color() = {};
		set_transform() = 1.0f;
	}

	EllipseBatch::EllipseReference::EllipseReference(const EllipseReference& other)
	{
		pos = context::ellipse_batch().generate_id();
		set_dimension() = other.get_dimension();
		set_color() = other.get_color();
		set_transform() = other.get_transform();
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

	const EllipseBatch::EllipseDimension& EllipseBatch::EllipseReference::get_dimension() const
	{
		return context::ellipse_batch().ssbo_block.get<DIMENSION>(pos.get());
	}

	EllipseBatch::EllipseDimension& EllipseBatch::EllipseReference::set_dimension()
	{
		return context::ellipse_batch().ssbo_block.set<DIMENSION>(pos.get());
	}

	const EllipseBatch::ColorGradient& EllipseBatch::EllipseReference::get_color() const
	{
		return context::ellipse_batch().ssbo_block.get<COLOR>(pos.get());
	}

	EllipseBatch::ColorGradient& EllipseBatch::EllipseReference::set_color()
	{
		return context::ellipse_batch().ssbo_block.set<COLOR>(pos.get());
	}

	const glm::mat3& EllipseBatch::EllipseReference::get_transform() const
	{
		return context::ellipse_batch().ssbo_block.get<TRANSFORM>(pos.get());
	}

	glm::mat3& EllipseBatch::EllipseReference::set_transform()
	{
		return context::ellipse_batch().ssbo_block.set<TRANSFORM>(pos.get());
	}

	void EllipseBatch::EllipseReference::draw() const
	{
		graphics::quad_indices(context::ellipse_batch().ebo.draw_primitive().data(), pos.get());
	}

	void Ellipse::draw() const
	{
		if (transformer.flush())
			const_cast<EllipseBatch::EllipseReference&>(ellipse).set_transform() = transformer.global();
		ellipse.draw();
	}
}
