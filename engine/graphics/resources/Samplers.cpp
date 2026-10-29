#include "Samplers.h"

#include "graphics/backend/basic/Sampler.h"

#include <memory>

namespace oly::graphics::samplers
{
    static std::unique_ptr<Sampler> linear_ptr_ = nullptr;
    static GLuint linear_ = 0;

    static std::unique_ptr<Sampler> nearest_ptr_ = nullptr;
    static GLuint nearest_ = 0;

    static std::unique_ptr<Sampler> linear_mipmap_linear_ptr_ = nullptr;
    static GLuint linear_mipmap_linear_ = 0;

    static std::unique_ptr<Sampler> linear_mipmap_nearest_ptr_ = nullptr;
    static GLuint linear_mipmap_nearest_ = 0;

    static std::unique_ptr<Sampler> nearest_mipmap_linear_ptr_ = nullptr;
    static GLuint nearest_mipmap_linear_ = 0;

    static std::unique_ptr<Sampler> nearest_mipmap_nearest_ptr_ = nullptr;
    static GLuint nearest_mipmap_nearest_ = 0;

    GLuint linear()
    {
        return linear_;
    }

    GLuint nearest()
    {
        return nearest_;
    }

    GLuint linear_mipmap_linear()
    {
        return linear_mipmap_linear_;
    }

    GLuint linear_mipmap_nearest()
    {
        return linear_mipmap_nearest_;
    }

    GLuint nearest_mipmap_linear()
    {
        return nearest_mipmap_linear_;
    }

    GLuint nearest_mipmap_nearest()
    {
        return nearest_mipmap_nearest_;
    }


    void internal::load()
    {
        linear_ptr_ = std::make_unique<Sampler>();
        linear_ptr_->set_parameter_i(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        linear_ptr_->set_parameter_i(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        linear_ = *linear_ptr_;

        nearest_ptr_ = std::make_unique<Sampler>();
        nearest_ptr_->set_parameter_i(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        nearest_ptr_->set_parameter_i(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        nearest_ = *nearest_ptr_;

        linear_mipmap_linear_ptr_ = std::make_unique<Sampler>();
        linear_mipmap_linear_ptr_->set_parameter_i(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        linear_mipmap_linear_ptr_->set_parameter_i(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        linear_mipmap_linear_ = *linear_mipmap_linear_ptr_;

        linear_mipmap_nearest_ptr_ = std::make_unique<Sampler>();
        linear_mipmap_nearest_ptr_->set_parameter_i(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        linear_mipmap_nearest_ptr_->set_parameter_i(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        linear_mipmap_nearest_ = *linear_mipmap_nearest_ptr_;

        nearest_mipmap_linear_ptr_ = std::make_unique<Sampler>();
        nearest_mipmap_linear_ptr_->set_parameter_i(GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        nearest_mipmap_linear_ptr_->set_parameter_i(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        nearest_mipmap_linear_ = *nearest_mipmap_linear_ptr_;

        nearest_mipmap_nearest_ptr_ = std::make_unique<Sampler>();
        nearest_mipmap_nearest_ptr_->set_parameter_i(GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        nearest_mipmap_nearest_ptr_->set_parameter_i(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        nearest_mipmap_nearest_ = *nearest_mipmap_nearest_ptr_;
    }

    void internal::unload()
    {
        linear_ptr_.reset();
        linear_ = 0;

        nearest_ptr_.reset();
        nearest_ = 0;

        linear_mipmap_linear_ptr_.reset();
        linear_mipmap_linear_ = 0;

        linear_mipmap_nearest_ptr_.reset();
        linear_mipmap_nearest_ = 0;

        nearest_mipmap_linear_ptr_.reset();
        nearest_mipmap_linear_ = 0;

        nearest_mipmap_nearest_ptr_.reset();
        nearest_mipmap_nearest_ = 0;
    }
}
