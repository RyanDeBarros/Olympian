#include "Line.h"

#include "core/base/Context.h"
#include "core/types/Approximate.h"

namespace oly::rendering
{
	LineExtension::LineExtension()
		: poly(context::polygon())
	{
		poly.polygon.colors.reserve(4);
		poly.polygon.points.reserve(4);
		set_default_polygon();
		poly.init();
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
					poly.polygon.points[i] = rotation * poly.polygon.points[i];
			}

			poly.polygon.points[0] += start;
			poly.polygon.points[1] += end;
			poly.polygon.points[2] += end;
			poly.polygon.points[3] += start;

			poly.send_polygon();
		}
		poly.draw();
	}

	void LineExtension::set_default_polygon() const
	{
		poly.polygon.colors[0] = start_color;
		poly.polygon.colors[1] = end_color;
		poly.polygon.colors[2] = end_color;
		poly.polygon.colors[3] = start_color;

		poly.polygon.points[0] = 0.5f * glm::vec2{ 0.0f, -width };
		poly.polygon.points[1] = 0.5f * glm::vec2{ 0.0f, -width };
		poly.polygon.points[2] = 0.5f * glm::vec2{ 0.0f,  width };
		poly.polygon.points[3] = 0.5f * glm::vec2{ 0.0f,  width };
	}
}
