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

		struct EmitterParams
		{

		};

		class Emitter
		{
			std::unique_ptr<Particle> instance;

			enum SSBO
			{
				TRANSFORM,
				COLOR
			};
			rendering::GLBufferBlock<2> ssbo_block;

			struct ParticleData
			{
				float lifespan = 0.0f;

				bool update(const Emitter& emitter);
			};
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
			bool use_front_buffer = true;
			std::vector<glm::mat3>& current_transform_buffer() { return use_front_buffer ? transform_buffer.front : transform_buffer.back; }
			std::vector<glm::mat3>& other_transform_buffer() { return use_front_buffer ? transform_buffer.back : transform_buffer.front; }
			std::vector<glm::vec4>& current_color_buffer() { return use_front_buffer ? color_buffer.front : color_buffer.back; }
			std::vector<glm::vec4>& other_color_buffer() { return use_front_buffer ? color_buffer.back : color_buffer.front; }
			std::vector<ParticleData>& current_particle_data() { return use_front_buffer ? particle_data.front : particle_data.back; }
			std::vector<ParticleData>& other_particle_data() { return use_front_buffer ? particle_data.back : particle_data.front; }

		public:
			EmitterParams params;

		private:
			struct
			{
				GLuint live_instances = 0;
				bool playing = true;
				float playtime = 0.0f;
				float delta_time = 0.0f;
			} state;

		public:
			Emitter(std::unique_ptr<Particle>&& instance, const glm::vec4& projection_bounds, GLuint initial_particle_capacity);

			void set_projection(const glm::vec4& projection_bounds) const;

			void resume();
			void pause();
			void update();
			void draw() const;
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
