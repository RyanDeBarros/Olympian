#include "AttributeGenerator.h"

#include "graphics/particles/ShaderStructs.h"
#include "graphics/particles/implementations/Spawners.h"
#include "core/util/StringParam.h"

namespace oly::particles
{
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

		if (spawner)
			spawner->overload(node);
	}

	Polymorphic<IParticleSpawner> IParticleSpawner::load(TOMLNode node)
	{
		Polymorphic<IParticleSpawner> spawner = nullptr;
		overload(spawner, node);
		return spawner;
	}

	void AttributeGenerator1D::apply(internal::Generator1D& generator) const
	{
		sampler->apply(generator.sampler);
		domain->apply(generator.domain);
	}

	void AttributeGenerator2D::apply(internal::Generator2D& generator) const
	{
		sampler->apply(generator.sampler);
		domain->apply(generator.domain);
	}

	void AttributeGenerator3D::apply(internal::Generator3D& generator) const
	{
		sampler->apply(generator.sampler);
		domain->apply(generator.domain);
	}

	void AttributeGenerator4D::apply(internal::Generator4D& generator) const
	{
		sampler->apply(generator.sampler);
		domain->apply(generator.domain);
	}
}

#undef _OLY_EXPAND_PARTICLE_SPAWNER_TYPE_HASH
