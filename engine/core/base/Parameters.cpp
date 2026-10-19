#include "Parameters.h"

namespace oly
{
    PowerInterval::PowerInterval(float inner_midpoint, float outer)
        : outer_(outer)
    {
        set_inner_midpoint(inner_midpoint);
    }

    PowerInterval& PowerInterval::operator=(float outer)
    {
        set_outer(outer);
        return *this;
    }

    float PowerInterval::inner() const
    {
        return inner_;
    }
    
    float PowerInterval::outer() const
    {
        return outer_;
    }
    
    void PowerInterval::set_outer(float outer)
    {
        outer_ = outer;
        sync_inner();
    }

    float PowerInterval::inner_midpoint() const
    {
        return inner_midpoint_;
    }
    
    float PowerInterval::power() const
    {
        return power_;
    }
    
    void PowerInterval::set_inner_midpoint(float midpoint)
    {
        inner_midpoint_ = midpoint;
        power_ = glm::log(inner_midpoint_) / glm::log(0.5f);
        sync_inner();
    }
    
    void PowerInterval::set_power(float power)
    {
        power_ = power;
        inner_midpoint_ = glm::pow(0.5f, power_);
        sync_inner();
    }

    void PowerInterval::sync_inner()
    {
        if (near_zero(outer_))
            inner_ = 0.f;
        else if (near_zero(1.f - outer_))
            inner_ = 1.f;
        else
            inner_ = glm::pow(outer_, power_);
    }

    double Distance<PowerInterval>::operator()(const PowerInterval& a, const PowerInterval& b) const
    {
        return static_cast<double>(glm::abs(a.inner() - b.inner()));
    }
}
