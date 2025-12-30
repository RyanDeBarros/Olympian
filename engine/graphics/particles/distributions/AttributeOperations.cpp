#include "AttributeOperations.h"

#include "graphics/particles/ParticleEmitter.h"
#include "core/base/UnitVector.h"

namespace oly::particles::operations
{
	void SineWave1D::op(const ParticleEmitter& emitter, AttributeSpan attribute) const
	{
		attribute = a * glm::sin(b * emitter.time_elapsed() - k) + c;
	}

	void Polarization2D::op(const ParticleEmitter& emitter, AttributeSpan attribute) const
	{
		attribute = amplitude * (glm::vec2)UnitVector2D(emitter.time_elapsed() + time_offset);
	}
}
