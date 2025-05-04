#include "Shaders.h"

#include "graphics/backend/basic/Shaders.h"

namespace oly
{

	namespace shaders
	{
		static std::string src_dir = "../../../src/"; // LATER better way of storing src_dir path. use macro?
		static std::string shaders_dir = src_dir + "shaders/"; // LATER embed shaders into built binary

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

		void load()
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

		void unload()
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
}
