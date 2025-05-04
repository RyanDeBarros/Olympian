#include "DrawCommands.h"

#include "core/base/Context.h"
#include "registries/Loader.h"

namespace oly
{
	namespace rendering
	{
		DrawCommand::Draw::Draw(Renderable type, const std::string& name)
		{
			switch (type)
			{
			case Renderable::SPRITE:
				renderable = context::ref_sprite(name).lock();
				break;
			case Renderable::POLYGON:
				renderable = context::ref_polygonal(name).lock();
				break;
			case Renderable::ELLIPSE:
				renderable = context::ref_ellipse(name).lock();
				break;
			case Renderable::TILEMAP:
				renderable = context::ref_tilemap(name).lock();
				break;
			case Renderable::PARAGRAPH:
				renderable = context::ref_paragraph(name).lock();
				break;
			}
		}

		void DrawCommand::Draw::execute() const
		{
			std::visit([](auto&& renderable) { renderable->draw(); }, renderable);
		}

		void DrawCommand::Render::execute() const
		{
			switch (batch)
			{
			case Batch::SPRITE:
				context::render_sprites();
				break;
			case Batch::POLYGON:
				context::render_polygons();
				break;
			case Batch::ELLIPSE:
				context::render_ellipses();
				break;
			case Batch::TEXT:
				context::render_text();
				break;
			}
		}

		void DrawCommand::execute() const
		{
			std::visit([](auto&& cmd) { cmd.execute(); }, cmd);
		}

		void DrawCommandList::execute() const
		{
			for (const auto& cmd : commands)
				cmd.execute();
		}
	}
}
