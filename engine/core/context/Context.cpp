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
#include "core/util/Parser.h"

#include "graphics/sprites/SpriteAtlas.h"
#include "graphics/particles/ParticleSystem.h"
#include "physics/dynamics/bodies/RigidBody.h"

#include "definitions/Keys.h"

namespace oly::context
{
	namespace internal
	{
		std::string resource_root;
	}

	static void init_logger(const assets::Parser& parser)
	{
		LoggerOptions options;

		if (auto logger_parser = parser.optional(detail::Key::Logger).subparser())
		{
			logger_parser->optional(detail::Key::UseLogfile)(options.use_logfile);
			logger_parser->optional(detail::Key::UseConsole)(options.use_console);
			if (logger_parser->defaulted(detail::Key::EnableMaxPriorLogFiles)(false))
				logger_parser->optional(detail::Key::MaxPriorLogFiles)(options.max_prior_log_files);
			if (logger_parser->defaulted(detail::Key::EnableMaxPriorLogBytes)(false))
				logger_parser->optional(detail::Key::MaxPriorLogBytes)(options.max_prior_log_bytes);
			
			if (auto enables_parser = logger_parser->optional(detail::Key::Enable).subparser())
			{
				enables_parser->optional(detail::Key::Debug)(LOG.enable.debug);
				enables_parser->optional(detail::Key::Info)(LOG.enable.info);
				enables_parser->optional(detail::Key::Warning)(LOG.enable.warning);
				enables_parser->optional(detail::Key::Error)(LOG.enable.error);
				enables_parser->optional(detail::Key::Fatal)(LOG.enable.fatal);
			}
		}

		oly::internal::LogAccess::start_log(options);
	}

	static void init_time(const assets::Parser& parser)
	{
		if (auto framerate_parser = parser.optional(detail::Key::FrameRate).subparser())
		{
			framerate_parser->optional(detail::Key::FrameLengthClip)(TIME.frame_length_clip);
			framerate_parser->optional(detail::Key::TimeScale)(TIME.time_scale);
		}
		TIME.init();
	}

	static void autoload_signals(const assets::Parser& parser)
	{
		if (auto register_files = parser.optional<TOMLArray>(detail::Key::Signals)())
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
			throw Error(ErrorCode::GlfwInit);
		}
		stbi_set_flip_vertically_on_load(true);

		detail::ResourcePath::set_resource_root(resource_root);
		internal::resource_root = resource_root;
		
		auto toml = io::load_toml(project_file);
		assets::Parser project_parser(toml, { "(project file)" }, ErrorCode::ContextInit, true);
		TOMLNode toml_context = project_parser.required<TOMLNode>(detail::Key::Context)();

		assets::Parser context_parser(toml_context);

		init_logger(context_parser);
		SingletonTickService<TickPhase::None, void, TerminatePhase::Finalization, TerminationFinalization>::instance();

		internal::init_platform(toml_context);
		init_time(context_parser);
		graphics::internal::load_resources();

		autoload_signals(context_parser);
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
			throw Error(ErrorCode::ContextInit, "Context was already initialized");

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
		// TODO v10 begin play on initial actors here
		LOG.flush();
		while (internal::render_frame())
			internal::TickServiceRegistry::instance().tick();
	}
}
