#pragma once

#include "external/TOML.h"

#include "graphics/shapes/Polygons.h"
#include "core/types/Variant.h"

namespace oly::reg
{
	namespace params
	{
		struct Polygon
		{
			Transform2D local;
			std::vector<glm::vec2> points;
			std::vector<glm::vec4> colors;
		};

		struct PolyComposite
		{
			Transform2D local;

			struct NGonMethod
			{
				std::vector<glm::vec2> points;
				std::vector<glm::vec4> colors;
			};

			struct BorderedNGonMethod
			{
				cmath::NGonBase ngon_base;
			};

			struct ConvexDecompositionMethod
			{
				std::vector<glm::vec2> points;
			};

			std::optional<Variant<NGonMethod, BorderedNGonMethod, ConvexDecompositionMethod>> method;
		};

		struct NGon
		{
			Transform2D local;
			cmath::NGonBase ngon_base;
			bool bordered = false;
		};
	}

	extern rendering::Polygon load_polygon(TOMLNode node);
	extern rendering::Polygon load_polygon(const params::Polygon& params);
	extern rendering::Polygon load_polygon(params::Polygon&& params);
	extern rendering::PolyComposite load_poly_composite(TOMLNode node);
	extern rendering::PolyComposite load_poly_composite(const params::PolyComposite& params);
	extern rendering::PolyComposite load_poly_composite(params::PolyComposite&& params);
	extern rendering::NGon load_ngon(TOMLNode node);
	extern rendering::NGon load_ngon(const params::NGon& params);
	extern rendering::NGon load_ngon(params::NGon&& params);
}
