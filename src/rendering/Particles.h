#pragma once

#include "core/Core.h"
#include "math/Geometry.h"
#include "math/Transforms.h"
#include "math/Random.h"
#include "SpecializedBuffers.h"

namespace oly
{
	namespace particles
	{
		class Emitter;
		struct EmitterParams;

		struct Particle
		{
		protected:
			friend Emitter;
			GLuint shader = 0;
			rendering::FixedLayoutEBO<GLuint> ebo;
			rendering::VertexArray vao;

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
			
			virtual void set_projection(const glm::vec4& projection_bounds) const {}
		};

		struct ParticleData
		{
			float lifespan = 0.0f;

			bool update();

		private:
			friend struct EmitterParams;
			const Emitter* emitter = nullptr;
		};

		// TODO vector of spawn rate functions, and float min, max where the spawn rate is zero outside [min, max].

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
				int k = 1;
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

			// TODO in continuous pulse, a and b should represent offsets, not absolutes. i.e., t - a --> a, b - t --> b
			struct ContinuousPulse
			{
				struct Point
				{
					// for tau in [a, t], summand(tau) = m() * ((tau - a) / (t - a)) ^ alpha
					// for tau in [t, b], summand(tau) = m() * ((tau - b) / (t - b)) ^ beta
					// where w is the area under the pulse
					float t;
					unsigned int w;
					float a, b;
					float alpha = 1.0f, beta = 1.0f;

					float m() const;
				};

				float global_multiplier = 1.0f;
				std::vector<Point> pts;
				float operator()(float t, float period) const;
				bool valid(float period) const;
			};

			enum
			{
				CONSTANT,
				LINEAR,
				SINE,
				DISCRETE_PULSE,
				CONTINUOUS_PULSE,
			};
			using Function = std::variant<Constant, Linear, Sine, DiscretePulse, ContinuousPulse>;
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
			template<typename T>
			concept has_debt_increment = requires(T fn, float t, float dt, float period) { { fn.debt_increment(t, dt, period) } -> std::convertible_to<float>; };
			inline float debt_increment(const Function& func, float t, float dt, float period)
			{
				return std::visit([t, dt, period](const auto& fn) {
					if constexpr (has_debt_increment<decltype(fn)>)
						return fn.debt_increment(t, dt, period);
					else
						return fn(fmod(t, period), period) - fn(fmod(t - dt, period), period) + period_sum(fn, period) * (floorf(t / period) - floorf((t - dt) / period));
					}, func);
			}
		}

		namespace lifespan
		{
			struct Constant
			{
				// R(t) = c
				float c = 3.0f;
				float operator()(float t, float period) const;
			};

			struct Linear
			{
				// R(0) = i, R(T) = f --> R(t) = (f - i) * t / T + i
				float i = 60.0f;
				float f = 0.0f;
				float operator()(float t, float period) const;
			};

			struct Sine
			{
				// R(t) = a * sin(k * pi * (t - b) / T) + c
				float a = 1.0f;
				int k = 1;
				float b = 0.0f;
				float c = 0.0f;
				float operator()(float t, float period) const;
			};

			enum
			{
				CONSTANT,
				LINEAR,
				SINE,
			};
			using Function = std::variant<Constant, Linear, Sine>;
			constexpr float eval(const Function& func, float t, float period)
			{
				return std::visit([t, period](const auto& fn) { return fn(t, period); }, func);
			}
		}

		struct EmitterParams
		{
			bool one_shot = false;
			float period = 3.0f;
			GLuint max_live_particles = 3000;
			spawn_rate::Function spawn_rate;
			lifespan::Function lifespan;
			random::domain1d::Domain lifespan_rng;
			random::domain2d::Domain position_rng;

			GLuint spawn_count() const;
			void spawn(ParticleData& data, glm::mat3& transform, glm::vec4& color);
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

		public:
			const EmitterParams& get_params() const { return params; }
			void set_params(const EmitterParams& params);

		private:
			struct
			{
				GLuint live_instances = 0;
				bool playing = true;
				float playtime = 0.0f;
				float delta_time = 0.0f;

				mutable float spawn_rate_debt = 0.0f;
			} state;
			GLuint buffer_live_instances = 0;

		public:
			Emitter(std::unique_ptr<Particle>&& instance, const EmitterParams& params, const glm::vec4& projection_bounds, GLuint initial_particle_capacity);

			void set_projection(const glm::vec4& projection_bounds) const;

			bool is_playing() const { return state.playing; }
			void restart();
			void resume();
			void pause();
			void update();
			void draw() const;
		
		private:
			void spawn_on_front_buffers();
			void swap_buffers();
		};

		class PolygonalParticle : public Particle
		{
			GLuint projection_location;

			rendering::VertexBufferBlock<glm::vec2> position_vbo;

		public:
			PolygonalParticle(const std::vector<glm::vec2>& polygon, const math::Triangulation& triangulation);

		public:
			virtual void set_projection(const glm::vec4& projection_bounds) const override;
		};

		extern std::unique_ptr<PolygonalParticle> create_polygonal_particle(const std::vector<glm::vec2>& polygon);
	}
}
