#include "Polygons.h"

#include "core/context/rendering/Rendering.h"
#include "graphics/primitives/Polygons.h"
#include "registries/Loader.h"

namespace oly::context
{
	namespace internal
	{
		std::unique_ptr<rendering::PolygonBatch> polygon_batch;
	}

	void internal::init_polygons(const TOMLNode& node)
	{
		if (auto toml_polygon_batch = node["polygon_batch"])
		{
			int primitives;
			reg::parse_int(toml_polygon_batch, "primitives", primitives);
			int degree = 6;
			reg::parse_int(toml_polygon_batch, "degree", degree);

			rendering::PolygonBatch::Capacity capacity{ (GLuint)primitives, (GLuint)degree };
			internal::polygon_batch = std::make_unique<rendering::PolygonBatch>(capacity);
		}
	}

	void internal::terminate_polygons()
	{
		internal::polygon_batch.reset();
	}

	rendering::PolygonBatch& polygon_batch()
	{
		return *internal::polygon_batch;
	}

	void render_polygons()
	{
		internal::polygon_batch->render();
		internal::set_batch_rendering_tracker(InternalBatch::POLYGON, false);
	}
}
