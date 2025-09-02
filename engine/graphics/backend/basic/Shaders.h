#pragma once

#include <string>
#include <memory>

#include "Buffers.h"

namespace oly::graphics
{
	struct ShaderBufferSource
	{
		std::string vertex_buffer;
		std::string fragment_buffer;
		std::optional<std::string> geometry_buffer;
	};

	struct ShaderPathSource
	{
		std::string vertex_path;
		std::string fragment_path;
		std::optional<std::string> geometry_path;
	};

	class Shader
	{
		GLuint id = 0;

	public:
		Shader();
		Shader(const ShaderBufferSource& buffer_source);
		Shader(const ShaderPathSource& path_source);
		Shader(const Shader&) = delete;
		Shader(Shader&&) noexcept;
		~Shader();
		Shader& operator=(Shader&&) noexcept;

		operator GLuint () const { return id; }
	};
}
