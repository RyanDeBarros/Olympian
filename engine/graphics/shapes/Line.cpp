#include "Line.h"

#include "core/types/Approximate.h"

namespace oly::rendering
{
	LineExtension::LineExtension(PolygonBatch* batch)
		: poly(batch)
	{
		poly.set_colors().reserve(4);
		poly.set_points().reserve(4);
		set_default_polygon();
	}

	void LineExtension::draw() const
	{
		if (dirty)
		{
			dirty = false;

			set_default_polygon();

			if (!approx(start, end))
			{
				glm::mat2 rotation{
					glm::normalize(end - start),
					glm::normalize(glm::vec2{ -(end - start).y, (end - start).x })
				};

				for (size_t i = 0; i < 4; ++i)
					poly.set_points()[i] = rotation * poly.get_points()[i];
			}

			poly.set_points()[0] += start;
			poly.set_points()[1] += end;
			poly.set_points()[2] += end;
			poly.set_points()[3] += start;
		}
		poly.draw();
	}

	void LineExtension::set_default_polygon() const
	{
		poly.set_colors()[0] = start_color;
		poly.set_colors()[1] = end_color;
		poly.set_colors()[2] = end_color;
		poly.set_colors()[3] = start_color;

		poly.set_points()[0] = 0.5f * glm::vec2{ 0.0f, -width };
		poly.set_points()[1] = 0.5f * glm::vec2{ 0.0f, -width };
		poly.set_points()[2] = 0.5f * glm::vec2{ 0.0f,  width };
		poly.set_points()[3] = 0.5f * glm::vec2{ 0.0f,  width };
	}
}
