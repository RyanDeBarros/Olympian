#include "Sprite.h"

#include "core/context/rendering/Sprites.h"
#include "core/context/rendering/Textures.h"
#include "core/util/Loader.h"
#include "graphics/sprites/Definitions.h"

#include ".gen/keys/Sprite.inl"

#include ".gen/enums/rendering/FrameFormat.inl"

namespace oly::rendering
{
	void StaticSprite::draw() const
	{
		ref.draw_quad();
	}

	void Sprite::draw() const
	{
		if (transformer.flush())
			ref.set_transform(transformer.global());
		ref.draw_quad();
	}

	Sprite Sprite::load(TOMLNode node)
	{
		if (!node)
			return {};

		Sprite sprite;
		sprite.transformer = Transformer2D::load(io::parse_key(node, _gen::keys::Sprite::Transformer));

		auto texture = io::parse_key(node, _gen::keys::Sprite::Texture).value<std::string>();
		if (texture)
		{
			graphics::BindlessTextureRef btex = context::load_texture(*texture, io::parse_uint_or(io::parse_key(node, _gen::keys::Sprite::TextureIndex), 0));
			sprite.set_texture(btex, context::get_texture_dimensions(btex));
		}

		if (auto toml_modulation = io::parse_key(node, _gen::keys::Sprite::Modulation))
		{
			glm::vec4 modulation;
			if (io::parse_vec(toml_modulation, modulation))
				sprite.set_modulation(modulation);
			else
				_OLY_ENGINE_LOG_WARNING("ASSETS") << "Cannot parse " << io::key_string(_gen::keys::Sprite::Modulation) << " field" << LOG.nl;
		}

		if (auto toml_tex_coords = io::parse_key(node, _gen::keys::Sprite::TextureCoordinates))
		{
			glm::vec4 uvs;
			if (io::parse_vec(toml_tex_coords, uvs))
				sprite.set_tex_coords({ .x1 = uvs[0], .x2 = uvs[1], .y1 = uvs[2], .y2 = uvs[3] });
			else
				_OLY_ENGINE_LOG_WARNING("ASSETS") << "Cannot parse " << io::key_string(_gen::keys::Sprite::TextureCoordinates) << " field" << LOG.nl;
		}

		if (auto toml_frame_format = io::parse_key(node, _gen::keys::Sprite::FrameFormat))
		{
			if (auto mode = io::parse_uint(io::parse_key(toml_frame_format, _gen::keys::Sprite::Mode)))
			{
				try
				{
					switch (_gen::rendering::FrameFormat::val(*mode))
					{
					case FrameFormat::Single:
						if (texture)
							sprite.set_frame_format(graphics::setup_anim_frame_format_Single(*texture, io::parse_uint_or(io::parse_key(toml_frame_format, _gen::keys::Sprite::Frame), 0)));
						else
							_OLY_ENGINE_LOG_WARNING("ASSETS") << "No texture was set for (single) frame format." << LOG.nl;
						break;
					case FrameFormat::Auto:
						if (texture)
							sprite.set_frame_format(graphics::setup_anim_frame_format(*texture, io::parse_float_or(io::parse_key(toml_frame_format, _gen::keys::Sprite::Speed), 1.0f),
								io::parse_uint_or(io::parse_key(toml_frame_format, _gen::keys::Sprite::StartingFrame), 0)));
						else
							_OLY_ENGINE_LOG_WARNING("ASSETS") << "No texture was set for (auto) frame format." << LOG.nl;
						break;
					default:
						throw std::out_of_range("");
					}
				}
				catch (const std::out_of_range&)
				{
					_OLY_ENGINE_LOG_WARNING("ASSETS") << "Unrecognized frame format mode (" << *mode << ")" << LOG.nl;
				}
			}
			else
			{
				sprite.set_frame_format({
					.starting_frame = io::parse_uint_or(io::parse_key(toml_frame_format, _gen::keys::Sprite::StartingFrame), 0),
					.num_frames = io::parse_uint_or(io::parse_key(toml_frame_format, _gen::keys::Sprite::NumFrames), 0),
					.starting_time = io::parse_float_or(io::parse_key(toml_frame_format, _gen::keys::Sprite::StartingTime), 0.0f),
					.delay_seconds = io::parse_float_or(io::parse_key(toml_frame_format, _gen::keys::Sprite::DelaySeconds), 0.0f)
					});
			};
		}

		sprite.set_camera_invariant(io::parse_bool_or(io::parse_key(node, _gen::keys::Sprite::CameraInvariant), false));

		return sprite;
	}

	Sprite Sprite::load(TOMLNode node, const DebugTrace& trace)
	{
		auto scope = trace.scope("ASSETS", "oly::rendering::Sprite::load()");
		return load(node);
	}
}
