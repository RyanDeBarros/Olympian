#include "Solvers.h"

#include "core/base/Errors.h"
#include "core/types/Approximate.h"

namespace oly::math::solver
{
	void Eigen2x2::solve(float values[2], glm::vec2 vectors[2]) const
	{
		float discriminant = (M[0][0] - M[1][1]) * (M[0][0] - M[1][1]) + 4.0f * M[1][0] * M[0][1];
		if (discriminant < 0.0f)
			throw Error(ErrorCode::SOLVER_NO_SOLUTION);

		float delta = glm::sqrt(discriminant);
		float v[2];
		v[0] = 0.5f * (M[0][0] + M[1][1] - delta);
		v[1] = 0.5f * (M[0][0] + M[1][1] + delta);

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

				float m00 = M[0][0] - lambda;
				float m11 = M[1][1] - lambda;
				if (glm::abs(m00) > glm::abs(m11))
				{
					// use row 1
					vectors[i].x = near_zero(m00) ? 0.0f : -M[1][0] / m00;
					vectors[i].y = 1.0f;
				}
				else
				{
					// use row 2
					vectors[i].x = 1.0f;
					vectors[i].y = near_zero(m11) ? 0.0f : -M[0][1] / m11;
				}

				vectors[i] = glm::normalize(vectors[i]);
			}
		}
	}

	float LinearCosine::solve() const
	{
		// trivial cases
		if (B == 0)
		{
			if (A == 0)
			{
				if (glm::cos(C) + D == 0)
					throw Error(ErrorCode::SOLVER_INFINITE_SOLUTIONS);
				else
					throw Error(ErrorCode::SOLVER_NO_SOLUTION);
			}
			else
				return -(glm::cos(C) + D) / A;
		}
		else if (A == 0)
			return (glm::acos(-D) - C) / B;

		// initial guess - model problem as Ax + 1 - (Bx + C) ^ 2 / 2 + D = 0
		float guess;
		{
			float discriminant = A * A - 2 * A * B * C + 2 * B * B * (D + 1);
			if (discriminant < 0.0f)
			{
				// use peak instead
				guess = (A / B - C) / B;
			}
			else
			{
				float sqrt_discriminant = glm::sqrt(discriminant);
				float guess_plus = (A - B * C + sqrt_discriminant) / (B * B);
				float acc_plus = glm::abs(A * guess_plus + glm::cos(B * guess_plus + C) + D);
				float guess_minus = (A - B * C - sqrt_discriminant) / (B * B);
				float acc_minus = glm::abs(A * guess_minus + glm::cos(B * guess_minus + C) + D);
				guess = acc_plus < acc_minus ? guess_plus : guess_minus;
			}
		}

		// Newton's method
		for (size_t i = 0; i < _iterations; ++i)
		{
			float function = A * guess + glm::cos(B * guess + C) + D;
			float derivative = A - B * glm::sin(B * guess + C);
			guess -= function / derivative;
		}
		return guess;
	}
}
