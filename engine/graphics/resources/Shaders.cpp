#include "Shaders.h"

#include "graphics/backend/basic/Shaders.h"

namespace oly::graphics::internal_shaders
{
	static std::string shaders_dir = "../../../engine/internal/shaders/"; // LATER embed shaders into built binary

	static std::unique_ptr<Shader> _sprite_batch = nullptr;
	GLuint sprite_batch;
	static std::unique_ptr<Shader> _polygon_batch = nullptr;
	GLuint polygon_batch;
	static std::unique_ptr<Shader> _ellipse_batch = nullptr;
	GLuint ellipse_batch;
	static std::unique_ptr<Shader> _text_batch = nullptr;
	GLuint text_batch;

	void load()
	{
		_sprite_batch = load_shader((shaders_dir + "sprite_batch.vert").c_str(), (shaders_dir + "sprite_batch.frag").c_str());
		sprite_batch = *_sprite_batch;
		_polygon_batch = load_shader((shaders_dir + "polygon_batch.vert").c_str(), (shaders_dir + "polygon_batch.frag").c_str());
		polygon_batch = *_polygon_batch;
		_ellipse_batch = load_shader((shaders_dir + "ellipse_batch.vert").c_str(), (shaders_dir + "ellipse_batch.frag").c_str());
		ellipse_batch = *_ellipse_batch;
		_text_batch = load_shader((shaders_dir + "text_batch.vert").c_str(), (shaders_dir + "text_batch.frag").c_str());
		text_batch = *_text_batch;
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
	}
}
