#pragma once

#include "graphics/particles/AttributeGenerator.h"

namespace oly::particles
{
    struct ConstantParticleSpawner
        : public IParticleSpawner
    {
        IMP_POLYMORPHIC_IMPL((ConstantParticleSpawner));

    public:
        float rate;

        ConstantParticleSpawner(float rate = 10.0f);

        float spawn_debt(float time, float delta_time, float period) const override;
        void overload(TOMLNode node) override;
    };

    struct BurstParticleSpawner
        : public IParticleSpawner
    {
        IMP_POLYMORPHIC_IMPL((BurstParticleSpawner));

    public:
        float rate;
        float duration;
        float time_offset;

        BurstParticleSpawner(float rate = 100.0f, float duration = 0.1f, float time_offset = 0.0f);

        float spawn_debt(float time, float delta_time, float period) const override;
        void overload(TOMLNode node) override;
    };
}
