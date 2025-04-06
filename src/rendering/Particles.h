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
			rendering::FixedLayoutEBO<GLushort> ebo;
			rendering::VertexArray vao;

			struct
			{
				GLenum mode = GL_TRIANGLES;
				GLsizei count = 0;
				GLenum type = GL_UNSIGNED_SHORT;
				GLintptr offset = 0;
			} draw_spec;
			GLushort max_instances;

			Particle(GLushort max_instances, GLushort ebo_size);
		
		public:
			virtual ~Particle() = default;
			
			virtual void set_instance_offset(GLuint offset) const {}
			virtual void set_projection(const glm::vec4& projection_bounds) const {}
			virtual void set_transform(GLushort pos, const glm::mat3& transform) {}
			virtual void set_color(GLushort pos, const glm::vec4& color) {}
			virtual void send_buffers() const {}
			virtual void pre_draw() const {}
		};

		struct EmitterParams
		{
		};

		class Emitter
		{
			std::unique_ptr<Particle> instance;

		public:
			EmitterParams params;

		private:
			struct
			{
				GLushort buf_offset = 0;
				GLushort draw_count = 0;
			} state;

		public:
			Emitter(std::unique_ptr<Particle>&& instance, const glm::vec4& projection_bounds);

			void set_projection(const glm::vec4& projection_bounds) const;

			void check_params();
			void update();
			void draw() const;
		};

		class PolygonalParticle : public Particle
		{
			GLuint projection_location, buf_offset_location;

			rendering::VertexBufferBlock<glm::vec2> position_vbo;
			rendering::IndexedSSBO<glm::mat3, GLushort> transform_ssbo;
			rendering::IndexedSSBO<glm::vec4, GLushort> color_ssbo;

		public:
			PolygonalParticle(GLushort max_instances, const std::vector<glm::vec2>& polygon, const math::Triangulation& triangulation);

		public:
			virtual void set_instance_offset(GLuint offset) const override;
			virtual void set_projection(const glm::vec4& projection_bounds) const override;
			virtual void set_transform(GLushort pos, const glm::mat3& transform) override;
			virtual void set_color(GLushort pos, const glm::vec4& color) override;
			virtual void send_buffers() const override;
			virtual void pre_draw() const override;
		};

		extern std::unique_ptr<PolygonalParticle> create_polygonal_particle(GLushort max_instances, const std::vector<glm::vec2>& polygon);
	}
}
