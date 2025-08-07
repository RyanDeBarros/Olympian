#include "Context.h"

#include "core/context/Platform.h"
#include "core/context/Collision.h"

#include "core/context/rendering/Rendering.h"
#include "core/context/rendering/Textures.h"
#include "core/context/rendering/Sprites.h"
#include "core/context/rendering/Ellipses.h"
#include "core/context/rendering/Polygons.h"
#include "core/context/rendering/Text.h"
#include "core/context/rendering/Fonts.h"
#include "core/context/rendering/Tilesets.h"

#include "graphics/resources/Resources.h"

#include "core/util/Time.h"

#include "registries/platform/Input.h"

#include "physics/dynamics/bodies/RigidBody.h"

namespace oly::context
{
	namespace internal
	{
		std::string resource_root;
		std::shared_ptr<Functor<void()>> render_frame;
		size_t this_frame = 0;
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
				LOG.set_logfile(logfile.value().c_str(), toml_logger["append"].value<bool>().value_or(true));
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

	static void autoload_signals(const TOMLNode& node)
	{
		auto register_files = node["signals"].as_array();
		if (register_files)
		{
			for (const auto& node : *register_files)
				if (auto file = node.value<std::string>())
					reg::load_signals((internal::resource_root + file.value()).c_str());
		}
	}
}

// TODO v3 Log engine initialization/terminatation steps.

namespace oly::context
{
	static void init(const char* project_file, const std::string& resource_root)
	{
		if (glfwInit() != GLFW_TRUE)
			throw oly::Error(oly::ErrorCode::GLFW_INIT);
		stbi_set_flip_vertically_on_load(true);

		internal::this_frame = 0;
		internal::resource_root = resource_root;
		auto toml = reg::load_toml(project_file);
		TOMLNode toml_context = toml["context"];

		init_logger(toml_context);

		internal::init_platform(toml_context);
		TIME.init();
		graphics::internal::load_resources();

		internal::init_sprites(toml_context);
		internal::init_polygons(toml_context);
		internal::init_ellipses(toml_context);
		internal::init_text(toml_context);

		autoload_signals(toml_context);
		internal::init_viewport(toml_context);

		oly::internal::check_errors();

		col2d::internal::load_luts();
	}

	static void terminate()
	{
		internal::terminate_collision();

		physics::internal::RigidBodyManager::instance().clear();

		oly::internal::PoolBatch::instance().clear();

		internal::terminate_textures();
		internal::terminate_tilesets();
		internal::terminate_fonts();

		internal::terminate_sprites();
		internal::terminate_polygons();
		internal::terminate_ellipses();
		internal::terminate_text();

		internal::terminate_platform();

		graphics::internal::unload_resources();

		glfwTerminate();

		LOG.flush();
	}

	static size_t active_contexts = 0;

	Context::Context(const char* project_file, const char* resource_root)
	{
		if (active_contexts == 0)
			init(project_file, resource_root);
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

	std::string resource_file(const std::string& file)
	{
		return internal::resource_root + file;
	}

	void set_render_function(const std::shared_ptr<Functor<void()>>& render_frame)
	{
		internal::render_frame = render_frame;
	}

	toml::parse_result load_toml(const char* file)
	{
		return reg::load_toml(resource_file(file));
	}

	bool frame()
	{
		if (internal::render_frame)
			(*internal::render_frame)();
		if (!internal::frame_platform())
			return false;

		TIME.sync();
		++internal::this_frame;
		internal::frame_collision();

		oly::internal::PoolBatch::instance().clean();

		physics::internal::RigidBodyManager::instance().on_tick();

		return true;
	}

	BigSize this_frame()
	{
		return internal::this_frame;
	}
}
