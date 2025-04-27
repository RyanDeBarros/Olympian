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
			: capacity(capacity), ebo(capacity.ellipses), ssbo(capacity.ellipses), projection_bounds(projection_bounds)
		{
			projection_location = shaders::location(shaders::ellipse_batch, "uProjection");

			glBindVertexArray(vao);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.buf.get_buffer());
			glBindVertexArray(0);
		}

		void EllipseBatch::render() const
		{
			ssbo.dimension.pre_draw();
			ssbo.color.pre_draw();
			ssbo.transform.pre_draw();
			ebo.pre_draw();

			glBindVertexArray(vao);
			glUseProgram(shaders::ellipse_batch);
			glUniformMatrix3fv(projection_location, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]))));

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo.dimension.buf.get_buffer());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo.color.buf.get_buffer());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo.transform.buf.get_buffer());
			glDrawElements(GL_TRIANGLES, num_ellipses_to_draw * 6, GL_UNSIGNED_INT, 0);
			num_ellipses_to_draw = 0;

			ssbo.dimension.post_draw();
			ssbo.color.post_draw();
			ssbo.transform.post_draw();
			ebo.post_draw();
		}

		void EllipseBatch::grow_ssbos()
		{
			ssbo.dimension.grow();
			ssbo.color.grow();
			ssbo.transform.grow();
		}

		void EllipseBatch::grow_ebo() const
		{
			ebo.grow();
			glBindVertexArray(vao);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.buf.get_buffer());
			glBindVertexArray(0);
		}

		EllipseBatch::EllipseReference::EllipseReference(EllipseBatch* batch)
			: _batch(batch)
		{
			pos = _batch->pos_generator.generate();
			if (pos.get() > _batch->ssbo.dimension.buf.get_size())
				_batch->grow_ssbos();
			dimension() = {};
			color() = {};
			transform() = 1.0f;
		}
		
		void EllipseBatch::EllipseReference::flag_dimension() const
		{
			_batch->ssbo.dimension.flag(pos.get());
		}

		void EllipseBatch::EllipseReference::flag_color() const
		{
			_batch->ssbo.color.flag(pos.get());
		}

		void EllipseBatch::EllipseReference::flag_transform() const
		{
			_batch->ssbo.transform.flag(pos.get());
		}

		void Ellipse::draw() const
		{
			if (transformer.flush())
			{
				transformer.pre_get();
				ellipse.transform() = transformer.global();
				ellipse.flag_transform();
			}

			GLuint& num_ellipses_to_draw = batch().num_ellipses_to_draw;
			if (num_ellipses_to_draw >= batch().ebo.buf.get_size())
				batch().grow_ebo();
			quad_indices(batch().ebo.buf.arr(num_ellipses_to_draw, 1)->data(), num_ellipses_to_draw);
			batch().ebo.flag(num_ellipses_to_draw);
			++num_ellipses_to_draw;
		}
	}
}
