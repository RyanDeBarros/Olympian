#include "AttributeOperations.h"

namespace oly::particles
{
	float GenericAttributeOperation1D::op(const ParticleEmitter& emitter, float attribute) const
	{
		return fn(emitter, attribute);
	}

	float SineAttributeOperation1D::op(const ParticleEmitter& emitter, float attribute) const
	{
		return a * glm::sin(b * emitter.time_elapsed() - k) + c;
	}

	glm::vec2 GenericAttributeOperation2D::op(const ParticleEmitter& emitter, glm::vec2 attribute) const
	{
		return fn(emitter, attribute);
	}

	glm::vec3 GenericAttributeOperation3D::op(const ParticleEmitter& emitter, glm::vec3 attribute) const
	{
		return fn(emitter, attribute);
	}

	glm::vec4 GenericAttributeOperation4D::op(const ParticleEmitter& emitter, glm::vec4 attribute) const
	{
		return fn(emitter, attribute);
	}
}
