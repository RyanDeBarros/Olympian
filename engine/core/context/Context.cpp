#include "Context.h"

#include "core/context/Collision.h"
#include "core/context/Platform.h"
#include "core/context/TickService.h"
#include "core/context/Vault.h"

#include "core/context/rendering/Fonts.h"
#include "core/context/rendering/Rendering.h"
#include "core/context/rendering/Sprites.h"
#include "core/context/rendering/Textures.h"
#include "core/context/rendering/Tilesets.h"

#include "core/util/Loader.h"
#include "core/util/Parser.h"
#include "core/util/Time.h"
#include "core/util/Timers.h"

#include "graphics/resources/Resources.h"

#include "graphics/particles/ParticleSystem.h"

#include "graphics/sprites/SpriteAtlas.h"

#include "physics/dynamics/bodies/RigidBody.h"

#include "external/STB.h"

#include "definitions/Keys.h"

namespace oly::context
{
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
                enables_parser->optional(detail::Key::Debug)(kLog.enable.debug);
                enables_parser->optional(detail::Key::Info)(kLog.enable.info);
                enables_parser->optional(detail::Key::Warning)(kLog.enable.warning);
                enables_parser->optional(detail::Key::Error)(kLog.enable.error);
                enables_parser->optional(detail::Key::Fatal)(kLog.enable.fatal);
            }
        }

        oly::internal::LogAccess::start_log(options);
    }

    static void init_time(const assets::Parser& parser)
    {
        if (auto framerate_parser = parser.optional(detail::Key::FrameRate).subparser())
        {
            framerate_parser->optional(detail::Key::FrameLengthClip)(kTime.frame_length_clip);
            framerate_parser->optional(detail::Key::TimeScale)(kTime.time_scale);
        }

        kTime.init();
        kRealTime.init();
    }

    struct TerminationFinalization
    {
        void operator()() const
        {
            glfwTerminate();
            oly::internal::LogAccess::end_log();
        }
    };

    Context::Context()
    {
        if (glfwInit() != GLFW_TRUE)
        {
            _OLY_ENGINE_LOG_FATAL("CONTEXT") << "glfwInit() failed." << kLog.nl;
            throw Error(ErrorCode::GlfwInit);
        }

        stbi_set_flip_vertically_on_load(true);

        detail::ResourcePath::set_resource_root("res/");

        auto toml = io::load_toml(detail::ResourcePath::_cmake_ported("Project.oly"));
        assets::Parser project_parser(toml, { "(project file)" }, ErrorCode::ContextInit, true);
        TOMLNode toml_context = project_parser.required<TOMLNode>(detail::Key::Context)();

        assets::Parser context_parser(toml_context);

        init_logger(context_parser);
        SingletonTickService<TickPhase::None, void, TerminatePhase::Finalization, TerminationFinalization>::instance();

        internal::init_platform(toml_context);
        init_time(context_parser);
        graphics::internal::load_resources();

        internal::init_collision(toml_context);
        internal::init_viewport(toml_context);
        internal::init_vault();

        internal::init_textures();
        internal::init_sprites();
        internal::init_fonts();

        oly::internal::check_errors();
    }

    Context::~Context()
    {
        internal::TickServiceRegistry::instance().terminate();
    }

    void Context::run()
    {
        kLog.flush();

        while (internal::render_frame())
            internal::TickServiceRegistry::instance().tick();

        kLog.flush();
    }

    namespace internal
    {
        bool render_frame()
        {
            kTime.sync();
            kRealTime.sync();

            internal::render_pipeline();
            return internal::platform_frame();
        }
    }
}
