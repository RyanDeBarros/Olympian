#include "Shaders.h"

#include "core/base/SmartReferenceLookup.h"

#include "core/util/IO.h"

namespace oly::graphics::internal_shaders
{
    static const detail::ResourcePath& shaders_dir()
    {
        static const detail::ResourcePath dir = detail::ResourcePath::_cmake_ported("internal/shaders/");
        return dir;
    }

    // ----------------------------------------------------------------------------------------------------------------

    struct SpriteBatchConstructor
    {
        GLushort modulations;
        GLushort anims;

        bool operator==(const SpriteBatchConstructor& other) const = default;

        Shader operator()() const
        {
            ShaderBufferSource vertex = {
                .buffer = io::read_template_file(
                    shaders_dir() / "sprite_batch.vert",
                    {
                        { "/*$MODULATIONS*/", std::to_string(modulations) },
                        { "/*$ANIMS*/",       std::to_string(anims) }
                    }),
                .type = ShaderType::Vertex
            };

            ShaderBufferSource fragment = {
                .buffer = io::read_file(shaders_dir() / "sprite_batch.frag"),
                .type = ShaderType::Fragment
            };

            return Shader({ std::move(vertex), std::move(fragment) });
        }

        size_t hash() const
        {
            return std::hash<GLushort>{}(modulations) ^ (std::hash<GLushort>{}(anims) << 1);
        }
    };

    static SmartReferenceLookup<Shader, SpriteBatchConstructor> sprite_batch_;

    SmartReference<Shader> sprite_batch(GLushort modulations, GLushort anims)
    {
        return sprite_batch_.get({ .modulations = modulations, .anims = anims });
    }

    // ----------------------------------------------------------------------------------------------------------------

    static std::unique_ptr<Shader> polygon_batch_ptr_ = nullptr;
    static GLuint polygon_batch_;

    GLuint polygon_batch()
    {
        return polygon_batch_;
    }

    static std::unique_ptr<Shader> ellipse_batch_ptr_ = nullptr;
    static GLuint ellipse_batch_;

    GLuint ellipse_batch()
    {
        return ellipse_batch_;
    }

    // ----------------------------------------------------------------------------------------------------------------

    static std::unique_ptr<Shader> particle_renderer_ptr_ = nullptr;
    static GLuint particle_renderer_;

    GLuint particle_renderer()
    {
        return particle_renderer_;
    }

    // ----------------------------------------------------------------------------------------------------------------

    struct ParticleComputeSpawnConstructor
    {
        GLushort x_threads;

        bool operator==(const ParticleComputeSpawnConstructor& other) const = default;

        Shader operator()() const
        {
            return Shader({ {
                .buffer = io::read_template_file(
                    shaders_dir() / "particles/spawn.comp",
                    { { "/*$X_THREADS*/", std::to_string(x_threads) } }),
                .type = ShaderType::Compute
            } });
        }

        size_t hash() const
        {
            return std::hash<GLushort>{}(x_threads);
        }
    };

    static SmartReferenceLookup<Shader, ParticleComputeSpawnConstructor> particle_compute_spawn_;

    SmartReference<Shader> particle_compute_spawn(GLushort x_threads)
    {
        return particle_compute_spawn_.get({ .x_threads = x_threads });
    }

    // ----------------------------------------------------------------------------------------------------------------

    struct ParticleComputeUpdateConstructor
    {
        GLushort x_threads;

        bool operator==(const ParticleComputeUpdateConstructor& other) const = default;

        Shader operator()() const
        {
            return Shader({ {
                .buffer = io::read_template_file(
                    shaders_dir() / "particles/update.comp",
                    { { "/*$X_THREADS*/", std::to_string(x_threads) } }),
                .type = ShaderType::Compute
            } });
        }

        size_t hash() const
        {
            return std::hash<GLushort>{}(x_threads);
        }
    };

    static SmartReferenceLookup<Shader, ParticleComputeUpdateConstructor> particle_compute_update_;

    SmartReference<Shader> particle_compute_update(GLushort x_threads)
    {
        return particle_compute_update_.get({ .x_threads = x_threads });
    }

    // ----------------------------------------------------------------------------------------------------------------

    void load()
    {
        polygon_batch_ptr_ = std::make_unique<Shader>(std::vector<ShaderPathSource>{
            { .path = shaders_dir() / "polygon_batch.vert", .type = ShaderType::Vertex },
            { .path = shaders_dir() / "polygon_batch.frag", .type = ShaderType::Fragment }
        });
        polygon_batch_ = *polygon_batch_ptr_;

        ellipse_batch_ptr_ = std::make_unique<Shader>(std::vector<ShaderPathSource>{
            { .path = shaders_dir() / "ellipse_batch.vert", .type = ShaderType::Vertex },
            { .path = shaders_dir() / "ellipse_batch.frag", .type = ShaderType::Fragment }
        });
        ellipse_batch_ = *ellipse_batch_ptr_;

        particle_renderer_ptr_ = std::make_unique<Shader>(std::vector<ShaderPathSource>{
            { .path = shaders_dir() / "particles/particle.vert", .type = ShaderType::Vertex },
            { .path = shaders_dir() / "particles/particle.frag", .type = ShaderType::Fragment }
        });
        particle_renderer_ = *particle_renderer_ptr_;
    }

    void unload()
    {
        sprite_batch_.clear();

        polygon_batch_ptr_.reset();
        polygon_batch_ = 0;

        ellipse_batch_ptr_.reset();
        ellipse_batch_ = 0;

        particle_renderer_ptr_.reset();
        particle_renderer_ = 0;

        particle_compute_spawn_.clear();
        particle_compute_update_.clear();
    }
}
