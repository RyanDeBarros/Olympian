#pragma once

#include <functional>

#include "core/Core.h"
#include "math/Geometry.h"
#include "math/Transforms.h"
#include "math/Random.h"
#include "SpecializedBuffers.h"

namespace oly
{
	namespace particles
	{
		namespace spawn_rate
		{
			struct Constant
			{
				// R(t) = c
				float c = 3.0f;
				float operator()(float t, float period) const;
				float debt_increment(float t, float dt, float period) const;
				float period_sum(float period) const;
			};

			struct Linear
			{
				// R(0) = i, R(T) = f --> R(t) = (f - i) * t / T + i
				float i = 60.0f;
				float f = 0.0f;
				float operator()(float t, float period) const;
				float period_sum(float period) const;
			};

			struct Sine
			{
				// R(t) = a * sin(k * pi * (t - b) / T) + c
				float a = 1.0f;
				float k = 1.0f;
				float b = 0.0f;
				float c = 0.0f;
				float operator()(float t, float period) const;
			};

			struct DiscretePulse
			{
				struct Point
				{
					// summand(tau) = w * delta(tau - t)
					float t;
					unsigned int w;
				};

				std::vector<Point> pts;
				float operator()(float t, float period) const;
				float debt_increment(float t, float dt, float period) const;
				bool valid(float period) const;
			};

			struct ContinuousPulse
			{
				struct Point
				{
					// for tau in [t - a, t], summand(tau) = m() * ((tau - a) / a) ^ alpha
					// for tau in [t, t + b], summand(tau) = m() * ((b - tau) / b) ^ beta
					// where w is the area under the pulse
					float t;
					unsigned int w;
					float a = 1.0f, b = 1.0f;
					float alpha = 1.0f, beta = 1.0f;

					float m() const;
				};

				float global_multiplier = 1.0f;
				std::vector<Point> pts;
				float operator()(float t, float period) const;
				bool valid(float period) const;
			};

			struct Piecewise
			{
				struct SubFunction
				{
					std::variant<
						Constant,
						Linear,
						Sine,
						DiscretePulse,
						ContinuousPulse
					> fn;
					Interval<float> interval;
				};
				
				std::vector<SubFunction> subfunctions;
				float operator()(float t, float period) const;
				bool valid(float period) const;
				float period_sum(float period) const;
			};

			using Function = std::variant<
				Constant,
				Linear,
				Sine,
				DiscretePulse,
				ContinuousPulse,
				Piecewise
			>;

			constexpr float eval(const Function& func, float t, float period)
			{
				return std::visit([t, period](const auto& fn) { return fn(t, period); }, func);
			}
			template<typename T>
			concept has_period_sum = requires(T fn, float period) { { fn.period_sum(period) } -> std::convertible_to<float>; };
			inline float period_sum(const Function& func, float period)
			{
				return std::visit([period](const auto& fn) {
					if constexpr (has_period_sum<decltype(fn)>)
						return fn.period_sum(period);
					else
						return fn(period, period);
					}, func);
			}
			struct DebtCache
			{
				float prev_accumulation = 0.0f;
				float period_sum = 0.0f;
			};
			template<typename T>
			concept has_debt_increment = requires(T fn, float t, float dt, float period) { { fn.debt_increment(t, dt, period) } -> std::convertible_to<float>; };
			inline float debt_increment(const Function& func, float t, float dt, float period, DebtCache& cache)
			{
				return std::visit([t, dt, period, &cache](const auto& fn) {
					if constexpr (has_debt_increment<decltype(fn)>)
						return fn.debt_increment(t, dt, period);
					else
					{
						float accumulation = fn(fmod(t, period), period);
						float incr = accumulation - cache.prev_accumulation + cache.period_sum * (floorf(t / period) - floorf((t - dt) / period));
						cache.prev_accumulation = accumulation;
						return incr;
					}
					}, func);
			}
			template<typename T>
			concept has_valid = requires(T fn, float period) { { fn.valid(period) } -> std::convertible_to<bool>; };
			inline bool valid(const Function& func, float period)
			{
				return std::visit([period](const auto& fn) {
					if constexpr (has_valid<decltype(fn)>)
						return fn.valid(period);
					else
						return true;
					}, func);
			}
		}

		namespace lifespan
		{
			struct Constant
			{
				// R(t) = c
				float c = 0.3f;
				float operator()(float t, float period) const;
			};

			struct Linear
			{
				// R(0) = i, R(T) = f --> R(t) = (f - i) * t / T + i
				float i = 0.5f;
				float f = 0.0f;
				float operator()(float t, float period) const;
			};

			struct Sine
			{
				// R(t) = a * sin(k * pi * (t - b) / T) + c
				float a = 1.0f;
				float k = 1.0f;
				float b = 0.0f;
				float c = 0.0f;
				float operator()(float t, float period) const;
			};

