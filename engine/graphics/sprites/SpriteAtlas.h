#pragma once

#include "graphics/sprites/Sprite.h"

#include <unordered_set>

namespace oly::rendering
{
	struct SpriteAtlas;

	namespace internal
	{
		class SpriteAtlasManager
		{
			friend struct SpriteAtlas;
			std::unordered_set<SpriteAtlas*> atlases;

			SpriteAtlasManager() = default;
			SpriteAtlasManager(const SpriteAtlasManager&) = delete;
			SpriteAtlasManager(SpriteAtlasManager&&) noexcept = delete;

		public:
			static SpriteAtlasManager& instance()
			{
				static SpriteAtlasManager manager;
				return manager;
			}

			void clear()
			{
				atlases.clear();
			}

			void on_tick();
		};
	}

	struct SpriteAtlas
	{
		Sprite sprite;
		std::vector<math::UVRect> atlas;
		graphics::AnimFrameFormat anim_format;
		bool auto_tick = true;
		
	private:
		mutable GLuint current_frame = -1;

	public:
		SpriteAtlas();
		SpriteAtlas(Unbatched);
		SpriteAtlas(SpriteBatch& batch);
		SpriteAtlas(Sprite&& sprite);
		SpriteAtlas(const SpriteAtlas&);
		SpriteAtlas(SpriteAtlas&&) noexcept;
		~SpriteAtlas();

		void draw() const;
		void on_tick() const;

		void select_static_frame(GLuint frame);
		void uvs_changed() const;
		void setup_uniform(GLuint rows, GLuint cols, float delay_seconds, bool row_major = true, bool row_up = true);

	private:
		void select(GLuint frame) const;

	public:
		static SpriteAtlas load(TOMLNode node);
		static SpriteAtlas load(TOMLNode node, const DebugTrace& trace);
	};

	typedef SmartReference<SpriteAtlas> SpriteAtlasRef;
}
