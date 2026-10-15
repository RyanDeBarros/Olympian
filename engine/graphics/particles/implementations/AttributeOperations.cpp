#include "AttributeOperations.h"

#include "graphics/particles/ParticleEmitter.h"
#include "core/base/UnitVector.h"

#include "definitions/Keys.h"

namespace oly::particles::ops
{
    SineWave1D::SineWave1D(float a, float b, float k, float c)
        : a(a)
        , b(b)
        , k(k)
        , c(c)
    {
    }

	void SineWave1D::op(const ParticleEmitter& emitter, AttributeSpan attribute) const
	{
		attribute = a * glm::sin(b * emitter.time_elapsed() - k) + c;
	}

	imp::poly<SineWave1D> SineWave1D::load(TOMLNode node)
	{
		assets::Parser parser(node);
		float a = parser.defaulted(detail::Key::A)(1.0f);
		float b = parser.defaulted(detail::Key::B)(1.0f);
		float k = parser.defaulted(detail::Key::K)(0.0f);
		float c = parser.defaulted(detail::Key::C)(0.0f);
		return imp::make_poly<SineWave1D>(a, b, k, c);
	}

    Polarization2D::Polarization2D(float amplitude, float time_offset)
        : amplitude(amplitude)
        , time_offset(time_offset)
    {
    }
	
    void Polarization2D::op(const ParticleEmitter& emitter, AttributeSpan attribute) const
	{
		attribute = amplitude * (glm::vec2)UnitVector2D(emitter.time_elapsed() + time_offset);
	}

	imp::poly<Polarization2D> Polarization2D::load(TOMLNode node)
	{
		assets::Parser parser(node);
		float amplitude = parser.defaulted(detail::Key::Amplitude)(1.0f);
		float time_offset = parser.defaulted(detail::Key::TimeOffset)(0.0f);
		return imp::make_poly<Polarization2D>(amplitude, time_offset);
	}
}
