#include "Particles.h"

#include <glm/gtc/type_ptr.hpp>

#include <algorithm>

#include "util/Assert.h"
#include "math/Units.h"
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
					if (t >= subfunc.interval.left)
						total += eval(convert_variant<Function>(subfunc.fn), std::min(t, subfunc.interval.right), period) - eval(convert_variant<Function>(subfunc.fn), subfunc.interval.left, period);
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

			float Piecewise::period_sum(float period) const
			{
				float total = 0.0f;
				for (const auto& subfunc : subfunctions)
					total += spawn_rate::period_sum(convert_variant<Function>(subfunc.fn), period);
				return total;
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

		void Particle::set_projection(const glm::vec4& projection_bounds) const
		{
			glm::mat3 proj = glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]);
			glUseProgram(shader);
			glUniformMatrix3fv(locations.projection, 1, GL_FALSE, glm::value_ptr(proj));
		}

		void Particle::set_transform(const glm::mat3& transform) const
		{
			glUseProgram(shader);
			glUniformMatrix3fv(locations.transform, 1, GL_FALSE, glm::value_ptr(transform));
		}

		namespace mass
		{
			float Constant::operator()(float t, glm::vec2 size) const
			{
				return std::max(m + t_factor * t, 0.0f);
			}

			float Proportional::operator()(float t, glm::vec2 size) const
			{
				return std::max((m + t_factor * t) * size.x * size.y, 0.0f);
			}
		}

		namespace acceleration
		{
			float Constant::operator()(float v, float t, float dt, float mass) const
			{
				return v + a * t;
			}

			float Force::operator()(float v, float t, float dt, float mass) const
			{
				return mass != 0.0f ? v + f * units::FORCE * t / (mass * units::MASS) : v;
			}

			float SinePosition::operator()(float v, float t, float dt, float mass) const
			{
				return a * k * glm::pi<float>() * glm::cos(k * glm::pi<float>() * (t - b));
			}

			float Custom::operator()(float v, float t, float dt, float mass) const
			{
				return func(v, t, dt, mass);
			}
		}

		namespace color
		{
			glm::vec4 Constant::operator()(const ParticleData& prt) const
			{
				return c;
			}

			glm::vec4 Interp::operator()(const ParticleData& prt) const
			{
				return glm::mix(c1, c2, glm::pow((prt.initial.spawn_time - t1) / (t2 - t1), power));
			}

			glm::vec4 Piecewise::operator()(const ParticleData& prt) const
			{
				for (const auto& subfunc : subfunctions)
				{
					if (subfunc.interval_end.use_index)
					{
						if (prt.initial.index < subfunc.interval_end.i)
							return eval(convert_variant<Function>(subfunc.fn), prt);
					}
					else
					{
						if (prt.initial.spawn_time < subfunc.interval_end.t)
							return eval(convert_variant<Function>(subfunc.fn), prt);
					}
				}
				return last_color;
			}
		}

		namespace gradient
		{
			glm::vec4 Keep::operator()(const ParticleData& prt, float dt) const
			{
				return prt.initial.color;
			}

			glm::vec4 Interp::operator()(const ParticleData& prt, float dt) const
			{
				return glm::mix(i, f, glm::pow(prt.t / prt.initial.lifespan, power));
			}

			glm::vec4 MultiInterp::operator()(const ParticleData& prt, float dt) const
			{
				if (steps.empty())
					return glm::mix(starting, ending, glm::pow(prt.t / prt.initial.lifespan, power));
				for (size_t i = 0; i < steps.size(); ++i)
				{
					if (prt.t <= steps[i].t_end)
					{
						if (i == 0)
							return glm::mix(starting, steps[0].col, glm::pow(prt.t / steps[0].t_end, steps[0].power));
						else
							return glm::mix(steps[i - 1].col, steps[i].col, glm::pow((prt.t - steps[i - 1].t_end) / (steps[i].t_end - steps[i - 1].t_end), steps[i].power));
					}
				}
				return glm::mix(steps.back().col, ending, glm::pow((prt.t - steps.back().t_end) / (prt.initial.lifespan - steps.back().t_end), power));
			}
		}

		GLuint EmitterParams::spawn_count() const
		{
			float debt = emitter->state.spawn_rate_debt;
			float debt_increase = spawn_rate::debt_increment(spawn_rate, emitter->state.playtime, emitter->state.delta_time, period, emitter->spawn_debt_cache);
			if (debt_increase > 0.0f)
			{
				debt += debt_increase;
				GLuint count = (GLuint)floor(debt);
				emitter->state.spawn_rate_debt = debt - count;
				return count;
			}
			else return 0;
		}

		void EmitterParams::spawn(ParticleData& data, glm::mat3& transform, glm::vec4& col, unsigned int index)
		{
			data.initial.spawn_time = fmod(emitter->state.playtime, period);
			data.initial.index = index;
			data.initial.lifespan = lifespan::eval(lifespan, data.initial.spawn_time, period) + random::bound::eval(lifespan_offset_rng);
			data.initial.velocity = velocity.eval();
			data.initial.mass = mass::eval(mass, data.initial.spawn_time, extract_scale(transform));
			transform = Transform2D{ transform_rng.position(), random::bound::eval(transform_rng.rotation), transform_rng.scale.eval() }.matrix();
			col = color::eval(color, data);
			data.initial.color = col;
		}

		bool EmitterParams::validate() const
		{
			if (period <= 0.0f)
				return false;

			return spawn_rate::valid(spawn_rate, period);
		}

		Emitter::Emitter(std::unique_ptr<Particle>&& inst, const EmitterParams& prms, const glm::vec4& projection_bounds, GLuint initial_particle_capacity, std::unique_ptr<Transformer2D>&& transformer)
			: instance(std::move(inst)), buffer_live_instances(initial_particle_capacity), transformer(std::move(transformer))
		{
			if (!prms.validate())
				throw Error(ErrorCode::BAD_EMITTER_PARAMS);
			glNamedBufferData(ssbo_block[SSBO::TRANSFORM], initial_particle_capacity * sizeof(glm::mat3), nullptr, GL_STREAM_DRAW);
			glNamedBufferData(ssbo_block[SSBO::COLOR], initial_particle_capacity * sizeof(glm::vec4), nullptr, GL_STREAM_DRAW);
			set_projection(projection_bounds);
			params = prms;
			params.emitter = this;
			spawn_debt_cache.period_sum = spawn_rate::period_sum(params.spawn_rate, params.period);
			restart();
		}

		Emitter::Emitter(Emitter&& other) noexcept
			: instance(std::move(other.instance)), ssbo_block(std::move(other.ssbo_block)), particle_data(std::move(other.particle_data)), transform_buffer(std::move(other.transform_buffer)),
			color_buffer(std::move(other.color_buffer)), params(std::move(other.params)), spawn_debt_cache(std::move(other.spawn_debt_cache)), state(std::move(other.state)),
			buffer_live_instances(std::move(other.buffer_live_instances)), transformer(std::move(other.transformer))
		{
			params.emitter = this;
		}

		void Emitter::set_projection(const glm::vec4& projection_bounds) const
		{
			instance->set_projection(projection_bounds);
		}

		void Emitter::restart()
		{
			state.playing = true;
			state.delta_time = TIME.delta<float>();
			state.playtime = state.delta_time;
			state.spawn_rate_debt = 0.0f;
			spawn_debt_cache.prev_accumulation = spawn_rate::eval(params.spawn_rate, fmod(state.playtime - state.delta_time, params.period), params.period);

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
				if (update_back_particle(i))
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

		void Emitter::flush_transform() const
		{
			transformer->pre_get();
			instance->set_transform(transformer->global());
			transformer->flush();
		}

		bool Emitter::update_back_particle(size_t i)
		{
			auto& prt = particle_data.back[i];
			
			prt.t += state.delta_time;
			if (prt.t >= prt.initial.lifespan)
				return false;

			glm::vec2 vel = params.acceleration.eval(prt, state.delta_time);
			transform_buffer.back[i][2][0] += vel.x * state.delta_time;
			transform_buffer.back[i][2][1] += vel.y * state.delta_time;
			color_buffer.back[i] = gradient::eval(params.gradient, prt, state.delta_time);
			return true;
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
				float period_sum = spawn_rate::period_sum(params.spawn_rate, params.period);
				GLuint period_particles = (GLuint)floorf(period_sum) + (GLuint)floorf(floorf(state.playtime / params.period) * (period_sum - floorf(period_sum)));
				for (GLuint i = 0; i < spawn_count; ++i)
				{
					size_t j = state.live_instances++;
					params.spawn(particle_data.front[j], transform_buffer.front[j], color_buffer.front[j], state.particle_index);
					++state.particle_index;
					if (state.particle_index == period_particles)
						state.particle_index = 0;
				}
			}
		}

		void Emitter::swap_buffers()
		{
			std::swap(particle_data.front, particle_data.back);
			std::swap(transform_buffer.front, transform_buffer.back);
			std::swap(color_buffer.front, color_buffer.back);
		}

		ParticleSystem::ParticleSystem(std::vector<Subemitter>&& subemitters_, std::unique_ptr<Transformer2D>&& transformer_)
			: subemitters(std::move(subemitters_)), transformer(std::move(transformer_))
		{
			for (auto& subemitter : subemitters)
				subemitter.emitter.transformer->attach_parent(transformer.get());
		}

		void ParticleSystem::update()
		{
			transformer->pre_get();
			for (auto& subemitter : subemitters)
				if (subemitter.enabled)
				{
					subemitter.emitter.update();
					subemitter.emitter.flush_transform();
				}
			transformer->flush();
		}

		void ParticleSystem::draw() const
		{
			for (const auto& subemitter : subemitters)
				if (subemitter.enabled)
					subemitter.emitter.draw();
		}

		PolygonalParticle::PolygonalParticle(const std::vector<glm::vec2>& polygon, const math::Triangulation& triangulation)
			: Particle((GLuint)(triangulation.size() * 3)), vbo((GLuint)polygon.size())
		{
			OLY_ASSERT(polygon.size() >= 3);

			shader = shaders::polygonal_particle;
			draw_spec.count = (GLsizei)ebo.vector().size();

			locations.projection = glGetUniformLocation(shader, "uProjection");
			locations.projection = glGetUniformLocation(shader, "uTransform");

			glBindVertexArray(vao);
			memcpy_s(vbo.vector<0>().data(), vbo.vector<0>().size() * sizeof(glm::vec2), polygon.data(), polygon.size() * sizeof(glm::vec2));
			vbo.init_layout(rendering::VertexAttribute<>{ 0, 2 });
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

		std::unique_ptr<PolygonalParticle> create_polygonal_particle(const std::vector<glm::vec2>& polygon)
		{
			return std::make_unique<PolygonalParticle>(polygon, math::ear_clipping(polygon));
		}

		EllipticParticle::EllipticParticle(float rx, float ry)
			: Particle(6), vbo((GLuint)4)
		{
			shader = shaders::elliptic_particle;
			draw_spec.count = (GLsizei)ebo.vector().size();

			locations.projection = glGetUniformLocation(shader, "uProjection");
			locations.projection = glGetUniformLocation(shader, "uTransform");

			glBindVertexArray(vao);
			glm::vec2 vertices[4] = {
				{ -rx, -ry },
				{  rx, -ry },
				{  rx,  ry },
				{ -rx,  ry }
			};
			memcpy_s(vbo.vector<0>().data(), vbo.vector<0>().size() * sizeof(glm::vec2), vertices, 4 * sizeof(glm::vec2));
			glm::vec2 radii[4] = {
				{ rx, ry },
				{ rx, ry },
				{ rx, ry },
				{ rx, ry }
			};
			memcpy_s(vbo.vector<1>().data(), vbo.vector<1>().size() * sizeof(glm::vec2), radii, 4 * sizeof(glm::vec2));
			vbo.init_layout(rendering::VertexAttribute<>{ 0, 2 }, rendering::VertexAttribute<>{ 1, 2 });
			auto& indices = ebo.vector();
			indices[0] = 0;
			indices[1] = 1;
			indices[2] = 2;
			indices[3] = 2;
			indices[4] = 3;
			indices[5] = 0;
			ebo.init();
			glBindVertexArray(0);
		}
}
}
