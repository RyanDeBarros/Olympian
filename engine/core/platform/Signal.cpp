#include "Signal.h"

namespace oly::input
{
    SignalID SignalTable::insert(const StringParam& name)
    {
        SignalID id = next_++;
        table_[name.transfer()] = id;
        return id;
    }

    SignalID SignalTable::get(const StringParam& name) const
    {
        auto it = table_.find(name.view());
        if (it != table_.end())
            return it->second;
        else
            return 0;
    }

    Signal::Signal(Phase phase, bool v, Source source)
        : phase(phase)
        , type_(Type::Boolean)
        , bool_value_(v)
        , source_(source)
    {
    }

    Signal::Signal(Phase phase, float v, Source source)
        : phase(phase)
        , type_(Type::Axis1D)
        , axis_1d_value_(v)
        , source_(source)
    {
    }

    Signal::Signal(Phase phase, glm::vec2 v, Source source)
        : phase(phase)
        , type_(Type::Axis2D)
        , axis_2d_value_(v)
        , source_(source)
    {
    }

    Signal::Signal(Phase phase, glm::vec3 v, Source source)
        : phase(phase)
        , type_(Type::Axis3D)
        , axis_3d_value_(v)
        , source_(source)
    {
    }

    template<>
    bool Signal::get<bool>() const
    {
        if (type_ != Type::Boolean)
            throw Error(ErrorCode::IncompatibleSignalType);

        return bool_value_;
    }

    template<>
    float Signal::get<float>() const
    {
        if (type_ != Type::Axis1D)
            throw Error(ErrorCode::IncompatibleSignalType);

        return axis_1d_value_;
    }

    template<>
    glm::vec2 Signal::get<glm::vec2>() const
    {
        if (type_ != Type::Axis2D)
            throw Error(ErrorCode::IncompatibleSignalType);

        return axis_2d_value_;
    }

    template<>
    glm::vec3 Signal::get<glm::vec3>() const
    {
        if (type_ != Type::Axis3D)
            throw Error(ErrorCode::IncompatibleSignalType);

        return axis_3d_value_;
    }

    bool Signal::from_controller() const
    {
        return source_ >= Source::JoystickBase;
    }
    
    int Signal::controller_id() const
    {
        if (source_ < Source::JoystickBase)
            throw Error(ErrorCode::InvalidControllerID);
        
        return source_;
    }
}
