#pragma once

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

		struct DiscretePulsePoint
		{
			float t;
			unsigned int w;
		};

		struct EmitterParams
		{
			bool one_shot = false;
			float period = 3.0f;
			GLuint max_live_particles = 3000;
			struct
			{
				enum class Mode
				{
					CONSTANT,
					LINEAR,
					SINE,
					PULSE,
				} mode = Mode::CONSTANT;
				struct
				{
					// R(t) = c
					float c = 1.0f;
				} constant;
				struct
				{
					// R(0) = i, R(T) = f --> R(t) = (f - i) * t / T + i
					float i = 10.0f;
					float f = 0.0f;
				} linear;
				struct
				{
					// R(t) = a * sin(k * pi * (t - b) / T) + c
					float a = 1.0f;
					int k = 1;
					float b = 0.0f;
					float c = 0.0f;
				} sine;
				struct
				{
					// R(t) = sum over pt in pts of pt.w * delta(t - pt.t)
					std::vector<DiscretePulsePoint> pts;
				} pulse;
			} spawn_rate;

			GLuint spawn_count() const;
			void spawn(ParticleData& data, glm::mat3& transform, glm::vec4& color);
			bool validate() const;

		private:
			mutable GLuint spawn_rate_pulse_phi = 0;

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

		public:
			Emitter(std::unique_ptr<Particle>&& instance, const glm::vec4& projection_bounds, GLuint initial_particle_capacity);

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
