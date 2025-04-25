#include "Context.h"

#include <stb/stb_image.h>

#include <filesystem>

#include "rendering/Loader.h"
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

		std::string root_dir = std::filesystem::path(filepath).parent_path().string() + "/";

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
						internal.texture_registry.load(this, root_dir + file.value());
			}
		}

		mut.context = this;

		if (auto toml_sprite_batch = toml_context["mut_sprite_batch"]) // init mutable sprite batch
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

			mut::SpriteBatch::Capacity capacity{ (GLuint)initial_sprites, (GLuint)new_textures, (GLuint)new_uvs, (GLuint)new_modulations, (GLuint)num_anims };
			mut.internal.sprite_batch = std::make_unique<mut::SpriteBatch>(capacity, internal.window->projection_bounds());
		}

		{ // init mutable sprite registry
			auto register_files = toml_context["mut sprite registries"].as_array();
			if (register_files)
			{
				for (const auto& node : *register_files)
					if (auto file = node.value<std::string>())
						internal.mut_sprite_registry.load(root_dir + file.value());
			}
		}

		immut.context = this;

		if (auto toml_polygon_batch = toml_context["immut_polygon_batch"]) // init immutable polygon batch
		{
			int primitives;
			assets::parse_int(toml_polygon_batch, "primitives", primitives);
			int degree = 6;
			assets::parse_int(toml_polygon_batch, "degree", degree);

			immut::PolygonBatch::Capacity capacity{ (GLushort)primitives, (GLushort)degree };
			immut.internal.polygon_batch = std::make_unique<immut::PolygonBatch>(capacity, internal.window->projection_bounds());
		}

		{ // init immutable polygon registry
			auto register_files = toml_context["immut polygon registries"].as_array();
			if (register_files)
			{
				for (const auto& node : *register_files)
					if (auto file = node.value<std::string>())
						internal.immut_polygon_registry.load(this, root_dir + file.value());
			}
		}

		if (auto toml_ellipse_batch = toml_context["immut_ellipse_batch"]) // init immutable ellipse batch
		{
			int ellipses;
			assets::parse_int(toml_ellipse_batch, "ellipses", ellipses);

			immut::EllipseBatch::Capacity capacity{ (GLushort)ellipses };
			immut.internal.ellipse_batch = std::make_unique<immut::EllipseBatch>(capacity, internal.window->projection_bounds());
		}

		{ // init immutable ellipse registry
			auto register_files = toml_context["immut ellipse registries"].as_array();
			if (register_files)
			{
				for (const auto& node : *register_files)
					if (auto file = node.value<std::string>())
						internal.immut_ellipse_registry.load(this, root_dir + file.value());
			}
		}
	}

	Context::~Context()
	{
		internal.texture_registry.clear();
		internal.mut_sprite_registry.clear();
		internal.immut_polygon_registry.clear();
		internal.immut_ellipse_registry.clear();
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

	mut::Sprite Context::Mut::sprite() const
	{
		return mut::Sprite(internal.sprite_batch.get());
	}

	mut::Sprite Context::Mut::sprite(const std::string& name) const
	{
		return context->internal.mut_sprite_registry.create_sprite(context, name);
	}

	std::weak_ptr<mut::Sprite> Context::Mut::ref_sprite(const std::string& name, bool register_if_nonexistant) const
	{
		return context->internal.mut_sprite_registry.get_sprite(context, name, register_if_nonexistant);
	}

	void Context::Mut::draw_sprite_list(const std::string& draw_list_name, bool register_if_nonexistant) const
	{
		context->internal.mut_sprite_registry.draw_sprites(context, draw_list_name, register_if_nonexistant);
		render_sprites();
	}

	void Context::sync_texture_handle(const rendering::BindlessTextureRes& texture) const
	{
		mut.internal.sprite_batch->update_texture_handle(texture);
	}

	void Context::sync_texture_handle(const rendering::BindlessTextureRes& texture, glm::vec2 dimensions) const
	{
		mut.internal.sprite_batch->update_texture_handle(texture, dimensions);
	}
}
