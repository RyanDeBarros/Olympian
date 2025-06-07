#include "UnitVector.h"

#include "core/types/Approximate.h"

namespace oly
{
	bool UnitVector2D::near_standard() const
	{
		return near_zero(_direction.x) || near_zero(_direction.y);
	}

	UnitVector2D::Parallel UnitVector2D::near_parallel(UnitVector2D other) const
	{
		if (near_zero(_direction.x))
		{
			if (near_zero(other._direction.x))
			{
				if (glm::sign(_direction.y) == glm::sign(other._direction.y))
					return Parallel::SAME_DIRECTION;
				else
					return Parallel::OPPOSITE_DIRECTION;
			}
			else
				return Parallel::NON_PARALLEL;
		}
		else
		{
			if (approx(glm::abs(_direction.x), glm::abs(other._direction.x)))
			{
				if (glm::sign(_direction.x) == glm::sign(other._direction.x))
				{
					if (glm::sign(_direction.y) == glm::sign(other._direction.y))
						return Parallel::SAME_DIRECTION;
					else
						return Parallel::NON_PARALLEL;
				}
				else
				{
					if (glm::sign(_direction.y) == glm::sign(other._direction.y))
						return Parallel::NON_PARALLEL;
					else
						return Parallel::OPPOSITE_DIRECTION;
				}
			}
			else
				return Parallel::NON_PARALLEL;
		}
	}
}
