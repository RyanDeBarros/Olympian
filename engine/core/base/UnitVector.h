#pragma once

#include "core/base/Approximate.h"

#include "external/GLM.h"

namespace oly
{
    class UnitVector2D
    {
        struct RightImpl {};
        struct UpImpl {};
        struct LeftImpl {};
        struct DownImpl {};
        struct DirectImpl {};

    public:
        enum ParallelState
        {
            SameDirection,
            OppositeDirection,
            NotParallel
        };

    private:
        glm::vec2 direction_;

    public:
        static const UnitVector2D kRight;
        static const UnitVector2D kUp;
        static const UnitVector2D kLeft;
        static const UnitVector2D kDown;

    public:
        UnitVector2D();
        UnitVector2D(glm::vec2 direction);
        UnitVector2D(float angle, float tolerance = 1e-7f);

    private:
        UnitVector2D(glm::vec2 direction, DirectImpl);
        UnitVector2D(RightImpl);
        UnitVector2D(UpImpl);
        UnitVector2D(LeftImpl);
        UnitVector2D(DownImpl);

    public:
        operator glm::vec2() const;
        UnitVector2D& operator=(glm::vec2 direction);

        UnitVector2D operator-() const;
        bool operator==(const UnitVector2D&) const = default;
        bool operator<(const UnitVector2D&) const;
        bool operator<=(const UnitVector2D&) const;

        UnitVector2D& rotate(float angle);
        UnitVector2D get_rotated(float angle) const;
        UnitVector2D& quarter_turn();
        UnitVector2D get_quarter_turn() const;
        UnitVector2D& flip_x();
        UnitVector2D get_flipped_x() const;
        UnitVector2D& flip_y();
        UnitVector2D get_flipped_y() const;

        float rotation() const;
        glm::mat2 rotation_matrix() const;
        glm::mat2 inverse_rotation_matrix() const;

        float x() const;
        float y() const;
        float dot(glm::vec2 v) const;
        bool near_cardinal(double tolerance = Tolerance<float>) const;

        glm::vec2 perp_project(glm::vec2 v) const;

        bool near_parallel(UnitVector2D o, double tolerance = Tolerance<float>) const;
        ParallelState near_parallel_state(UnitVector2D o, double tolerance = Tolerance<float>) const;

        size_t hash() const;
    };

    inline const UnitVector2D UnitVector2D::kRight = UnitVector2D(RightImpl{});
    inline const UnitVector2D UnitVector2D::kUp = UnitVector2D(UpImpl{});
    inline const UnitVector2D UnitVector2D::kLeft = UnitVector2D(LeftImpl{});
    inline const UnitVector2D UnitVector2D::kDown = UnitVector2D(DownImpl{});

    template<>
    struct Distance<UnitVector2D>
    {
        double operator()(const UnitVector2D& a, const UnitVector2D& b) const;
    };

    template<>
    struct ToleranceImpl<UnitVector2D>
    {
        static constexpr double kTol = 1e-7;
    };
}

template<>
struct std::hash<oly::UnitVector2D>
{
    size_t operator()(const oly::UnitVector2D& v) const;
};
