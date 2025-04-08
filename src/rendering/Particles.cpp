#include "Particles.h"

#include <glm/gtc/type_ptr.hpp>

#include "util/Assert.h"
#include "Resources.h"

namespace oly
{
	namespace particles
	{
		Particle::Particle(GLuint ebo_size)
			: ebo(ebo_size, rendering::BufferSendConfig(rendering::BufferSendType::SUBDATA, false))
		{
		}

		Emitter::Emitter(std::unique_ptr<Particle>&& inst, const glm::vec4& projection_bounds, GLuint initial_particle_capacity)
			: instance(std::move(inst))
		{
			glNamedBufferData(ssbo_block[SSBO::TRANSFORM], initial_particle_capacity * sizeof(glm::mat3), nullptr, GL_STREAM_DRAW);
			glNamedBufferData(ssbo_block[SSBO::COLOR], initial_particle_capacity * sizeof(glm::vec4), nullptr, GL_STREAM_DRAW);
			set_projection(projection_bounds);

			auto& particles = current_particle_data();
			auto& transforms = current_transform_buffer();
			auto& colors = current_color_buffer();
			
			particles.push_back({ 3 });
			particles.push_back({ 6 });
			particles.push_back({ 9 });
			particles.push_back({ 12 });
			transforms.push_back(Transform2D{ {   0,  0 }, 0, { 15, 15 } }.matrix());
			transforms.push_back(Transform2D{ {  60, 20 }, 0, { 20, 20 } }.matrix());
			transforms.push_back(Transform2D{ { 120, 40 }, 0, { 25, 25 } }.matrix());
			transforms.push_back(Transform2D{ { 180, 60 }, 0, { 30, 30 } }.matrix());
			colors.push_back({ 1.0f, 0.0f, 0.0f, 1.0f });
			colors.push_back({ 0.8f, 0.1f, 0.1f, 1.0f });
			colors.push_back({ 0.6f, 0.2f, 0.2f, 1.0f });
			colors.push_back({ 0.4f, 0.3f, 0.3f, 1.0f });
			state.live_instances = 4;
		}

		void Emitter::set_projection(const glm::vec4& projection_bounds) const
		{
			instance->set_projection(projection_bounds);
		}

		void Emitter::resume()
		{
			state.playing = true;
		}

		void Emitter::pause()
		{
			state.playing = false;
		}

		bool Emitter::ParticleData::update(const Emitter& emitter)
		{
			lifespan -= emitter.state.delta_time;
			return lifespan > 0.0f;
		}

		void Emitter::update()
		{
			if (!state.playing)
				return;
			state.delta_time = TIME.delta<float>();
			state.playtime += state.delta_time;

			auto& current_particles = current_particle_data();
			auto& other_particles = other_particle_data();
			other_particles.resize(current_particles.size());
			auto& current_transforms = current_transform_buffer();
			auto& other_transforms = other_transform_buffer();
			other_transforms.resize(current_transforms.size());
			auto& current_colors = current_color_buffer();
			auto& other_colors = other_color_buffer();
			other_colors.resize(current_colors.size());
			use_front_buffer = !use_front_buffer;
			auto prev_live_instances = state.live_instances;
			state.live_instances = 0;

			// copy and update live particles
			for (size_t i = 0; i < prev_live_instances; ++i)
			{
				if (current_particles[i].update(*this))
				{
					size_t j = state.live_instances++;
					other_particles[j] = std::move(current_particles[i]);
					other_transforms[j] = std::move(current_transforms[i]);
					other_colors[j] = std::move(current_colors[i]);
				}
			}
			// TODO put some kind of check of overflowing number of particles
			// TODO spawn new particles according to spawn rate
			if (state.live_instances > 0)
			{
				if (state.live_instances > prev_live_instances)
				{
					glNamedBufferData(ssbo_block[SSBO::TRANSFORM], state.live_instances * sizeof(glm::mat3), other_transforms.data(), GL_STREAM_DRAW);
					glNamedBufferData(ssbo_block[SSBO::COLOR], state.live_instances * sizeof(glm::vec4), other_colors.data(), GL_STREAM_DRAW);
				}
				else
				{
					glNamedBufferSubData(ssbo_block[SSBO::TRANSFORM], 0, state.live_instances * sizeof(glm::mat3), other_transforms.data());
					glNamedBufferSubData(ssbo_block[SSBO::COLOR], 0, state.live_instances * sizeof(glm::vec4), other_colors.data());
				}
			}
		}

		void Emitter::draw() const
		{
			glUseProgram(instance->shader);
			glBindVertexArray(instance->vao);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_block[SSBO::TRANSFORM]);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_block[SSBO::COLOR]);
			glDrawElementsInstanced(instance->draw_spec.mode, instance->draw_spec.count, instance->draw_spec.type, (void*)instance->draw_spec.offset, state.live_instances);
		}

		PolygonalParticle::PolygonalParticle(const std::vector<glm::vec2>& polygon, const math::Triangulation& triangulation)
			: Particle((GLuint)(triangulation.size() * 3)), position_vbo((GLuint)polygon.size())
		{
			OLY_ASSERT(polygon.size() >= 3);

			shader = shaders::polygonal_particle;
			draw_spec.count = ebo.vector().size();

			projection_location = glGetUniformLocation(shader, "uProjection");

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

		void PolygonalParticle::set_projection(const glm::vec4& projection_bounds) const
		{
			glm::mat3 proj = glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]);
			glUseProgram(shader);
			glUniformMatrix3fv(projection_location, 1, GL_FALSE, glm::value_ptr(proj));
		}

		std::unique_ptr<PolygonalParticle> create_polygonal_particle(const std::vector<glm::vec2>& polygon)
		{
			return std::make_unique<PolygonalParticle>(polygon, math::ear_clipping(polygon));
		}
}
}
