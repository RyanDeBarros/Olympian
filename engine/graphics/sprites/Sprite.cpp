#include "Sprite.h"

#include "core/context/rendering/Sprites.h"
#include "core/context/rendering/Textures.h"
#include "assets/Loader.h"

namespace oly::rendering
{
	void StaticSprite::draw() const
	{
		ref.draw_quad();
		if (ref.is_in_context()) [[likely]]
			context::internal::set_sprite_batch_rendering(true);
	}

	void Sprite::draw() const
	{
		if (transformer.flush())
			ref.set_transform(transformer.global());
		ref.draw_quad();
		if (ref.is_in_context()) [[likely]]
			context::internal::set_sprite_batch_rendering(true);
	}

	Sprite Sprite::load(TOMLNode node, const char* source)
	{
		if (!node)
			return {};

		_OLY_ENGINE_LOG_DEBUG("ASSETS") << "Parsing sprite [" << (source ? source : "") << "]..." << LOG.nl;

		Sprite sprite;
		sprite.transformer = Transformer2D::load(node["transformer"]);

		auto texture = node["texture"].value<std::string>();
		if (texture)
		{
			graphics::BindlessTextureRef btex = context::load_texture(*texture, assets::parse_uint_or(node["texture_index"], 0));
			sprite.set_texture(btex, context::get_texture_dimensions(btex));
		}

		if (auto toml_modulation = node["modulation"])
		{
			glm::vec4 modulation;
			if (assets::parse_vec(toml_modulation, modulation))
				sprite.set_modulation(modulation);
			else
				_OLY_ENGINE_LOG_WARNING("ASSETS") << "Cannot parse \"modulation\" field." << LOG.nl;
		}

		if (auto toml_tex_coords = node["tex_coords"])
		{
			glm::vec4 uvs;
			if (assets::parse_vec(toml_tex_coords, uvs))
				sprite.set_tex_coords({ .x1 = uvs[0], .x2 = uvs[1], .y1 = uvs[2], .y2 = uvs[3] });
			else
				_OLY_ENGINE_LOG_WARNING("ASSETS") << "Cannot parse \"tex_coords\" field." << LOG.nl;
		}

		if (auto toml_frame_format = node["frame_format"])
		{
			if (auto mode = toml_frame_format["mode"].value<std::string>())
			{
				std::string mode_str = *mode;
				if (mode_str == "single")
				{
					if (texture)
						sprite.set_frame_format(graphics::setup_anim_frame_format_single(*texture, assets::parse_uint_or(toml_frame_format["frame"], 0)));
					else
						_OLY_ENGINE_LOG_WARNING("ASSETS") << "No texture was set for single frame format." << LOG.nl;
				}
				else if (mode_str == "auto")
				{
					if (texture)
						sprite.set_frame_format(graphics::setup_anim_frame_format(*texture, assets::parse_float_or(toml_frame_format["speed"], 1.0f),
							assets::parse_uint_or(toml_frame_format["starting_frame"], 0)));
					else
						_OLY_ENGINE_LOG_WARNING("ASSETS") << "No texture was set for auto frame format." << LOG.nl;
				}
				else
					_OLY_ENGINE_LOG_WARNING("ASSETS") << "Unrecognized frame format mode \"" << mode_str << "\"." << LOG.nl;
			}
			else
			{
				sprite.set_frame_format({
					.starting_frame = assets::parse_uint_or(toml_frame_format["starting_frame"], 0),
					.num_frames = assets::parse_uint_or(toml_frame_format["num_frames"], 0),
					.starting_time = assets::parse_float_or(toml_frame_format["starting_time"], 0.0f),
					.delay_seconds = assets::parse_float_or(toml_frame_format["delay_seconds"], 0.0f)
					});
			};
		}

		_OLY_ENGINE_LOG_DEBUG("ASSETS") << "...Sprite [" << (source ? source : "") << "] parsed." << LOG.nl;

		return sprite;
	}
}
