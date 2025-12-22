#pragma once

#include "external/GL.h"
#include "external/GLM.h"

namespace oly::particles::internal
{
	struct Particle
	{
		float timeElapsed;
		float lifetime;
		GLuint attached;
		glm::mat3 localTransform;
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
			UNIFORM = 0,
			TILTED = 1
		} type = Type::UNIFORM;

		float params[1] = { 0.0f };
	};

	struct alignas(16) Domain1D
	{
		enum Type : GLuint
		{
			CONSTANT = 0,
			LINE = 1,
			BILINE = 2
		} type = Type::CONSTANT;

		float params[4] = { 0.0 };
	};

	struct alignas(16) Generator1D
	{
		Sampler1D sampler = {};
		Domain1D domain = {};
	};

	struct alignas(16) EmitterParams
	{
		GLuint max_particles = 2000;
		GLuint attached = false;

	private:
		float _pad0[2] = { 0.0f };

	public:
		Generator1D lifetime = {};

		glm::vec2 position = {}; // TODO v6 use generator

	private:
		float _pad1[2] = { 0.0f };

	public:
		Generator1D rotation = {};

		glm::vec2 size = { 10.0f, 10.0f }; // TODO v6 use generator
		glm::vec2 velocity = { 10.0f, 0.0f }; // TODO v6 use generator

		glm::vec4 color = { 1.0f, 0.0f, 0.0f, 1.0f }; // TODO v6 use generator
	};
}
