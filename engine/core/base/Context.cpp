#include "Context.h"

#include <filesystem>

#include "core/util/IO.h"
#include "core/util/Time.h"
#include "graphics/resources/Resources.h"
#include "registries/Loader.h"
#include "registries/platform/Input.h"

namespace oly::context
{
	namespace internal
	{
		std::string context_filepath;
		std::unique_ptr<platform::Platform> platform;
		const std::function<void()>* render_frame;

		std::unique_ptr<rendering::SpriteBatch> sprite_batch;
		std::unique_ptr<rendering::PolygonBatch> polygon_batch;
		std::unique_ptr<rendering::EllipseBatch> ellipse_batch;
		std::unique_ptr<rendering::TextBatch> text_batch;

		graphics::NSVGContext nsvg_context;
		reg::TextureRegistry texture_registry;
		reg::TileSetRegistry tileset_registry;			
		reg::FontFaceRegistry font_face_registry;
		reg::FontAtlasRegistry font_atlas_registry;

		platform::StandardWindowResize standard_window_resize;
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

#undef INIT_REGISTRY
}

namespace oly
{
	struct Internal
	{
		void time_init()
		{
			TIME.init();
		}

		void time_sync()
		{
			TIME.sync();
		}
	};
}

namespace oly::context
{
	static void init(const char* context_filepath)
	{
		if (glfwInit() != GLFW_TRUE)
			throw oly::Error(oly::ErrorCode::GLFW_INIT);
		stbi_set_flip_vertically_on_load(true);

		internal::context_filepath = io::directory_of(context_filepath);

		auto toml = reg::load_toml(context_filepath);
		const TOMLNode& toml_context = (const TOMLNode&)toml["context"];
		init_logger(toml_context);

		internal::platform = std::make_unique<platform::Platform>(toml_context);
		Internal{}.time_init();
		graphics::internal::load_resources();

		init_sprite_batch(toml_context);
		init_polygon_batch(toml_context);
		init_ellipse_batch(toml_context);
		init_text_batch(toml_context);
			
		autoload_signals(toml_context);

		internal::standard_window_resize.attach(&internal::platform->window().handlers.window_resize);
		glm::ivec2 size = context::get_platform().window().get_size();
		internal::standard_window_resize.projection_bounds = 0.5f * glm::vec4{ -size.x, size.x, -size.y, size.y };
	}

	static void terminate()
	{
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

	void set_render_function(const std::function<void()>* render_frame)
	{
		internal::render_frame = render_frame;
		internal::standard_window_resize.render_frame = render_frame;
		internal::standard_window_resize.target_aspect_ratio = internal::platform->window().aspect_ratio();
	}

	void set_window_resized_parameters(bool boxed, bool stretch)
	{
		internal::standard_window_resize.boxed = boxed;
		internal::standard_window_resize.stretch = stretch;
	}

	platform::StandardWindowResize& get_standard_window_resize()
	{
		return internal::standard_window_resize;
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
		if (internal::render_frame && *internal::render_frame)
			(*internal::render_frame)();
		if (!internal::platform->frame())
			return false;
		Internal{}.time_sync();
		return true;
	}

	graphics::BindlessTextureRes load_texture(const std::string& file, unsigned int texture_index)
	{
		return internal::texture_registry.load_texture(file, { .texture_index = texture_index });
	}

	graphics::BindlessTextureRes load_svg_texture(const std::string& file, float svg_scale, unsigned int texture_index)
	{
		return internal::texture_registry.load_svg_texture(file, svg_scale, { .texture_index = texture_index });
	}

	glm::vec2 get_texture_dimensions(const std::string& file, unsigned int texture_index)
	{
		return internal::texture_registry.get_dimensions(file, texture_index);
	}

	void sync_texture_handle(const graphics::BindlessTextureRes& texture)
	{
		internal::sprite_batch->update_texture_handle(texture);
	}

	rendering::Sprite sprite()
	{
		return rendering::Sprite(*internal::sprite_batch);
	}

	void render_sprites()
	{
		internal::sprite_batch->render();
	}

	rendering::Polygon polygon()
	{
		return rendering::Polygon(*internal::polygon_batch);
	}

	rendering::PolyComposite poly_composite()
	{
		return rendering::PolyComposite(*internal::polygon_batch);
	}

	rendering::NGon ngon()
	{
		return rendering::NGon(*internal::polygon_batch);
	}

	void render_polygons()
	{
		internal::polygon_batch->render();
	}

	rendering::Ellipse ellipse()
	{
		return rendering::Ellipse(*internal::ellipse_batch);
	}

	void render_ellipses()
	{
		internal::ellipse_batch->render();
	}

	rendering::TileSetRes load_tileset(const std::string& file)
	{
		return internal::tileset_registry.load_tileset(file);
	}

	rendering::FontFaceRes load_font_face(const std::string& file)
	{
		return internal::font_face_registry.load_font_face(file);
	}

	rendering::FontAtlasRes load_font_atlas(const std::string& file, unsigned int index)
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
	}

	glm::vec2 get_cursor_screen_pos()
	{
		double x, y;
		glfwGetCursorPos(internal::platform->window(), &x, &y);
		return { (float)x - 0.5f * internal::platform->window().get_width(), 0.5f * internal::platform->window().get_height() - (float)y};
	}
}
