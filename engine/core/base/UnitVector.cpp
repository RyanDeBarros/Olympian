#include "UnitVector.h"

namespace oly
{
	bool UnitVector2D::operator<(const UnitVector2D& rhs) const
	{
		if (above_zero(_direction.y))
		{
			if (below_zero(rhs._direction.y))
				return true;
			else if (above_zero(rhs._direction.y))
				return _direction.x > rhs._direction.x;
			else
				return rhs._direction.x < 0.0f;
		}
		else if (below_zero(_direction.y))
		{
			if (above_zero(rhs._direction.y))
				return false;
			else if (below_zero(rhs._direction.y))
				return _direction.x < rhs._direction.x;
			else
				return false;
		}
		else
		{
			if (above_zero(rhs._direction.y))
				return _direction.x > 0.0f;
			else if (below_zero(rhs._direction.y))
				return true;
			else
				return _direction.x > 0.0f && rhs._direction.x < 0.0f;
		}
	}

	bool UnitVector2D::operator<=(const UnitVector2D& rhs) const
	{
		if (above_zero(_direction.y))
		{
			if (below_zero(rhs._direction.y))
				return true;
			else if (above_zero(rhs._direction.y))
				return _direction.x >= rhs._direction.x;
			else
				return rhs._direction.x < 0.0f;
		}
		else if (below_zero(_direction.y))
		{
			if (above_zero(rhs._direction.y))
				return false;
			else if (below_zero(rhs._direction.y))
				return _direction.x <= rhs._direction.x;
			else
				return false;
		}
		else
		{
			if (above_zero(rhs._direction.y))
				return _direction.x > 0.0f;
			else if (below_zero(rhs._direction.y))
				return true;
			else
				return _direction.x >= rhs._direction.x;
		}
	}

	bool UnitVector2D::near_standard(double tolerance) const
	{
		return near_zero(_direction.x, tolerance) || near_zero(_direction.y, tolerance);
	}

	bool UnitVector2D::near_parallel(UnitVector2D other, double tolerance) const
	{
		if (near_zero(_direction.x, tolerance))
			return near_zero(other._direction.x, tolerance);
		else
		{
			if (approx(glm::abs(_direction.x), glm::abs(other._direction.x), tolerance))
			{
				bool sx = glm::sign(_direction.x) == glm::sign(other._direction.x);
				bool sy = glm::sign(_direction.y) == glm::sign(other._direction.y);
				return (sx && sy) || (!sx && !sy);
			}
			else
				return false;
		}
	}

	UnitVector2D::ParallelState UnitVector2D::near_parallel_state(UnitVector2D other, double tolerance) const
	{
		if (near_zero(_direction.x, tolerance))
		{
			if (near_zero(other._direction.x, tolerance))
			{
				if (glm::sign(_direction.y) == glm::sign(other._direction.y))
					return ParallelState::SAME_DIRECTION;
				else
					return ParallelState::OPPOSITE_DIRECTION;
			}
			else
				return ParallelState::NON_PARALLEL;
		}
		else
		{
			if (approx(glm::abs(_direction.x), glm::abs(other._direction.x), tolerance))
			{
				if (glm::sign(_direction.x) == glm::sign(other._direction.x))
				{
					if (glm::sign(_direction.y) == glm::sign(other._direction.y))
						return ParallelState::SAME_DIRECTION;
					else
						return ParallelState::NON_PARALLEL;
				}
				else
				{
					if (glm::sign(_direction.y) == glm::sign(other._direction.y))
						return ParallelState::NON_PARALLEL;
					else
						return ParallelState::OPPOSITE_DIRECTION;
				}
			}
			else
				return ParallelState::NON_PARALLEL;
		}
	}
}
