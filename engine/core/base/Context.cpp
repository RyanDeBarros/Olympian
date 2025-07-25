#include "Context.h"

#include <filesystem>

#include "core/util/IO.h"
#include "core/util/Time.h"

#include "graphics/resources/Resources.h"

#include "registries/Loader.h"
#include "registries/platform/Input.h"
#include "registries/graphics/TextureRegistry.h"
#include "registries/graphics/extensions/TileSetRegistry.h"
#include "registries/graphics/text/FontFaceRegistry.h"
#include "registries/graphics/text/FontAtlasRegistry.h"

#include "physics/collision/scene/LUT.h"
#include "physics/collision/scene/CollisionDispatcher.h"
#include "physics/dynamics/Material.h"
#include "physics/dynamics/RigidBody.h"

namespace oly::context
{
	namespace internal
	{
		std::string context_filepath;
		std::unique_ptr<platform::Platform> platform;
		std::shared_ptr<Functor<void()>> render_frame;
		size_t this_frame = 0;
		glm::ivec2 initial_window_size;

		std::unique_ptr<rendering::SpriteBatch> sprite_batch;
		std::unique_ptr<rendering::PolygonBatch> polygon_batch;
		std::unique_ptr<rendering::EllipseBatch> ellipse_batch;
		std::unique_ptr<rendering::TextBatch> text_batch;
		
		InternalBatch last_internal_batch_rendered = InternalBatch::NONE;

		graphics::NSVGContext nsvg_context;
		reg::TextureRegistry texture_registry;
		reg::TileSetRegistry tileset_registry;			
		reg::FontFaceRegistry font_face_registry;
		reg::FontAtlasRegistry font_atlas_registry;

		platform::WRViewport wr_viewport;
		platform::WRDrawer wr_drawer;

		col2d::CollisionDispatcher collision_dispatcher;
	}

	static void init_logger(const TOMLNode& node)
	{
		auto toml_logger = node["logger"];
		if (toml_logger)
		{
			LOG.target.console = toml_logger["console"].value<bool>().value_or(true);
			auto logfile = toml_logger["logfile"].value<std::string>();
			if (logfile)
			{
				LOG.target.logfile = true;
				LOG.set_logfile((internal::context_filepath + logfile.value()).c_str(), toml_logger["append"].value<bool>().value_or(true));
				LOG.flush();
			}
			else
				LOG.target.logfile = false;
		}
		else
		{
			LOG.target.logfile = false;
			LOG.target.console = true;
		}
	}

	static void init_sprite_batch(const TOMLNode& node)
	{
		if (auto toml_sprite_batch = node["sprite_batch"])
		{
			int initial_sprites = 0;
			reg::parse_int(toml_sprite_batch, "initial sprites", initial_sprites);
			int new_textures = 0;
			reg::parse_int(toml_sprite_batch, "new textures", new_textures);
			int new_uvs = 0;
			reg::parse_int(toml_sprite_batch, "new uvs", new_uvs);
			int new_modulations = 0;
			reg::parse_int(toml_sprite_batch, "new modulations", new_modulations);
			int num_anims = 0;
			reg::parse_int(toml_sprite_batch, "num anims", num_anims);

			rendering::SpriteBatch::Capacity capacity{ (GLuint)initial_sprites, (GLuint)new_textures, (GLuint)new_uvs, (GLuint)new_modulations, (GLuint)num_anims };
			internal::sprite_batch = std::make_unique<rendering::SpriteBatch>(capacity);
		}
	}

	static void init_polygon_batch(const TOMLNode& node)
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

	static void init_ellipse_batch(const TOMLNode& node)
	{
		if (auto toml_ellipse_batch = node["ellipse_batch"])
		{
			int ellipses;
			reg::parse_int(toml_ellipse_batch, "ellipses", ellipses);

			rendering::EllipseBatch::Capacity capacity{ (rendering::EllipseBatch::Index)ellipses };
			internal::ellipse_batch = std::make_unique<rendering::EllipseBatch>(capacity);
		}
	}

	static void init_text_batch(const TOMLNode& node)
	{
		if (auto toml_text_batch = node["text_batch"])
		{
			int initial_glyphs = 0;
			reg::parse_int(toml_text_batch, "initial glyphs", initial_glyphs);
			int new_textures = 0;
			reg::parse_int(toml_text_batch, "new textures", new_textures);
			int new_text_colors = 0;
			reg::parse_int(toml_text_batch, "new text colors", new_text_colors);
			int new_modulations = 0;
			reg::parse_int(toml_text_batch, "new modulations", new_modulations);

			rendering::TextBatch::Capacity capacity{ (GLuint)initial_glyphs, (GLuint)new_textures, (GLuint)new_text_colors, (GLuint)new_modulations };
			internal::text_batch = std::make_unique<rendering::TextBatch>(capacity);
		}
	}