			using Function = std::variant<
				Constant,
				Linear,
				Sine
			>;
			constexpr float eval(const Function& func, float t, float period)
			{
				return std::visit([t, period](const auto& fn) { return fn(t, period); }, func);
			}
		}

		class Emitter;
		struct EmitterParams;

		struct Particle
		{
		protected:
			friend Emitter;
			GLuint shader = 0;
			rendering::CPUSideEBO<GLuint, rendering::Mutability::IMMUTABLE> ebo;
			rendering::VertexArray vao;

			struct
			{
				GLuint projection = 0, transform = 0;
			} locations;

			struct
			{
				GLenum mode = GL_TRIANGLES;
				GLsizei count = 0;
				GLenum type = GL_UNSIGNED_INT;
				GLintptr offset = 0;
			} draw_spec;

			Particle(GLuint ebo_size);

		public:
			virtual ~Particle() = default;

			void set_projection(const glm::vec4& projection_bounds) const;
			void set_transform(const glm::mat3& transform) const;
		};

		struct ParticleData
		{
			float t = 0.0f;

			struct
			{
				float lifespan = 0.0f;
				float mass = 1.0f;
				float spawn_time = 0.0f;
				unsigned int index = -1;
				glm::vec2 velocity = {};
				glm::vec4 color = glm::vec4(1.0f);
			} initial;
		};

		namespace mass
		{
			struct Constant
			{
				float m = 1.0f;
				float t_factor = 0.0f;
				float operator()(float t, glm::vec2 size) const;
			};

			struct Proportional
			{
				float m = 1.0f;
				float t_factor = 0.0f;
				float operator()(float t, glm::vec2 size) const;
			};

			using Function = std::variant<Constant, Proportional>;
			constexpr float eval(const Function& func, float t, glm::vec2 size)
			{
				return std::visit([t, size](const auto& fn) { return fn(t, size); }, func);
			}
		}

		namespace acceleration
		{
			struct Constant
			{
				float a = 0.0f;
				float operator()(float v, float t, float dt, float mass) const;
			};

			struct Force
			{
				float f = 0.0f;
				float operator()(float v, float t, float dt, float mass) const;
			};

			// not recommended as it is sensitive to frame lengths
			struct SinePosition
			{
				// x(t) = a * sin(k * pi * (t - b)) + c
				float a = 1.0f;
				float k = 1.0f;
				float b = 0.0f;
				float operator()(float v, float t, float dt, float mass) const;
			};

			struct Custom
			{
				std::function<float(float v, float t, float dt, float mass)> func;
				float operator()(float v, float t, float dt, float mass) const;
			};

			using Function = std::variant<
				Constant,
				Force,
				SinePosition,
				Custom
			>;
			constexpr float eval(const Function& func, float v, float t, float dt, float mass)
			{
				return std::visit([v, t, dt, mass](const auto& fn) { return fn(v, t, dt, mass); }, func);
			}
			struct Function2D
			{
				Function x, y;

				glm::vec2 eval(const ParticleData& prt, float dt) const { return { acceleration::eval(x, prt.initial.velocity.x, prt.t, dt, prt.initial.mass),
					acceleration::eval(y, prt.initial.velocity.y, prt.t, dt, prt.initial.mass) }; }
			};
		}

		namespace color
		{
			struct Constant
			{
				glm::vec4 c = glm::vec4(1.0f);
				glm::vec4 operator()(const ParticleData& prt) const;
			};

			struct Interp
			{
				float t1 = 0.0f, t2 = 1.0f;
				glm::vec4 c1 = glm::vec4(1.0f);
				glm::vec4 c2 = glm::vec4(1.0f);
				float power = 1.0f;
				glm::vec4 operator()(const ParticleData& prt) const;
			};

			struct Piecewise
			{
				struct SubFunction
				{
					struct
					{
						float t;
						bool use_index = false;
						unsigned int i = 0;
					} interval_end;
					std::variant<
						Constant,
						Interp
					> fn;
				};
				std::vector<SubFunction> subfunctions;
				glm::vec4 last_color = glm::vec4(1.0f);

				glm::vec4 operator()(const ParticleData& prt) const;
			};

			using Function = std::variant<
				Constant,
				Interp,
				Piecewise
			>;
			constexpr glm::vec4 eval(const Function& func, const ParticleData& prt)
			{
				return std::visit([prt](const auto& fn) { return fn(prt); }, func);
			}
		}

		namespace gradient
		{
			struct Keep
			{
				glm::vec4 operator()(const ParticleData& prt, float dt) const;
			};

			struct Interp
			{
				glm::vec4 i = glm::vec4(1.0f), f = glm::vec4(1.0f);
				float power = 1.0f;
				glm::vec4 operator()(const ParticleData& prt, float dt) const;
			};

