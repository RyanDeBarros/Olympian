#include "Resources.h"

namespace oly
{
	namespace samplers
	{
		static std::unique_ptr<rendering::Sampler> _linear = nullptr;
		GLuint linear = 0;
		static std::unique_ptr<rendering::Sampler> _nearest = nullptr;
		GLuint nearest = 0;

		void load()
		{
			_linear = std::make_unique<rendering::Sampler>();
			_linear->set_parameter_i(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			_linear->set_parameter_i(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			linear = *_linear;

			_nearest = std::make_unique<rendering::Sampler>();
			_nearest->set_parameter_i(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			_nearest->set_parameter_i(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			nearest = *_nearest;
		}

		void unload()
		{
			_linear.reset();
			linear = 0;
			_nearest.reset();
			nearest = 0;
		}
	}

	namespace shaders
	{
		static std::string src_dir = "../../../src/"; // LATER better way of storing src_dir path.
		static std::string shaders_dir = src_dir + "rendering/shaders/";

		static std::unique_ptr<rendering::Shader> _sprite_batch = nullptr;
		GLuint sprite_batch;
		static std::unique_ptr<rendering::Shader> _polygon_batch = nullptr;
		GLuint polygon_batch;
		static std::unique_ptr<rendering::Shader> _ellipse_batch = nullptr;
		GLuint ellipse_batch;

		void load()
		{
			_sprite_batch = rendering::load_shader((shaders_dir + "sprite_batch.vert").c_str(), (shaders_dir + "sprite_batch.frag").c_str());
			sprite_batch = *_sprite_batch;
			_polygon_batch = rendering::load_shader((shaders_dir + "polygon_batch.vert").c_str(), (shaders_dir + "polygon_batch.frag").c_str());
			polygon_batch = *_polygon_batch;
			_ellipse_batch = rendering::load_shader((shaders_dir + "ellipse_batch.vert").c_str(), (shaders_dir + "ellipse_batch.frag").c_str());
			ellipse_batch = *_ellipse_batch;
		}

		void unload()
		{
			_sprite_batch.reset();
			sprite_batch = 0;
			_polygon_batch.reset();
			polygon_batch = 0;
			_ellipse_batch.reset();
			ellipse_batch = 0;
		}
	}

	void load_resources()
	{
		samplers::load();
		shaders::load();
	}

	void unload_resources()
	{
		samplers::unload();
		shaders::unload();
	}
}
