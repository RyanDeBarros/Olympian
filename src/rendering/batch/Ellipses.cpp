#include "Ellipses.h"

#include <glm/gtc/type_ptr.hpp>

#include <algorithm>

#include "../Resources.h"
#include "math/Transforms.h"

namespace oly
{
	namespace rendering
	{
		EllipseBatch::EllipseBatch(Capacity capacity, const glm::vec4& projection_bounds)
			: ebo(vao, capacity.ellipses), ssbo_block(capacity.ellipses), projection_bounds(projection_bounds)
		{
			projection_location = shaders::location(shaders::ellipse_batch, "uProjection");
		}

		void EllipseBatch::render() const
		{
			ssbo_block.pre_draw_all();

			glBindVertexArray(vao);
			glUseProgram(shaders::ellipse_batch);
			glUniformMatrix3fv(projection_location, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]))));

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

		EllipseBatch::EllipseReference::EllipseReference(EllipseBatch* batch)
			: _batch(batch)
		{
			pos = _batch->generate_id();
			set_dimension() = {};
			set_color() = {};
			set_transform() = 1.0f;
		}

		EllipseBatch::EllipseReference::EllipseReference(const EllipseReference& other)
			: _batch(other._batch)
		{
			pos = _batch->generate_id();
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

		const Transform2D& Ellipse::get_local() const
		{
			transformer.pre_get();
			return transformer.local;
		}

		Transform2D& Ellipse::set_local()
		{
			transformer.pre_get();
			transformer.post_set();
			return transformer.local;
		}

		void Ellipse::draw() const
		{
			if (transformer.flush())
			{
				transformer.pre_get();
				const_cast<EllipseBatch::EllipseReference&>(ellipse).set_transform() = transformer.global();
			}
			quad_indices(batch().ebo.draw_primitive().data(), ellipse.pos.get());
		}
	}
}
