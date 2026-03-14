#include "AttributeGenerator.h"

#include "graphics/particles/ShaderStructs.h"
#include "graphics/particles/implementations/Spawners.h"
#include "graphics/particles/implementations/Samplers.h"
#include "graphics/particles/implementations/Domains.h"
#include "core/util/StringParam.h"

namespace oly::particles
{
	void IParticleSpawner::overload(Polymorphic<IParticleSpawner>& spawner, TOMLNode node)
	{
		std::string klass = node["klass"].value_or<>("");

		// TODO v7 use polyklass macros more often
		_OLY_POLYKLASS_CASES_BEGIN(spawner)
			_OLY_POLYKLASS_IF_CASE(ConstantParticleSpawner)
			_OLY_POLYKLASS_ELSE_IF_CASE(BurstParticleSpawner)
			_OLY_POLYKLASS_CASES_END;

		if (spawner)
			spawner->overload(node);
	}

	void ISampler1D::overload(Polymorphic<ISampler1D>& sampler, TOMLNode node)
	{
		std::string klass = node["klass"].value_or<>("");

		_OLY_POLYKLASS_CASES_BEGIN(sampler)
			_OLY_POLYKLASS_IF_CASE(UniformSampler1D)
			_OLY_POLYKLASS_ELSE_IF_CASE(TiltedSampler1D)
			_OLY_POLYKLASS_CASES_END;

		if (sampler)
			sampler->overload(node);
	}

	void IDomain1D::overload(Polymorphic<IDomain1D>& domain, TOMLNode node)
	{
		std::string klass = node["klass"].value_or<>("");

		_OLY_POLYKLASS_CASES_BEGIN(domain)
			_OLY_POLYKLASS_IF_CASE(ConstantDomain1D)
			_OLY_POLYKLASS_ELSE_IF_CASE(LineDomain1D)
			_OLY_POLYKLASS_ELSE_IF_CASE(BiLineDomain1D)
			_OLY_POLYKLASS_CASES_END;

		if (domain)
			domain->overload(node);
	}

	void AttributeGenerator1D::apply(internal::Generator1D& generator) const
	{
		sampler->apply(generator.sampler);
		domain->apply(generator.domain);
	}

	void ISampler2D::overload(Polymorphic<ISampler2D>& sampler, TOMLNode node)
	{
		std::string klass = node["klass"].value_or<>("");

		_OLY_POLYKLASS_CASES_BEGIN(sampler)
			_OLY_POLYKLASS_IF_CASE(UniformSampler2D)
			_OLY_POLYKLASS_CASES_END;

		if (sampler)
			sampler->overload(node);
	}

	void IDomain2D::overload(Polymorphic<IDomain2D>& domain, TOMLNode node)
	{
		std::string klass = node["klass"].value_or<>("");

		_OLY_POLYKLASS_CASES_BEGIN(domain)
			_OLY_POLYKLASS_IF_CASE(ConstantDomain2D)
			_OLY_POLYKLASS_CASES_END;

		if (domain)
			domain->overload(node);
	}

	void AttributeGenerator2D::apply(internal::Generator2D& generator) const
	{
		sampler->apply(generator.sampler);
		domain->apply(generator.domain);
	}

	void ISampler3D::overload(Polymorphic<ISampler3D>& sampler, TOMLNode node)
	{
		std::string klass = node["klass"].value_or<>("");

		_OLY_POLYKLASS_CASES_BEGIN(sampler)
			_OLY_POLYKLASS_IF_CASE(UniformSampler3D)
			_OLY_POLYKLASS_CASES_END;

		if (sampler)
			sampler->overload(node);
	}

	void IDomain3D::overload(Polymorphic<IDomain3D>& domain, TOMLNode node)
	{
		std::string klass = node["klass"].value_or<>("");

		_OLY_POLYKLASS_CASES_BEGIN(domain)
			_OLY_POLYKLASS_IF_CASE(ConstantDomain3D)
			_OLY_POLYKLASS_CASES_END;

		if (domain)
			domain->overload(node);
	}

	void AttributeGenerator3D::apply(internal::Generator3D& generator) const
	{
		sampler->apply(generator.sampler);
		domain->apply(generator.domain);
	}

	void ISampler4D::overload(Polymorphic<ISampler4D>& sampler, TOMLNode node)
	{
		std::string klass = node["klass"].value_or<>("");

		_OLY_POLYKLASS_CASES_BEGIN(sampler)
			_OLY_POLYKLASS_IF_CASE(UniformSampler4D)
			_OLY_POLYKLASS_CASES_END;

		if (sampler)
			sampler->overload(node);
	}

	void IDomain4D::overload(Polymorphic<IDomain4D>& domain, TOMLNode node)
	{
		std::string klass = node["klass"].value_or<>("");

		_OLY_POLYKLASS_CASES_BEGIN(domain)
			_OLY_POLYKLASS_IF_CASE(ConstantDomain4D)
			_OLY_POLYKLASS_CASES_END;

		if (domain)
			domain->overload(node);
	}

	void AttributeGenerator4D::apply(internal::Generator4D& generator) const
	{
		sampler->apply(generator.sampler);
		domain->apply(generator.domain);
	}
}

