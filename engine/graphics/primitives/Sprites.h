#pragma once

#include "external/GLM.h"

#include "core/base/Transforms.h"
#include "core/base/Constants.h"
#include "core/math/Shapes.h"
#include "core/containers/IDGenerator.h"

#include "graphics/backend/basic/Textures.h"
#include "graphics/backend/specialized/ElementBuffers.h"
#include "graphics/backend/specialized/UsageSlotTracker.h"

namespace oly::rendering
{
	struct UVRect
	{
		glm::vec2 uvs[4] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };

		bool operator==(const UVRect&) const = default;

		UVRect& from_rect(const math::Rect2D& rect);
	};
	typedef std::vector<UVRect> UVAtlas;
	struct UVRectHash
	{
		size_t operator()(const UVRect& uvs) const {
			return std::hash<glm::vec2>{}(uvs.uvs[0]) ^ (std::hash<glm::vec2>{}(uvs.uvs[1]) << 1)
				^ (std::hash<glm::vec2>{}(uvs.uvs[2]) << 2) ^ (std::hash<glm::vec2>{}(uvs.uvs[3]) << 3);
		}
	};

	struct ModulationRect
	{
		glm::vec4 colors[4] = { glm::vec4(1.0f), glm::vec4(1.0f), glm::vec4(1.0f), glm::vec4(1.0f) };

		glm::vec4 mix(glm::vec2 uv) const;

		bool operator==(const ModulationRect&) const = default;
	};
	struct ModulationHash
	{
		size_t operator()(const ModulationRect& mod) const {
			return std::hash<glm::vec4>{}(mod.colors[0]) ^ (std::hash<glm::vec4>{}(mod.colors[1]) << 1)
				^ (std::hash<glm::vec4>{}(mod.colors[2]) << 2) ^ (std::hash<glm::vec4>{}(mod.colors[3]) << 3);
		}
	};

	class Sprite;
	class StaticSprite;

	class SpriteBatch
	{
		friend class Sprite;
		friend class StaticSprite;

		graphics::VertexArray vao;
		graphics::PersistentEBO<> ebo;

		struct TexData
		{
			GLuint64 handle = 0;
			glm::vec2 dimensions = {};
		};

		struct QuadInfo
		{
			GLuint tex_slot = 0;
			GLuint tex_coord_slot = 0;
			GLuint color_slot = 0;
			GLuint frame_slot = 0;
		};

		graphics::LightweightSSBO<graphics::Mutability::MUTABLE> tex_data_ssbo;
		enum
		{
			INFO,
			TRANSFORM
		};
		graphics::LazyPersistentGPUBufferBlock<QuadInfo, glm::mat3> quad_ssbo_block;

		const QuadInfo& get_quad_info(GLuint vb_pos) const;
		QuadInfo& set_quad_info(GLuint vb_pos);

		struct
		{
			GLuint projection, modulation, time;
		} shader_locations;
		
	public:
		struct AnimHash
		{
			size_t operator()(const graphics::AnimFrameFormat& anim) const {
				return std::hash<GLuint>{}(anim.starting_frame) ^ (std::hash<GLuint>{}(anim.num_frames) << 1)
					^ (std::hash<float>{}(anim.starting_time) << 2) ^ (std::hash<float>{}(anim.delay_seconds) << 3);
			}
		};

	private:
		struct UBO
		{
			graphics::LightweightUBO<graphics::Mutability::MUTABLE> tex_coords, modulation, anim;

			UBO(GLuint uvs, GLuint modulations, GLuint anims)
				: tex_coords(uvs * sizeof(UVRect)), modulation(modulations * sizeof(ModulationRect)), anim(anims * sizeof(graphics::AnimFrameFormat)) {}
		} ubo;

	public:
		glm::mat3 projection = 1.0f;
		glm::vec4 global_modulation = glm::vec4(1.0f);

		struct Capacity
		{
			Capacity(GLuint initial_sprites, GLuint new_textures, GLuint new_uvs = 0, GLuint new_modulations = 0, GLuint num_anims = 0)
				: sprites(initial_sprites), textures(new_textures + 1), uvs(new_uvs + 1), modulations(new_modulations + 1), anims(num_anims + 1)
			{
				OLY_ASSERT(4 * initial_sprites <= nmax<unsigned int>());
				OLY_ASSERT(uvs <= 500);
				OLY_ASSERT(modulations <= 250);
				OLY_ASSERT(anims <= 1000);
			}

		private:
			friend class SpriteBatch;
			GLuint sprites, textures, uvs, modulations, anims;
		};

		SpriteBatch(Capacity capacity);

		void render() const;

	private:
		StrictIDGenerator<GLuint> vbid_generator;
		typedef StrictIDGenerator<GLuint>::ID VBID;
		VBID gen_sprite_id();
		void erase_sprite_id(const VBID& id);

		struct QuadInfoStore
		{
			struct SizedTexture
			{
				graphics::BindlessTextureRef texture;
				glm::vec2 dimensions = {};

				bool operator==(const SizedTexture& t) const = default;
			};
			struct SizedTextureHash
			{
				size_t operator()(const SizedTexture& t) const { return std::hash<graphics::BindlessTextureRef>{}(t.texture) ^ std::hash<glm::vec2>{}(t.dimensions); }
			};
			graphics::UsageSlotTracker<SizedTexture, SizedTextureHash> textures;
			graphics::UsageSlotTracker<UVRect, UVRectHash> tex_coords;
			graphics::UsageSlotTracker<ModulationRect, ModulationHash> modulations;
			graphics::UsageSlotTracker<graphics::AnimFrameFormat, AnimHash> anims;

			std::unordered_map<graphics::BindlessTextureRef, std::unordered_set<GLuint>> dimensionless_texture_slot_map;
		} quad_info_store;

		void set_texture(GLuint vb_pos, const graphics::BindlessTextureRef& texture, glm::vec2 dimensions);
		void set_tex_coords(GLuint vb_pos, const UVRect& uvs);
		void set_modulation(GLuint vb_pos, const ModulationRect& modulation);
		void set_frame_format(GLuint vb_pos, const graphics::AnimFrameFormat& anim);

		graphics::BindlessTextureRef get_texture(GLuint vb_pos, glm::vec2& dimensions) const;
		UVRect get_tex_coords(GLuint vb_pos) const;
		ModulationRect get_modulation(GLuint vb_pos) const;
		graphics::AnimFrameFormat get_frame_format(GLuint vb_pos) const;

	public:
		void update_texture_handle(const graphics::BindlessTextureRef& texture);
	};

	// ASSET
	class StaticSprite
	{
		SpriteBatch::VBID vbid;

	public:
		StaticSprite();
		StaticSprite(const StaticSprite&);
		StaticSprite(StaticSprite&&) noexcept;
		StaticSprite& operator=(const StaticSprite&);
		StaticSprite& operator=(StaticSprite&&) noexcept;
		~StaticSprite();

		void draw() const;

		void set_texture(const std::string& texture_file, unsigned int texture_index = 0) const;
		void set_texture(const std::string& texture_file, float svg_scale, unsigned int texture_index = 0) const;
		void set_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions) const;
		void set_tex_coords(const UVRect& tex_coords) const;
		void set_tex_coords(const math::Rect2D& rect) const;
		void set_modulation(const ModulationRect& modulation) const;
		void set_modulation(glm::vec4 modulation) const;
		void set_frame_format(const graphics::AnimFrameFormat& anim) const;
		void set_transform(const glm::mat3& transform);

		graphics::BindlessTextureRef get_texture() const;
		graphics::BindlessTextureRef get_texture(glm::vec2& dimensions) const;
		UVRect get_tex_coords() const;
		ModulationRect get_modulation() const;
		graphics::AnimFrameFormat get_frame_format() const;
		glm::mat3 get_transform() const;
	};

	class Sprite
	{
		SpriteBatch::VBID vbid;

	public:
		Transformer2D transformer;

		Sprite();
		Sprite(const Sprite&);
		Sprite(Sprite&&) noexcept;
		Sprite& operator=(const Sprite&);
		Sprite& operator=(Sprite&&) noexcept;
		~Sprite();

		void draw() const;

		void set_texture(const std::string& texture_file, unsigned int texture_index = 0) const;
		void set_texture(const std::string& texture_file, float svg_scale, unsigned int texture_index = 0) const;
		void set_texture(const graphics::BindlessTextureRef& texture, glm::vec2 dimensions) const;
		void set_tex_coords(const UVRect& tex_coords) const;
		void set_tex_coords(const math::Rect2D& rect) const;
		void set_modulation(const ModulationRect& modulation) const;
		void set_modulation(glm::vec4 modulation) const;
		void set_frame_format(const graphics::AnimFrameFormat& anim) const;

		graphics::BindlessTextureRef get_texture() const;
		graphics::BindlessTextureRef get_texture(glm::vec2& dimensions) const;
		UVRect get_tex_coords() const;
		ModulationRect get_modulation() const;
		graphics::AnimFrameFormat get_frame_format() const;

		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }
	};

	typedef SmartHandle<Sprite> SpriteRef;
}
