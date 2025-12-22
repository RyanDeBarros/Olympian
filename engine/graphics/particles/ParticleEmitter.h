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

		AttributeGenerator1D lifetime;
		AttributeGenerator2D position;
		AttributeGenerator1D rotation;
		AttributeGenerator2D size;
		AttributeGenerator2D velocity;
		AttributeGenerator4D color;

	private:
		mutable float _spawn_debt = 0.0f;

	public:
		ParticleEmitter();

		void apply(internal::EmitterParams&) const;

		void on_tick(float delta_time) const;
		GLuint spawn_debt() const;
	};

}
