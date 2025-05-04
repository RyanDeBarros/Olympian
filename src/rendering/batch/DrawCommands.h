#pragma once

#include "Sprites.h"
#include "Polygons.h"
#include "Ellipses.h"
#include "TileMap.h"
#include "text/Paragraph.h"

namespace oly
{
	namespace rendering
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

		class DrawCommandRegistry
		{
			std::unordered_map<std::string, DrawCommandList> command_lists;

		public:
			void load(const char* draw_command_registry_file);
			void load(const std::string& draw_command_registry_file) { load(draw_command_registry_file.c_str()); }
			void clear();
			void execute(const std::string& name) const;
		};
	}
}
