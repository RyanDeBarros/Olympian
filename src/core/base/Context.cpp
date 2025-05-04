#include "Context.h"

#include <filesystem>

#include "core/util/IO.h"
#include "core/util/Time.h"
#include "graphics/resources/Resources.h"
#include "registries/Loader.h"
#include "registries/platform/InputRegistry.h"

namespace oly
{
	namespace context
	{
		namespace internal
		{
			std::unique_ptr<Platform> platform;

			std::unique_ptr<rendering::SpriteBatch> sprite_batch;
			std::unique_ptr<rendering::PolygonBatch> polygon_batch;
			std::unique_ptr<rendering::EllipseBatch> ellipse_batch;
			std::unique_ptr<rendering::TextBatch> text_batch;

			TextureRegistry texture_registry;
			rendering::NSVGContext nsvg_context;

			rendering::SpriteRegistry sprite_registry;
			rendering::PolygonRegistry polygon_registry;
			rendering::EllipseRegistry ellipse_registry;

			rendering::TileSetRegistry tileset_registry;
			rendering::TileMapRegistry tilemap_registry;
			
			rendering::FontFaceRegistry font_face_registry;
			rendering::FontAtlasRegistry font_atlas_registry;
			rendering::ParagraphRegistry paragraph_registry;

			rendering::DrawCommandRegistry draw_command_registry;
		}

		static void init_logger(const TOMLNode& node, const std::string& root_dir)
		{
			auto toml_logger = node["logger"];
			if (toml_logger)
			{
				LOG.target.console = toml_logger["console"].value<bool>().value_or(true);
				auto logfile = toml_logger["logfile"].value<std::string>();
				if (logfile)
				{
					LOG.target.logfile = true;
					LOG.set_logfile((root_dir + logfile.value()).c_str(), toml_logger["append"].value<bool>().value_or(true));
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
				internal::sprite_batch = std::make_unique<rendering::SpriteBatch>(capacity, internal::platform->window().projection_bounds());
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
				internal::polygon_batch = std::make_unique<rendering::PolygonBatch>(capacity, internal::platform->window().projection_bounds());
			}
		}

		static void init_ellipse_batch(const TOMLNode& node)
		{
			if (auto toml_ellipse_batch = node["ellipse_batch"])
			{
				int ellipses;
				reg::parse_int(toml_ellipse_batch, "ellipses", ellipses);

				rendering::EllipseBatch::Capacity capacity{ (rendering::EllipseBatch::Index)ellipses };
				internal::ellipse_batch = std::make_unique<rendering::EllipseBatch>(capacity, internal::platform->window().projection_bounds());
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
				internal::text_batch = std::make_unique<rendering::TextBatch>(capacity, internal::platform->window().projection_bounds());
			}
		}

#define INIT_REGISTRY(function, node_name, registry)\
		static void function(const TOMLNode& node, const std::string& root_dir)\
		{\
			auto register_files = node[node_name].as_array();\
			if (register_files)\
			{\
				for (const auto& node : *register_files)\
					if (auto file = node.value<std::string>())\
						internal::registry.load(root_dir + file.value());\
			}\
		}

		INIT_REGISTRY(init_texture_registry, "texture registries", texture_registry);
		INIT_REGISTRY(init_sprite_registry, "sprite registries", sprite_registry);
		INIT_REGISTRY(init_polygon_registry, "polygon registries", polygon_registry);
		INIT_REGISTRY(init_ellipse_registry, "ellipse registries", ellipse_registry);
		INIT_REGISTRY(init_tileset_registry, "tileset registries", tileset_registry);
		INIT_REGISTRY(init_tilemap_registry, "tilemap registries", tilemap_registry);
		INIT_REGISTRY(init_font_face_registry, "font face registries", font_face_registry);
		INIT_REGISTRY(init_font_atlas_registry, "font atlas registries", font_atlas_registry);
		INIT_REGISTRY(init_paragraph_registry, "paragraph registries", paragraph_registry);
		INIT_REGISTRY(init_draw_command_registry, "draw command registries", draw_command_registry);

		static void init_signal_registry(const TOMLNode& node, const std::string& root_dir)
		{
			auto register_files = node["signal registries"].as_array();
			if (register_files)
			{
				for (const auto& node : *register_files)
					if (auto file = node.value<std::string>())
						input::load_signal_registry((root_dir + file.value()).c_str());
			}
		}

#undef INIT_REGISTRY
	}
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

