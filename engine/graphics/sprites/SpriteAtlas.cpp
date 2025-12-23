#include "SpriteAtlas.h"

#include "core/util/Time.h"
#include "core/util/Loader.h"

namespace oly::rendering
{
	void internal::SpriteAtlasManager::on_tick()
	{
		for (SpriteAtlas* atlas : atlases)
			if (atlas->auto_tick) [[likely]]
				atlas->on_tick();
	}

	SpriteAtlas::SpriteAtlas()
	{
		internal::SpriteAtlasManager::instance().atlases.insert(this);
	}

	SpriteAtlas::SpriteAtlas(Unbatched)
		: sprite(UNBATCHED)
	{
		internal::SpriteAtlasManager::instance().atlases.insert(this);
	}

	SpriteAtlas::SpriteAtlas(SpriteBatch& batch)
		: sprite(batch)
	{
		internal::SpriteAtlasManager::instance().atlases.insert(this);
	}

	SpriteAtlas::SpriteAtlas(Sprite&& sprite)
		: sprite(std::move(sprite))
	{
		internal::SpriteAtlasManager::instance().atlases.insert(this);
	}

	SpriteAtlas::SpriteAtlas(const SpriteAtlas& other)
		: sprite(other.sprite), atlas(other.atlas), anim_format(other.anim_format), current_frame(other.current_frame)
	{
		internal::SpriteAtlasManager::instance().atlases.insert(this);
	}

	SpriteAtlas::SpriteAtlas(SpriteAtlas&& other) noexcept
		: sprite(std::move(other.sprite)), atlas(std::move(other.atlas)), anim_format(std::move(other.anim_format)), current_frame(other.current_frame)
	{
		internal::SpriteAtlasManager::instance().atlases.insert(this);
	}

	SpriteAtlas::~SpriteAtlas()
	{
		internal::SpriteAtlasManager::instance().atlases.erase(this);
	}

	void SpriteAtlas::draw() const
	{
		sprite.draw();
	}

	void SpriteAtlas::on_tick() const
	{
		if (anim_format.delay_seconds != 0.0f)
			select(anim_format.starting_frame + (int)floor((TIME.now<>() - anim_format.starting_time) / anim_format.delay_seconds));
	}

	void SpriteAtlas::select_static_frame(GLuint frame)
	{
		anim_format.delay_seconds = 0.0f;
		select(frame);
	}

	void SpriteAtlas::uvs_changed() const
	{
		current_frame = -1;
	}

	void SpriteAtlas::setup_uniform(GLuint rows, GLuint cols, float delay_seconds, bool row_major, bool row_up)
	{
		atlas.clear();
		if (row_major)
		{
			if (row_up)
			{
				for (GLuint row = 0; row < rows; ++row)
					for (GLuint col = 0; col < cols; ++col)
						atlas.push_back(math::UVRect::from_grid(row, col, rows, cols));
			}
			else
			{
				for (int row = rows - 1; row >= 0; --row)
					for (GLuint col = 0; col < cols; ++col)
						atlas.push_back(math::UVRect::from_grid(row, col, rows, cols));
			}
		}
		else
		{
			if (row_up)
			{
				for (GLuint col = 0; col < cols; ++col)
					for (GLuint row = 0; row < rows; ++row)
						atlas.push_back(math::UVRect::from_grid(row, col, rows, cols));
			}
			else
			{
				for (GLuint col = 0; col < cols; ++col)
					for (int row = rows - 1; row >= 0; --row)
						atlas.push_back(math::UVRect::from_grid(row, col, rows, cols));
			}
		}
		uvs_changed();
		anim_format.num_frames = rows * cols;
		anim_format.delay_seconds = delay_seconds;
		anim_format.starting_frame = 0;
		anim_format.starting_time = 0.0f;

		glm::vec2 dimensions;
		auto texture = sprite.get_texture(dimensions);
		sprite.set_texture(texture, dimensions * glm::vec2{ 1.0f / cols, 1.0f / rows });
	}

	void SpriteAtlas::select(GLuint frame) const
	{
		frame %= anim_format.num_frames;
		if (current_frame != frame)
		{
			current_frame = frame;
			sprite.set_tex_coords(atlas[frame]);
		}
	}

	static SpriteAtlas load_sprite_atlas(TOMLNode node, const DebugTrace* trace)
	{
		if (!node)
			return {};

		SpriteAtlas sprite_atlas(trace ? Sprite::load(node["sprite"], *trace) : Sprite::load(node["sprite"]));

		GLuint rows, cols;
		float delay_seconds;
		if (io::parse_uint(node["rows"], rows) && io::parse_uint(node["cols"], cols) && io::parse_float(node["delay_seconds"], delay_seconds))
			sprite_atlas.setup_uniform(rows, cols, delay_seconds, io::parse_bool_or(node["row_major"], true), io::parse_bool_or(node["row_up"], true));
		else
		{
			GLuint static_frame;
			if (io::parse_uint(node["static_frame"], static_frame))
				sprite_atlas.select_static_frame(static_frame);
		}

		sprite_atlas.anim_format.starting_frame = io::parse_int_or(node["starting_frame"], 0);
		sprite_atlas.anim_format.starting_time = io::parse_float_or(node["starting_time"], 0.0f);

		sprite_atlas.auto_tick = io::parse_bool_or(node["auto_tick"], true);

		return sprite_atlas;
	}

	SpriteAtlas SpriteAtlas::load(TOMLNode node)
	{
		return load_sprite_atlas(node, nullptr);
	}

	SpriteAtlas SpriteAtlas::load(TOMLNode node, const DebugTrace& trace)
	{
		auto scope = trace.scope("ASSETS", "oly::rendering::SpriteAtlas::load()");
		return load_sprite_atlas(node, &trace);
	}
}
