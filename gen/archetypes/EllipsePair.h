#pragma once

#include "Olympian.h"

#include "registries/graphics/primitives/Ellipses.h"

namespace oly::gen
{
    struct EllipsePair
    {
		rendering::Ellipse ellipse1;
		rendering::Ellipse ellipse2;

    private:
        struct Constructor
        {
			reg::params::Ellipse ellipse1;
			reg::params::Ellipse ellipse2;

            Constructor();
        };

    public:
        EllipsePair(Constructor = {});

        void draw(bool flush_ellipses) const;

        void on_tick() const;
    };
}
