#include "Resources.h"

namespace oly
{
	namespace samplers
	{
		static std::unique_ptr<rendering::Sampler> _linear = nullptr;
		GLuint linear = 0;
		static std::unique_ptr<rendering::Sampler> _nearest = nullptr;
		GLuint nearest = 0;
		static std::unique_ptr<rendering::Sampler> _linear_mipmap_linear = nullptr;
		GLuint linear_mipmap_linear = 0;
		static std::unique_ptr<rendering::Sampler> _linear_mipmap_nearest = nullptr;
		GLuint linear_mipmap_nearest = 0;
		static std::unique_ptr<rendering::Sampler> _nearest_mipmap_linear = nullptr;
		GLuint nearest_mipmap_linear = 0;
		static std::unique_ptr<rendering::Sampler> _nearest_mipmap_nearest = nullptr;
		GLuint nearest_mipmap_nearest = 0;

		static void load()
		{
			_linear = std::make_unique<rendering::Sampler>();
			_linear->set_parameter_i(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			_linear->set_parameter_i(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			linear = *_linear;

			_nearest = std::make_unique<rendering::Sampler>();
			_nearest->set_parameter_i(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			_nearest->set_parameter_i(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			nearest = *_nearest;

			_linear_mipmap_linear = std::make_unique<rendering::Sampler>();
			_linear_mipmap_linear->set_parameter_i(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			_linear_mipmap_linear->set_parameter_i(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			linear_mipmap_linear = *_linear_mipmap_linear;

			_linear_mipmap_nearest = std::make_unique<rendering::Sampler>();
			_linear_mipmap_nearest->set_parameter_i(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			_linear_mipmap_nearest->set_parameter_i(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			linear_mipmap_nearest = *_linear_mipmap_nearest;

			_nearest_mipmap_linear = std::make_unique<rendering::Sampler>();
			_nearest_mipmap_linear->set_parameter_i(GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			_nearest_mipmap_linear->set_parameter_i(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			nearest_mipmap_linear = *_nearest_mipmap_linear;

			_nearest_mipmap_nearest = std::make_unique<rendering::Sampler>();
			_nearest_mipmap_nearest->set_parameter_i(GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			_nearest_mipmap_nearest->set_parameter_i(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			nearest_mipmap_nearest = *_nearest_mipmap_nearest;
		}

		static void unload()
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

	namespace shaders
	{
		static std::string src_dir = "../../../src/"; // LATER better way of storing src_dir path. use macro?
		static std::string shaders_dir = src_dir + "rendering/shaders/";

		static std::unique_ptr<rendering::Shader> _sprite_batch = nullptr;
		GLuint sprite_batch;
		static std::unique_ptr<rendering::Shader> _polygon_batch = nullptr;
		GLuint polygon_batch;
		static std::unique_ptr<rendering::Shader> _ellipse_batch = nullptr;
		GLuint ellipse_batch;
		static std::unique_ptr<rendering::Shader> _text_batch = nullptr;
		GLuint text_batch;
		static std::unique_ptr<rendering::Shader> _polygonal_particle = nullptr;
		GLuint polygonal_particle;
		static std::unique_ptr<rendering::Shader> _elliptic_particle = nullptr;
		GLuint elliptic_particle;

		std::unordered_map<GLuint, std::unordered_map<std::string, GLuint>> locations;

		GLuint location(GLuint shader, const std::string& uniform)
		{
			auto locmap = locations.find(shader);
			if (locmap == locations.end())
				locmap = locations.insert({ shader, {} }).first;
			auto it = locmap->second.find(uniform);
			if (it == locmap->second.end())
				it = locmap->second.insert({ uniform, glGetUniformLocation(shader, uniform.c_str()) }).first;
			return it->second;
		}

		static void load()
		{
			_sprite_batch = rendering::load_shader((shaders_dir + "sprite_batch.vert").c_str(), (shaders_dir + "sprite_batch.frag").c_str());
			sprite_batch = *_sprite_batch;
			_polygon_batch = rendering::load_shader((shaders_dir + "polygon_batch.vert").c_str(), (shaders_dir + "polygon_batch.frag").c_str());
			polygon_batch = *_polygon_batch;
			_ellipse_batch = rendering::load_shader((shaders_dir + "ellipse_batch.vert").c_str(), (shaders_dir + "ellipse_batch.frag").c_str());
			ellipse_batch = *_ellipse_batch;
			_text_batch = rendering::load_shader((shaders_dir + "text_batch.vert").c_str(), (shaders_dir + "text_batch.frag").c_str());
			text_batch = *_text_batch;
			_polygonal_particle = rendering::load_shader((shaders_dir + "polygonal_particle.vert").c_str(), (shaders_dir + "polygonal_particle.frag").c_str());
			polygonal_particle = *_polygonal_particle;
			_elliptic_particle = rendering::load_shader((shaders_dir + "elliptic_particle.vert").c_str(), (shaders_dir + "elliptic_particle.frag").c_str());
			elliptic_particle = *_elliptic_particle;
		}

		static void unload()
		{
			_sprite_batch.reset();
			sprite_batch = 0;
			_polygon_batch.reset();
			polygon_batch = 0;
			_ellipse_batch.reset();
			ellipse_batch = 0;
			_text_batch.reset();
			text_batch = 0;
			_polygonal_particle.reset();
			polygonal_particle = 0;
			_elliptic_particle.reset();
			elliptic_particle = 0;

			locations.clear();
		}
	}

	namespace textures
	{
		rendering::BindlessTextureRes white1x1_1;
		
		static void load()
		{
			{
				rendering::ImageDimensions dim{ 1, 1, 1 };
				unsigned char* buf = dim.pxnew();
				buf[0] = 255;
				rendering::Image image(buf, dim);
				white1x1_1 = move_shared(rendering::load_bindless_texture_2d(image));
				white1x1_1->set_and_use_handle();
			}
		}

		static void unload()
		{
			white1x1_1.reset();
		}
	}

	void load_resources()
	{
		samplers::load();
		shaders::load();
		textures::load();
	}

	void unload_resources()
	{
		samplers::unload();
		shaders::unload();
		textures::unload();
	}
}
