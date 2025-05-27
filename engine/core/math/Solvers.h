#pragma once

#include "external/GLM.h"

namespace oly::math::solver
{
	struct Eigen2x2
	{
		glm::mat2 M;

		void solve(float values[2] = nullptr, glm::vec2 vectors[2] = nullptr) const;
	};

	struct LinearCosine
	{
		// Ax + cos(Bx + C) + D = 0
		float A, B, C, D;
		unsigned int _iterations = 3;

		float solve() const;
	};

	struct Quadratic
	{
		// Ax^2 + Bx + C = 0
		float A, B, C;

		int solve(float& t1, float& t2) const;
	};

	struct Cubic
	{
		// Ax^3 + Bx^2 + Cx + D = 0
		float A, B, C, D;

		int solve(float& t1, float& t2, float& t3) const;
	};

	struct Quartic
	{
		// Ax^4 + Bx^3 + Cx^2 + Dx + E = 0
		float A, B, C, D, E;

		int solve(float& t1, float& t2, float& t3, float& t4) const;
	};
}
