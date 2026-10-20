#include "UnitVector.h"

namespace oly
{
    UnitVector2D::UnitVector2D()
        : direction_({ 1.0f, 0.0f })
    {
    }
    
    UnitVector2D::UnitVector2D(glm::vec2 direction)
        : direction_(glm::normalize(direction))
    {
    }
    
    UnitVector2D::UnitVector2D(float angle, float tolerance)
        : direction_(glm::cos(angle), glm::sin(angle))
    {
        if (tolerance > 0.0f)
        {
            const float inv_tolerance = 1.0f / tolerance;
            direction_.x = roundf(direction_.x * inv_tolerance) * tolerance;
            direction_.y = roundf(direction_.y * inv_tolerance) * tolerance;
            direction_ = glm::normalize(direction_);
        }
    }

    UnitVector2D::UnitVector2D(glm::vec2 direction, DirectImpl)
        : direction_(direction)
    {
    }

    UnitVector2D::UnitVector2D(RightImpl)
        : direction_({ 1.0f, 0.0f })
    {
    }

    UnitVector2D::UnitVector2D(UpImpl)
        : direction_({ 0.0f, 1.0f })
    {
    }

    UnitVector2D::UnitVector2D(LeftImpl)
        : direction_({ -1.0f, 0.0f })
    {
    }

    UnitVector2D::UnitVector2D(DownImpl)
        : direction_({ 0.0f, -1.0f })
    {
    }

    UnitVector2D::operator glm::vec2() const
    {
        return direction_;
    }
    
    UnitVector2D& UnitVector2D::operator=(glm::vec2 direction)
    {
        direction_ = glm::normalize(direction);
        return *this;
    }

    UnitVector2D UnitVector2D::operator-() const
    {
        return UnitVector2D(-direction_, DirectImpl{});
    }

    bool UnitVector2D::operator<(const UnitVector2D& rhs) const
    {
        if (above_zero(direction_.y))
        {
            if (below_zero(rhs.direction_.y))
                return true;
            else if (above_zero(rhs.direction_.y))
                return direction_.x > rhs.direction_.x;
            else if (rhs.direction_.x == 1.0f)
                return false;
            else
                return rhs.direction_.x < 0.0f;
        }
        else if (below_zero(direction_.y))
        {
            if (above_zero(rhs.direction_.y))
                return false;
            else if (below_zero(rhs.direction_.y))
                return direction_.x < rhs.direction_.x;
            else
                return false;
        }
        else if (direction_.x == 1.0f)
            return true;
        else
        {
            if (above_zero(rhs.direction_.y))
                return direction_.x > 0.0f;
            else if (below_zero(rhs.direction_.y))
                return true;
            else
                return direction_.x > 0.0f && rhs.direction_.x < 0.0f;
        }
    }

    bool UnitVector2D::operator<=(const UnitVector2D& rhs) const
    {
        if (above_zero(direction_.y))
        {
            if (below_zero(rhs.direction_.y))
                return true;
            else if (above_zero(rhs.direction_.y))
                return direction_.x >= rhs.direction_.x;
            else
                return rhs.direction_.x < 0.0f;
        }
        else if (below_zero(direction_.y))
        {
            if (above_zero(rhs.direction_.y))
                return false;
            else if (below_zero(rhs.direction_.y))
                return direction_.x <= rhs.direction_.x;
            else
                return false;
        }
        else
        {
            if (above_zero(rhs.direction_.y))
                return direction_.x > 0.0f;
            else if (below_zero(rhs.direction_.y))
                return true;
            else
                return direction_.x >= rhs.direction_.x;
        }
    }

    UnitVector2D& UnitVector2D::rotate(float angle)
    {
        const float c = glm::cos(angle);
        const float s = glm::sin(angle);
        direction_ = glm::mat2{ { c, s }, { -s, c } } * direction_;
        return *this;
    }

    UnitVector2D UnitVector2D::get_rotated(float angle) const
    {
        UnitVector2D copy = *this;
        return copy.rotate(angle);
    }

