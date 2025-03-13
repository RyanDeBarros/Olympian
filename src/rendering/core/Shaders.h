#pragma once

#include <GL/glew.h>

#include <string>
#include <unordered_map>
#include <memory>

namespace oly
{
	namespace rendering
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
		struct glsl_source { std::string source; glsl_source(std::string&& source) : source(source) {} };

		std::shared_ptr<Shader> load_shader(vertex_path vertex_shader, fragment_path fragment_shader);
		std::shared_ptr<Shader> load_shader(vertex_path vertex_shader, fragment_path fragment_shader, geometry_path geometry_shader);
		std::shared_ptr<Shader> load_shader(glsl_path glsl_shader);
		std::shared_ptr<Shader> load_shader(vertex_source vertex_shader, fragment_source fragment_shader);
		std::shared_ptr<Shader> load_shader(vertex_source vertex_shader, fragment_source fragment_shader, geometry_source geometry_shader);
		std::shared_ptr<Shader> load_shader(glsl_source glsl_shader);
	}
}