			struct MultiInterp
			{
				struct Step
				{
					float t_end;
					glm::vec4 col = glm::vec4(1.0f);
					float power = 1.0f;
				};

				glm::vec4 starting = glm::vec4(1.0f), ending = glm::vec4(1.0f);
				float power = 1.0f;
				std::vector<Step> steps;
				glm::vec4 operator()(const ParticleData& prt, float dt) const;
			};

			using Function = std::variant<
				Keep,
				Interp,
				MultiInterp
			>;
			constexpr glm::vec4 eval(const Function& func, const ParticleData& prt, float dt)
			{
				return std::visit([prt, dt](const auto& fn) { return fn(prt, dt); }, func);
			}
		}

		struct EmitterParams
		{
			bool one_shot = false; // LATER use one_shot
			float period = 3.0f;
			GLuint max_live_particles = 3000;
			spawn_rate::Function spawn_rate;
			lifespan::Function lifespan;
			random::bound::Function lifespan_offset_rng;
			struct
			{
				random::domain2d::Domain position;
				random::bound::Function rotation;
				random::bound::Function2D scale;
			} transform_rng;
			random::bound::Function2D velocity;
			mass::Function mass;
			acceleration::Function2D acceleration;
			color::Function color;
			gradient::Function gradient;

			GLuint spawn_count() const;
			void spawn(ParticleData& data, glm::mat3& transform, glm::vec4& color, unsigned int index);
			bool validate() const;

		private:
			friend class Emitter;
			const Emitter* emitter = nullptr;
		};

		class Emitter
		{
			friend struct ParticleData;
			friend struct EmitterParams;
			std::unique_ptr<Particle> instance;

			enum SSBO
			{
				TRANSFORM,
				COLOR
			};
			rendering::GLBufferBlock<2> ssbo_block;
			struct
			{
				std::vector<ParticleData> front, back;
			} particle_data;
			struct
			{
				std::vector<glm::mat3> front, back;
			} transform_buffer;
			struct
			{
				std::vector<glm::vec4> front, back;
			} color_buffer;

			EmitterParams params;
			mutable spawn_rate::DebtCache spawn_debt_cache;

			struct
			{
				GLuint live_instances = 0;
				bool playing = true;
				float playtime = 0.0f;
				float delta_time = 0.0f;
				unsigned int particle_index = 0;

				mutable float spawn_rate_debt = 0.0f;
			} state;
			GLuint buffer_live_instances = 0;

		public:
			std::unique_ptr<Transformer2D> transformer;

			Emitter(std::unique_ptr<Particle>&& instance, const EmitterParams& params, const glm::vec4& projection_bounds, GLuint initial_particle_capacity, std::unique_ptr<Transformer2D>&& transformer = std::make_unique<Transformer2D>());
			Emitter(Emitter&&) noexcept;
			Emitter& operator=(Emitter&&) = delete;

			void set_projection(const glm::vec4& projection_bounds) const;

			bool is_playing() const { return state.playing; }
			void restart();
			void resume();
			void pause();
			void update();
			void draw() const;
			void flush_transform() const;
		
		private:
			bool update_back_particle(size_t i);
			void spawn_on_front_buffers();
			void swap_buffers();
		};

		class ParticleSystem
		{
		public:
			struct Subemitter
			{
				Emitter emitter;
				bool enabled = true;

				Subemitter(Emitter&& emitter, bool enabled = true) : emitter(std::move(emitter)), enabled(enabled) {}
				Subemitter(Subemitter&&) noexcept = default;
			};

		private:
			std::vector<Subemitter> subemitters;
			std::unique_ptr<Transformer2D> transformer;

		public:
			ParticleSystem(std::vector<Subemitter>&& subemitters, std::unique_ptr<Transformer2D>&& transformer = std::make_unique<Transformer2D>());

			void update();
			void draw() const;

			size_t size() const { return subemitters.size(); }
			const Emitter& get_subemitter(size_t i) const { return subemitters[i].emitter; }
			Emitter& get_subemitter(size_t i) { return subemitters[i].emitter; }
			bool subemitter_enabled(size_t i) const { return subemitters[i].enabled; }
			bool& subemitter_enabled(size_t i) { return subemitters[i].enabled; }

			const Transformer2D& get_transformer() const { return *transformer; }
			Transformer2D& get_transformer() { return *transformer; }
		};

		class PolygonalParticle : public Particle
		{
			rendering::GLBuffer vbo;

		public:
			PolygonalParticle(const std::vector<glm::vec2>& polygon, const math::Triangulation& triangulation);
		};

		extern std::unique_ptr<PolygonalParticle> create_polygonal_particle(const std::vector<glm::vec2>& polygon);

		class EllipticParticle : public Particle
		{
			rendering::GLBufferBlock<2> vbo_block;

		public:
			EllipticParticle(float rx, float ry);
		};
	}
}