	namespace context
	{
		static void init(const char* context_filepath)
		{
			if (glfwInit() != GLFW_TRUE)
				throw oly::Error(oly::ErrorCode::GLFW_INIT);
			stbi_set_flip_vertically_on_load(true);

			auto toml = reg::load_toml(context_filepath);
			const TOMLNode& toml_context = (const TOMLNode&)toml["context"];
			std::string root_dir = io::directory_of(context_filepath);
			init_logger(toml_context, root_dir);

			internal::platform = std::make_unique<Platform>(toml_context);
			Internal{}.time_init();
			load_resources();

			init_sprite_batch(toml_context);
			init_polygon_batch(toml_context);
			init_ellipse_batch(toml_context);
			init_text_batch(toml_context);
			
			init_texture_registry(toml_context, root_dir);
			init_sprite_registry(toml_context, root_dir);
			init_polygon_registry(toml_context, root_dir);
			init_ellipse_registry(toml_context, root_dir);
			init_tileset_registry(toml_context, root_dir);
			init_tilemap_registry(toml_context, root_dir);
			init_font_face_registry(toml_context, root_dir);
			init_font_atlas_registry(toml_context, root_dir);
			init_paragraph_registry(toml_context, root_dir);
			init_draw_command_registry(toml_context, root_dir);
			init_signal_registry(toml_context, root_dir);
		}

