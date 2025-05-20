#include "Arrow.h"

#include "core/base/Context.h"
#include "core/math/Triangulation.h" // TODO separate core/math and core/cmath
#include "core/types/Approximate.h"

namespace oly::rendering
{
	ArrowExtension::ArrowExtension()
		: poly(context::poly_composite())
	{
		poly.composite = cmath::convex_decompose_polygon(default_polygon());

		poly.init();
	}

	void ArrowExtension::draw() const
	{
		if (dirty)
		{
			dirty = false;

			cmath::Polygon2D polygon = default_polygon();

			if (!approx(start, end))
			{
				glm::mat2 rotation{
					glm::normalize(end - start),
					glm::normalize(glm::vec2{ -(end - start).y, (end - start).x })
				};

				for (size_t i = 0; i < polygon.points.size(); ++i)
					polygon.points[i] = rotation * polygon.points[i];
			}

			polygon.points.front() += start;
			polygon.points.back() += start;
			for (size_t i = 1; i < polygon.points.size() - 1; ++i)
				polygon.points[i] += end;

			poly.composite = cmath::convex_decompose_polygon(polygon);

			poly.send_polygon();
		}
		poly.draw();
	}

	cmath::Polygon2D ArrowExtension::default_polygon() const
	{
		cmath::Polygon2D polygon;

		float hw = 0.5f * width;
		if (head_cavity == 0.0f && head_width == width)
		{
			polygon.colors = {
				start_color,
				end_color,
				end_color,
				end_color,
				start_color
			};

			polygon.points = {
				glm::vec2{         0.0f,  -hw },
				glm::vec2{ -head_height,  -hw },
				glm::vec2{         0.0f, 0.0f },
				glm::vec2{ -head_height,   hw },
				glm::vec2{         0.0f,   hw }
			};
		}
		else
		{
			polygon.colors = {
				start_color,
				end_color,
				end_color,
				end_color,
				end_color,
				end_color,
				start_color
			};

			polygon.points = {
				glm::vec2{                       0.0f,                -hw },
				glm::vec2{               -head_height,                -hw },
				glm::vec2{ -head_height - head_cavity, -0.5f * head_width },
				glm::vec2{                       0.0f,               0.0f },
				glm::vec2{ -head_height - head_cavity,  0.5f * head_width },
				glm::vec2{               -head_height,                 hw },
				glm::vec2{                       0.0f,                 hw }
			};
		}

		return polygon;
	}
}
