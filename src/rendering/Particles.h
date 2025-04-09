#pragma once

#include <variant>

#include "core/Core.h"
#include "math/Geometry.h"
#include "math/Transforms.h"
#include "SpecializedBuffers.h"

namespace oly
{
	namespace particles
	{
		class Emitter;

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

			bool update(const Emitter& emitter);
		};

		namespace random1d
		{
			struct Uniform
			{
				float offset = 0.0f;
				float operator()(float v) const;
			};

			struct LogisticBell
			{
				float height = 1.0f;
				float cutoff = 0.0f;
				float operator()(float v) const;
			};

			enum
			{
				UNIFORM,
				LOGISTIC_BELL,
			};
			using Function = std::variant<Uniform, LogisticBell>;
			constexpr float eval(const Function& func, float t)
			{
				return std::visit([t](const auto& fn) { return fn(t); }, func);
			}
			constexpr glm::vec2 eval(const Function& func, glm::vec2 v)
			{
				return std::visit([v](const auto& fn) -> glm::vec2 { return { fn(v[0]), fn(v[1]) }; }, func);
			}
			constexpr glm::vec3 eval(const Function& func, glm::vec3 v)
			{
				return std::visit([v](const auto& fn) -> glm::vec3 { return { fn(v[0]), fn(v[1]), fn(v[2])}; }, func);
			}
			constexpr glm::vec4 eval(const Function& func, glm::vec4 v)
			{
				return std::visit([v](const auto& fn) -> glm::vec4 { return { fn(v[0]), fn(v[1]), fn(v[2]), fn(v[3])}; }, func);
			}
			constexpr glm::mat3 eval(const Function& func, const glm::mat3& m)
			{
				return { eval(func, m[0]), eval(func, m[1]), eval(func, m[2]) };
			}
		}

		// TODO piecewise constant
		// TODO piecewise linear
		// TODO continuous pulse

		namespace spawn_rate
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
			struct DiscretePulse
			{
				struct Point
				{
					float t;
					unsigned int w;
				};

				// R(t) = sum over pt in pts of pt.w * delta(t - pt.t)
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
			};
			using Function = std::variant<Constant, Linear, Sine, DiscretePulse>;
			constexpr float eval(const Function& func, float t, float period)
			{
				return std::visit([t, period](const auto& fn) { return fn(t, period); }, func);
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
			random1d::Function lifespan_rng;
			random1d::Function transform_rng; // TODO use bounding box or other shape to generate from, and use 2d random distributions for them instead

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
