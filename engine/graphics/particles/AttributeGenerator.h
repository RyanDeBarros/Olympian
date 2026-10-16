#pragma once

#include "external/TOML.h"

#include <imp/polymorphic.hpp>

namespace oly::particles
{
	struct ParticleEmitter;

	struct IParticleSpawner
        : public imp::polymorphic
	{
		virtual ~IParticleSpawner() = default;
		virtual float spawn_debt(float time, float delta_time, float period) const = 0;
		virtual void overload(TOMLNode node) {}

		static void overload(imp::poly<IParticleSpawner>& spawner, TOMLNode node);
        static imp::poly<IParticleSpawner> load(TOMLNode node);
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
        : public imp::polymorphic
	{
		virtual ~ISampler1D() = default;
        virtual void apply(internal::Sampler1D&) const {}
        virtual void on_tick(const ParticleEmitter&) {}
		virtual void overload(TOMLNode node) {}

		static void overload(imp::poly<ISampler1D>& sampler, TOMLNode node);
        static imp::poly<ISampler1D> load(TOMLNode node);
	};

	struct IDomain1D
        : public imp::polymorphic
	{
		virtual ~IDomain1D() = default;
        virtual void apply(internal::Domain1D&) const {}
        virtual void on_tick(const ParticleEmitter&) {}
		virtual void overload(TOMLNode node) {}

		static void overload(imp::poly<IDomain1D>& domain, TOMLNode node);
        static imp::poly<IDomain1D> load(TOMLNode node);
	};

	struct AttributeGenerator1D
	{
		imp::poly<ISampler1D> sampler = nullptr;
		imp::poly<IDomain1D> domain = nullptr;

		void apply(internal::Generator1D& generator) const;
        void on_tick(const ParticleEmitter& emitter);
		void overload(TOMLNode node);

        static void overload(AttributeGenerator1D& generator, TOMLNode node);
        static AttributeGenerator1D load(TOMLNode node);
	};

	struct ISampler2D
        : public imp::polymorphic
	{
		virtual ~ISampler2D() = default;
        virtual void apply(internal::Sampler2D&) const {}
        virtual void on_tick(const ParticleEmitter&) {}
		virtual void overload(TOMLNode node) {}

		static void overload(imp::poly<ISampler2D>& sampler, TOMLNode node);
        static imp::poly<ISampler2D> load(TOMLNode node);
	};

	struct IDomain2D
        : public imp::polymorphic
	{
		virtual ~IDomain2D() = default;
        virtual void apply(internal::Domain2D&) const {}
        virtual void on_tick(const ParticleEmitter&) {}
		virtual void overload(TOMLNode node) {}

		static void overload(imp::poly<IDomain2D>& domain, TOMLNode node);
        static imp::poly<IDomain2D> load(TOMLNode node);
	};

	struct AttributeGenerator2D
	{
		imp::poly<ISampler2D> sampler = nullptr;
		imp::poly<IDomain2D> domain = nullptr;

		void apply(internal::Generator2D& generator) const;
        void on_tick(const ParticleEmitter& emitter);
		void overload(TOMLNode node);

        static void overload(AttributeGenerator2D& generator, TOMLNode node);
        static AttributeGenerator2D load(TOMLNode node);
	};

	struct ISampler3D
        : public imp::polymorphic
	{
		virtual ~ISampler3D() = default;
		virtual void apply(internal::Sampler3D&) const {}
		virtual void on_tick(const ParticleEmitter&) {}
		virtual void overload(TOMLNode node) {}

		static void overload(imp::poly<ISampler3D>& sampler, TOMLNode node);
        static imp::poly<ISampler3D> load(TOMLNode node);
	};

	struct IDomain3D
        : public imp::polymorphic
	{
		virtual ~IDomain3D() = default;
		virtual void apply(internal::Domain3D&) const {}
		virtual void on_tick(const ParticleEmitter&) {}
		virtual void overload(TOMLNode node) {}

        static void overload(imp::poly<IDomain3D>& domain, TOMLNode node);
		static imp::poly<IDomain3D> load(TOMLNode node);
	};

	struct AttributeGenerator3D
	{
		imp::poly<ISampler3D> sampler = nullptr;
		imp::poly<IDomain3D> domain = nullptr;

		void apply(internal::Generator3D& generator) const;
        void on_tick(const ParticleEmitter& emitter);
		void overload(TOMLNode node);

        static void overload(AttributeGenerator3D& generator, TOMLNode node);
        static AttributeGenerator3D load(TOMLNode node);
	};

	struct ISampler4D
        : public imp::polymorphic
	{
		virtual ~ISampler4D() = default;
		virtual void apply(internal::Sampler4D&) const {}
		virtual void on_tick(const ParticleEmitter&) {}
		virtual void overload(TOMLNode node) {}

		static void overload(imp::poly<ISampler4D>& sampler, TOMLNode node);
        static imp::poly<ISampler4D> load(TOMLNode node);
	};

	struct IDomain4D
        : public imp::polymorphic
	{
		virtual ~IDomain4D() = default;
		virtual void apply(internal::Domain4D&) const {}
		virtual void on_tick(const ParticleEmitter&) {}
		virtual void overload(TOMLNode node) {}

		static void overload(imp::poly<IDomain4D>& domain, TOMLNode node);
        static imp::poly<IDomain4D> load(TOMLNode node);
	};

	struct AttributeGenerator4D
	{
		imp::poly<ISampler4D> sampler = nullptr;
		imp::poly<IDomain4D> domain = nullptr;

		void apply(internal::Generator4D& generator) const;
        void on_tick(const ParticleEmitter& emitter);
		void overload(TOMLNode node);

        static void overload(AttributeGenerator4D& generator, TOMLNode node);
        static AttributeGenerator4D load(TOMLNode node);
	};
}