    UnitVector2D& UnitVector2D::quarter_turn()
    {
        direction_ = { -direction_.y, direction_.x };
        return *this;
    }

    UnitVector2D UnitVector2D::get_quarter_turn() const
    {
        UnitVector2D copy = *this;
        return copy.quarter_turn();
    }

    UnitVector2D& UnitVector2D::flip_x()
    {
        direction_.x = -direction_.x;
        return *this;
    }

    UnitVector2D UnitVector2D::get_flipped_x() const
    {
        UnitVector2D copy = *this;
        return copy.flip_x();
    }

    UnitVector2D& UnitVector2D::flip_y()
    {
        direction_.y = -direction_.y;
        return *this;
    }

    UnitVector2D UnitVector2D::get_flipped_y() const
    {
        UnitVector2D copy = *this;
        return copy.flip_y();
    }

    float UnitVector2D::rotation() const
    {
        return glm::atan(direction_.y, direction_.x);
    }
    
    glm::mat2 UnitVector2D::rotation_matrix() const
    {
        return { direction_, { -direction_.y, direction_.x } };
    }
    
    glm::mat2 UnitVector2D::inverse_rotation_matrix() const
    {
        return { { direction_.x, -direction_.y }, { direction_.y, direction_.x } };
    }

    float UnitVector2D::x() const
    {
        return direction_.x;
    }

    float UnitVector2D::y() const
    {
        return direction_.y;
    }

    float UnitVector2D::dot(glm::vec2 v) const
    {
        return glm::dot(v, direction_);
    }

    bool UnitVector2D::near_cardinal(double tolerance) const
    {
        return near_zero(direction_.x, tolerance) || near_zero(direction_.y, tolerance);
    }

    glm::vec2 UnitVector2D::perp_project(glm::vec2 v) const
    {
        return v - glm::dot(v, direction_) * direction_;
    }

    bool UnitVector2D::near_parallel(UnitVector2D o, double tolerance) const
    {
        if (near_zero(direction_.x, tolerance))
            return near_zero(o.direction_.x, tolerance);
        else
        {
            if (approx(glm::abs(direction_.x), glm::abs(o.direction_.x), tolerance))
            {
                bool sx = glm::sign(direction_.x) == glm::sign(o.direction_.x);
                bool sy = glm::sign(direction_.y) == glm::sign(o.direction_.y);
                return (sx && sy) || (!sx && !sy);
            }
            else
                return false;
        }
    }

    UnitVector2D::ParallelState UnitVector2D::near_parallel_state(UnitVector2D o, double tolerance) const
    {
        if (near_zero(direction_.x, tolerance))
        {
            if (near_zero(o.direction_.x, tolerance))
            {
                if (glm::sign(direction_.y) == glm::sign(o.direction_.y))
                    return ParallelState::SameDirection;
                else
                    return ParallelState::OppositeDirection;
            }
            else
                return ParallelState::NotParallel;
        }
        else
        {
            if (approx(glm::abs(direction_.x), glm::abs(o.direction_.x), tolerance))
            {
                if (glm::sign(direction_.x) == glm::sign(o.direction_.x))
                {
                    if (glm::sign(direction_.y) == glm::sign(o.direction_.y))
                        return ParallelState::SameDirection;
                    else
                        return ParallelState::NotParallel;
                }
                else
                {
                    if (glm::sign(direction_.y) == glm::sign(o.direction_.y))
                        return ParallelState::NotParallel;
                    else
                        return ParallelState::OppositeDirection;
                }
            }
            else
                return ParallelState::NotParallel;
        }
    }

    size_t UnitVector2D::hash() const
    {
        return std::hash<glm::vec2>{}(direction_);
    }

    double Distance<UnitVector2D>::operator()(const UnitVector2D& a, const UnitVector2D& b) const
    {
        return static_cast<double>(glm::abs(a.rotation() - b.rotation()));
    }
}

size_t std::hash<oly::UnitVector2D>::operator()(const oly::UnitVector2D& v) const
{
    return v.hash();
}
