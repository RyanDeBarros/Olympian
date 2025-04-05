#pragma once

#include "core/Core.h"
#include "SpecializedBuffers.h"

namespace oly
{
	namespace batch
	{
		class EllipseBatch
		{
			GLuint shader;

			rendering::VertexArray vao;
			rendering::QuadLayoutEBO<> ebo;
			rendering::IndexedSSBO<glm::vec2, GLushort> size_ssbo;
			rendering::IndexedSSBO<glm::vec4, GLushort> color_ssbo;
			rendering::IndexedSSBO<glm::mat3, GLushort> transform_ssbo;

			GLuint projection_location;

		public:
			struct Capacity
			{
				GLushort ellipses;

				Capacity(GLushort ellipses)
					: ellipses(ellipses)
				{
					OLY_ASSERT(4 * ellipses <= USHRT_MAX);
				}
			};

		private:
			Capacity capacity;

		public:
			EllipseBatch(Capacity capacity, const glm::vec4& projection_bounds);

			void draw(size_t draw_spec = 0);

			void set_projection(const glm::vec4& projection_bounds) const;

			typedef GLushort EllipsePos;
			std::vector<Range<EllipsePos>> draw_specs;

			void flush() const;
		};
	}
}
