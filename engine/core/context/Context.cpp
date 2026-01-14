#include "Context.h"

#include "external/STB.h"

#include "core/context/TickService.h"
#include "core/context/Platform.h"
#include "core/context/Collision.h"
#include "core/context/Vault.h"

#include "core/context/rendering/Rendering.h"
#include "core/context/rendering/Textures.h"
#include "core/context/rendering/Sprites.h"
#include "core/context/rendering/Fonts.h"
#include "core/context/rendering/Tilesets.h"

#include "graphics/resources/Resources.h"

#include "core/util/Time.h"
#include "core/util/Timers.h"
#include "core/util/Loader.h"

#include "graphics/sprites/SpriteAtlas.h"
#include "graphics/particles/ParticleSystem.h"
#include "physics/dynamics/bodies/RigidBody.h"

namespace oly::context
{
	namespace internal
	{
		std::string resource_root;
	}

	static void init_logger(TOMLNode node)
	{
		std::string logfile = "";
		bool append = false;
		bool console = true;
		if (auto toml_logger = node["logger"])
		{
			if (auto file = toml_logger["logfile"].value<std::string>())
				logfile = *file;
			append = io::parse_bool_or(toml_logger["append"], true);
			io::parse_bool(toml_logger["console"], console);
			
			if (auto logger_enable = toml_logger["enable"])
			{
				io::parse_bool(logger_enable["debug"], LOG.enable.debug);
				io::parse_bool(logger_enable["info"], LOG.enable.info);
				io::parse_bool(logger_enable["warning"], LOG.enable.warning);
				io::parse_bool(logger_enable["error"], LOG.enable.error);
				io::parse_bool(logger_enable["fatal"], LOG.enable.fatal);
			}
		}

		oly::internal::LogAccess::start_log(logfile.c_str(), append, true);
	}

	static void init_time(TOMLNode node)
	{
		if (auto framerate = node["framerate"])
		{
			io::parse_double(framerate["frame_length_clip"], TIME.frame_length_clip);
			io::parse_double(framerate["time_scale"], TIME.time_scale);
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
					load_signals(*file);
		}
	}

	struct TerminationFinalization
	{
		void operator()() const
		{
			glfwTerminate();
			oly::internal::LogAccess::end_log();
		}
	};

	static void init(const char* project_file, const std::string& resource_root)
	{
		if (glfwInit() != GLFW_TRUE)
		{
			_OLY_ENGINE_LOG_FATAL("CONTEXT") << "glfwInit() failed." << LOG.nl;
			throw Error(ErrorCode::GLFW_INIT);
		}
		stbi_set_flip_vertically_on_load(true);

		internal::set_resource_root(resource_root);
		internal::resource_root = resource_root;
		auto toml = io::load_toml(project_file);
		TOMLNode toml_context = toml["context"];
		if (!toml_context)
		{
			_OLY_ENGINE_LOG_FATAL("CONTEXT") << "Project file missing \"context\" table." << LOG.nl;
			throw Error(ErrorCode::CONTEXT_INIT);
		}

		init_logger(toml_context);
		SingletonTickService<TickPhase::None, void, TerminatePhase::Finalization, TerminationFinalization>::instance();

		internal::init_platform(toml_context);
		init_time(toml_context);
		graphics::internal::load_resources();

		autoload_signals(toml_context);
		internal::init_collision(toml_context);
		internal::init_viewport(toml_context);
		internal::init_vault(toml_context);

		internal::init_textures(toml_context);
		internal::init_sprites(toml_context);
		internal::init_fonts(toml_context);

		oly::internal::check_errors();
	}

	static bool active_context = false;

	Context::Context(const char* project_file, const char* resource_root)
	{
		if (active_context)
			throw Error(ErrorCode::CONTEXT_INIT, "Context was already initialized");

		active_context = true;
		init(project_file, resource_root);
	}

	Context::~Context()
	{
		internal::TickServiceRegistry::instance().terminate();
		active_context = false;
	}

	namespace internal
	{
		bool render_frame()
		{
			TIME.sync();
			internal::render_pipeline();
			return internal::platform_frame();
		}
	}

	void run()
	{
		// TODO v7 begin play on initial actors here
		LOG.flush();
		while (internal::render_frame())
			internal::TickServiceRegistry::instance().tick();
	}
}
