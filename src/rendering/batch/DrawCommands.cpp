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

		void DrawCommandRegistry::load(const char* draw_command_registry_file)
		{
			auto toml = reg::load_toml(draw_command_registry_file);
			auto toml_draw_command_list = toml["draw_command"].as_array();
			if (!toml_draw_command_list)
				return;
			toml_draw_command_list->for_each([this](auto&& node) {
				if constexpr (toml::is_table<decltype(node)>)
				{
					auto _name = node["name"].value<std::string>();
					if (!_name)
						return;
					DrawCommandList draw_command_list;
					auto toml_commands = node["commands"].as_array();
					if (!toml_commands)
						return;
					toml_commands->for_each([&draw_command_list](auto&& node) {
						if constexpr (toml::is_table<decltype(node)>)
						{
							auto _type = node["type"].value<std::string>();
							if (!_type)
								return;
							const std::string& type = _type.value();
							if (type == "draw")
							{
								auto _name = node["name"].value<std::string>();
								auto _renderable = node["renderable"].value<std::string>();
								if (!_name || !_renderable)
									return;
								const std::string& name = _name.value();
								const std::string& toml_renderable = _renderable.value();
								DrawCommand::Draw::Renderable renderable;
								if (toml_renderable == "sprite")
									renderable = DrawCommand::Draw::Renderable::SPRITE;
								else if (toml_renderable == "polygon")
									renderable = DrawCommand::Draw::Renderable::POLYGON;
								else if (toml_renderable == "ellipse")
									renderable = DrawCommand::Draw::Renderable::ELLIPSE;
								else if (toml_renderable == "tilemap")
									renderable = DrawCommand::Draw::Renderable::TILEMAP;
								else if (toml_renderable == "paragraph")
									renderable = DrawCommand::Draw::Renderable::PARAGRAPH;
								else
									return;
								draw_command_list.commands.push_back({ DrawCommand::Draw(renderable, name) });
							}
							else if (type == "render")
							{
								auto _batch = node["batch"].value<std::string>();
								if (!_batch)
									return;
								const std::string& toml_batch = _batch.value();
								DrawCommand::Render::Batch batch;
								if (toml_batch == "sprite")
									batch = DrawCommand::Render::Batch::SPRITE;
								else if (toml_batch == "polygon")
									batch = DrawCommand::Render::Batch::POLYGON;
								else if (toml_batch == "ellipse")
									batch = DrawCommand::Render::Batch::ELLIPSE;
								else if (toml_batch == "text")
									batch = DrawCommand::Render::Batch::TEXT;
								else
									return;
								draw_command_list.commands.push_back({ DrawCommand::Render(batch) });
							}
						}
						});
					if (!draw_command_list.commands.empty())
						command_lists.emplace(_name.value(), std::move(draw_command_list));
					}
				});
		}

		void DrawCommandRegistry::clear()
		{
			command_lists.clear();
		}
		
		void DrawCommandRegistry::execute(const std::string& name) const
		{
			auto it = command_lists.find(name);
			if (it == command_lists.end())
				throw Error(ErrorCode::UNREGISTERED_DRAW_COMMAND);
			it->second.execute();
		}
	}
}
