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
			: ebo(vao, capacity.ellipses), ssbo(capacity.ellipses), projection_bounds(projection_bounds)
		{
			projection_location = shaders::location(shaders::ellipse_batch, "uProjection");
		}

		void EllipseBatch::render() const
		{
			ssbo.dimension.pre_draw();
			ssbo.color.pre_draw();
			ssbo.transform.pre_draw();

			glBindVertexArray(vao);
			glUseProgram(shaders::ellipse_batch);
			glUniformMatrix3fv(projection_location, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]))));

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo.dimension.buf.get_buffer());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo.color.buf.get_buffer());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo.transform.buf.get_buffer());
			ebo.render_elements(GL_TRIANGLES);

			ssbo.dimension.post_draw();
			ssbo.color.post_draw();
			ssbo.transform.post_draw();
		}

		EllipseBatch::EllipseID EllipseBatch::generate_id()
		{
			EllipseID id = pos_generator.generate();
			if (id.get() >= ssbo.dimension.buf.get_size())
			{
				ssbo.dimension.grow();
				ssbo.color.grow();
				ssbo.transform.grow();
			}
			return id;
		}

		EllipseBatch::EllipseReference::EllipseReference(EllipseBatch* batch)
			: _batch(batch)
		{
			pos = _batch->generate_id();
			dimension() = {};
			color() = {};
			transform() = 1.0f;
		}

		EllipseBatch::EllipseReference::EllipseReference(const EllipseReference& other)
			: _batch(other._batch)
		{
			pos = _batch->generate_id();
			dimension() = other.dimension();
			color() = other.color();
			transform() = other.transform();
		}

		EllipseBatch::EllipseReference& EllipseBatch::EllipseReference::operator=(const EllipseReference& other)
		{
			if (this != &other)
			{
				dimension() = other.dimension();
				color() = other.color();
				transform() = other.transform();
			}
			return *this;
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
			quad_indices(batch().ebo.draw_primitive().data(), ellipse.pos.get());
		}
	}
}
