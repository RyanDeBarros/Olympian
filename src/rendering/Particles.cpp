#include "Particles.h"

#include <glm/gtc/type_ptr.hpp>

#include <algorithm>

#include "util/Assert.h"
#include "Resources.h"

namespace oly
{
	namespace particles
	{
		namespace spawn_rate
		{
			float Constant::operator()(float t, float period) const
			{
				return c * t;
			}

			float Constant::debt_increment(float t, float dt, float period) const
			{
				return c * dt;
			}

			float Constant::period_sum(float period) const
			{
				return c * period;
			}

			float Linear::operator()(float t, float period) const
			{
				return 0.5f * (f - i) * t * t / period + i * t;
			}

			float Linear::period_sum(float period) const
			{
				return 0.5f * (f - i) * period + i * period;
			}

			float Sine::operator()(float t, float period) const
			{
				float mult = k * glm::pi<float>() / period;
				return c * t - (a / mult) * glm::cos(mult * (t - b));
			}

			float DiscretePulse::operator()(float t, float period) const
			{
				float weighted_sum = 0.0f;
				for (const auto& pt : pts)
				{
					if (pt.t <= t)
						weighted_sum += pt.w;
					else
						break;
				}
				return weighted_sum;
			}

			float DiscretePulse::debt_increment(float t, float dt, float period) const
			{
				float debt = 0.0f;
				float tmod = fmod(t, period);
				float dtmod = fmod(t - dt, period);
				float floor_term = floorf(t / period) - floorf((t - dt) / period);
				for (const auto& pt : pts)
					debt += pt.w * ((int)(pt.t <= tmod) - (int)(pt.t <= dtmod) + floor_term);
				return debt;
			}

			bool DiscretePulse::valid(float period) const
			{
				float prev_point = -1.0f;
				for (const auto& pt : pts)
				{
					if (pt.t < 0.0f || pt.t <= prev_point || pt.t >= period)
						return false;
					prev_point = pt.t;
					if (pt.w == 0)
						return false;
				}
				return true;
			}

			float ContinuousPulse::Point::m() const
			{
				return w / (a / (alpha + 1) + b / (beta + 1));
			}

			float ContinuousPulse::operator()(float t, float period) const
			{
				float weighted_sum = 0.0f;
				for (const auto& pt : pts)
				{
					if (t > pt.t - pt.a)
					{
						if (pt.alpha == 0 && pt.beta == 0)
						{
							if (t < pt.t + pt.b)
								weighted_sum += pt.w * (t - pt.t + pt.a) / (pt.b + pt.a);
							else
								weighted_sum += pt.w;
						}
						else
						{
							if (t < pt.t)
								weighted_sum += pt.m() * pow(t - pt.t + pt.a, pt.alpha + 1) / ((pt.alpha + 1) * pow(pt.a, pt.alpha));
							else if (t < pt.t + pt.b)
								weighted_sum += pt.w - pt.m() * pow(pt.t + pt.b - t, pt.beta + 1) / ((pt.beta + 1) * pow(pt.b, pt.beta));
							else
								weighted_sum += pt.w;
						}
					}
				}
				return global_multiplier * weighted_sum;
			}

			bool ContinuousPulse::valid(float period) const
			{
				float prev_point = -1.0f;
				for (const auto& pt : pts)
				{
					if (pt.t < 0.0f || pt.t <= prev_point || pt.t >= period)
						return false;
					prev_point = pt.t;
					if (pt.w == 0 || pt.alpha < 0 || pt.beta < 0 || pt.a < 0 || pt.b < 0 || (pt.a == 0 && pt.b == 0))
						return false;
				}
				return true;
			}

			float Piecewise::operator()(float t, float period) const
			{
				float total = 0.0f;
				for (const auto& subfunc : subfunctions)
				{
					if (subfunc.interval.contains(t))
						total += eval(convert_variant<Function>(subfunc.fn), t, period);
				}
				return total;
			}

