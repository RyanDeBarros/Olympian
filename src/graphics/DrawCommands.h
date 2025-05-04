#pragma once

#include "graphics/primitives/Sprites.h"
#include "graphics/primitives/Polygons.h"
#include "graphics/primitives/Ellipses.h"
#include "graphics/extensions/TileMap.h"
#include "graphics/text/Paragraph.h"

namespace oly::rendering
{
	struct DrawCommand
	{
		struct Draw
		{
			enum class Renderable
			{
				SPRITE,
				POLYGON,
				ELLIPSE,
				TILEMAP,
				PARAGRAPH,
			};

			std::variant<std::shared_ptr<Sprite>, std::shared_ptr<Polygonal>, std::shared_ptr<Ellipse>, std::shared_ptr<TileMap>, std::shared_ptr<Paragraph>> renderable;
			Draw(Renderable renderable, const std::string& name);
			void execute() const;
		};

		struct Render
		{
			enum class Batch
			{
				SPRITE,
				POLYGON,
				ELLIPSE,
				TEXT,
			} batch;

			void execute() const;
		};

		std::variant<Draw, Render> cmd;
		void execute() const;
	};

	struct DrawCommandList
	{
		std::vector<DrawCommand> commands;

		void execute() const;
	};
}
