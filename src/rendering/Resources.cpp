#include "Resources.h"

namespace oly
{
	namespace shaders
	{
		inline std::string src_dir = "../../../src/"; // LATER better way of storing src_dir path.
		inline std::string shaders_dir = src_dir + "rendering/shaders/";

		static rendering::ShaderRes _sprite_batch;
		rendering::ShaderRes sprite_batch()
		{
			if (_sprite_batch == nullptr)
				_sprite_batch = rendering::load_shader((shaders_dir + "sprite_batch.vert").c_str(), (shaders_dir + "sprite_batch.frag").c_str());
			return _sprite_batch;
		}

		static rendering::ShaderRes _polygon_batch;
		rendering::ShaderRes polygon_batch()
		{
			if (_polygon_batch == nullptr)
				_polygon_batch = rendering::load_shader((shaders_dir + "polygon_batch.vert").c_str(), (shaders_dir + "polygon_batch.frag").c_str());
			return _polygon_batch;
		}

		static rendering::ShaderRes _ellipse_batch;
		rendering::ShaderRes ellipse_batch()
		{
			if (_ellipse_batch == nullptr)
				_ellipse_batch = rendering::load_shader((shaders_dir + "ellipse_batch.vert").c_str(), (shaders_dir + "ellipse_batch.frag").c_str());
			return _ellipse_batch;
		}

		void unload()
		{
			_sprite_batch.reset();
			_polygon_batch.reset();
			_ellipse_batch.reset();
		}
	}
}

