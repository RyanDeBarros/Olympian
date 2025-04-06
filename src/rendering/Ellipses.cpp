#include "Ellipses.h"

#include <glm/gtc/type_ptr.hpp>

#include "Resources.h"
#include "math/Transforms.h"

namespace oly
{
	namespace batch
	{
		EllipseBatch::EllipseBatch(Capacity capacity, const glm::vec4& projection_bounds)
			: capacity(capacity), ebo(capacity.ellipses), dimension_ssbo(capacity.ellipses), color_ssbo(capacity.ellipses), transform_ssbo(capacity.ellipses)
		{
			shader = shaders::ellipse_batch;
			glUseProgram(shader);
			projection_location = glGetUniformLocation(shader, "uProjection");

			glBindVertexArray(vao);
			rendering::pre_init(ebo);
			ebo.init();
			glBindVertexArray(0);

			set_projection(projection_bounds);
			draw_specs.push_back({ 0, capacity.ellipses });

			dimension_ssbo.vector()[0] = { 2, 1, 0.3f, 1.0f, 1.0f };
			dimension_ssbo.lazy_send(0);
			ColorGradient c;
			c.fill_inner = { 1.0f, 0.9f, 0.8f, 0.5f };
			c.fill_outer = { 1.0f, 0.6f, 0.2f, 1.0f };
			c.border_inner = { 0.2f, 0.6f, 1.0f, 1.0f };
			c.border_outer = { 0.8f, 0.9f, 1.0f, 1.0f };
			color_ssbo.vector()[0] = c;
			color_ssbo.lazy_send(0);
			transform_ssbo.vector()[0] = Transform2D{ { -300, 0 }, 0, { 150, 150 } }.matrix();
			transform_ssbo.lazy_send(0);

			dimension_ssbo.vector()[1] = { 1, 3, 0.4f, 0.5f, 2.0f };
			dimension_ssbo.lazy_send(1);
			c.fill_inner = { 1.0f, 0.9f, 0.8f, 0.5f };
			c.fill_outer = { 1.0f, 0.6f, 0.2f, 1.0f };
			c.border_inner = { 0.0f, 0.0f, 0.0f, 1.0f };
			c.border_outer = { 0.8f, 0.9f, 1.0f, 0.0f };
			color_ssbo.vector()[1] = c;
			color_ssbo.lazy_send(1);
			transform_ssbo.vector()[1] = Transform2D{ { 300, 0 }, 0, { 150, 150 } }.matrix();
			transform_ssbo.lazy_send(1);
		}

		void EllipseBatch::draw(size_t draw_spec)
		{
			glUseProgram(shader);
			glBindVertexArray(vao);

			dimension_ssbo.bind_base(0);
			color_ssbo.bind_base(1);
			transform_ssbo.bind_base(2);
			ebo.set_draw_spec(draw_specs[draw_spec].initial, draw_specs[draw_spec].length);
			ebo.draw(GL_TRIANGLES, GL_UNSIGNED_SHORT);
		}

		void EllipseBatch::set_projection(const glm::vec4& projection_bounds) const
		{
			glm::mat3 proj = glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]);
			glUseProgram(shader);
			glUniformMatrix3fv(projection_location, 1, GL_FALSE, glm::value_ptr(proj));
		}

		void EllipseBatch::flush() const
		{
			dimension_ssbo.flush();
			color_ssbo.flush();
			transform_ssbo.flush();
		}
	}
}
