#pragma once

#include "external/TOML.h"

#include "graphics/shapes/Polygons.h"

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

			enum MethodIndex
			{
				NGON,
				BORDERED_NGON,
				CONVEX_DECOMPOSITION
			};
			std::optional<std::variant<NGonMethod, BorderedNGonMethod, ConvexDecompositionMethod>> method;
		};

		struct NGon
		{
			Transform2D local;
			cmath::NGonBase ngon_base;
			bool bordered = false;
		};
	}

	extern rendering::Polygon load_polygon(rendering::PolygonBatch* batch, const TOMLNode& node);
	extern rendering::Polygon load_polygon(rendering::PolygonBatch* batch, const params::Polygon& params);
	extern rendering::Polygon load_polygon(rendering::PolygonBatch* batch, params::Polygon&& params);
	extern rendering::PolyComposite load_poly_composite(rendering::PolygonBatch* batch, const TOMLNode& node);
	extern rendering::PolyComposite load_poly_composite(rendering::PolygonBatch* batch, const params::PolyComposite& params);
	extern rendering::PolyComposite load_poly_composite(rendering::PolygonBatch* batch, params::PolyComposite&& params);
	extern rendering::NGon load_ngon(rendering::PolygonBatch* batch, const TOMLNode& node);
	extern rendering::NGon load_ngon(rendering::PolygonBatch* batch, const params::NGon& params);
	extern rendering::NGon load_ngon(rendering::PolygonBatch* batch, params::NGon&& params);
}
