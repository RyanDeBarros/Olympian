#include "Solvers.h"

#include "external/GLM.h"

namespace oly
{
	namespace math
	{
		namespace solver
		{
			float LinearCosine::solve() const
			{
				// trivial cases
				if (B == 0)
				{
					if (A == 0)
					{
						if (glm::cos(C) + D == 0)
							throw infinite_solutions();
						else
							throw no_solution();
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

				// newton's method
				for (size_t i = 0; i < _iterations; ++i)
				{
					float function = A * guess + glm::cos(B * guess + C) + D;
					float derivative = A - B * glm::sin(B * guess + C);
					guess -= function / derivative;
				}
				return guess;
			}
		}
	}
}
