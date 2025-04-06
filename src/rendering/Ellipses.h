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
			
		public:
			struct EllipseDimension
			{
				float width, height, border;
				float fill_exp = 1.0f, border_exp = 1.0f;
			};
			struct ColorGradient {
				glm::vec4 fill_inner;
				glm::vec4 fill_outer;
				glm::vec4 border_inner;
				glm::vec4 border_outer;
			};

		private:
			rendering::IndexedSSBO<EllipseDimension, GLushort> dimension_ssbo;
			rendering::IndexedSSBO<ColorGradient, GLushort> color_ssbo;
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
