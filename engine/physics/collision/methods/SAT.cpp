#include "SAT.h"

namespace oly::col2d::sat
{
    namespace internal
    {
        float sat(float min1, float max1, float min2, float max2, UnitVector2D& axis)
        {
            if (min1 > max2)
                return max2 - min1;
            if (min2 > max1)
            {
                axis = -axis;
                return max1 - min2;
            }
            if (min1 < min2)
            {
                if (max1 < max2)
                {
                    axis = -axis;
                    return max1 - min2;
                }
                else
                {
                    if (min1 + max1 < min2 + max2)
                    {
                        axis = -axis;
                        return max1 - min2;
                    }
                    else
                        return max2 - min1;
                }
            }
            else
            {
                if (max1 > max2)
                    return max2 - min1;
                else
                {
                    if (min1 + max1 < min2 + max2)
                    {
                        axis = -axis;
                        return max1 - min2;
                    }
                    else
                        return max2 - min1;
                }
            }
        }
    }
}
