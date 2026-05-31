#include "AttributeOperations.h"

#include "graphics/particles/ParticleEmitter.h"
#include "core/base/UnitVector.h"

#include "definitions/Keys.h"

namespace oly::particles::ops
{
	void SineWave1D::op(const ParticleEmitter& emitter, AttributeSpan attribute) const
	{
		attribute = a * glm::sin(b * emitter.time_elapsed() - k) + c;
	}

	Polymorphic<SineWave1D> SineWave1D::load(TOMLNode node)
	{
		assets::Parser parser(node);
		float a = parser.defaulted(detail::Key::A)(1.0f);
		float b = parser.defaulted(detail::Key::B)(1.0f);
		float k = parser.defaulted(detail::Key::K)(0.0f);
		float c = parser.defaulted(detail::Key::C)(0.0f);
		return make_polymorphic<SineWave1D>(a, b, k, c);
	}

	void Polarization2D::op(const ParticleEmitter& emitter, AttributeSpan attribute) const
	{
		attribute = amplitude * (glm::vec2)UnitVector2D(emitter.time_elapsed() + time_offset);
	}

	Polymorphic<Polarization2D> Polarization2D::load(TOMLNode node)
	{
		assets::Parser parser(node);
		float amplitude = parser.defaulted(detail::Key::Amplitude)(1.0f);
		float time_offset = parser.defaulted(detail::Key::TimeOffset)(0.0f);
		return make_polymorphic<Polarization2D>(amplitude, time_offset);
	}
}
