#pragma once

#include <string>
#include <memory>

#include "Buffers.h"

namespace oly::graphics
{
	class Shader
	{
		GLuint id;

	public:
		Shader();
		Shader(const Shader&) = delete;
		Shader(Shader&&) noexcept;
		~Shader();
		Shader& operator=(Shader&&) noexcept;

		operator GLuint () const { return id; }
	};

	struct vertex_path { const char* path; inline vertex_path(const char* path) : path(path) {} };
	struct fragment_path { const char* path; inline fragment_path(const char* path) : path(path) {} };
	struct geometry_path { const char* path; inline geometry_path(const char* path) : path(path) {} };
	struct glsl_path { const char* path; inline glsl_path(const char* path) : path(path) {} };
	struct vertex_source { const char* source; GLint length; vertex_source(const char* source, GLint length) : source(source), length(length) {} };
	struct fragment_source { const char* source; GLint length; fragment_source(const char* source, GLint length) : source(source), length(length) {} };
	struct geometry_source { const char* source; GLint length; geometry_source(const char* source, GLint length) : source(source), length(length) {} };
	struct glsl_source { std::string source; glsl_source(std::string&& source) : source(std::move(source)) {} };

	extern std::unique_ptr<Shader> load_shader(vertex_path vertex_shader, fragment_path fragment_shader);
	extern std::unique_ptr<Shader> load_shader(vertex_path vertex_shader, fragment_path fragment_shader, geometry_path geometry_shader);
	extern std::unique_ptr<Shader> load_shader(glsl_path glsl_shader);
	extern std::unique_ptr<Shader> load_shader(vertex_source vertex_shader, fragment_source fragment_shader);
	extern std::unique_ptr<Shader> load_shader(vertex_source vertex_shader, fragment_source fragment_shader, geometry_source geometry_shader);
	extern std::unique_ptr<Shader> load_shader(glsl_source glsl_shader);
}
