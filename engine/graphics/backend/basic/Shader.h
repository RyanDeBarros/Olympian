#pragma once

#include "graphics/backend/basic/Buffers.h"

#include "assets/ResourcePath.h"

#include <string>
#include <memory>

namespace oly::graphics
{
    enum class ShaderType
    {
        Vertex = GL_VERTEX_SHADER,
        Fragment = GL_FRAGMENT_SHADER,
        Geometry = GL_GEOMETRY_SHADER,
        Compute = GL_COMPUTE_SHADER
    };

    struct ShaderBufferSource
    {
        std::string buffer;
        ShaderType type;
    };

    struct ShaderPathSource
    {
        detail::ResourcePath path;
        ShaderType type;
    };

    class Shader
    {
        GLuint id_ = 0;

    public:
        Shader();
        Shader(const std::vector<ShaderBufferSource>& buffer_sources);
        Shader(const std::vector<ShaderPathSource>& path_sources);
        Shader(const Shader&) = delete;
        Shader(Shader&&) noexcept;
        ~Shader();

        Shader& operator=(Shader&&) noexcept;

        operator GLuint () const;
    };

    extern void dispatch_compute(
        GLuint x_size,
        GLuint y_size,
        GLuint z_size,
        GLuint x_threads,
        GLuint y_threads,
        GLuint z_threads);
}
