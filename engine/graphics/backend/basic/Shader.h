#pragma once

#include <string>
#include <memory>

#include "Buffers.h"
#include "core/util/ResourcePath.h"

namespace oly::graphics
{
	enum class ShaderType
	{
		VERTEX = GL_VERTEX_SHADER,
		FRAGMENT = GL_FRAGMENT_SHADER,
		GEOMETRY = GL_GEOMETRY_SHADER,
		COMPUTE = GL_COMPUTE_SHADER
	};

	struct ShaderBufferSource
	{
		std::string buffer;
		ShaderType type;
	};

	struct ShaderPathSource
	{
		ResourcePath path;
		ShaderType type;
	};

	class Shader
	{
		GLuint id = 0;

	public:
		Shader();
		Shader(const std::vector<ShaderBufferSource>& buffer_sources);
		Shader(const std::vector<ShaderPathSource>& path_sources);
		Shader(const Shader&) = delete;
		Shader(Shader&&) noexcept;
		~Shader();
		Shader& operator=(Shader&&) noexcept;

		operator GLuint () const { return id; }
	};

	extern void dispatch_compute(GLuint x_size, GLuint y_size, GLuint z_size, GLuint x_threads, GLuint y_threads, GLuint z_threads);
}
