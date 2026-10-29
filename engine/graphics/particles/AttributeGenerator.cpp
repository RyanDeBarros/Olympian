#include "AttributeGenerator.h"

#include "core/util/Polyklass.h"
#include "core/util/StringParam.h"

#include "graphics/particles/ShaderStructs.h"

#include "graphics/particles/implementations/Spawners.h"
#include "graphics/particles/implementations/Samplers.h"
#include "graphics/particles/implementations/Domains.h"

#include "definitions/Keys.h"

namespace oly::particles
{
    void IParticleSpawner::overload(imp::poly<IParticleSpawner>& spawner, TOMLNode node)
    {
        std::string klass = assets::Parser(node).defaulted<std::string>(detail::Key::Klass)();

        _OLY_POLYKLASS_CASES_BEGIN(spawner)
            _OLY_POLYKLASS_IF_CASE(ConstantParticleSpawner)
            _OLY_POLYKLASS_ELSE_IF_CASE(BurstParticleSpawner)
            _OLY_POLYKLASS_CASES_END;

        if (spawner)
            spawner->overload(node);
    }

    imp::poly<IParticleSpawner> IParticleSpawner::load(TOMLNode node)
    {
        imp::poly<IParticleSpawner> spawner = nullptr;
        overload(spawner, node);
        return spawner;
    }

    void ISampler1D::overload(imp::poly<ISampler1D>& sampler, TOMLNode node)
    {
        std::string klass = assets::Parser(node).defaulted<std::string>(detail::Key::Klass)();

        _OLY_POLYKLASS_CASES_BEGIN(sampler)
            _OLY_POLYKLASS_IF_CASE(UniformSampler1D)
            _OLY_POLYKLASS_ELSE_IF_CASE(TiltedSampler1D)
            _OLY_POLYKLASS_CASES_END;

        if (sampler)
            sampler->overload(node);
    }

    imp::poly<ISampler1D> ISampler1D::load(TOMLNode node)
    {
        imp::poly<ISampler1D> sampler = nullptr;
        overload(sampler, node);
        return sampler;
    }

    void IDomain1D::overload(imp::poly<IDomain1D>& domain, TOMLNode node)
    {
        std::string klass = assets::Parser(node).defaulted<std::string>(detail::Key::Klass)();

        _OLY_POLYKLASS_CASES_BEGIN(domain)
            _OLY_POLYKLASS_IF_CASE(ConstantDomain1D)
            _OLY_POLYKLASS_ELSE_IF_CASE(LineDomain1D)
            _OLY_POLYKLASS_ELSE_IF_CASE(BiLineDomain1D)
            _OLY_POLYKLASS_CASES_END;

        if (domain)
            domain->overload(node);
    }

    imp::poly<IDomain1D> IDomain1D::load(TOMLNode node)
    {
        imp::poly<IDomain1D> domain = nullptr;
        overload(domain, node);
        return domain;
    }

    void AttributeGenerator1D::apply(internal::Generator1D& generator) const
    {
        sampler->apply(generator.sampler);
        domain->apply(generator.domain);
    }

    void AttributeGenerator1D::on_tick(const ParticleEmitter& emitter)
    {
        sampler->on_tick(emitter);
        domain->on_tick(emitter);
    }

    void AttributeGenerator1D::overload(TOMLNode node)
    {
        assets::Parser parser(node);
        sampler->overload(parser.field(detail::Key::Sampler));
        domain->overload(parser.field(detail::Key::Domain));
    }

    void AttributeGenerator1D::overload(AttributeGenerator1D& generator, TOMLNode node)
    {
        generator.overload(node);
    }

    AttributeGenerator1D AttributeGenerator1D::load(TOMLNode node)
    {
        AttributeGenerator1D generator{ .sampler = nullptr, .domain = nullptr };
        overload(generator, node);
        return generator;
    }

    void ISampler2D::overload(imp::poly<ISampler2D>& sampler, TOMLNode node)
    {
        std::string klass = assets::Parser(node).defaulted<std::string>(detail::Key::Klass)();

        _OLY_POLYKLASS_CASES_BEGIN(sampler)
            _OLY_POLYKLASS_IF_CASE(UniformSampler2D)
            _OLY_POLYKLASS_CASES_END;

        if (sampler)
            sampler->overload(node);
    }

    imp::poly<ISampler2D> ISampler2D::load(TOMLNode node)
    {
        imp::poly<ISampler2D> sampler = nullptr;
        overload(sampler, node);
        return sampler;
    }

    void IDomain2D::overload(imp::poly<IDomain2D>& domain, TOMLNode node)
    {
        std::string klass = assets::Parser(node).defaulted<std::string>(detail::Key::Klass)();

        _OLY_POLYKLASS_CASES_BEGIN(domain)
            _OLY_POLYKLASS_IF_CASE(ConstantDomain2D)
            _OLY_POLYKLASS_CASES_END;

        if (domain)
            domain->overload(node);
    }

    imp::poly<IDomain2D> IDomain2D::load(TOMLNode node)
    {
        imp::poly<IDomain2D> domain = nullptr;
        overload(domain, node);
        return domain;
    }

    void AttributeGenerator2D::apply(internal::Generator2D& generator) const
    {
        sampler->apply(generator.sampler);
        domain->apply(generator.domain);
    }

