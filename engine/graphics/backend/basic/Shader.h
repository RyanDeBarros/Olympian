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

	extern void dispatch_compute_workers(GLuint x_workers, GLuint y_workers, GLuint z_workers);
	extern void dispatch_compute_workers(GLuint x_size, GLuint y_size, GLuint z_size, GLuint x_threads, GLuint y_threads, GLuint z_threads);

	enum MemoryBarrierBit
	{
		VERTEX_ATTRIB_ARRAY = GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT,
		ELEMENT_ARRAY = GL_ELEMENT_ARRAY_BARRIER_BIT,
		UNIFORM = GL_UNIFORM_BARRIER_BIT,
		TEXTURE_FETCH = GL_TEXTURE_FETCH_BARRIER_BIT,
		SHADER_IMAGE_ACCESS = GL_SHADER_IMAGE_ACCESS_BARRIER_BIT,
		COMMAND = GL_COMMAND_BARRIER_BIT,
		PIXEL_BUFFER = GL_PIXEL_BUFFER_BARRIER_BIT,
		TEXTURE_UPDATE = GL_TEXTURE_UPDATE_BARRIER_BIT,
		BUFFER_UPDATE = GL_BUFFER_UPDATE_BARRIER_BIT,
		FRAMEBUFFER = GL_FRAMEBUFFER_BARRIER_BIT,
		TRANSFORM_FEEDBACK = GL_TRANSFORM_FEEDBACK_BARRIER_BIT,
		ATOMIC_COUNTER = GL_ATOMIC_COUNTER_BARRIER_BIT,
		SHADER_STORAGE = GL_SHADER_STORAGE_BARRIER_BIT,
		ALL = GL_ALL_BARRIER_BITS
	};

	inline MemoryBarrierBit operator|(MemoryBarrierBit a, MemoryBarrierBit b) { return MemoryBarrierBit((int)a | (int)b); }

	extern void memory_barrier(MemoryBarrierBit barriers);
}
