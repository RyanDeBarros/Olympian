#include "Shader.h"

#include <sstream>

#include "core/base/Errors.h"
#include "core/util/IO.h"

namespace oly::graphics
{
	static void validate_subshader(GLuint& subshader)
	{
		GLint result;
		glGetShaderiv(subshader, GL_COMPILE_STATUS, &result);
		if (result == GL_FALSE)
		{
			GLint length;
			glGetShaderiv(subshader, GL_INFO_LOG_LENGTH, &length);
			if (length)
			{
				std::string message;
				message.resize(length);
				glGetShaderInfoLog(subshader, length, &length, message.data());
				glDeleteShader(subshader);
				subshader = 0;
				_OLY_ENGINE_LOG_ERROR("GRAPHICS") << "Subshader compilation failed - " << message << LOG.nl;
				throw Error(ErrorCode::SUBSHADER_COMPILATION);
			}
			else
			{
				glDeleteShader(subshader);
				subshader = 0;
				_OLY_ENGINE_LOG_ERROR("GRAPHICS") << "Subshader compilation failed - no info log provided" << LOG.nl;
				throw Error(ErrorCode::SUBSHADER_COMPILATION);
			}
		}
	}

	static GLuint create_compiled_subshader(const std::string& source, GLenum type)
	{
		GLuint subshader = glCreateShader(type);
		const GLchar* src = source.c_str();
		GLint src_length = (GLint)source.size();
		glShaderSource(subshader, 1, &src, &src_length);
		glCompileShader(subshader);
		validate_subshader(subshader);
		return subshader;
	}

	static void validate_shader(GLuint shader)
	{
		GLint result;
		glGetProgramiv(shader, GL_LINK_STATUS, &result);
		if (result == GL_FALSE)
		{
			GLint length;
			glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &length);
			if (length)
			{
				std::string message;
				message.resize(length);
				glGetProgramInfoLog(shader, length, &length, message.data());
				glDeleteProgram(shader);
				_OLY_ENGINE_LOG_ERROR("GRAPHICS") << "Shader linkage failed - " << message << LOG.nl;
				throw Error(ErrorCode::SHADER_LINKAGE);
			}
			else
			{
				glDeleteProgram(shader);
				_OLY_ENGINE_LOG_ERROR("GRAPHICS") << "Shader linkage failed - no info log provided" << LOG.nl;
				throw Error(ErrorCode::SHADER_LINKAGE);
			}
		}
	}

	static GLuint create_shader(const std::vector<ShaderBufferSource>& buffer_sources)
	{
		GLuint shader = glCreateProgram();

		std::vector<GLint> subshaders;
		subshaders.reserve(buffer_sources.size());
		for (const ShaderBufferSource& buffer_source : buffer_sources)
			subshaders.push_back(create_compiled_subshader(buffer_source.buffer, (GLenum)buffer_source.type));

		for (GLuint subshader : subshaders)
			glAttachShader(shader, subshader);

		glLinkProgram(shader);

		for (GLuint subshader : subshaders)
			glDeleteShader(subshader);

		validate_shader(shader);
		return shader;
	}

	static GLuint create_shader(const std::vector<ShaderPathSource>& path_sources)
	{
		std::vector<ShaderBufferSource> buffer_sources;
		buffer_sources.reserve(path_sources.size());
		for (const ShaderPathSource& path_source : path_sources)
			buffer_sources.push_back({ .buffer = io::read_file(path_source.path), .type = path_source.type });
		return create_shader(buffer_sources);
	}

	Shader::Shader()
		: id(glCreateProgram())
	{
	}

	Shader::Shader(const std::vector<ShaderBufferSource>& buffer_sources)
		: id(create_shader(buffer_sources))
	{
	}

	Shader::Shader(const std::vector<ShaderPathSource>& path_sources)
		: id(create_shader(path_sources))
	{
	}

	Shader::Shader(Shader&& other) noexcept
		: id(other.id)
	{
		other.id = 0;
	}

	Shader::~Shader()
	{
		glDeleteProgram(id);
	}

	Shader& Shader::operator=(Shader&& other) noexcept
	{
		if (this != &other)
		{
			glDeleteProgram(id);
			id = other.id;
			other.id = 0;
		}
		return *this;
	}

	void dispatch_compute_workers(GLuint x_workers, GLuint y_workers, GLuint z_workers)
	{
		glDispatchCompute(x_workers, y_workers, z_workers);
	}

	void dispatch_compute_workers(GLuint x_size, GLuint y_size, GLuint z_size, GLuint x_threads, GLuint y_threads, GLuint z_threads)
	{
		glDispatchCompute(
			x_threads > 0 ? (GLuint)ceilf((float)x_size / x_threads) : 1,
			y_threads > 0 ? (GLuint)ceilf((float)y_size / y_threads) : 1,
			z_threads > 0 ? (GLuint)ceilf((float)z_size / z_threads) : 1
		);
	}

	void memory_barrier(MemoryBarrierBit barriers)
	{
		glMemoryBarrier((GLbitfield)barriers);
	}
}
