#include "Resources.h"

namespace oly
{
	namespace apollo
	{
		namespace shaders
		{
			inline std::string src_dir = "../../../src/"; // TODO better way of storing src_dir path.
			inline std::string shaders_dir = src_dir + "rendering/apollo/shaders/";
			rendering::ShaderRes sprite_list;

			static void load_sprite_list()
			{
				sprite_list = rendering::load_shader((shaders_dir + "sprite_list.vert").c_str(), (shaders_dir + "sprite_list.frag").c_str());
			}

			void load()
			{
				load_sprite_list();
			}

			void unload()
			{
				sprite_list.reset();
			}
		}
	}
}

