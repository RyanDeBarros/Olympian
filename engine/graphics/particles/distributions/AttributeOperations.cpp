#include "AttributeOperations.h"

#include "graphics/particles/ParticleEmitter.h"
#include "core/base/UnitVector.h"

namespace oly::particles::operations
{
	void SineWave1D::op(const ParticleEmitter& emitter, AttributeSpan attribute) const
	{
		attribute = a * glm::sin(b * emitter.time_elapsed() - k) + c;
	}

	Polymorphic<SineWave1D> SineWave1D::load(TOMLNode node)
	{
		float a = io::parse_float_or(node["a"], 1.0f);
		float b = io::parse_float_or(node["b"], 1.0f);
		float k = io::parse_float_or(node["k"], 0.0f);
		float c = io::parse_float_or(node["c"], 0.0f);
		return make_polymorphic<SineWave1D>(a, b, k, c);
	}

	void Polarization2D::op(const ParticleEmitter& emitter, AttributeSpan attribute) const
	{
		attribute = amplitude * (glm::vec2)UnitVector2D(emitter.time_elapsed() + time_offset);
	}

	Polymorphic<Polarization2D> Polarization2D::load(TOMLNode node)
	{
		float amplitude = io::parse_float_or(node["amplitude"], 1.0f);
		float time_offset = io::parse_float_or(node["time_offset"], 0.0f);
		return make_polymorphic<Polarization2D>(amplitude, time_offset);
	}
}
