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
		init_window(toml_context);
		init_gamepads();
		init_binding_context();
		std::string root_dir = io::directory_of(filepath);
		init_texture_registry(toml_context, root_dir);
		init_sprite_batch(toml_context);
		init_sprite_registry(toml_context, root_dir);
		init_polygon_batch(toml_context);
		init_polygon_registry(toml_context, root_dir);
		init_ellipse_batch(toml_context);
		init_ellipse_registry(toml_context, root_dir);
		init_tileset_registry(toml_context, root_dir);
		init_tilemap_registry(toml_context, root_dir);
		init_font_face_registry(toml_context, root_dir);
		init_font_atlas_registry(toml_context, root_dir);
		init_draw_command_registry(toml_context, root_dir);
	}

	Context::~Context()
	{
		internal.texture_registry.clear();
		internal.sprite_registry.clear();
		internal.polygon_registry.clear();
		internal.ellipse_registry.clear();
		internal.tileset_registry.clear();
		internal.tilemap_registry.clear();
		internal.font_face_registry.clear();
		internal.font_atlas_registry.clear();
		internal.draw_command_registry.clear();
		unload_resources();
		glfwTerminate();
	}

	void Context::init_window(const assets::AssetNode& node)
	{
		auto toml_window = node["window"];
		if (!toml_window)
			throw Error(ErrorCode::CONTEXT_INIT);
		auto width = toml_window["width"].value<int64_t>();
		auto height = toml_window["height"].value<int64_t>();
		auto title = toml_window["title"].value<std::string>();
		if (!width || !height || !title)
			throw Error(ErrorCode::CONTEXT_INIT);

		rendering::WindowHint hint;
		if (auto toml_window_hint = node["window_hint"])
		{
			assets::parse_vec4(toml_window_hint, "clear color", hint.context.clear_color);
			// TODO rest of window hint options
		}

		internal.window = std::make_unique<rendering::Window>((int)width.value(), (int)height.value(), title.value().c_str(), hint);
		TIME.init();
		load_resources();
	}

	void Context::init_gamepads()
	{
		for (int i = 0; i < 16; ++i)
		{
			internal.gamepads[i] = std::make_unique<input::Gamepad>(GLFW_JOYSTICK_1 + i);
			internal.gamepads[i]->set_handler();
		}
	}

	void Context::init_binding_context()
	{
		internal.binding_context.attach_key(&internal.window->handlers.key);
		internal.binding_context.attach_mouse_button(&internal.window->handlers.mouse_button);
		internal.binding_context.attach_cursor_pos(&internal.window->handlers.cursor_pos);
	}

	void Context::init_texture_registry(const assets::AssetNode& node, const std::string& root_dir)
	{
		auto register_files = node["texture registries"].as_array();
		if (register_files)
		{
			for (const auto& node : *register_files)
				if (auto file = node.value<std::string>())
					internal.texture_registry.load(*this, root_dir + file.value());
		}
	}

	void Context::init_sprite_batch(const assets::AssetNode& node)
	{
		if (auto toml_sprite_batch = node["sprite_batch"])
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
	}

	void Context::init_sprite_registry(const assets::AssetNode& node, const std::string& root_dir)
	{
		auto register_files = node["sprite registries"].as_array();
		if (register_files)
		{
			for (const auto& node : *register_files)
				if (auto file = node.value<std::string>())
					internal.sprite_registry.load(*this, root_dir + file.value());
		}
	}

	void Context::init_polygon_batch(const assets::AssetNode& node)
	{
		if (auto toml_polygon_batch = node["polygon_batch"])
		{
			int primitives;
			assets::parse_int(toml_polygon_batch, "primitives", primitives);
			int degree = 6;
			assets::parse_int(toml_polygon_batch, "degree", degree);

			rendering::PolygonBatch::Capacity capacity{ (GLushort)primitives, (GLushort)degree };
			internal.polygon_batch = std::make_unique<rendering::PolygonBatch>(capacity, internal.window->projection_bounds());
		}
	}

	void Context::init_polygon_registry(const assets::AssetNode& node, const std::string& root_dir)
	{
		auto register_files = node["polygon registries"].as_array();
		if (register_files)
		{
			for (const auto& node : *register_files)
				if (auto file = node.value<std::string>())
					internal.polygon_registry.load(*this, root_dir + file.value());
		}
	}

	void Context::init_ellipse_batch(const assets::AssetNode& node)
	{
		if (auto toml_ellipse_batch = node["ellipse_batch"])
		{
			int ellipses;
			assets::parse_int(toml_ellipse_batch, "ellipses", ellipses);

			rendering::EllipseBatch::Capacity capacity{ (GLushort)ellipses };
			internal.ellipse_batch = std::make_unique<rendering::EllipseBatch>(capacity, internal.window->projection_bounds());
		}

	}

	void Context::init_ellipse_registry(const assets::AssetNode& node, const std::string& root_dir)
	{
		auto register_files = node["ellipse registries"].as_array();
		if (register_files)
		{
			for (const auto& node : *register_files)
				if (auto file = node.value<std::string>())
					internal.ellipse_registry.load(*this, root_dir + file.value());
		}
	}

	void Context::init_tileset_registry(const assets::AssetNode& node, const std::string& root_dir)
	{
		auto register_files = node["tileset registries"].as_array();
		if (register_files)
		{
			for (const auto& node : *register_files)
				if (auto file = node.value<std::string>())
					internal.tileset_registry.load(*this, root_dir + file.value());
		}
	}

	void Context::init_tilemap_registry(const assets::AssetNode& node, const std::string& root_dir)
	{
		auto register_files = node["tilemap registries"].as_array();
		if (register_files)
		{
			for (const auto& node : *register_files)
				if (auto file = node.value<std::string>())
					internal.tilemap_registry.load(*this, root_dir + file.value());
		}
	}

	void Context::init_font_face_registry(const assets::AssetNode& node, const std::string& root_dir)
	{
		auto register_files = node["font face registries"].as_array();
		if (register_files)
		{
			for (const auto& node : *register_files)
				if (auto file = node.value<std::string>())
					internal.font_face_registry.load(root_dir + file.value());
		}
	}

	void Context::init_font_atlas_registry(const assets::AssetNode& node, const std::string& root_dir)
	{
		auto register_files = node["font atlas registries"].as_array();
		if (register_files)
		{
			for (const auto& node : *register_files)
				if (auto file = node.value<std::string>())
					internal.font_atlas_registry.load(internal.font_face_registry, root_dir + file.value());
		}
	}

	void Context::init_draw_command_registry(const assets::AssetNode& node, const std::string& root_dir)
	{
		auto register_files = node["draw command registries"].as_array();
		if (register_files)
		{
			for (const auto& node : *register_files)
				if (auto file = node.value<std::string>())
					internal.draw_command_registry.load(*this, root_dir + file.value());
		}
	}

	bool Context::frame()
	{
		internal.window->swap_buffers();
		check_errors();
		LOG.flush();
		glfwPollEvents();
		internal.binding_context.poll(*internal.window);
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
