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
		static Polymorphic<IParticleSpawner> load(TOMLNode node);
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
	};

	struct IDomain1D
	{
		virtual ~IDomain1D() = default;
		virtual void apply(internal::Domain1D&) const = 0;
		virtual void on_tick(const ParticleEmitter&) = 0;
		virtual void overload(TOMLNode node) {}

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(IDomain1D);
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
			// TODO v6 load Polymorphic<ISampler1D>/Polymorphic<IDomain1D>
			sampler->overload(node["sampler"]);
			domain->overload(node["domain"]);
		}
	};

	struct ISampler2D
	{
		virtual ~ISampler2D() = default;
		virtual void apply(internal::Sampler2D&) const = 0;
		virtual void on_tick(const ParticleEmitter&) = 0;
		virtual void overload(TOMLNode node) {}

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(ISampler2D);
	};

	struct IDomain2D
	{
		virtual ~IDomain2D() = default;
		virtual void apply(internal::Domain2D&) const = 0;
		virtual void on_tick(const ParticleEmitter&) = 0;
		virtual void overload(TOMLNode node) {}

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(IDomain2D);
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
			// TODO v6 load Polymorphic<ISampler2D>/Polymorphic<IDomain2D>
			sampler->overload(node["sampler"]);
			domain->overload(node["domain"]);
		}
	};

	struct ISampler3D
	{
		virtual ~ISampler3D() = default;
		virtual void apply(internal::Sampler3D&) const = 0;
		virtual void on_tick(const ParticleEmitter&) = 0;
		virtual void overload(TOMLNode node) {}

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(ISampler3D);
	};

	struct IDomain3D
	{
		virtual ~IDomain3D() = default;
		virtual void apply(internal::Domain3D&) const = 0;
		virtual void on_tick(const ParticleEmitter&) = 0;
		virtual void overload(TOMLNode node) {}

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(IDomain3D);
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
			// TODO v6 load Polymorphic<ISampler3D>/Polymorphic<IDomain3D>
			sampler->overload(node["sampler"]);
			domain->overload(node["domain"]);
		}
	};

	struct ISampler4D
	{
		virtual ~ISampler4D() = default;
		virtual void apply(internal::Sampler4D&) const = 0;
		virtual void on_tick(const ParticleEmitter&) = 0;
		virtual void overload(TOMLNode node) {}

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(ISampler4D);
	};

	struct IDomain4D
	{
		virtual ~IDomain4D() = default;
		virtual void apply(internal::Domain4D&) const = 0;
		virtual void on_tick(const ParticleEmitter&) = 0;
		virtual void overload(TOMLNode node) {}

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(IDomain4D);
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
			// TODO v6 load Polymorphic<ISampler4D>/Polymorphic<IDomain4D>
			sampler->overload(node["sampler"]);
			domain->overload(node["domain"]);
		}
	};
}
