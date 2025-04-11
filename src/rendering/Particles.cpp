#include "Particles.h"

#include <glm/gtc/type_ptr.hpp>

#include <algorithm>

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

		namespace random1d
		{
			float Uniform::operator()() const
			{
				return offset * (2 * rng() - 1);
			}

			float LogisticBell::operator()() const
			{
				float r = rng();
				return glm::log(r / (1 - r)) / (4 * height);
			}

			float PowerSpike::operator()() const
			{
				float r = rng();
				if (power == 0)
					return r * (b - a) + a;
				else
				{
					float m = (power + 1.0f) / (b - a);
					float comp = -a * m / (power + 1.0f);
					if (r < comp)
						return a + pow((power + 1.0f) * pow(-a, power) * r / m, 1.0f / (power + 1.0f));
					else
						return b - pow((power + 1.0f) * pow(b, power) * (1.0f - r) / m, 1.0f / (power + 1.0f));
				}
			}

			float DualPowerSpike::operator()() const
			{
				float r = rng();
				if (alpha == 0 && beta == 0)
					return r * (b - a) + a;
				else
				{
					float m = 1.0f / (b / (beta + 1.0f) - a / (alpha + 1.0f));
					float comp = -a * m / (alpha + 1.0f);
					if (r < comp)
						return a + pow((alpha + 1.0f) * pow(-a, alpha) * r / m, 1.0f / (alpha + 1.0f));
					else
						return b - pow((beta + 1.0f) * pow(b, beta) * (1.0f - r) / m, 1.0f / (beta + 1.0f));
				}
			}
		}

		namespace random2d
		{
			glm::vec2 Uniform::operator()() const
			{
				return { offset.x * (2 * rng() - 1), offset.y * (2 * rng() - 1) };
			}

			glm::vec2 LogisticBellIndependent::operator()() const
			{
				float r1 = rng(), r2 = rng();
				return { glm::log(r1 / (1 - r1)) / (4 * height), glm::log(r2 / (1 - r2)) / (4 * height) };
			}

			glm::vec2 LogisticBellDependent::operator()() const
			{
				float r = rng();
				float radius = glm::log(r / (1 - r)) / (4 * height);
				float angle = 2 * glm::pi<float>() * rng();
				return math::coordinates::to_cartesian({ radius, angle });
			}
		}

		namespace random3d
		{
			glm::vec3 Uniform::operator()() const
			{
				return { offset.x * (2 * rng() - 1), offset.y * (2 * rng() - 1), offset.z * (2 * rng() - 1) };
			}

			glm::vec3 LogisticBellIndependent::operator()() const
			{
				float r1 = rng(), r2 = rng(), r3 = rng();
				return { glm::log(r1 / (1 - r1)) / (4 * height), glm::log(r2 / (1 - r2)) / (4 * height), glm::log(r3 / (1 - r3)) / (4 * height) };
			}

			glm::vec3 LogisticBellDependent::operator()() const
			{
				float r = rng();
				float radius = glm::log(r / (1 - r)) / (4 * height);
				float azimuthal = 2 * glm::pi<float>() * rng();
				float polar = glm::pi<float>() * rng();
				return math::coordinates::to_cartesian({ radius, azimuthal, polar });
			}
		}

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
				return w / ((t - a) / (alpha + 1) + (b - t) / (beta + 1));
			}
			
			float ContinuousPulse::operator()(float t, float period) const
			{
				float weighted_sum = 0.0f;
				for (const auto& pt : pts)
				{
					if (t > pt.a)
					{
						if (pt.alpha == 0 && pt.beta == 0)
						{
							if (t < pt.b)
								weighted_sum += pt.w * (t - pt.a) / (pt.b - pt.a);
							else
								weighted_sum += pt.w;
						}
						else
						{
							if (t < pt.t)
								weighted_sum += pt.m() * pow(t - pt.a, pt.alpha + 1) / ((pt.alpha + 1) * pow(pt.t - pt.a, pt.alpha));
							else if (t < pt.b)
								weighted_sum += pt.w - pt.m() * pow(pt.b - t, pt.beta + 1) / ((pt.beta + 1) * pow(pt.b - pt.t, pt.beta));
							else
								weighted_sum += pt.w;
						}
					}
				}
				return weighted_sum;
			}
			
			bool ContinuousPulse::valid(float period) const
			{
				float prev_point = -1.0f;
				for (const auto& pt : pts)
				{
					if (pt.t < 0.0f || pt.t <= prev_point || pt.t >= period)
						return false;
					prev_point = pt.t;
					if (pt.w == 0 || pt.alpha < 0 || pt.beta < 0 || pt.a > pt.t || pt.b < pt.t || pt.a == pt.b)
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

		bool ParticleData::update()
		{
			lifespan -= emitter->state.delta_time;
			return lifespan > 0.0f;
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
			data.lifespan = lifespan::eval(lifespan, fmod(emitter->state.playtime, period), period) + std::clamp(random1d::eval(lifespan_rng), -lifespan_rng_max_offset, lifespan_rng_max_offset);
			glm::vec2 pos = spawn_bounds.clamp(spawn_bounds.center() + random2d::eval(transform_rng));
			transform = Transform2D{ pos, 0, { 5, 5 }}.matrix();
			color = { fmod(emitter->state.playtime, period) / period, 1.0f, 0.0f, 1.0f };
		}

		bool EmitterParams::validate() const
		{
			if (period <= 0.0f)
				return false;

			if (spawn_rate.index() == spawn_rate::DISCRETE_PULSE)
			{
				if (!std::get<spawn_rate::DISCRETE_PULSE>(spawn_rate).valid(period))
					return false;
			}
			else if (spawn_rate.index() == spawn_rate::CONTINUOUS_PULSE)
			{
				if (!std::get<spawn_rate::CONTINUOUS_PULSE>(spawn_rate).valid(period))
					return false;
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
				if (particle_data.back[i].update())
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
