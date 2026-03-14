#pragma once

#include "external/GL.h"
#include "external/GLM.h"

namespace oly::particles::internal
{
	struct Particle
	{
		float time_elapsed;
		float lifetime;
		GLuint attached;
		glm::mat3 local_transform;
		glm::vec4 color;
		glm::vec2 velocity;
	};

	struct ParticleSystemData
	{
		GLuint max_time_elapsed_bits;
	};

	struct alignas(16) Sampler1D
	{
		enum Type : GLuint
		{
			Uniform = 0,
			Tilted = 1
		} type = Type::Uniform;

		float params[1] = { 0.0f };
	};

	struct alignas(16) Domain1D
	{
		enum Type : GLuint
		{
			Constant = 0,
			Line = 1,
			BiLine = 2
		} type = Type::Constant;

		float params[3] = { 0.0 };
	};

	struct alignas(16) Generator1D
	{
		Sampler1D sampler = {};
		Domain1D domain = {};
	};

	struct alignas(16) Sampler2D
	{
		enum Type : GLuint
		{
			Uniform = 0
		} type = Type::Uniform;

		float params[2] = { 0.0f };
	};

	struct alignas(16) Domain2D
	{
		enum Type : GLuint
		{
			Constant = 0
		} type = Type::Constant;

		float params[6] = { 0.0 };
	};

	struct alignas(16) Generator2D
	{
		Sampler2D sampler = {};
		Domain2D domain = {};
	};

	struct alignas(16) Sampler3D
	{
		enum Type : GLuint
		{
			Uniform = 0
		} type = Type::Uniform;

		float params[3] = { 0.0f };
	};

	struct alignas(16) Domain3D
	{
		enum Type : GLuint
		{
			Constant = 0
		} type = Type::Constant;

		float params[9] = { 0.0 };
	};

	struct alignas(16) Generator3D
	{
		Sampler3D sampler = {};
		Domain3D domain = {};
	};

	struct alignas(16) Sampler4D
	{
		enum Type : GLuint
		{
			Uniform = 0
		} type = Type::Uniform;

		float params[4] = { 0.0f };
	};

	struct alignas(16) Domain4D
	{
		enum Type : GLuint
		{
			Constant = 0
		} type = Type::Constant;

		float params[12] = { 0.0 };
	};

	struct alignas(16) Generator4D
	{
		Sampler4D sampler = {};
		Domain4D domain = {};
	};

	struct alignas(16) EmitterParams
	{
		GLuint max_particles = 2000;
		GLuint attached = false;

	private:
		float _pad[2] = { 0.0f };

	public:
		Generator1D lifetime = {};
		Generator2D position = {};
		Generator1D rotation = {};
		Generator2D size = {};
		Generator2D velocity = {};
		Generator4D color = {};
	};
}
