#include "Solvers.h"

#include "core/base/Approximate.h"
#include "core/base/Errors.h"

#include "external/Quartic.h"

#include <algorithm>

namespace oly::math::solver
{
    void Eigen2x2::solve(float values[2], glm::vec2 vectors[2]) const
    {
        const float discriminant = (m[0][0] - m[1][1]) * (m[0][0] - m[1][1]) + 4.0f * m[1][0] * m[0][1];
        if (discriminant < 0.0f)
            throw Error(ErrorCode::SolverNoSolution);

        const float delta = glm::sqrt(discriminant);
        const float v[2] = {
            0.5f * (m[0][0] + m[1][1] - delta),
            0.5f * (m[0][0] + m[1][1] + delta)
        };

        if (values)
        {
            values[0] = v[0];
            values[1] = v[1];
        }

        if (vectors)
        {
            for (size_t i = 0; i < 2; ++i)
            {
                float lambda = v[i];

                float m00 = m[0][0] - lambda;
                float m11 = m[1][1] - lambda;
                if (glm::abs(m00) > glm::abs(m11))
                {
                    // use row 1
                    vectors[i].x = near_zero(m00) ? 0.0f : -m[1][0] / m00;
                    vectors[i].y = 1.0f;
                }
                else
                {
                    // use row 2
                    vectors[i].x = 1.0f;
                    vectors[i].y = near_zero(m11) ? 0.0f : -m[0][1] / m11;
                }

                vectors[i] = glm::normalize(vectors[i]);
            }
        }
    }

    int Quadratic::solve(float& t1, float& t2) const
    {
        if (near_zero(a))
        {
            if (near_zero(b))
            {
                if (near_zero(c))
                    throw Error(ErrorCode::SolverInfiniteSolutions);
                else
                    throw Error(ErrorCode::SolverNoSolution);
            }
            else
            {
                t1 = -c / b;
                return 1;
            }
        }

        float discriminant = b * b - 4 * a * c;
        if (near_zero(discriminant))
        {
            t1 = -b / (2 * a);
            return 1;
        }
        else if (discriminant < 0.0f)
            throw Error(ErrorCode::SolverNoSolution);
        else
        {
            discriminant = glm::sqrt(discriminant);
            const float denom = 1 / (2 * a);
            t1 = (-b - discriminant) * denom;
            t2 = (-b + discriminant) * denom;
            return 2;
        }
    }

    int Cubic::solve(float& t1, float& t2, float& t3) const
    {
        if (near_zero(a))
            return Quadratic{ .a = b, .b = c, .c = d }.solve(t1, t2);

        const float b2 = b * b;
        const float b3 = b2 * b;
        const float i_a = 1 / a;
        const float i_a2 = i_a * i_a;
        const float i_a3 = i_a2 * i_a;

        const float p = (c / 3) * i_a - (b2 / 9) * i_a2;
        const float q = -(b3 / 27) * i_a3 + (b * c / 6) * i_a2 - (d / 2) * i_a;
        const float shift = -(b / 3) * i_a;
        
        float discriminant = q * q + p * p * p;

        if (near_zero(discriminant))
        {
            float cbrt = std::cbrtf(-q);
            t1 = 2 * cbrt + shift;
            t2 = -cbrt + shift;
            return 2;
        }
        else if (discriminant < 0.0f)
        {
            std::vector<float> ts(3);
            float sqP = glm::sqrt(-p);
            float prefix = 2 * sqP;
            float angle = glm::acos(glm::clamp(q * glm::inversesqrt(-p) / p, -1.0f, 1.0f)) / 3;
            for (int k = 0; k < 3; ++k)
                ts[k] = prefix * glm::cos(angle - (2 * glm::pi<float>() * k) / 3);

            std::sort(ts.begin(), ts.end());
            t1 = ts[0] + shift;
            t2 = ts[1] + shift;
            t3 = ts[2] + shift;
            return 3;
        }
        else
        {
            discriminant = glm::sqrt(discriminant);
            t1 = std::cbrtf(-q + discriminant) + std::cbrtf(-q - discriminant) + shift;
            return 1;
        }
    }

    int Quartic::solve(float& t1, float& t2, float& t3, float& t4) const
    {
        if (near_zero(a))
            return Cubic{ .a = b, .b = c, .c = d, .d = e }.solve(t1, t2, t3);

        std::complex<double>* solution = solve_quartic(b / a, c / a, d / a, e / a);
        
        std::vector<float> ts;
        for (size_t i = 0; i < 4; ++i)
        {
            if (near_zero(solution[i].imag()))
                ts.push_back((float)solution[i].real());
        }

        delete[] solution;

        std::sort(ts.begin(), ts.end());

        if (ts.size() > 0)
            t1 = ts[0];
        else
            return 0;

        if (ts.size() > 1)
            t2 = ts[1];
        else
            return 1;

        if (ts.size() > 2)
            t3 = ts[2];
        else
            return 2;

        if (ts.size() > 3)
            t4 = ts[3];
        else
            return 3;

        return 4;
    }

    float LinearCosine::solve() const
    {
        // trivial cases
        if (b == 0)
        {
            if (a == 0)
            {
                if (glm::cos(c) + d == 0)
                    throw Error(ErrorCode::SolverInfiniteSolutions);
                else
                    throw Error(ErrorCode::SolverNoSolution);
            }
            else
                return -(glm::cos(c) + d) / a;
        }
        else if (a == 0)
            return (glm::acos(-d) - c) / b;

        // initial guess - model problem as Ax + 1 - (Bx + c) ^ 2 / 2 + d = 0
        float guess;
        {
            float discriminant = a * a - 2 * a * b * c + 2 * b * b * (d + 1);
            if (discriminant < 0.0f)
            {
                // use peak instead
                guess = (a / b - c) / b;
            }
            else
            {
                const float sqrt_discriminant = glm::sqrt(discriminant);
                const float guess_plus = (a - b * c + sqrt_discriminant) / (b * b);
                const float acc_plus = glm::abs(a * guess_plus + glm::cos(b * guess_plus + c) + d);
                const float guess_minus = (a - b * c - sqrt_discriminant) / (b * b);
                const float acc_minus = glm::abs(a * guess_minus + glm::cos(b * guess_minus + c) + d);
                guess = acc_plus < acc_minus ? guess_plus : guess_minus;
            }
        }

        // Newton's method
        for (size_t i = 0; i < iterations; ++i)
        {
            const float function = a * guess + glm::cos(b * guess + c) + d;
            const float derivative = a - b * glm::sin(b * guess + c);
            guess -= function / derivative;
        }

        return guess;
    }
}
