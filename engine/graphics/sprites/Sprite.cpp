#include "Sprite.h"

#include "core/context/rendering/Sprites.h"
#include "core/context/rendering/Textures.h"
#include "core/util/Parser.h"
#include "graphics/sprites/Definitions.h"

#include "definitions/Keys.h"

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

		assets::Parser parser(node);

		Sprite sprite;
		sprite.transformer = Transformer2D::load(parser.field(detail::Key::Transformer));

		auto texture = parser.optional<std::string>(detail::Key::Texture)();
		if (texture)
		{
			graphics::BindlessTextureRef btex = context::load_texture(*texture, parser.defaulted(detail::Key::TextureIndex)(0u));
			sprite.set_texture(btex, context::get_texture_dimensions(btex));
		}

		if (auto modulation = parser.optional<glm::vec4>(detail::Key::Modulation)())
			sprite.set_modulation(*modulation);

		if (auto uvs = parser.optional<glm::vec4>(detail::Key::TextureCoordinates)())
			sprite.set_tex_coords({ .x1 = (*uvs)[0], .x2 = (*uvs)[1], .y1 = (*uvs)[2], .y2 = (*uvs)[3] });

		if (auto ff_parser = parser.optional(detail::Key::FrameFormat).subparser())
		{
			if (auto mode = ff_parser->optional<FrameFormat>(detail::Key::Mode)())
			{
				switch (*mode)
				{
				case FrameFormat::Single:
					if (texture)
						sprite.set_frame_format(graphics::setup_anim_frame_format_Single(*texture, ff_parser->defaulted(detail::Key::Frame)(0u)));
					else
						_OLY_ENGINE_LOG_WARNING("ASSETS") << "No texture was set for (single) frame format." << LOG.nl;
					break;
				case FrameFormat::Auto:
					if (texture)
						sprite.set_frame_format(graphics::setup_anim_frame_format(*texture, ff_parser->defaulted(detail::Key::Speed)(1.f),
							ff_parser->defaulted(detail::Key::StartingFrame)(0u)));
					else
						_OLY_ENGINE_LOG_WARNING("ASSETS") << "No texture was set for (auto) frame format." << LOG.nl;
					break;
				}
			}
			else
			{
				sprite.set_frame_format({
					.starting_frame = ff_parser->defaulted(detail::Key::StartingFrame)(0u),
					.num_frames = ff_parser->defaulted(detail::Key::NumFrames)(0u),
					.starting_time = ff_parser->defaulted(detail::Key::StartingTime)(0.f),
					.delay_seconds = ff_parser->defaulted(detail::Key::DelaySeconds)(0.f)
					});
			};
		}

		sprite.set_camera_invariant(parser.defaulted(detail::Key::CameraInvariant)(false));

		return sprite;
	}

	Sprite Sprite::load(TOMLNode node, const DebugTrace& trace)
	{
		auto scope = trace.scope("ASSETS", "oly::rendering::Sprite::load()");
		return load(node);
	}
}
