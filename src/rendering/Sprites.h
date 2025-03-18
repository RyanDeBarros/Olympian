#pragma once

#include "core/Core.h"
#include "Transforms.h"
#include "util/MathDS.h"

#include <set>
#include <unordered_set>
#include <algorithm>

namespace oly
{
	// LATER add resizing mechanism for SSBOs
	class SpriteList
	{
		friend class Sprite;

		oly::rendering::ShaderRes shader;
		oly::rendering::VertexArray vao;
		oly::rendering::GLBuffer ebo;

		std::vector<oly::rendering::TextureRes> textures;

		struct TexData
		{
			oly::rendering::BindlessTextureHandle handle;
			glm::vec2 dimensions = {};
		};
		struct QuadInfo
		{
			GLuint tex_slot = 0;
			GLuint tex_coord_slot = 0;
			GLuint color_slot = 0;
		};
		std::vector<QuadInfo> quad_infos;
		std::vector<glm::mat3> quad_transforms;

		oly::rendering::GLBuffer tex_data_ssbo;
		oly::rendering::GLBuffer quad_info_ssbo;
		oly::rendering::GLBuffer quad_transform_ssbo;

	public:
		struct TexUVRect
		{
			glm::vec2 uvs[4] = {};
		};
	private:
		oly::rendering::GLBuffer tex_coords_ubo;
	public:
		struct Modulation
		{
			glm::vec4 colors[4] = {};
		};
	private:
		oly::rendering::GLBuffer modulation_ubo;

		struct QuadIndexLayout
		{
			GLushort data[6];
		};
		std::vector<QuadIndexLayout> indices;

	public:
		struct Capacity
		{
			size_t quads = 0;
			size_t textures = 1;
			size_t uvs = 1;
			size_t modulations = 1;
		};

	private:
		Capacity capacity;

	public:
		SpriteList(Capacity capacity, const glm::vec4& projection_bounds);

		void draw() const;

		GLuint get_shader() const { return *shader; }
		void set_texture(const oly::rendering::TextureRes& texture, oly::rendering::ImageDimensions dim, size_t pos);
		void set_uvs(const TexUVRect& tex_coords, size_t pos) const;
		void set_modulation(const Modulation& modulation, size_t pos) const;
		void set_projection(const glm::vec4& projection_bounds) const;

		typedef size_t QuadPos;
	private:
		struct
		{
			QuadPos first = 0;
			QuadPos count = 0;
			size_t offset = 0;
		} draw_spec;

	public:
		void get_draw_spec(QuadPos& first, QuadPos& count) const { first = draw_spec.first; count = draw_spec.count; }
		void set_draw_spec(QuadPos first, QuadPos count);

		class Quad
		{
			friend SpriteList;
			QuadInfo* _info = nullptr;
			glm::mat3* _transform = nullptr;
			SpriteList* _sprite_list = nullptr;
			QuadPos _ssbo_pos = -1;

		public:
			Quad() = default;
			Quad(const Quad&) = delete;
			Quad(Quad&& other) noexcept = default;

			QuadInfo& info() { return *_info; }
			const QuadInfo& info() const { return *_info; }
			glm::mat3& transform() { return *_transform; }
			const glm::mat3& transform() const { return *_transform; }
			const SpriteList& sprite_list() const { return *_sprite_list; }
			SpriteList& sprite_list() { return *_sprite_list; }
			QuadPos index_pos() const { return _sprite_list->z_order.range_of(_ssbo_pos); }
			void set_z_index(QuadPos z) { _sprite_list->move_quad_order(index_pos(), z); }
			void move_z_index(int by) { _sprite_list->move_quad_order(index_pos(), std::clamp((int)index_pos() + by, 0, (int)_sprite_list->quads.size() - 1)); }

			void send_info() const;
			void send_transform() const;
			void send_data() const;
		};
		friend Quad;

	private:
		std::vector<Quad> quads;
		math::IndexBijection<QuadPos> z_order;

	public:
		Quad& get_quad(QuadPos pos);
		void swap_quad_order(QuadPos pos1, QuadPos pos2);
		void move_quad_order(QuadPos from, QuadPos to);

		enum Dirty
		{
			QUAD_INFO,
			TRANSFORM,
			INDICES
		};

	private:
		std::set<QuadPos> dirty[3] = { {}, {}, {} };

	public:
		enum class BufferSendType
		{
			SUBDATA,
			MAP
		};

		BufferSendType send_types[3] = { BufferSendType::SUBDATA, BufferSendType::SUBDATA, BufferSendType::SUBDATA };
		void process();

	private:
		void process_set(Dirty flag, void* data, GLuint buf, size_t element_size);
		std::unordered_set<Sprite*> sprites;
	};

	class Sprite
	{
		friend SpriteList;
		SpriteList* sprite_list;
		SpriteList::Quad* _quad;
		std::unique_ptr<Transformer2D> _transformer;
			
	public:
		Sprite(SpriteList* sprite_list, SpriteList::QuadPos pos);
		Sprite(SpriteList* sprite_list, SpriteList::QuadPos pos, std::unique_ptr<Transformer2D>&& transformer);
		Sprite(const Sprite&) = delete;
		Sprite(Sprite&&) noexcept;
		~Sprite();
		Sprite& operator=(Sprite&&) noexcept;

		const SpriteList* get_sprite_list() const { return sprite_list; }
		SpriteList* get_sprite_list() { return sprite_list; }
		const SpriteList::Quad& quad() const { return *_quad; }
		SpriteList::Quad& quad() { return *_quad; }
		const Transformer2D& transformer() const { return *_transformer; }
		Transformer2D& transformer() { return *_transformer; }
		const Transform2D& local() const { return _transformer->local; }
		Transform2D& local() { return _transformer->local; }
		void post_set() const; // call after modifying local
		void pre_get() const; // call before reading global

	private:
		void flush() const;
	};
}
