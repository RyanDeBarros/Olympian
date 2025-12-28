#include "AttributeOperations.h"

#include "graphics/particles/ParticleEmitter.h"

namespace oly::particles
{
	void SineAttributeOperation1D::op(const ParticleEmitter& emitter, float& attribute) const
	{
		attribute = a * glm::sin(b * emitter.time_elapsed() - k) + c;
	}
}
