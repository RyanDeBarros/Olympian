#pragma once

#include "graphics/particles/Attribute.h"
#include "graphics/particles/AttributeGenerator.h"

#include "external/GL.h"
#include "external/GLM.h"

namespace oly::particles
{
    struct ConstantDomain1D
        : public IDomain1D
    {
        IMP_POLYMORPHIC_IMPL((ConstantDomain1D));

    public:
        Attribute<float> c;

        ConstantDomain1D(float c = 0.f);

        void apply(internal::Domain1D& domain) const override;
        void on_tick(const ParticleEmitter& emitter) override;
        void overload(TOMLNode node) override;
    };

    struct LineDomain1D
        : public IDomain1D
    {
        IMP_POLYMORPHIC_IMPL((LineDomain1D));

    public:
        Attribute<float> a;
        Attribute<float> b;

        LineDomain1D(float a = 0.f, float b = 0.f);

        void apply(internal::Domain1D& domain) const override;
        void on_tick(const ParticleEmitter& emitter) override;
        void overload(TOMLNode node) override;
    };

    struct BiLineDomain1D
        : public IDomain1D
    {
        IMP_POLYMORPHIC_IMPL((BiLineDomain1D));

    public:
        Attribute<float> a;
        Attribute<float> b;
        Attribute<float> c;

        BiLineDomain1D(float a = 0.f, float b = 0.f, float c = 0.f);

        void apply(internal::Domain1D& domain) const override;
        void on_tick(const ParticleEmitter& emitter) override;
        void overload(TOMLNode node) override;
    };

    struct ConstantDomain2D
        : public IDomain2D
    {
        IMP_POLYMORPHIC_IMPL((ConstantDomain2D));

    public:
        Attribute<glm::vec2> c;

        ConstantDomain2D(glm::vec2 c = {});

        void apply(internal::Domain2D& domain) const override;
        void on_tick(const ParticleEmitter& emitter) override;
        void overload(TOMLNode node) override;
    };

    struct ConstantDomain3D
        : public IDomain3D
    {
        IMP_POLYMORPHIC_IMPL((ConstantDomain3D));

    public:
        Attribute<glm::vec3> c;

        ConstantDomain3D(glm::vec3 c = {});

        void apply(internal::Domain3D& domain) const override;
        void on_tick(const ParticleEmitter& emitter) override;
        void overload(TOMLNode node) override;
    };

    struct ConstantDomain4D
        : public IDomain4D
    {
        IMP_POLYMORPHIC_IMPL((ConstantDomain4D));

    public:
        Attribute<glm::vec4> c;

        ConstantDomain4D(glm::vec4 c = {});

        void apply(internal::Domain4D& domain) const override;
        void on_tick(const ParticleEmitter& emitter) override;
        void overload(TOMLNode node) override;
    };
}
