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

		bool ParticleData::update(const Emitter& emitter)
		{
			lifespan -= emitter.state.delta_time;
			return lifespan > 0.0f;
		}

		GLuint EmitterParams::spawn_count() const
		{
			float t = emitter->state.playtime;
			float dt = emitter->state.delta_time;
			float debt = emitter->state.spawn_rate_debt;
			float debt_increase = 0.0f;
			switch (spawn_rate.mode)
			{
			case decltype(spawn_rate.mode)::CONSTANT:
			{
				debt_increase = spawn_rate.constant.c * dt;
				break;
			}
			case decltype(spawn_rate.mode)::LINEAR:
			{
				float tmod = fmod(t, period);
				float dtmod = fmod(t - dt, period);
				float tfloor = floorf(t / period);
				float dtfloor = floorf((t - dt) / period);
				float integral = 0.5f * tmod * tmod - 0.5f * dtmod * dtmod + 0.5f * period * period * (tfloor - dtfloor);
				debt_increase = integral * (spawn_rate.linear.f - spawn_rate.linear.i) / period + spawn_rate.linear.i * dt;
				break;
			}
			case decltype(spawn_rate.mode)::SINE:
			{
				float tfloor = floorf(t / period);
				float dtfloor = floorf((t - dt) / period);
				float mult = spawn_rate.sine.k * glm::pi<float>() / period;
				float inv_mult = spawn_rate.sine.a / mult;
				float cos_t = glm::cos(mult * (fmod(t, period) - spawn_rate.sine.b));
				float cos_dt = glm::cos(mult * (fmod(t - dt, period) - spawn_rate.sine.b));
				float cos_r = glm::cos(mult * spawn_rate.sine.b);
				debt_increase = spawn_rate.sine.c * dt - inv_mult * cos_t + inv_mult * cos_dt + (tfloor - dtfloor) * 2 * inv_mult * unsigned_mod(spawn_rate.sine.k, 2) * cos_r;
				break;
			}
			case decltype(spawn_rate.mode)::PULSE:
			{
				float tmod = fmod(t, period);
				float dtmod = fmod(t - dt, period);
				float full_integral = (floorf(t / period) - floorf((t - dt) / period)) * spawn_rate_pulse_phi;
				int weighted_sum = 0;
				for (const auto& pt : spawn_rate.pulse.pts)
				{
					weighted_sum += pt.w * (
						(pt.t <= tmod ? 1 : 0)
						- (pt.t <= dtmod ? 1 : 0)
						);
				}
				debt_increase = weighted_sum + full_integral;
				break;
			}
			}
			if (debt_increase > 0.0f)
			{
				debt += debt_increase;
				GLuint count = (GLuint)floor(debt);
				emitter->state.spawn_rate_debt = debt - count;
				return count;
			}
			else return 0;
		}

		void EmitterParams::spawn(ParticleData& data, glm::mat3& transform, glm::vec4& color)
		{
			data.lifespan = 0.5f;
			transform = Transform2D{ { glm::mix(-100, 100, rng()), glm::mix(-100, 100, rng()) }, 0, { 5, 5 }}.matrix();
			color = { fmod(emitter->state.playtime, period) / period, 1.0f, 0.0f, 1.0f };
		}

		bool EmitterParams::validate() const
		{
			if (period <= 0.0f)
				return false;

			float prev_point = -1.0f;
			spawn_rate_pulse_phi = 0;
			for (const auto& pt : spawn_rate.pulse.pts)
			{
				if (pt.t <= prev_point || pt.t >= period)
					return false;
				prev_point = pt.t;
				if (pt.w == 0)
					return false;
				spawn_rate_pulse_phi += pt.w;
			}
			return true;
		}

		void Emitter::set_params(const EmitterParams& params)
		{
			if (params.validate())
			{
				this->params = params;
				this->params.emitter = this;
			}
		}

		Emitter::Emitter(std::unique_ptr<Particle>&& inst, const glm::vec4& projection_bounds, GLuint initial_particle_capacity)
			: instance(std::move(inst))
		{
			glNamedBufferData(ssbo_block[SSBO::TRANSFORM], initial_particle_capacity * sizeof(glm::mat3), nullptr, GL_STREAM_DRAW);
			glNamedBufferData(ssbo_block[SSBO::COLOR], initial_particle_capacity * sizeof(glm::vec4), nullptr, GL_STREAM_DRAW);
			set_projection(projection_bounds);

			EmitterParams test_params;
			test_params.period = 3.0f;
			test_params.spawn_rate.mode = decltype(test_params.spawn_rate.mode)::PULSE;
			test_params.spawn_rate.pulse.pts.push_back({ 0.0f, 10 });
			test_params.spawn_rate.pulse.pts.push_back({ 1.0f, 15 });
			test_params.spawn_rate.pulse.pts.push_back({ 2.0f, 5 });
			test_params.spawn_rate.pulse.pts.push_back({ 2.5f, 15 });
			set_params(test_params);

			restart();
		}

		void Emitter::set_projection(const glm::vec4& projection_bounds) const
		{
			instance->set_projection(projection_bounds);
		}

		void Emitter::restart()
		{
			state.playing = true;
			state.delta_time = TIME.delta<float>();
			state.playtime = 0.0f;
			state.spawn_rate_debt = 0.0f;

			state.live_instances = 0;
			particle_data.front.clear();
			particle_data.back.clear();
			transform_buffer.front.clear();
			transform_buffer.back.clear();
			color_buffer.front.clear();
			color_buffer.back.clear();

			spawn_on_front_buffers();
			swap_buffers();
		}

		void Emitter::resume()
		{
			state.playing = true;
		}

		void Emitter::pause()
		{
			state.playing = false;
		}

		void Emitter::update()
		{
			if (!state.playing)
				return;
			state.delta_time = TIME.delta<float>();
			state.playtime += state.delta_time;

			particle_data.front.resize(particle_data.back.size());
			transform_buffer.front.resize(transform_buffer.back.size());
			color_buffer.front.resize(color_buffer.back.size());
			auto prev_live_instances = state.live_instances;
			state.live_instances = 0;

			// copy and update live particles
			for (size_t i = 0; i < prev_live_instances; ++i)
			{
				if (particle_data.back[i].update(*this)) // TODO consider adding Emitter& data member to ParticleData
				{
					size_t j = state.live_instances++;
					particle_data.front[j] = std::move(particle_data.back[i]);
					transform_buffer.front[j] = std::move(transform_buffer.back[i]);
					color_buffer.front[j] = std::move(color_buffer.back[i]);
				}
			}
			spawn_on_front_buffers();

			// update buffers
			if (state.live_instances > 0)
			{
				if (state.live_instances > prev_live_instances)
				{
					glNamedBufferData(ssbo_block[SSBO::TRANSFORM], state.live_instances * sizeof(glm::mat3), transform_buffer.front.data(), GL_STREAM_DRAW);
					glNamedBufferData(ssbo_block[SSBO::COLOR], state.live_instances * sizeof(glm::vec4), color_buffer.front.data(), GL_STREAM_DRAW);
				}
				else
				{
					glNamedBufferSubData(ssbo_block[SSBO::TRANSFORM], 0, state.live_instances * sizeof(glm::mat3), transform_buffer.front.data());
					glNamedBufferSubData(ssbo_block[SSBO::COLOR], 0, state.live_instances * sizeof(glm::vec4), color_buffer.front.data());
				}
			}
			swap_buffers();
		}

		void Emitter::draw() const
		{
			glUseProgram(instance->shader);
			glBindVertexArray(instance->vao);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_block[SSBO::TRANSFORM]);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_block[SSBO::COLOR]);
			glDrawElementsInstanced(instance->draw_spec.mode, instance->draw_spec.count, instance->draw_spec.type, (void*)instance->draw_spec.offset, state.live_instances);
		}

		void Emitter::spawn_on_front_buffers()
		{
			GLuint spawn_count = params.spawn_count();
			if (spawn_count > 0)
			{
				if (state.live_instances + spawn_count > params.max_live_particles)
					spawn_count = params.max_live_particles - state.live_instances;
				particle_data.front.resize(state.live_instances + spawn_count);
				transform_buffer.front.resize(state.live_instances + spawn_count);
				color_buffer.front.resize(state.live_instances + spawn_count);
				for (GLuint i = 0; i < spawn_count; ++i)
				{
					size_t j = state.live_instances++;
					params.spawn(particle_data.front[j], transform_buffer.front[j], color_buffer.front[j]);
				}
			}
		}

		void Emitter::swap_buffers()
		{
			std::swap(particle_data.front, particle_data.back);
			std::swap(transform_buffer.front, transform_buffer.back);
			std::swap(color_buffer.front, color_buffer.back);
		}

		PolygonalParticle::PolygonalParticle(const std::vector<glm::vec2>& polygon, const math::Triangulation& triangulation)
			: Particle((GLuint)(triangulation.size() * 3)), position_vbo((GLuint)polygon.size())
		{
			OLY_ASSERT(polygon.size() >= 3);

			shader = shaders::polygonal_particle;
			draw_spec.count = (GLsizei)ebo.vector().size();

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
