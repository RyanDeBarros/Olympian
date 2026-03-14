#pragma once

#include "core/types/Polymorphic.h"
#include "external/TOML.h"

namespace oly::particles
{
	struct ParticleEmitter;

	struct IParticleSpawner
	{
		virtual ~IParticleSpawner() = default;
		virtual float spawn_debt(float time, float delta_time, float period) const = 0;
		virtual void overload(TOMLNode node) {}

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(IParticleSpawner);

		static void overload(Polymorphic<IParticleSpawner>& spawner, TOMLNode node);

		static Polymorphic<IParticleSpawner> load(TOMLNode node)
		{
			Polymorphic<IParticleSpawner> spawner = nullptr;
			overload(spawner, node);
			return spawner;
		}
	};

	namespace internal
	{
		struct Sampler1D;
		struct Domain1D;
		struct Generator1D;
		struct Sampler2D;
		struct Domain2D;
		struct Generator2D;
		struct Sampler3D;
		struct Domain3D;
		struct Generator3D;
		struct Sampler4D;
		struct Domain4D;
		struct Generator4D;
	}

	struct ISampler1D
	{
		virtual ~ISampler1D() = default;
		virtual void apply(internal::Sampler1D&) const = 0;
		virtual void on_tick(const ParticleEmitter&) = 0;
		virtual void overload(TOMLNode node) {}

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(ISampler1D);

		static void overload(Polymorphic<ISampler1D>& sampler, TOMLNode node);
		
		static Polymorphic<ISampler1D> load(TOMLNode node)
		{
			Polymorphic<ISampler1D> sampler = nullptr;
			overload(sampler, node);
			return sampler;
		}
	};

	struct IDomain1D
	{
		virtual ~IDomain1D() = default;
		virtual void apply(internal::Domain1D&) const = 0;
		virtual void on_tick(const ParticleEmitter&) = 0;
		virtual void overload(TOMLNode node) {}

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(IDomain1D);

		static void overload(Polymorphic<IDomain1D>& domain, TOMLNode node);

		static Polymorphic<IDomain1D> load(TOMLNode node)
		{
			Polymorphic<IDomain1D> domain = nullptr;
			overload(domain, node);
			return domain;
		}
	};

	struct AttributeGenerator1D
	{
		Polymorphic<ISampler1D> sampler;
		Polymorphic<IDomain1D> domain;

		void apply(internal::Generator1D& generator) const;
		
		void on_tick(const ParticleEmitter& emitter)
		{
			sampler->on_tick(emitter);
			domain->on_tick(emitter);
		}

		void overload(TOMLNode node)
		{
			sampler->overload(node["sampler"]);
			domain->overload(node["domain"]);
		}

		static void overload(AttributeGenerator1D& generator, TOMLNode node)
		{
			generator.overload(node);
		}

		static AttributeGenerator1D load(TOMLNode node)
		{
			AttributeGenerator1D generator{ .sampler = nullptr, .domain = nullptr };
			overload(generator, node);
			return generator;
		}
	};

	struct ISampler2D
	{
		virtual ~ISampler2D() = default;
		virtual void apply(internal::Sampler2D&) const = 0;
		virtual void on_tick(const ParticleEmitter&) = 0;
		virtual void overload(TOMLNode node) {}

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(ISampler2D);

		static void overload(Polymorphic<ISampler2D>& sampler, TOMLNode node);

		static Polymorphic<ISampler2D> load(TOMLNode node)
		{
			Polymorphic<ISampler2D> sampler = nullptr;
			overload(sampler, node);
			return sampler;
		}
	};

	struct IDomain2D
	{
		virtual ~IDomain2D() = default;
		virtual void apply(internal::Domain2D&) const = 0;
		virtual void on_tick(const ParticleEmitter&) = 0;
		virtual void overload(TOMLNode node) {}

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(IDomain2D);

		static void overload(Polymorphic<IDomain2D>& domain, TOMLNode node);

		static Polymorphic<IDomain2D> load(TOMLNode node)
		{
			Polymorphic<IDomain2D> domain = nullptr;
			overload(domain, node);
			return domain;
		}
	};

	struct AttributeGenerator2D
	{
		Polymorphic<ISampler2D> sampler;
		Polymorphic<IDomain2D> domain;

		void apply(internal::Generator2D& generator) const;

