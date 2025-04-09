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

		namespace random1d
		{
			float Uniform::operator()(float v) const
			{
				return offset != 0.0f ? v + offset * (2 * rng() - 1) : v;
			}

			float LogisticBell::operator()(float v) const
			{
				// f(x) = 4 * height * e ^ (-4 * height * x) / (1 + e ^ (-4 * height * x)) ^ 2
				// CDF = int _(-infty) ^x 4 * height * e ^ (-4 * height * x) / (1 + e ^ (-4 * height * x)) ^ 2 dx
				// CDF = 1 / (1 + e ^ (-4 * height * x))
				// CDF^-1 = ln(x / (1 - x)) / (4 * height)
				if (cutoff == 0.0f)
					return v;
				float r = rng();
				float offset = glm::log(r / (1 - r)) / (4 * height);
				return v + glm::clamp(offset, -cutoff, cutoff);
			}
		}

		namespace spawn_rate
		{
			float Constant::operator()(float t, float period) const
			{
				return c * t;
			}

			float Linear::operator()(float t, float period) const
			{
				return 0.5f * (f - i) * t * t / period + i * t;
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

			bool DiscretePulse::valid(float period) const
			{
				float prev_point = -1.0f;
				for (const auto& pt : pts)
				{
					if (pt.t <= prev_point || pt.t >= period)
						return false;
					prev_point = pt.t;
					if (pt.w == 0)
						return false;
				}
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
			float debt_increase = spawn_rate::eval(spawn_rate, fmod(t, period), period) - spawn_rate::eval(spawn_rate, fmod(t - dt, period), period)
				+ (floorf(t / period) - floorf((t - dt) / period)) * spawn_rate::eval(spawn_rate, period, period);
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
			data.lifespan = lifespan::eval(lifespan, fmod(emitter->state.playtime, period), period);
			data.lifespan = random1d::eval(lifespan_rng, data.lifespan);
			glm::vec2 pos = random1d::eval(transform_rng, glm::vec2{ 0.0f, 0.0f });
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
