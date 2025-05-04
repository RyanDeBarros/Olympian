#include "Shaders.h"

#include <sstream>

#include "core/base/Errors.h"
#include "core/util/IO.h"

namespace oly::graphics
{
	Shader::Shader()
		: id(glCreateProgram())
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
				throw oly::Error(oly::ErrorCode::SUBSHADER_COMPILATION, "Subshader compilation failed - " + message);
			}
			else
			{
				glDeleteShader(subshader);
				subshader = 0;
				throw oly::Error(oly::ErrorCode::SUBSHADER_COMPILATION, "Subshader compilation failed - no info log provided");
			}
		}
	}

	static GLuint create_compiled_subshader(const char* source, GLint source_length, GLenum type)
	{
		GLuint subshader = glCreateShader(type);
		glShaderSource(subshader, 1, &source, &source_length);
		glCompileShader(subshader);

#ifdef OLYMPIAN_SUBSHADER_VALIDATION
		validate_subshader(subshader);
#endif

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
				throw oly::Error(oly::ErrorCode::SHADER_LINKAGE, "Shader linkage failed - " + message);
			}
			else
			{
				glDeleteProgram(shader);
				throw oly::Error(oly::ErrorCode::SHADER_LINKAGE, "Shader linkage failed - no info log provided");
			}
		}
	}

	static std::unique_ptr<Shader> create_linked_shader(GLuint vert, GLuint frag)
	{
		std::unique_ptr<Shader> shader = std::make_unique<Shader>();
		glAttachShader(*shader, vert);
		glAttachShader(*shader, frag);
		glLinkProgram(*shader);
		glDeleteShader(vert);
		glDeleteShader(frag);

#ifdef OLYMPIAN_SHADER_VALIDATION
		validate_shader(*shader);
#endif

		return shader;
	}

	static std::unique_ptr<Shader> create_linked_shader(GLuint vert, GLuint frag, GLuint geom)
	{
		std::unique_ptr<Shader> shader = std::make_unique<Shader>();
		glAttachShader(*shader, vert);
		glAttachShader(*shader, frag);
		glAttachShader(*shader, geom);
		glLinkProgram(*shader);
		glDeleteShader(vert);
		glDeleteShader(frag);
		glDeleteShader(geom);

#ifdef OLYMPIAN_SHADER_VALIDATION
		validate_shader(*shader);
#endif

		return shader;
	}

	struct parsed_glsl_source
	{
		std::string vertex, fragment, geometry;
	};

	static int parse_glsl_source(const std::string& glsl, parsed_glsl_source& full_source)
	{
		full_source.vertex.clear();
		full_source.fragment.clear();
		full_source.geometry.clear();

		std::istringstream stream(glsl);
		std::string line;

		std::string source;
		enum
		{
			V, F, G, NONE
		} type = NONE;

		while (std::getline(stream, line))
		{
			if (line.find("***vert***") != std::string::npos || line.find("***VERT***") != std::string::npos)
			{
				if (!full_source.vertex.empty())
					return 1;
				switch (type)
				{
				case F:
					full_source.fragment = std::move(source);
					break;
				case G:
					full_source.geometry = std::move(source);
					break;
				}
				type = V;
				source.clear();
			}
			else if (line.find("***frag***") != std::string::npos || line.find("***FRAG***") != std::string::npos)
			{
				if (!full_source.fragment.empty())
					return 2;
				switch (type)
				{
				case V:
					full_source.vertex = std::move(source);
					break;
				case G:
					full_source.geometry = std::move(source);
					break;
				}
				type = F;
				source.clear();
			}
			else if (line.find("***geom***") != std::string::npos || line.find("***GEOM***") != std::string::npos)
			{
				if (!full_source.geometry.empty())
					return 3;
				switch (type)
				{
				case V:
					full_source.vertex = std::move(source);
					break;
				case F:
					full_source.fragment = std::move(source);
					break;
				}
				type = G;
				source.clear();
			}
			else
				source += line + "\n";
		}
		switch (type)
		{
		case V:
			full_source.vertex = std::move(source);
			break;
		case F:
			full_source.fragment = std::move(source);
			break;
		case G:
			full_source.geometry = std::move(source);
			break;
		}

		return 0;
	}

	std::unique_ptr<Shader> load_shader(vertex_path vertex_shader, fragment_path fragment_shader)
	{
		std::string file_content;
		file_content = io::read_file(vertex_shader.path);
		GLuint vert = create_compiled_subshader(file_content.c_str(), (GLint)file_content.length(), GL_VERTEX_SHADER);
		file_content = io::read_file(fragment_shader.path);
		GLuint frag = create_compiled_subshader(file_content.c_str(), (GLint)file_content.length(), GL_FRAGMENT_SHADER);

		return create_linked_shader(vert, frag);
	}

	std::unique_ptr<Shader> load_shader(vertex_path vertex_shader, fragment_path fragment_shader, geometry_path geometry_shader)
	{
		std::string file_content;
		file_content = io::read_file(vertex_shader.path);
		GLuint vert = create_compiled_subshader(file_content.c_str(), (GLint)file_content.length(), GL_VERTEX_SHADER);
		file_content = io::read_file(fragment_shader.path);
		GLuint frag = create_compiled_subshader(file_content.c_str(), (GLint)file_content.length(), GL_FRAGMENT_SHADER);
		file_content = io::read_file(geometry_shader.path);
		GLuint geom = create_compiled_subshader(file_content.c_str(), (GLint)file_content.length(), GL_GEOMETRY_SHADER);

		return create_linked_shader(vert, frag, geom);
	}

	std::unique_ptr<Shader> load_shader(glsl_path glsl_shader)
	{
		std::string file_content = io::read_file(glsl_shader.path);
		parsed_glsl_source source;
		int res = parse_glsl_source(file_content, source);
		if (res == 1)
			throw Error(ErrorCode::SUBSHADER_COMPILATION, "repeated vertex subshader in GLSL file");
		else if (res == 2)
			throw Error(ErrorCode::SUBSHADER_COMPILATION, "repeated fragment subshader in GLSL file");
		else if (res == 3)
			throw Error(ErrorCode::SUBSHADER_COMPILATION, "repeated geometry subshader in GLSL file");
		else if (source.vertex.empty() || source.fragment.empty())
			throw Error(ErrorCode::SHADER_LINKAGE, "no vertex or fragment subshader exists in GLSL file");

		GLuint vert = create_compiled_subshader(source.vertex.c_str(), (GLint)source.vertex.length(), GL_VERTEX_SHADER);
		GLuint frag = create_compiled_subshader(source.fragment.c_str(), (GLint)source.fragment.length(), GL_FRAGMENT_SHADER);
		if (source.geometry.empty())
			return create_linked_shader(vert, frag);
		else
		{
			GLuint geom = create_compiled_subshader(source.geometry.c_str(), (GLint)source.geometry.length(), GL_GEOMETRY_SHADER);
			return create_linked_shader(vert, frag, geom);
		}
	}

	std::unique_ptr<Shader> load_shader(vertex_source vertex_shader, fragment_source fragment_shader)
	{
		GLuint vert = create_compiled_subshader(vertex_shader.source, vertex_shader.length, GL_VERTEX_SHADER);
		GLuint frag = create_compiled_subshader(fragment_shader.source, fragment_shader.length, GL_FRAGMENT_SHADER);
		return create_linked_shader(vert, frag);
	}

	std::unique_ptr<Shader> load_shader(vertex_source vertex_shader, fragment_source fragment_shader, geometry_source geometry_shader)
	{
		GLuint vert = create_compiled_subshader(vertex_shader.source, vertex_shader.length, GL_VERTEX_SHADER);
		GLuint frag = create_compiled_subshader(fragment_shader.source, fragment_shader.length, GL_FRAGMENT_SHADER);
		GLuint geom = create_compiled_subshader(geometry_shader.source, geometry_shader.length, GL_GEOMETRY_SHADER);
		return create_linked_shader(vert, frag, geom);
	}

	std::unique_ptr<Shader> load_shader(glsl_source glsl_shader)
	{
		parsed_glsl_source source;
		int res = parse_glsl_source(glsl_shader.source, source);
		if (res == 1)
			throw Error(ErrorCode::SUBSHADER_COMPILATION, "repeated vertex subshader in GLSL source");
		else if (res == 2)
			throw Error(ErrorCode::SUBSHADER_COMPILATION, "repeated fragment subshader in GLSL source");
		else if (res == 3)
			throw Error(ErrorCode::SUBSHADER_COMPILATION, "repeated geometry subshader in GLSL source");
		else if (source.vertex.empty() || source.fragment.empty())
			throw Error(ErrorCode::SHADER_LINKAGE, "no vertex or fragment subshader exists in GLSL file");

		GLuint vert = create_compiled_subshader(source.vertex.c_str(), (GLint)source.vertex.length(), GL_VERTEX_SHADER);
		GLuint frag = create_compiled_subshader(source.fragment.c_str(), (GLint)source.fragment.length(), GL_FRAGMENT_SHADER);
		if (source.geometry.empty())
			return create_linked_shader(vert, frag);
		else
		{
			GLuint geom = create_compiled_subshader(source.geometry.c_str(), (GLint)source.geometry.length(), GL_GEOMETRY_SHADER);
			return create_linked_shader(vert, frag, geom);
		}
	}
}