	static void autoload_signals(const TOMLNode& node)
	{
		auto register_files = node["signals"].as_array();
		if (register_files)
		{
			for (const auto& node : *register_files)
				if (auto file = node.value<std::string>())
					reg::load_signals((internal::context_filepath + file.value()).c_str());
		}
	}

	static void init_viewport(const TOMLNode& node)
	{
		internal::wr_viewport.boxed = node["window"]["boxed"].value_or<bool>(true);
		internal::wr_viewport.stretch = node["window"]["stretch"].value_or<bool>(true);

		platform::internal::invoke_initialize_viewport(internal::wr_viewport);
		internal::wr_viewport.attach(&internal::platform->window().handlers.window_resize);
		internal::wr_drawer.attach(&internal::platform->window().handlers.window_resize);
	}
}

// TODO v3 Log engine initialization/terminatation steps.

namespace oly::context
{
	static void init(const char* context_filepath)
	{
		if (glfwInit() != GLFW_TRUE)
			throw oly::Error(oly::ErrorCode::GLFW_INIT);
		stbi_set_flip_vertically_on_load(true);

		internal::this_frame = 0;
		internal::context_filepath = io::directory_of(context_filepath);

		auto toml = reg::load_toml(context_filepath);
		const TOMLNode& toml_context = (const TOMLNode&)toml["context"];
		init_logger(toml_context);

		platform::PlatformSetup platform_setup(toml_context);
		internal::initial_window_size = platform_setup.window_size();
		internal::platform = std::make_unique<platform::Platform>(platform_setup);
		TIME.init();
		graphics::internal::load_resources();

		init_sprite_batch(toml_context);
		init_polygon_batch(toml_context);
		init_ellipse_batch(toml_context);
		init_text_batch(toml_context);
		
		autoload_signals(toml_context);
		init_viewport(toml_context);

		oly::internal::check_errors();

		col2d::internal::load_luts();
	}

	static void terminate()
	{
		internal::collision_dispatcher.clear();

		physics::internal::RigidBodyManager::instance().clear();

		oly::internal::PoolBatch::instance().clear();

		internal::texture_registry.clear();
		internal::tileset_registry.clear();
		internal::font_face_registry.clear();
		internal::font_atlas_registry.clear();

		internal::sprite_batch.reset();
		internal::polygon_batch.reset();
		internal::ellipse_batch.reset();
		internal::text_batch.reset();

		internal::platform.reset();

		graphics::internal::unload_resources();
		
		glfwTerminate();

		LOG.flush();
	}

	static size_t active_contexts = 0;

	Context::Context(const char* context_filepath)
	{
		if (active_contexts == 0)
			init(context_filepath);
		++active_contexts;
	}

	Context::Context(const Context&)
	{
		++active_contexts;
	}
		
	Context::Context(Context&&) noexcept
	{
		++active_contexts;
	}
		
	Context::~Context()
	{
		--active_contexts;
		if (active_contexts == 0)
			terminate();
	}
		
	Context& Context::operator=(const Context& other)
	{
		return *this;
	}
		
	Context& Context::operator=(Context&&) noexcept
	{
		return *this;
	}

	const std::string& context_filepath()
	{
		return internal::context_filepath;
	}

	platform::Platform& get_platform()
	{
		return *internal::platform;
	}

	void set_render_function(const std::shared_ptr<Functor<void()>>& render_frame)
	{
		internal::render_frame = render_frame;
	}

	void set_window_resize_mode(bool boxed, bool stretch)
	{
		internal::wr_viewport.boxed = boxed;
		internal::wr_viewport.stretch = stretch;
	}

	const platform::WRViewport& get_wr_viewport()
	{
		return internal::wr_viewport;
	}

	platform::WRDrawer& get_wr_drawer()
	{
		return internal::wr_drawer;
	}

	void set_standard_viewport()
	{
		internal::wr_viewport.set_viewport();
	}

	rendering::SpriteBatch& sprite_batch()
	{
		return *internal::sprite_batch;
	}

	rendering::PolygonBatch& polygon_batch()
	{
		return *internal::polygon_batch;
	}

	rendering::EllipseBatch& ellipse_batch()
	{
		return *internal::ellipse_batch;
	}

	rendering::TextBatch& text_batch()
	{
		return *internal::text_batch;
	}