		static void terminate()
		{
			internal::texture_registry.clear();
			internal::sprite_registry.clear();
			internal::polygon_registry.clear();
			internal::ellipse_registry.clear();
			internal::tileset_registry.clear();
			internal::tilemap_registry.clear();
			internal::font_face_registry.clear();
			internal::font_atlas_registry.clear();
			internal::paragraph_registry.clear();
			internal::draw_command_registry.clear();

			internal::sprite_batch.reset();
			internal::polygon_batch.reset();
			internal::ellipse_batch.reset();
			internal::text_batch.reset();

			internal::platform.reset();

			unload_resources(); // TODO move resources to context namespace
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

		Platform& platform()
		{
			return *internal::platform;
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

		TextureRegistry& texture_registry()
		{
			return internal::texture_registry;
		}
		
		rendering::NSVGContext& nsvg_context()
		{
			return internal::nsvg_context;
		}

		rendering::SpriteRegistry& sprite_registry()
		{
			return internal::sprite_registry;
		}
		
		rendering::PolygonRegistry& polygon_registry()
		{
			return internal::polygon_registry;
		}
		
		rendering::EllipseRegistry& ellipse_registry()
		{
			return internal::ellipse_registry;
		}
		
		rendering::TileSetRegistry& tileset_registry()
		{
			return internal::tileset_registry;
		}
		
		rendering::TileMapRegistry& tilemap_registry()
		{
			return internal::tilemap_registry;
		}
		
		rendering::FontFaceRegistry& font_face_registry()
		{
			return internal::font_face_registry;
		}
		
		rendering::FontAtlasRegistry& font_atlas_registry()
		{
			return internal::font_atlas_registry;
		}

		rendering::ParagraphRegistry& paragraph_registry()
		{
			return internal::paragraph_registry;
		}

		rendering::DrawCommandRegistry draw_command_registry()
		{
			return internal::draw_command_registry;
		}

		bool frame()
		{
			if (!internal::platform->frame())
				return false;
			Internal{}.time_sync();
			return true;
		}

		void sync_texture_handle(const rendering::BindlessTextureRes& texture)
		{
			internal::sprite_batch->update_texture_handle(texture);
		}

		void sync_texture_handle(const rendering::BindlessTextureRes& texture, glm::vec2 dimensions)
		{
			internal::sprite_batch->update_texture_handle(texture, dimensions);
		}

		rendering::Sprite sprite()
		{
			return rendering::Sprite(*internal::sprite_batch);
		}

		rendering::Sprite sprite(const std::string& name)
		{
			return internal::sprite_registry.create_sprite(name);
		}

		std::weak_ptr<rendering::Sprite> ref_sprite(const std::string& name)
		{
			return internal::sprite_registry.ref_sprite(name);
		}

		void render_sprites()
		{
			internal::sprite_batch->render();
		}

		rendering::AtlasResExtension atlas_extension(const std::string& name)
		{
			return internal::sprite_registry.create_atlas_extension(name);
		}

		std::weak_ptr<rendering::AtlasResExtension> ref_atlas_extension(const std::string& name)
		{
			return internal::sprite_registry.ref_atlas_extension(name);
		}

		rendering::Polygon polygon()
		{
			return rendering::Polygon(*internal::polygon_batch);
		}

		rendering::Polygon polygon(const std::string& name)
		{
			return internal::polygon_registry.create_polygon(name);
		}

		rendering::Composite composite()
		{
			return rendering::Composite(*internal::polygon_batch);
		}

		rendering::Composite composite(const std::string& name)
		{
			return internal::polygon_registry.create_composite(name);
		}

		rendering::NGon ngon()
		{
			return rendering::NGon(*internal::polygon_batch);
		}

		rendering::NGon ngon(const std::string& name)
		{
			return internal::polygon_registry.create_ngon(name);
		}

		std::weak_ptr<rendering::Polygonal> ref_polygonal(const std::string& name)
		{
			return internal::polygon_registry.ref_polygonal(name);
		}

		std::weak_ptr<rendering::Polygon> ref_polygon(const std::string& name)
		{
			return std::dynamic_pointer_cast<rendering::Polygon>(ref_polygonal(name).lock());
		}

		std::weak_ptr<rendering::Composite> ref_composite(const std::string& name)
		{
			return std::dynamic_pointer_cast<rendering::Composite>(ref_polygonal(name).lock());
		}

		std::weak_ptr<rendering::NGon> ref_ngon(const std::string& name)
		{
			return std::dynamic_pointer_cast<rendering::NGon>(ref_polygonal(name).lock());
		}

		void render_polygons()
		{
			internal::polygon_batch->render();
		}

		rendering::Ellipse ellipse()
		{
			return rendering::Ellipse(*internal::ellipse_batch);
		}

		rendering::Ellipse ellipse(const std::string& name)
		{
			return internal::ellipse_registry.create_ellipse(name);
		}

		std::weak_ptr<rendering::Ellipse> ref_ellipse(const std::string& name)
		{
			return internal::ellipse_registry.ref_ellipse(name);
		}

		void render_ellipses()
		{
			internal::ellipse_batch->render();
		}

		rendering::TileSet tileset(const std::string& name)
		{
			return internal::tileset_registry.create_tileset(name);
		}

		std::weak_ptr<rendering::TileSet> ref_tileset(const std::string& name)
		{
			return internal::tileset_registry.ref_tileset(name);
		}
		
		rendering::TileMap tilemap(const std::string& name)
		{
			return internal::tilemap_registry.create_tilemap(name);
		}
		
		std::weak_ptr<rendering::TileMap> ref_tilemap(const std::string& name)
		{
			return internal::tilemap_registry.ref_tilemap(name);
		}
		
		rendering::FontFace font_face(const std::string& name)
		{
			return internal::font_face_registry.create_font_face(name);
		}
		
		std::weak_ptr<rendering::FontFace> ref_font_face(const std::string& name)
		{
			return internal::font_face_registry.ref_font_face(name);
		}
		
		rendering::FontAtlas font_atlas(const std::string& name)
		{
			return internal::font_atlas_registry.create_font_atlas(name);
		}
		
		std::weak_ptr<rendering::FontAtlas> ref_font_atlas(const std::string& name)
		{
			return internal::font_atlas_registry.ref_font_atlas(name);
		}

		rendering::Paragraph paragraph(const rendering::FontAtlasRes& font_atlas, const rendering::ParagraphFormat& format, utf::String&& text)
		{
			return rendering::Paragraph(*internal::text_batch, font_atlas, format, std::move(text));
		}

		rendering::Paragraph paragraph(const std::string& name)
		{
			return internal::paragraph_registry.create_paragraph(name);
		}

		std::weak_ptr<rendering::Paragraph> ref_paragraph(const std::string& name)
		{
			return internal::paragraph_registry.ref_paragraph(name);
		}

		void render_text()
		{
			internal::text_batch->render();
		}
		
		void execute_draw_command(const std::string& name)
		{
			internal::draw_command_registry.execute(name);
		}
	}
}
