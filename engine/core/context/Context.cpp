#include "Context.h"

#include "external/STB.h"

#include "core/context/Platform.h"
#include "core/context/Collision.h"

#include "core/context/rendering/Rendering.h"
#include "core/context/rendering/Textures.h"
#include "core/context/rendering/Sprites.h"
#include "core/context/rendering/Fonts.h"
#include "core/context/rendering/Tilesets.h"

#include "graphics/resources/Resources.h"

#include "core/util/Time.h"
#include "core/util/Timers.h"

#include "registries/platform/Input.h"

#include "physics/dynamics/bodies/RigidBody.h"

namespace oly::context
{
	namespace internal
	{
		std::string resource_root;
		size_t this_frame = 0;
	}

	static void init_logger(TOMLNode node)
	{
		if (auto toml_logger = node["logger"])
		{
			reg::parse_bool(toml_logger["console"], LOG.target.console);
			if (auto logfile = toml_logger["logfile"].value<std::string>())
			{
				LOG.target.logfile = true;
				LOG.set_logfile(logfile->c_str(), reg::parse_bool_or(toml_logger["append"], true));
				LOG.flush();
			}
			else
				LOG.target.logfile = false;
			
			if (auto logger_enable = toml_logger["enable"])
			{
				reg::parse_bool(logger_enable["debug"], LOG.enable.debug);
				reg::parse_bool(logger_enable["info"], LOG.enable.info);
				reg::parse_bool(logger_enable["warning"], LOG.enable.warning);
				reg::parse_bool(logger_enable["error"], LOG.enable.error);
				reg::parse_bool(logger_enable["fatal"], LOG.enable.fatal);
			}
		}
		else
		{
			LOG.target.logfile = false;
			LOG.target.console = true;
		}
	}

	static void init_time(TOMLNode node)
	{
		if (auto framerate = node["framerate"])
		{
			reg::parse_double(framerate["frame_length_clip"], TIME.frame_length_clip);
			reg::parse_double(framerate["time_scale"], TIME.time_scale);
		}
		TIME.init();
	}

	static void autoload_signals(TOMLNode node)
	{
		auto register_files = node["signals"].as_array();
		if (register_files)
		{
			for (const auto& node : *register_files)
				if (auto file = node.value<std::string>())
					reg::load_signals(*file);
		}
	}

	static void init(const char* project_file, const std::string& resource_root)
	{
		if (glfwInit() != GLFW_TRUE)
		{
			OLY_LOG_FATAL(true, "CONTEXT") << LOG.source_info.full_source() << "glfwInit() failed." << LOG.nl;
			throw Error(ErrorCode::GLFW_INIT);
		}
		stbi_set_flip_vertically_on_load(true);

		internal::this_frame = 0;
		internal::set_resource_root(resource_root);
		internal::resource_root = resource_root;
		auto toml = reg::load_toml(project_file);
		TOMLNode toml_context = toml["context"];
		if (!toml_context)
		{
			OLY_LOG_FATAL(true, "CONTEXT") << LOG.source_info.full_source() << "Project file missing \"context\" table." << LOG.nl;
			throw Error(ErrorCode::CONTEXT_INIT);
		}

		init_logger(toml_context);

		internal::init_platform(toml_context);
		init_time(toml_context);
		graphics::internal::load_resources();

		autoload_signals(toml_context);
		internal::init_collision(toml_context);
		internal::init_viewport(toml_context);

		internal::init_sprites(toml_context);

		oly::internal::check_errors();
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

	bool frame()
	{
		if (!render_frame())
			return false;

		// Time / frame counter
		TIME.sync();
		++internal::this_frame;

		// Clean references
		oly::internal::PoolBatch::instance().clean();

		// Poll timers
		oly::internal::TimerRegistry::instance().poll_all();

		// Update physics
		internal::frame_collision();
		physics::internal::RigidBodyManager::instance().on_tick();

		return true;
	}

	bool render_frame()
	{
		internal::render_frame();
		return internal::frame_platform();
	}

	BigSize this_frame()
	{
		return internal::this_frame;
	}
}
