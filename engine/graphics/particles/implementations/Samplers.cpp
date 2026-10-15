#include "Samplers.h"

#include "graphics/particles/ShaderStructs.h"

#include "definitions/Keys.h"

namespace oly::particles
{
	void UniformSampler1D::apply(internal::Sampler1D& sampler) const
	{
		sampler.type = internal::Sampler1D::Uniform;
	}

    TiltedSampler1D::TiltedSampler1D(float tilt)
        : tilt(tilt)
    {
    }

	void TiltedSampler1D::apply(internal::Sampler1D& sampler) const
	{
		sampler.type = internal::Sampler1D::Tilted;
		sampler.params[0] = tilt.value;
	}

    void TiltedSampler1D::on_tick(const ParticleEmitter& emitter)
    {
        tilt.on_tick(emitter);
    }

	void TiltedSampler1D::overload(TOMLNode node)
	{
		tilt.overload(assets::Parser(node).field(detail::Key::Tilt));
	}

	void UniformSampler2D::apply(internal::Sampler2D& sampler) const
	{
		sampler.type = internal::Sampler2D::Uniform;
	}

	void UniformSampler3D::apply(internal::Sampler3D& sampler) const
	{
		sampler.type = internal::Sampler3D::Uniform;
	}

	void UniformSampler4D::apply(internal::Sampler4D& sampler) const
	{
		sampler.type = internal::Sampler4D::Uniform;
	}
}
