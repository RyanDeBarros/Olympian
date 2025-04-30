#include "Context.h"

#include <stb/stb_image.h>

#include <filesystem>

#include "rendering/Loader.h"
#include "util/IO.h"
#include "rendering/Resources.h"

namespace oly
{
	Context::Context(const char* filepath)
	{
		if (glfwInit() != GLFW_TRUE)
			throw oly::Error(oly::ErrorCode::GLFW_INIT);
		stbi_set_flip_vertically_on_load(true);
		LOG.set_logfile("../../../Olympian.log", true); // LATER use independent filepath
		LOG.flush();

		auto toml = assets::load_toml(filepath);
		auto toml_context = toml["context"];
		auto toml_window = toml_context["window"];
		if (!toml_window)
			throw Error(ErrorCode::CONTEXT_INIT);

		std::string root_dir = io::directory_of(filepath) + "/";

		{ // init window
			auto width = toml_window["width"].value<int64_t>();
			auto height = toml_window["height"].value<int64_t>();
			auto title = toml_window["title"].value<std::string>();
			if (!width || !height || !title)
				throw Error(ErrorCode::CONTEXT_INIT);

			rendering::WindowHint hint;
			if (auto toml_window_hint = toml_context["window_hint"])
			{
				assets::parse_vec4(toml_window_hint, "clear color", hint.context.clear_color);
				// TODO rest of window hint options
			}

			internal.window = std::make_unique<rendering::Window>((int)width.value(), (int)height.value(), title.value().c_str(), hint);
			load_resources();
			TIME.init();
		}

		{ // init texture registry
			auto register_files = toml_context["texture registries"].as_array();
			if (register_files)
			{
				for (const auto& node : *register_files)
					if (auto file = node.value<std::string>())
						internal.texture_registry.load(*this, root_dir + file.value());
			}
		}

		if (auto toml_sprite_batch = toml_context["sprite_batch"]) // init sprite batch
		{
			int initial_sprites = 0;
			assets::parse_int(toml_sprite_batch, "initial sprites", initial_sprites);
			int new_textures = 0;
			assets::parse_int(toml_sprite_batch, "new textures", new_textures);
			int new_uvs = 0;
			assets::parse_int(toml_sprite_batch, "new uvs", new_uvs);
			int new_modulations = 0;
			assets::parse_int(toml_sprite_batch, "new modulations", new_modulations);
			int num_anims = 0;
			assets::parse_int(toml_sprite_batch, "num anims", num_anims);

			rendering::SpriteBatch::Capacity capacity{ (GLuint)initial_sprites, (GLuint)new_textures, (GLuint)new_uvs, (GLuint)new_modulations, (GLuint)num_anims };
			internal.sprite_batch = std::make_unique<rendering::SpriteBatch>(capacity, internal.window->projection_bounds());
		}

		{ // init sprite registry
			auto register_files = toml_context["sprite registries"].as_array();
			if (register_files)
			{
				for (const auto& node : *register_files)
					if (auto file = node.value<std::string>())
						internal.sprite_registry.load(*this, root_dir + file.value());
			}
		}

		if (auto toml_polygon_batch = toml_context["polygon_batch"]) // init polygon batch
		{
			int primitives;
			assets::parse_int(toml_polygon_batch, "primitives", primitives);
			int degree = 6;
			assets::parse_int(toml_polygon_batch, "degree", degree);

			rendering::PolygonBatch::Capacity capacity{ (GLushort)primitives, (GLushort)degree };
			internal.polygon_batch = std::make_unique<rendering::PolygonBatch>(capacity, internal.window->projection_bounds());
		}

		{ // init polygon registry
			auto register_files = toml_context["polygon registries"].as_array();
			if (register_files)
			{
				for (const auto& node : *register_files)
					if (auto file = node.value<std::string>())
						internal.polygon_registry.load(*this, root_dir + file.value());
			}
		}

		if (auto toml_ellipse_batch = toml_context["ellipse_batch"]) // init ellipse batch
		{
			int ellipses;
			assets::parse_int(toml_ellipse_batch, "ellipses", ellipses);

			rendering::EllipseBatch::Capacity capacity{ (GLushort)ellipses };
			internal.ellipse_batch = std::make_unique<rendering::EllipseBatch>(capacity, internal.window->projection_bounds());
		}

		{ // init ellipse registry
			auto register_files = toml_context["ellipse registries"].as_array();
			if (register_files)
			{
				for (const auto& node : *register_files)
					if (auto file = node.value<std::string>())
						internal.ellipse_registry.load(*this, root_dir + file.value());
			}
		}

		{ // init draw command registry
			auto register_files = toml_context["draw command registries"].as_array();
			if (register_files)
			{
				for (const auto& node : *register_files)
					if (auto file = node.value<std::string>())
						internal.draw_command_registry.load(*this, root_dir + file.value());
			}
		}

		{ // init tileset registry
			auto register_files = toml_context["tileset registries"].as_array();
			if (register_files)
			{
				for (const auto& node : *register_files)
					if (auto file = node.value<std::string>())
						internal.tileset_registry.load(*this, root_dir + file.value());
			}
		}
	}

	Context::~Context()
	{
		internal.texture_registry.clear();
		internal.sprite_registry.clear();
		internal.polygon_registry.clear();
		internal.ellipse_registry.clear();
		unload_resources();
		glfwTerminate();
	}

	bool Context::frame() const
	{
		internal.window->swap_buffers();
		check_errors();
		LOG.flush();
		glfwPollEvents();
		TIME.sync();
		glClear(per_frame_clear_mask);
		return !internal.window->should_close();
	}

	void Context::sync_texture_handle(const rendering::BindlessTextureRes& texture) const
	{
		internal.sprite_batch->update_texture_handle(texture);
	}

	void Context::sync_texture_handle(const rendering::BindlessTextureRes& texture, glm::vec2 dimensions) const
	{
		internal.sprite_batch->update_texture_handle(texture, dimensions);
	}
}
