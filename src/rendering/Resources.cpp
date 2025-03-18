#include "Resources.h"

namespace oly
{
	namespace shaders
	{
		inline std::string src_dir = "../../../src/"; // LATER better way of storing src_dir path.
		inline std::string shaders_dir = src_dir + "rendering/shaders/";
			
		static rendering::ShaderRes _sprite_list;

		rendering::ShaderRes sprite_list()
		{
			if (_sprite_list == nullptr)
				_sprite_list = rendering::load_shader((shaders_dir + "sprite_list.vert").c_str(), (shaders_dir + "sprite_list.frag").c_str());
			return _sprite_list;
		}

		void unload()
		{
			_sprite_list.reset();
		}
	}
}

