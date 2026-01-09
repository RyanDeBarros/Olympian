#include "AttributeGenerator.h"

#include "graphics/particles/ShaderStructs.h"
#include "graphics/particles/distributions/Spawners.h"
#include "core/algorithms/STLUtils.h"

namespace oly::particles
{
	void IParticleSpawner::overload(Polymorphic<IParticleSpawner>& spawner, TOMLNode node)
	{
		std::string type = algo::to_lower(node["class"].value_or<std::string>(""));
		static auto initialize = []<typename T>(Polymorphic<IParticleSpawner>&spawner) { if (!spawner.castable<T>()) spawner = make_polymorphic<T>(); };

		// TODO v6 use enum instead of string name for class types for all particle system loading
		if (type == "constantparticlespawner")
			initialize.template operator()<ConstantParticleSpawner>(spawner);
		else if (type == "burstparticlespawner")
			initialize.template operator()<BurstParticleSpawner>(spawner);

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