    void AttributeGenerator2D::on_tick(const ParticleEmitter& emitter)
    {
        sampler->on_tick(emitter);
        domain->on_tick(emitter);
    }

    void AttributeGenerator2D::overload(TOMLNode node)
    {
        assets::Parser parser(node);
        sampler->overload(parser.field(detail::Key::Sampler));
        domain->overload(parser.field(detail::Key::Domain));
    }

    void AttributeGenerator2D::overload(AttributeGenerator2D& generator, TOMLNode node)
    {
        generator.overload(node);
    }

    AttributeGenerator2D AttributeGenerator2D::load(TOMLNode node)
    {
        AttributeGenerator2D generator{ .sampler = nullptr, .domain = nullptr };
        overload(generator, node);
        return generator;
    }

    void ISampler3D::overload(imp::poly<ISampler3D>& sampler, TOMLNode node)
    {
        std::string klass = assets::Parser(node).defaulted<std::string>(detail::Key::Klass)();

        _OLY_POLYKLASS_CASES_BEGIN(sampler)
            _OLY_POLYKLASS_IF_CASE(UniformSampler3D)
            _OLY_POLYKLASS_CASES_END;

        if (sampler)
            sampler->overload(node);
    }

    imp::poly<ISampler3D> ISampler3D::load(TOMLNode node)
    {
        imp::poly<ISampler3D> sampler = nullptr;
        overload(sampler, node);
        return sampler;
    }

    void IDomain3D::overload(imp::poly<IDomain3D>& domain, TOMLNode node)
    {
        std::string klass = assets::Parser(node).defaulted<std::string>(detail::Key::Klass)();

        _OLY_POLYKLASS_CASES_BEGIN(domain)
            _OLY_POLYKLASS_IF_CASE(ConstantDomain3D)
            _OLY_POLYKLASS_CASES_END;

        if (domain)
            domain->overload(node);
    }

    imp::poly<IDomain3D> IDomain3D::load(TOMLNode node)
    {
        imp::poly<IDomain3D> domain = nullptr;
        overload(domain, node);
        return domain;
    }

    void AttributeGenerator3D::apply(internal::Generator3D& generator) const
    {
        sampler->apply(generator.sampler);
        domain->apply(generator.domain);
    }

    void AttributeGenerator3D::on_tick(const ParticleEmitter& emitter)
    {
        sampler->on_tick(emitter);
        domain->on_tick(emitter);
    }

    void AttributeGenerator3D::overload(TOMLNode node)
    {
        assets::Parser parser(node);
        sampler->overload(parser.field(detail::Key::Sampler));
        domain->overload(parser.field(detail::Key::Domain));
    }

    void AttributeGenerator3D::overload(AttributeGenerator3D& generator, TOMLNode node)
    {
        generator.overload(node);
    }

    AttributeGenerator3D AttributeGenerator3D::load(TOMLNode node)
    {
        AttributeGenerator3D generator{ .sampler = nullptr, .domain = nullptr };
        overload(generator, node);
        return generator;
    }

    void ISampler4D::overload(imp::poly<ISampler4D>& sampler, TOMLNode node)
    {
        std::string klass = assets::Parser(node).defaulted<std::string>(detail::Key::Klass)();

        _OLY_POLYKLASS_CASES_BEGIN(sampler)
            _OLY_POLYKLASS_IF_CASE(UniformSampler4D)
            _OLY_POLYKLASS_CASES_END;

        if (sampler)
            sampler->overload(node);
    }

    imp::poly<ISampler4D> ISampler4D::load(TOMLNode node)
    {
        imp::poly<ISampler4D> sampler = nullptr;
        overload(sampler, node);
        return sampler;
    }

    void IDomain4D::overload(imp::poly<IDomain4D>& domain, TOMLNode node)
    {
        std::string klass = assets::Parser(node).defaulted<std::string>(detail::Key::Klass)();

        _OLY_POLYKLASS_CASES_BEGIN(domain)
            _OLY_POLYKLASS_IF_CASE(ConstantDomain4D)
            _OLY_POLYKLASS_CASES_END;

        if (domain)
            domain->overload(node);
    }

    imp::poly<IDomain4D> IDomain4D::load(TOMLNode node)
    {
        imp::poly<IDomain4D> domain = nullptr;
        overload(domain, node);
        return domain;
    }

    void AttributeGenerator4D::apply(internal::Generator4D& generator) const
    {
        sampler->apply(generator.sampler);
        domain->apply(generator.domain);
    }

    void AttributeGenerator4D::on_tick(const ParticleEmitter& emitter)
    {
        sampler->on_tick(emitter);
        domain->on_tick(emitter);
    }

    void AttributeGenerator4D::overload(TOMLNode node)
    {
        assets::Parser parser(node);
        sampler->overload(parser.field(detail::Key::Sampler));
        domain->overload(parser.field(detail::Key::Domain));
    }

    void AttributeGenerator4D::overload(AttributeGenerator4D& generator, TOMLNode node)
    {
        generator.overload(node);
    }

    AttributeGenerator4D AttributeGenerator4D::load(TOMLNode node)
    {
        AttributeGenerator4D generator{ .sampler = nullptr, .domain = nullptr };
        overload(generator, node);
        return generator;
    }
}

