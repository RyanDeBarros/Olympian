#include "UnitVector.h"

#include "core/types/Approximate.h"

namespace oly
{
	bool UnitVector2D::near_standard() const
	{
		return near_zero(_direction.x) || near_zero(_direction.y);
	}
}