		void on_tick(const ParticleEmitter& emitter)
		{
			sampler->on_tick(emitter);
			domain->on_tick(emitter);
		}

		void overload(TOMLNode node)
		{
			sampler->overload(node["sampler"]);
			domain->overload(node["domain"]);
		}

		static void overload(AttributeGenerator2D& generator, TOMLNode node)
		{
			generator.overload(node);
		}

		static AttributeGenerator2D load(TOMLNode node)
		{
			AttributeGenerator2D generator{ .sampler = nullptr, .domain = nullptr };
			overload(generator, node);
			return generator;
		}
	};

	struct ISampler3D
	{
		virtual ~ISampler3D() = default;
		virtual void apply(internal::Sampler3D&) const = 0;
		virtual void on_tick(const ParticleEmitter&) = 0;
		virtual void overload(TOMLNode node) {}

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(ISampler3D);

		static void overload(Polymorphic<ISampler3D>& sampler, TOMLNode node);

		static Polymorphic<ISampler3D> load(TOMLNode node)
		{
			Polymorphic<ISampler3D> sampler = nullptr;
			overload(sampler, node);
			return sampler;
		}
	};

	struct IDomain3D
	{
		virtual ~IDomain3D() = default;
		virtual void apply(internal::Domain3D&) const = 0;
		virtual void on_tick(const ParticleEmitter&) = 0;
		virtual void overload(TOMLNode node) {}

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(IDomain3D);

		static void overload(Polymorphic<IDomain3D>& domain, TOMLNode node);

		static Polymorphic<IDomain3D> load(TOMLNode node)
		{
			Polymorphic<IDomain3D> domain = nullptr;
			overload(domain, node);
			return domain;
		}
	};

	struct AttributeGenerator3D
	{
		Polymorphic<ISampler3D> sampler;
		Polymorphic<IDomain3D> domain;

		void apply(internal::Generator3D& generator) const;

		void on_tick(const ParticleEmitter& emitter)
		{
			sampler->on_tick(emitter);
			domain->on_tick(emitter);
		}

		void overload(TOMLNode node)
		{
			sampler->overload(node["sampler"]);
			domain->overload(node["domain"]);
		}

		static void overload(AttributeGenerator3D& generator, TOMLNode node)
		{
			generator.overload(node);
		}

		static AttributeGenerator3D load(TOMLNode node)
		{
			AttributeGenerator3D generator{ .sampler = nullptr, .domain = nullptr };
			overload(generator, node);
			return generator;
		}
	};

	struct ISampler4D
	{
		virtual ~ISampler4D() = default;
		virtual void apply(internal::Sampler4D&) const = 0;
		virtual void on_tick(const ParticleEmitter&) = 0;
		virtual void overload(TOMLNode node) {}

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(ISampler4D);

		static void overload(Polymorphic<ISampler4D>& sampler, TOMLNode node);

		static Polymorphic<ISampler4D> load(TOMLNode node)
		{
			Polymorphic<ISampler4D> sampler = nullptr;
			overload(sampler, node);
			return sampler;
		}
	};

	struct IDomain4D
	{
		virtual ~IDomain4D() = default;
		virtual void apply(internal::Domain4D&) const = 0;
		virtual void on_tick(const ParticleEmitter&) = 0;
		virtual void overload(TOMLNode node) {}

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(IDomain4D);

		static void overload(Polymorphic<IDomain4D>& domain, TOMLNode node);

		static Polymorphic<IDomain4D> load(TOMLNode node)
		{
			Polymorphic<IDomain4D> domain = nullptr;
			overload(domain, node);
			return domain;
		}
	};

	struct AttributeGenerator4D
	{
		Polymorphic<ISampler4D> sampler;
		Polymorphic<IDomain4D> domain;

		void apply(internal::Generator4D& generator) const;

		void on_tick(const ParticleEmitter& emitter)
		{
			sampler->on_tick(emitter);
			domain->on_tick(emitter);
		}

		void overload(TOMLNode node)
		{
			sampler->overload(node["sampler"]);
			domain->overload(node["domain"]);
		}

		static void overload(AttributeGenerator4D& generator, TOMLNode node)
		{
			generator.overload(node);
		}

		static AttributeGenerator4D load(TOMLNode node)
		{
			AttributeGenerator4D generator{ .sampler = nullptr, .domain = nullptr };
			overload(generator, node);
			return generator;
		}
	};
}
