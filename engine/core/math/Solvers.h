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
}
