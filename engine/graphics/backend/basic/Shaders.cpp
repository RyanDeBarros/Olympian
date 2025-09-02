#include "Shaders.h"

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
				OLY_LOG_ERROR(true, "GRAPHICS") << LOG.source_info.full_source() << "Subshader compilation failed - " << message << LOG.nl;
				throw Error(ErrorCode::SUBSHADER_COMPILATION);
			}
			else
			{
				glDeleteShader(subshader);
				subshader = 0;
				OLY_LOG_ERROR(true, "GRAPHICS") << LOG.source_info.full_source() << "Subshader compilation failed - no info log provided" << LOG.nl;
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
				OLY_LOG_ERROR(true, "GRAPHICS") << LOG.source_info.full_source() << "Shader linkage failed - " << message << LOG.nl;
				throw Error(ErrorCode::SHADER_LINKAGE);
			}
			else
			{
				glDeleteProgram(shader);
				OLY_LOG_ERROR(true, "GRAPHICS") << LOG.source_info.full_source() << "Shader linkage failed - no info log provided" << LOG.nl;
				throw Error(ErrorCode::SHADER_LINKAGE);
			}
		}
	}

	static GLuint create_shader(const ShaderBufferSource& buffer_source)
	{
		GLuint shader = glCreateProgram();
		GLuint vert = create_compiled_subshader(buffer_source.vertex_buffer, GL_VERTEX_SHADER);
		GLuint frag = create_compiled_subshader(buffer_source.fragment_buffer, GL_FRAGMENT_SHADER);
		GLuint geom = buffer_source.geometry_buffer ? create_compiled_subshader(*buffer_source.geometry_buffer, GL_GEOMETRY_SHADER) : 0;

		glAttachShader(shader, vert);
		glAttachShader(shader, frag);
		if (geom)
			glAttachShader(shader, geom);

		glLinkProgram(shader);

		glDeleteShader(vert);
		glDeleteShader(frag);
		if (geom)
			glDeleteShader(geom);

		validate_shader(shader);
		return shader;
	}

	static GLuint create_shader(const ShaderPathSource& path_source)
	{
		ShaderBufferSource buffer_source;
		buffer_source.vertex_buffer = io::read_file(path_source.vertex_path);
		buffer_source.fragment_buffer = io::read_file(path_source.fragment_path);
		if (path_source.geometry_path)
			buffer_source.geometry_buffer = io::read_file(*path_source.geometry_path);
		return create_shader(buffer_source);
	}

	Shader::Shader()
		: id(glCreateProgram())
	{
	}

	Shader::Shader(const ShaderBufferSource& buffer_source)
		: id(create_shader(buffer_source))
	{
	}

	Shader::Shader(const ShaderPathSource& path_source)
		: id(create_shader(path_source))
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
}
