#include "Particles.h"

#include <glm/gtc/type_ptr.hpp>

#include "util/Assert.h"
#include "Resources.h"

namespace oly
{
	namespace particles
	{
		Particle::Particle(GLushort max_instances, GLushort ebo_size)
			: max_instances(max_instances), ebo(ebo_size, rendering::BufferSendConfig(rendering::BufferSendType::SUBDATA, false))
		{
		}

		Emitter::Emitter(std::unique_ptr<Particle>&& inst, const glm::vec4& projection_bounds)
			: instance(std::move(inst))
		{
			set_projection(projection_bounds);

			instance->set_transform(0, Transform2D{ {   0,  0 }, 0, { 30, 30 } }.matrix());
			instance->set_transform(1, Transform2D{ {  60, 20 }, 0, { 30, 30 } }.matrix());
			instance->set_transform(2, Transform2D{ { 120, 40 }, 0, { 30, 30 } }.matrix());
			instance->set_transform(3, Transform2D{ { 180, 60 }, 0, { 30, 30 } }.matrix());
			instance->set_color(0, { 1.0f, 0.0f, 0.0f, 1.0f });
			instance->set_color(1, { 0.8f, 0.1f, 0.1f, 1.0f });
			instance->set_color(2, { 0.6f, 0.2f, 0.2f, 1.0f });
			instance->set_color(3, { 0.4f, 0.3f, 0.3f, 1.0f });
			state.buf_offset = 0;
			state.draw_count = 4;
		}

		void Emitter::set_projection(const glm::vec4& projection_bounds) const
		{
			instance->set_projection(projection_bounds);
		}

		void Emitter::check_params()
		{
		}

		void Emitter::update()
		{
			instance->send_buffers();
		}

		void Emitter::draw() const
		{
			glUseProgram(instance->shader);
			glBindVertexArray(instance->vao);
			instance->pre_draw();
			if (state.buf_offset + state.draw_count > instance->max_instances)
			{
				instance->set_instance_offset(state.buf_offset);
				glDrawElementsInstanced(instance->draw_spec.mode, instance->draw_spec.count, instance->draw_spec.type, (void*)instance->draw_spec.offset, instance->max_instances - state.buf_offset);
				instance->set_instance_offset(0);
				glDrawElementsInstanced(instance->draw_spec.mode, instance->draw_spec.count, instance->draw_spec.type, (void*)instance->draw_spec.offset, state.buf_offset + state.draw_count - instance->max_instances);
			}
			else
			{
				instance->set_instance_offset(state.buf_offset);
				glDrawElementsInstanced(instance->draw_spec.mode, instance->draw_spec.count, instance->draw_spec.type, (void*)instance->draw_spec.offset, state.draw_count);
			}
		}

		PolygonalParticle::PolygonalParticle(GLushort max_instances, const std::vector<glm::vec2>& polygon, const math::Triangulation& triangulation)
			: Particle(max_instances, (GLushort)(triangulation.size() * 3)), position_vbo((GLushort)polygon.size()), transform_ssbo(max_instances), color_ssbo(max_instances)
		{
			OLY_ASSERT(polygon.size() >= 3);

			shader = shaders::polygonal_particle;
			draw_spec.count = ebo.vector().size();

			projection_location = glGetUniformLocation(shader, "uProjection");
			buf_offset_location = glGetUniformLocation(shader, "uBufOffset");

			glBindVertexArray(vao);
			memcpy_s(position_vbo.vector<0>().data(), position_vbo.vector<0>().size() * sizeof(glm::vec2), polygon.data(), polygon.size() * sizeof(glm::vec2));
			position_vbo.init_layout(rendering::VertexAttribute<>{ 0, 2 });
			auto& indices = ebo.vector();
			for (size_t i = 0; i < triangulation.size(); ++i)
			{
				indices[3 * i + 0] = triangulation[i][0];
				indices[3 * i + 1] = triangulation[i][1];
				indices[3 * i + 2] = triangulation[i][2];
			}
			ebo.init();
			glBindVertexArray(0);
		}

		void PolygonalParticle::set_instance_offset(GLuint offset) const
		{
			glUseProgram(shader);
			glUniform1ui(buf_offset_location, offset);
		}

		void PolygonalParticle::set_projection(const glm::vec4& projection_bounds) const
		{
			glm::mat3 proj = glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]);
			glUseProgram(shader);
			glUniformMatrix3fv(projection_location, 1, GL_FALSE, glm::value_ptr(proj));
		}

		void PolygonalParticle::set_transform(GLushort pos, const glm::mat3& transform)
		{
			transform_ssbo.vector()[pos] = transform;
			transform_ssbo.lazy_send(pos);
		}

		void PolygonalParticle::set_color(GLushort pos, const glm::vec4& color)
		{
			color_ssbo.vector()[pos] = color;
			color_ssbo.lazy_send(pos);
		}
		
		void PolygonalParticle::send_buffers() const
		{
			transform_ssbo.flush();
			color_ssbo.flush();
		}

		void PolygonalParticle::pre_draw() const
		{
			transform_ssbo.bind_base(0);
			color_ssbo.bind_base(1);
		}

		std::unique_ptr<PolygonalParticle> create_polygonal_particle(GLushort max_instances, const std::vector<glm::vec2>& polygon)
		{
			return std::make_unique<PolygonalParticle>(max_instances, polygon, math::ear_clipping(polygon));
		}
	}
}
