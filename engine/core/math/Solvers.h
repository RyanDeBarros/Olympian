#pragma once

#include "external/GLM.h"

namespace oly::math::solver
{
    struct Eigen2x2
    {
        glm::mat2 m;

        void solve(float values[2] = nullptr, glm::vec2 vectors[2] = nullptr) const;
    };

    struct Quadratic
    {
        // a*x^2 + b*x + c = 0

        float a;
        float b;
        float c;

        int solve(float& t1, float& t2) const;
    };

    struct Cubic
    {
        // a*x^3 + b*x^2 + c*x + d = 0

        float a;
        float b;
        float c;
        float d;

        int solve(float& t1, float& t2, float& t3) const;
    };

    struct Quartic
    {
        // a*x^4 + b*x^3 + c*x^2 + d*x + e = 0

        float a;
        float b;
        float c;
        float d;
        float e;

        int solve(float& t1, float& t2, float& t3, float& t4) const;
    };

    struct LinearCosine
    {
        // a*x + cos(b*x + c) + d = 0

        float a;
        float b;
        float c;
        float d;

        unsigned int iterations = 3;

        float solve() const;
    };
}