			bool Piecewise::valid(float period) const
			{
				for (const auto& subfunc : subfunctions)
				{
					if (!spawn_rate::valid(convert_variant<Function>(subfunc.fn), period))
						return false;
				}
				return true;
			}
}

		namespace lifespan
		{
			float Constant::operator()(float t, float period) const
			{
				return c;
			}

			float Linear::operator()(float t, float period) const
			{
				return (f - i) * t / period + i;
			}

			float Sine::operator()(float t, float period) const
			{
				return a * glm::sin(k * glm::pi<float>() * (t - b) / period) + c;
			}
		}

		Particle::Particle(GLuint ebo_size)
			: ebo(ebo_size, rendering::BufferSendConfig(rendering::BufferSendType::SUBDATA, false))
		{
		}

		void ParticleData::update(glm::mat3& transform, glm::vec4& color)
		{
			lifespan -= emitter->state.delta_time;
			if (lifespan <= 0.0f)
				alive = false;

			transform[2][0] += velocity.x * emitter->state.delta_time;
			transform[2][1] += velocity.y * emitter->state.delta_time;
		}

		GLuint EmitterParams::spawn_count() const
		{
			float debt = emitter->state.spawn_rate_debt;
			float debt_increase = spawn_rate::debt_increment(spawn_rate, emitter->state.playtime, emitter->state.delta_time, period);
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
			data.emitter = emitter;
			data.lifespan = lifespan::eval(lifespan, fmod(emitter->state.playtime, period), period) + random::bound1d::eval(lifespan_offset_rng);
			data.velocity = { random::bound1d::eval(velocity_x), random::bound1d::eval(velocity_y) };
			transform = Transform2D{ transform_rng.position(), random::bound1d::eval(transform_rng.rotation), { random::bound1d::eval(transform_rng.scale_x), random::bound1d::eval(transform_rng.scale_y) } }.matrix();
			color = { fmod(emitter->state.playtime, period) / period, 1.0f, 0.0f, 1.0f };
			LOG << "(" << transform[2][0] << ", " << transform[2][1] << ")" << LOG.nl;
		}

		bool EmitterParams::validate() const
		{
			if (period <= 0.0f)
				return false;

			return spawn_rate::valid(spawn_rate, period);
		}

		void Emitter::set_params(const EmitterParams& params)
		{
			if (params.validate())
			{
				this->params = params;
				this->params.emitter = this;
			}
		}

		Emitter::Emitter(std::unique_ptr<Particle>&& inst, const EmitterParams& params, const glm::vec4& projection_bounds, GLuint initial_particle_capacity)
			: instance(std::move(inst)), buffer_live_instances(initial_particle_capacity)
		{
			glNamedBufferData(ssbo_block[SSBO::TRANSFORM], initial_particle_capacity * sizeof(glm::mat3), nullptr, GL_STREAM_DRAW);
			glNamedBufferData(ssbo_block[SSBO::COLOR], initial_particle_capacity * sizeof(glm::vec4), nullptr, GL_STREAM_DRAW);
			set_projection(projection_bounds);
			set_params(params);
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
				particle_data.back[i].update(transform_buffer.back[i], color_buffer.back[i]);
				if (particle_data.back[i].alive)
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
				if (state.live_instances > buffer_live_instances)
				{
					glNamedBufferData(ssbo_block[SSBO::TRANSFORM], state.live_instances * sizeof(glm::mat3), transform_buffer.front.data(), GL_STREAM_DRAW);
					glNamedBufferData(ssbo_block[SSBO::COLOR], state.live_instances * sizeof(glm::vec4), color_buffer.front.data(), GL_STREAM_DRAW);
					buffer_live_instances = state.live_instances;
				}
				else
				{
				oly::check_errors();
					glNamedBufferSubData(ssbo_block[SSBO::TRANSFORM], 0, state.live_instances * sizeof(glm::mat3), transform_buffer.front.data());
				oly::check_errors();
					glNamedBufferSubData(ssbo_block[SSBO::COLOR], 0, state.live_instances * sizeof(glm::vec4), color_buffer.front.data());
				oly::check_errors();
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
