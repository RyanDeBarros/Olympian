#include "Context.h"

#include <stb/stb_image.h>

#include <filesystem>

#include "rendering/Loader.h"
#include "util/IO.h"
#include "rendering/Resources.h"

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

			TextureRegistry texture_registry;
			rendering::NSVGContext nsvg_context;

			rendering::SpriteRegistry sprite_registry;
			rendering::PolygonRegistry polygon_registry;
			rendering::EllipseRegistry ellipse_registry;

			rendering::TileSetRegistry tileset_registry;
			rendering::TileMapRegistry tilemap_registry;
			rendering::FontFaceRegistry font_face_registry;
			rendering::FontAtlasRegistry font_atlas_registry;

			rendering::DrawCommandRegistry draw_command_registry;
		}

		static void init_sprite_batch(const assets::AssetNode& node)
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
				internal::sprite_batch = std::make_unique<rendering::SpriteBatch>(capacity, internal::platform->window().projection_bounds());
			}
		}

		static void init_polygon_batch(const assets::AssetNode& node)
		{
			if (auto toml_polygon_batch = node["polygon_batch"])
			{
				int primitives;
				assets::parse_int(toml_polygon_batch, "primitives", primitives);
				int degree = 6;
				assets::parse_int(toml_polygon_batch, "degree", degree);

				rendering::PolygonBatch::Capacity capacity{ (GLushort)primitives, (GLushort)degree };
				internal::polygon_batch = std::make_unique<rendering::PolygonBatch>(capacity, internal::platform->window().projection_bounds());
			}
		}

		static void init_ellipse_batch(const assets::AssetNode& node)
		{
			if (auto toml_ellipse_batch = node["ellipse_batch"])
			{
				int ellipses;
				assets::parse_int(toml_ellipse_batch, "ellipses", ellipses);

				rendering::EllipseBatch::Capacity capacity{ (GLushort)ellipses };
				internal::ellipse_batch = std::make_unique<rendering::EllipseBatch>(capacity, internal::platform->window().projection_bounds());
			}
		}

		static void init_texture_registry(const assets::AssetNode& node, const std::string& root_dir)
		{
			auto register_files = node["texture registries"].as_array();
			if (register_files)
			{
				for (const auto& node : *register_files)
					if (auto file = node.value<std::string>())
						internal::texture_registry.load(root_dir + file.value());
			}
		}

		static void init_sprite_registry(const assets::AssetNode& node, const std::string& root_dir)
		{
			auto register_files = node["sprite registries"].as_array();
			if (register_files)
			{
				for (const auto& node : *register_files)
					if (auto file = node.value<std::string>())
						internal::sprite_registry.load(root_dir + file.value());
			}
		}

		static void init_polygon_registry(const assets::AssetNode& node, const std::string& root_dir)
		{
			auto register_files = node["polygon registries"].as_array();
			if (register_files)
			{
				for (const auto& node : *register_files)
					if (auto file = node.value<std::string>())
						internal::polygon_registry.load(root_dir + file.value());
			}
		}

		static void init_ellipse_registry(const assets::AssetNode& node, const std::string& root_dir)
		{
			auto register_files = node["ellipse registries"].as_array();
			if (register_files)
			{
				for (const auto& node : *register_files)
					if (auto file = node.value<std::string>())
						internal::ellipse_registry.load(root_dir + file.value());
			}
		}

		static void init_tileset_registry(const assets::AssetNode& node, const std::string& root_dir)
		{
			auto register_files = node["tileset registries"].as_array();
			if (register_files)
			{
				for (const auto& node : *register_files)
					if (auto file = node.value<std::string>())
						internal::tileset_registry.load(root_dir + file.value());
			}
		}

		static void init_tilemap_registry(const assets::AssetNode& node, const std::string& root_dir)
		{
			auto register_files = node["tilemap registries"].as_array();
			if (register_files)
			{
				for (const auto& node : *register_files)
					if (auto file = node.value<std::string>())
						internal::tilemap_registry.load(root_dir + file.value());
			}
		}

		static void init_font_face_registry(const assets::AssetNode& node, const std::string& root_dir)
		{
			auto register_files = node["font face registries"].as_array();
			if (register_files)
			{
				for (const auto& node : *register_files)
					if (auto file = node.value<std::string>())
						internal::font_face_registry.load(root_dir + file.value());
			}
		}

		static void init_font_atlas_registry(const assets::AssetNode& node, const std::string& root_dir)
		{
			auto register_files = node["font atlas registries"].as_array();
			if (register_files)
			{
				for (const auto& node : *register_files)
					if (auto file = node.value<std::string>())
						internal::font_atlas_registry.load(font_face_registry(), root_dir + file.value());
			}
		}

		static void init_draw_command_registry(const assets::AssetNode& node, const std::string& root_dir)
		{
			auto register_files = node["draw command registries"].as_array();
			if (register_files)
			{
				for (const auto& node : *register_files)
					if (auto file = node.value<std::string>())
						internal::draw_command_registry.load(root_dir + file.value());
			}
		}

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
			LOG.set_logfile("../../../Olympian.log", true); // TODO use filepath set by context.toml
			LOG.flush();

			auto toml = assets::load_toml(context_filepath);
			auto toml_context = toml["context"];

			internal::platform = std::make_unique<Platform>(toml_context);
			Internal{}.time_init();
			load_resources();

			context::init_sprite_batch(toml_context);
			context::init_polygon_batch(toml_context);
			context::init_ellipse_batch(toml_context);
			std::string root_dir = io::directory_of(context_filepath);
			context::init_texture_registry(toml_context, root_dir);
			context::init_sprite_registry(toml_context, root_dir);
			context::init_polygon_registry(toml_context, root_dir);
			context::init_ellipse_registry(toml_context, root_dir);
			context::init_tileset_registry(toml_context, root_dir);
			context::init_tilemap_registry(toml_context, root_dir);
			context::init_font_face_registry(toml_context, root_dir);
			context::init_font_atlas_registry(toml_context, root_dir);
			context::init_draw_command_registry(toml_context, root_dir);
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
			internal::draw_command_registry.clear();

			internal::sprite_batch.reset();
			internal::polygon_batch.reset();
			internal::ellipse_batch.reset();

			internal::platform.reset();

			unload_resources(); // TODO move resources to context namespace
			glfwTerminate();
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
			return internal::font_atlas_registry.create_font_atlas(internal::font_face_registry, name);
		}
		
		std::weak_ptr<rendering::FontAtlas> ref_font_atlas(const std::string& name)
		{
			return internal::font_atlas_registry.ref_font_atlas(name);
		}
		
		void execute_draw_command(const std::string& name)
		{
			internal::draw_command_registry.execute(name);
		}
	}
}
