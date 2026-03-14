#include "AttributeGenerator.h"

#include "graphics/particles/ShaderStructs.h"
#include "graphics/particles/implementations/Spawners.h"
#include "core/util/StringParam.h"

namespace oly::particles
{
	// TODO v7 use this enum pattern preferably over string enums in loaders
	namespace internal
	{
		enum ParticleSpawnerTypeHash
		{
			ConstantParticleSpawner = 1,
			BurstParticleSpawner = 2
		};

#define _OLY_EXPAND_PARTICLE_SPAWNER_TYPE_HASH(M)\
		M(ConstantParticleSpawner)\
		M(BurstParticleSpawner)
	}

	void IParticleSpawner::overload(Polymorphic<IParticleSpawner>& spawner, TOMLNode node)
	{
#define _OLY_ENUM_CASE(Name)\
		case internal::ParticleSpawnerTypeHash::Name:\
			if (!spawner.castable<Name>()) spawner = make_polymorphic<Name>();\
			break;

		switch (node["class"].value_or<int64_t>(0))
		{
			_OLY_EXPAND_PARTICLE_SPAWNER_TYPE_HASH(_OLY_ENUM_CASE)
		}

#undef _OLY_ENUM_CASE
#undef _OLY_EXPAND_PARTICLE_SPAWNER_TYPE_HASH

		if (spawner)
			spawner->overload(node);
	}

	void ISampler1D::overload(Polymorphic<ISampler1D>& sampler, TOMLNode node)
	{
		// TODO v6
	}

	void IDomain1D::overload(Polymorphic<IDomain1D>& domain, TOMLNode node)
	{
		// TODO v6
	}

	void AttributeGenerator1D::apply(internal::Generator1D& generator) const
	{
		sampler->apply(generator.sampler);
		domain->apply(generator.domain);
	}

	void ISampler2D::overload(Polymorphic<ISampler2D>& sampler, TOMLNode node)
	{
		// TODO v6
	}

	void IDomain2D::overload(Polymorphic<IDomain2D>& domain, TOMLNode node)
	{
		// TODO v6
	}

	void AttributeGenerator2D::apply(internal::Generator2D& generator) const
	{
		sampler->apply(generator.sampler);
		domain->apply(generator.domain);
	}

	void ISampler3D::overload(Polymorphic<ISampler3D>& sampler, TOMLNode node)
	{
		// TODO v6
	}

	void IDomain3D::overload(Polymorphic<IDomain3D>& domain, TOMLNode node)
	{
		// TODO v6
	}

	void AttributeGenerator3D::apply(internal::Generator3D& generator) const
	{
		sampler->apply(generator.sampler);
		domain->apply(generator.domain);
	}

	void ISampler4D::overload(Polymorphic<ISampler4D>& sampler, TOMLNode node)
	{
		// TODO v6
	}

	void IDomain4D::overload(Polymorphic<IDomain4D>& domain, TOMLNode node)
	{
		// TODO v6
	}

	void AttributeGenerator4D::apply(internal::Generator4D& generator) const
	{
		sampler->apply(generator.sampler);
		domain->apply(generator.domain);
	}
}

