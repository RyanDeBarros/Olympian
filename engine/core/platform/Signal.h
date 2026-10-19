#pragma once

#include "core/base/Errors.h"

#include "core/util/StringParam.h"

#include "external/GLM.h"

#include <imp/dependent_false.hpp>
#include <imp/hetero.hpp>

namespace oly::input
{
    using SignalID = unsigned int;

    class SignalTable
    {
        SignalID next_ = 1;
        imp::hetero::string_umap<SignalID> table_;

    public:
        SignalID insert(const StringParam& name);
        SignalID get(const StringParam& name) const;
    };

    using SignalRoutingTable = imp::hetero::string_umap<std::vector<std::string>>;

    enum class Phase
    {
        Started,
        Completed,
        Ongoing
    };

    struct Signal
    {
        enum class Type
        {
            Boolean,
            Axis1D,
            Axis2D,
            Axis3D
        };

        enum Source
        {
            Keyboard = -2,
            Mouse = -1,
            JoystickBase = 0,
        };

    public:
        Phase phase;

    private:
        Type type_;
        Source source_;

        union
        {
            bool bool_value_;
            float axis_1d_value_;
            glm::vec2 axis_2d_value_;
            glm::vec3 axis_3d_value_;
        };

    public:
        Signal(Phase phase, bool v, Source source);
        Signal(Phase phase, float v, Source source);
        Signal(Phase phase, glm::vec2 v, Source source);
        Signal(Phase phase, glm::vec3 v, Source source);

        template<typename T>
        T get() const
        {
            static_assert(imp::dependent_false_v<T>, "oly::input::Signal::get<T>() does not support the invoked type");
        }

        template<>
        bool get<bool>() const;

        template<>
        float get<float>() const;

        template<>
        glm::vec2 get<glm::vec2>() const;

        template<>
        glm::vec3 get<glm::vec3>() const;

        bool from_controller() const;
        int controller_id() const;
    };
}
