#include "Solvers.h"

#include <algorithm>

#include "core/base/Errors.h"
#include "core/types/Approximate.h"
#include "external/Quartic.h"

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

	int Quadratic::solve(float& t1, float& t2) const
	{
		if (near_zero(A))
		{
			if (near_zero(B))
			{
				if (near_zero(C))
					throw Error(ErrorCode::SOLVER_INFINITE_SOLUTIONS);
				else
					throw Error(ErrorCode::SOLVER_NO_SOLUTION);
			}
			else
			{
				t1 = -C / B;
				return 1;
			}
		}

		float discriminant = B * B - 4 * A * C;
		if (near_zero(discriminant))
		{
			t1 = -B / (2 * A);
			return 1;
		}
		else if (discriminant < 0.0f)
			throw Error(ErrorCode::SOLVER_NO_SOLUTION);
		else
		{
			discriminant = glm::sqrt(discriminant);
			float denom = 1 / (2 * A);
			t1 = (-B - discriminant) * denom;
			t2 = (-B + discriminant) * denom;
			return 2;
		}
	}

	int Cubic::solve(float& t1, float& t2, float& t3) const
	{
		if (near_zero(A))
			return Quadratic{ .A = B, .B = C, .C = D }.solve(t1, t2);

		float B2 = B * B;
		float B3 = B2 * B;
		float iA = 1 / A;
		float iA2 = iA * iA;
		float iA3 = iA2 * iA;
		
		float P = (C / 3) * iA - (B2 / 9) * iA2;
		float Q = -(B3 / 27) * iA3 + (B * C / 6) * iA2 - (D / 2) * iA;
		float shift = -(B / 3) * iA;
		float discriminant = Q * Q + P * P * P;
		if (near_zero(discriminant))
		{
			float cbrt = std::cbrtf(-Q);
			t1 = 2 * cbrt + shift;
			t2 = -cbrt + shift;
			return 2;
		}
		else if (discriminant < 0.0f)
		{
			std::vector<float> ts(3);
			float sqP = glm::sqrt(-P);
			float prefix = 2 * sqP;
			float angle = glm::acos(glm::clamp(Q * glm::inversesqrt(-P) / P, -1.0f, 1.0f)) / 3;
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
			t1 = std::cbrtf(-Q + discriminant) + std::cbrtf(-Q - discriminant) + shift;
			return 1;
		}
	}

	int Quartic::solve(float& t1, float& t2, float& t3, float& t4) const
	{
		if (near_zero(A))
			return Cubic{ .A = B, .B = C, .C = D, .D = E }.solve(t1, t2, t3);

		std::complex<double>* solution = solve_quartic(B / A, C / A, D / A, E / A);
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
}
