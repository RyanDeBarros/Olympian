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
		if (auto toml_logger = node["logger"])
		{
			io::parse_bool(toml_logger["console"], LOG.target.console);
			if (auto logfile = toml_logger["logfile"].value<std::string>())
			{
				LOG.target.logfile = true;
				LOG.set_logfile(logfile->c_str(), io::parse_bool_or(toml_logger["append"], true));
				LOG.flush();
			}
			else
				LOG.target.logfile = false;
			
			if (auto logger_enable = toml_logger["enable"])
			{
				io::parse_bool(logger_enable["debug"], LOG.enable.debug);
				io::parse_bool(logger_enable["info"], LOG.enable.info);
				io::parse_bool(logger_enable["warning"], LOG.enable.warning);
				io::parse_bool(logger_enable["error"], LOG.enable.error);
				io::parse_bool(logger_enable["fatal"], LOG.enable.fatal);
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
		// TODO v6 better use of tick service instead of just creating generic tick services here.
		GenericTickService vault(TerminatePhase::Vault, &internal::terminate_vault);
		GenericTickService textures(TerminatePhase::Graphics, &internal::terminate_textures);
		GenericTickService tilesets(TerminatePhase::Graphics, &internal::terminate_tilesets);
		GenericTickService fonts(TerminatePhase::Graphics, &internal::terminate_fonts);
		GenericTickService sprites(TerminatePhase::Graphics, &internal::terminate_sprites);
		GenericTickService platform(TerminatePhase::Platform, &internal::terminate_platform);
		GenericTickService resources(TerminatePhase::Resources, &graphics::internal::unload_resources);
		GenericTickService finalization(TerminatePhase::Finalization, []() { glfwTerminate(); LOG.flush(); });

		internal::TickServiceRegistry::instance().terminate();
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

	namespace internal
	{
		bool render_frame()
		{
			TIME.sync();
			internal::render_pipeline();
			return internal::frame_platform();
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
