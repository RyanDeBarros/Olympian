#include "Domains.h"

#include "graphics/particles/ShaderStructs.h"

#include "definitions/Keys.h"

namespace oly::particles
{
    ConstantDomain1D::ConstantDomain1D(float c)
        : c(c)
    {
    }

    void ConstantDomain1D::apply(internal::Domain1D& domain) const
    {
        domain.type = internal::Domain1D::Constant;
        domain.params[0] = c;
    }

    void ConstantDomain1D::on_tick(const ParticleEmitter& emitter)
    {
        c.on_tick(emitter);
    }

    void ConstantDomain1D::overload(TOMLNode node)
    {
        c.overload(assets::Parser(node).field(detail::Key::C));
    }

    LineDomain1D::LineDomain1D(float a, float b)
        : a(a)
        , b(b)
    {
    }

    void LineDomain1D::apply(internal::Domain1D& domain) const
    {
        domain.type = internal::Domain1D::Line;
        domain.params[0] = a;
        domain.params[1] = b;
    }

    void LineDomain1D::on_tick(const ParticleEmitter& emitter)
    {
        a.on_tick(emitter);
        b.on_tick(emitter);
    }

    void LineDomain1D::overload(TOMLNode node)
    {
        a.overload(assets::Parser(node).field(detail::Key::A));
        b.overload(assets::Parser(node).field(detail::Key::B));
    }

    BiLineDomain1D::BiLineDomain1D(float a, float b, float c)
        : a(a)
        , b(b)
        , c(c)
    {
    }

    void BiLineDomain1D::apply(internal::Domain1D& domain) const
    {
        domain.type = internal::Domain1D::BiLine;
        domain.params[0] = a;
        domain.params[1] = b;
        domain.params[2] = c;
    }

    void BiLineDomain1D::on_tick(const ParticleEmitter& emitter)
    {
        a.on_tick(emitter);
        b.on_tick(emitter);
        c.on_tick(emitter);
    }

    void BiLineDomain1D::overload(TOMLNode node)
    {
        a.overload(assets::Parser(node).field(detail::Key::A));
        b.overload(assets::Parser(node).field(detail::Key::B));
        c.overload(assets::Parser(node).field(detail::Key::C));
    }

    ConstantDomain2D::ConstantDomain2D(glm::vec2 c)
        : c(c)
    {
    }

    void ConstantDomain2D::apply(internal::Domain2D& domain) const
    {
        domain.type = internal::Domain2D::Constant;
        domain.params[0] = c[0];
        domain.params[1] = c[1];
    }

    void ConstantDomain2D::on_tick(const ParticleEmitter& emitter)
    {
        c.on_tick(emitter);
    }

    void ConstantDomain2D::overload(TOMLNode node)
    {
        c.overload(assets::Parser(node).field(detail::Key::C));
    }

    ConstantDomain3D::ConstantDomain3D(glm::vec3 c)
        : c(c)
    {
    }

    void ConstantDomain3D::apply(internal::Domain3D& domain) const
    {
        domain.type = internal::Domain3D::Constant;
        domain.params[0] = c[0];
        domain.params[1] = c[1];
        domain.params[2] = c[2];
    }

    void ConstantDomain3D::on_tick(const ParticleEmitter& emitter)
    {
        c.on_tick(emitter);
    }

    void ConstantDomain3D::overload(TOMLNode node)
    {
        c.overload(assets::Parser(node).field(detail::Key::C));
    }

    ConstantDomain4D::ConstantDomain4D(glm::vec4 c)
        : c(c)
    {
    }

    void ConstantDomain4D::apply(internal::Domain4D& domain) const
    {
        domain.type = internal::Domain4D::Constant;
        domain.params[0] = c[0];
        domain.params[1] = c[1];
        domain.params[2] = c[2];
        domain.params[3] = c[3];
    }

    void ConstantDomain4D::on_tick(const ParticleEmitter& emitter)
    {
        c.on_tick(emitter);
    }

    void ConstantDomain4D::overload(TOMLNode node)
    {
        c.overload(assets::Parser(node).field(detail::Key::C));
    }
}
