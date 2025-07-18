#include "Samplers.h"

#include "graphics/backend/basic/Sampler.h"

#include <memory>

namespace oly::graphics::samplers
{
	static std::unique_ptr<Sampler> _linear = nullptr;
	GLuint linear = 0;
	static std::unique_ptr<Sampler> _nearest = nullptr;
	GLuint nearest = 0;
	static std::unique_ptr<Sampler> _linear_mipmap_linear = nullptr;
	GLuint linear_mipmap_linear = 0;
	static std::unique_ptr<Sampler> _linear_mipmap_nearest = nullptr;
	GLuint linear_mipmap_nearest = 0;
	static std::unique_ptr<Sampler> _nearest_mipmap_linear = nullptr;
	GLuint nearest_mipmap_linear = 0;
	static std::unique_ptr<Sampler> _nearest_mipmap_nearest = nullptr;
	GLuint nearest_mipmap_nearest = 0;

	void internal::load()
	{
		_linear = std::make_unique<Sampler>();
		_linear->set_parameter_i(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		_linear->set_parameter_i(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		linear = *_linear;

		_nearest = std::make_unique<Sampler>();
		_nearest->set_parameter_i(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		_nearest->set_parameter_i(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		nearest = *_nearest;

		_linear_mipmap_linear = std::make_unique<Sampler>();
		_linear_mipmap_linear->set_parameter_i(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		_linear_mipmap_linear->set_parameter_i(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		linear_mipmap_linear = *_linear_mipmap_linear;

		_linear_mipmap_nearest = std::make_unique<Sampler>();
		_linear_mipmap_nearest->set_parameter_i(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		_linear_mipmap_nearest->set_parameter_i(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		linear_mipmap_nearest = *_linear_mipmap_nearest;

		_nearest_mipmap_linear = std::make_unique<Sampler>();
		_nearest_mipmap_linear->set_parameter_i(GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		_nearest_mipmap_linear->set_parameter_i(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		nearest_mipmap_linear = *_nearest_mipmap_linear;

		_nearest_mipmap_nearest = std::make_unique<Sampler>();
		_nearest_mipmap_nearest->set_parameter_i(GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		_nearest_mipmap_nearest->set_parameter_i(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		nearest_mipmap_nearest = *_nearest_mipmap_nearest;
	}

	void internal::unload()
	{
		_linear.reset();
		linear = 0;
		_nearest.reset();
		nearest = 0;
		_linear_mipmap_linear.reset();
		linear_mipmap_linear = 0;
		_linear_mipmap_nearest.reset();
		linear_mipmap_nearest = 0;
		_nearest_mipmap_linear.reset();
		nearest_mipmap_linear = 0;
		_nearest_mipmap_nearest.reset();
		nearest_mipmap_nearest = 0;
	}
}
