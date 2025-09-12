#include "Ellipses.h"

#include "core/context/rendering/Rendering.h"
#include "graphics/shapes/Ellipses.h"
#include "registries/Loader.h"

namespace oly::context
{
	namespace internal
	{
		std::unique_ptr<rendering::EllipseBatch> ellipse_batch;
	}

	void internal::init_ellipses(const TOMLNode& node)
	{
		if (auto toml_ellipse_batch = node["ellipse_batch"])
		{
			int ellipses;
			reg::parse_int(toml_ellipse_batch, "ellipses", ellipses);

			rendering::EllipseBatch::Capacity capacity{ (rendering::EllipseBatch::Index)ellipses };
			internal::ellipse_batch = std::make_unique<rendering::EllipseBatch>(capacity);
		}
	}

	void internal::terminate_ellipses()
	{
		internal::ellipse_batch.reset();
	}

	rendering::EllipseBatch& ellipse_batch()
	{
		return *internal::ellipse_batch;
	}

	void render_ellipses()
	{
		internal::ellipse_batch->render();
		internal::set_batch_rendering_tracker(InternalBatch::ELLIPSE, false);
	}
}
