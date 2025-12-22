#pragma once

#include "graphics/particles/AttributeGenerator.h"

#include "external/GL.h"
#include "external/GLM.h"

namespace oly::particles
{
	namespace internal
	{
		struct EmitterParams;
	}

	struct ParticleEmitter
	{
		GLuint max_particles;
		GLuint attached;

		AttributeGenerator lifetime;

		glm::vec2 position; // TODO v6 use generator
		float rotation; // TODO v6 use generator

		glm::vec2 size; // TODO v6 use generator
		glm::vec2 velocity; // TODO v6 use generator

		glm::vec4 color; // TODO v6 use generator

	private:
		mutable float _spawn_debt = 0.0f;

	public:
		ParticleEmitter();

		void apply(internal::EmitterParams&) const;

		void on_tick(float delta_time) const;
		GLuint spawn_debt() const;
	};

}