	reg::TextureRegistry& texture_registry()
	{
		return internal::texture_registry;
	}
		
	InternalBatch last_internal_batch_rendered()
	{
		return internal::last_internal_batch_rendered;
	}

	graphics::NSVGContext& nsvg_context()
	{
		return internal::nsvg_context;
	}

	reg::TileSetRegistry& tileset_registry()
	{
		return internal::tileset_registry;
	}
		
	reg::FontFaceRegistry& font_face_registry()
	{
		return internal::font_face_registry;
	}
		
	reg::FontAtlasRegistry& font_atlas_registry()
	{
		return internal::font_atlas_registry;
	}

	toml::parse_result load_toml(const char* file)
	{
		return reg::load_toml(context_filepath() + file);
	}

	bool frame()
	{
		if (internal::render_frame)
			(*internal::render_frame)();
		if (!internal::platform->frame())
			return false;
		
		TIME.sync();
		++internal::this_frame;
		internal::collision_dispatcher.poll();

		oly::internal::PoolBatch::instance().clean();

		physics::internal::RigidBodyManager::instance().on_tick();

		return true;
	}

	BigSize this_frame()
	{
		return internal::this_frame;
	}

	graphics::BindlessTextureRef load_texture(const std::string& file, unsigned int texture_index)
	{
		return internal::texture_registry.load_texture(file, { .texture_index = texture_index });
	}

	graphics::BindlessTextureRef load_svg_texture(const std::string& file, float svg_scale, unsigned int texture_index)
	{
		return internal::texture_registry.load_svg_texture(file, svg_scale, { .texture_index = texture_index });
	}

	glm::vec2 get_texture_dimensions(const std::string& file, unsigned int texture_index)
	{
		return internal::texture_registry.get_dimensions(file, texture_index);
	}

	void sync_texture_handle(const graphics::BindlessTextureRef& texture)
	{
		internal::sprite_batch->update_texture_handle(texture);
	}

	void render_sprites()
	{
		internal::sprite_batch->render();
		internal::last_internal_batch_rendered = InternalBatch::SPRITE;
	}

	void render_polygons()
	{
		internal::polygon_batch->render();
		internal::last_internal_batch_rendered = InternalBatch::POLYGON;
	}

	void render_ellipses()
	{
		internal::ellipse_batch->render();
		internal::last_internal_batch_rendered = InternalBatch::ELLIPSE;
	}

	rendering::TileSetRef load_tileset(const std::string& file)
	{
		return internal::tileset_registry.load_tileset(file);
	}

	rendering::FontFaceRef load_font_face(const std::string& file)
	{
		return internal::font_face_registry.load_font_face(file);
	}

	rendering::FontAtlasRef load_font_atlas(const std::string& file, unsigned int index)
	{
		return internal::font_atlas_registry.load_font_atlas(file, index);
	}

	rendering::Paragraph paragraph(const std::string& font_atlas, const rendering::ParagraphFormat& format, utf::String&& text, unsigned int atlas_index)
	{
		return rendering::Paragraph(*internal::text_batch, internal::font_atlas_registry.load_font_atlas(font_atlas, atlas_index), format, std::move(text));
	}

	void render_text()
	{
		internal::text_batch->render();
		internal::last_internal_batch_rendered = InternalBatch::TEXT;
	}

	glm::vec2 get_cursor_screen_pos()
	{
		double x, y;
		glfwGetCursorPos(internal::platform->window(), &x, &y);
		return { (float)x - 0.5f * internal::platform->window().get_width(), 0.5f * internal::platform->window().get_height() - (float)y};
	}

	glm::vec2 get_initial_window_size()
	{
		return internal::initial_window_size;
	}

	glm::vec2 get_view_stretch()
	{
		if (internal::wr_viewport.stretch)
		{
			auto v = internal::wr_viewport.get_viewport();
			return glm::vec2(v.w, v.h) / glm::vec2(internal::initial_window_size);
		}
		else
			return { 1.0f, 1.0f };
	}

	glm::vec2 get_cursor_view_pos()
	{
		return get_cursor_screen_pos() / get_view_stretch();
	}

	bool blend_enabled()
	{
		GLboolean enabled;
		glGetBooleanv(GL_BLEND, &enabled);
		return (bool)enabled;
	}

	glm::vec4 clear_color()
	{
		glm::vec4 color;
		glGetFloatv(GL_COLOR_CLEAR_VALUE, glm::value_ptr(color));
		return color;
	}

	col2d::CollisionDispatcher& collision_dispatcher()
	{
		return internal::collision_dispatcher;
	}
}
