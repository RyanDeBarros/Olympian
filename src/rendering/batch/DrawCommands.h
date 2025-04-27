#pragma once

#include "Sprites.h"
#include "Polygons.h"
#include "Ellipses.h"

namespace oly
{
	class Context;
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
					ELLIPSE
				};

				std::variant<std::shared_ptr<Sprite>, std::shared_ptr<Polygonal>, std::shared_ptr<Ellipse>> renderable;
				Draw(const Context& context, Renderable renderable, const std::string& name);
				void execute() const;
			};

			struct Render
			{
				enum class Batch
				{
					SPRITE,
					POLYGON,
					ELLIPSE
				};

				std::variant<const SpriteBatch*, const PolygonBatch*, const EllipseBatch*> batch;
				Render(const Context& context, Batch batch);
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
			void load(const Context& context, const char* draw_command_registry_file);
			void load(const Context& context, const std::string& draw_command_registry_file) { load(context, draw_command_registry_file.c_str()); }
			void clear();
			void execute(const std::string& name) const;
		};
	}
}
