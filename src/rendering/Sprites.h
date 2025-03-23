#pragma once

#include "core/Core.h"
#include "math/Transforms.h"
#include "math/DataStructures.h"

#include <set>
#include <unordered_set>
#include <algorithm>

namespace oly
{
	// LATER add resizing mechanism for SSBOs
	class SpriteBatch
	{
		friend class Sprite;

		GLuint shader;
		rendering::VertexArray vao;
		rendering::GLBuffer ebo;

		GLuint projection_location;

		std::vector<rendering::BindlessTextureRes> textures;

		struct TexData
		{
			GLuint64 handle;
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

		rendering::GLBuffer tex_data_ssbo;
		rendering::GLBuffer quad_info_ssbo;
		rendering::GLBuffer quad_transform_ssbo;

	public:
		struct TexUVRect
		{
			glm::vec2 uvs[4] = {};
		};
	private:
		rendering::GLBuffer tex_coords_ubo;
	public:
		struct Modulation
		{
			glm::vec4 colors[4] = {};
		};
	private:
		rendering::GLBuffer modulation_ubo;

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
		SpriteBatch(Capacity capacity, const glm::vec4& projection_bounds);

		void draw() const;

		void set_texture(size_t pos, const rendering::BindlessTextureRes& texture, rendering::ImageDimensions dim);
		void refresh_handle(size_t pos, rendering::ImageDimensions dim);
		void refresh_handle(size_t pos);
		void set_uvs(size_t pos, const TexUVRect& tex_coords) const;
		void set_modulation(size_t pos, const Modulation& modulation) const;
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
			friend SpriteBatch;
			QuadInfo* _info = nullptr;
			glm::mat3* _transform = nullptr;
			SpriteBatch* _sprite_batch = nullptr;
			QuadPos _ssbo_pos = -1;

		public:
			Quad() = default;
			Quad(const Quad&) = delete;
			Quad(Quad&& other) noexcept = default;

			QuadInfo& info() { return *_info; }
			const QuadInfo& info() const { return *_info; }
			glm::mat3& transform() { return *_transform; }
			const glm::mat3& transform() const { return *_transform; }
			const SpriteBatch& sprite_batch() const { return *_sprite_batch; }
			SpriteBatch& sprite_batch() { return *_sprite_batch; }
			QuadPos index_pos() const { return _sprite_batch->z_order.range_of(_ssbo_pos); }
			void set_z_index(QuadPos z) { _sprite_batch->move_quad_order(index_pos(), z); }
			void move_z_index(int by) { _sprite_batch->move_quad_order(index_pos(), std::clamp((int)index_pos() + by, 0, (int)_sprite_batch->quads.size() - 1)); }

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
		std::set<QuadPos> dirty_quad_infos;
		std::set<QuadPos> dirty_transforms;
		std::set<QuadPos> dirty_indices;

	public:
		enum class BufferSendType
		{
			SUBDATA,
			MAP
		};

		BufferSendType send_types[3] = { BufferSendType::SUBDATA, BufferSendType::SUBDATA, BufferSendType::SUBDATA };
		void process();

	private:
		void process_set(std::set<size_t>& set, Dirty flag, void* data, GLuint buf, size_t element_size);
		std::unordered_set<Sprite*> sprites;
	};

	class Sprite
	{
		friend SpriteBatch;
		SpriteBatch::Quad* _quad;
		std::unique_ptr<Transformer2D> _transformer;
			
	public:
		Sprite(SpriteBatch* sprite_batch, SpriteBatch::QuadPos pos);
		Sprite(SpriteBatch* sprite_batch, SpriteBatch::QuadPos pos, std::unique_ptr<Transformer2D>&& transformer);
		Sprite(const Sprite&) = delete;
		Sprite(Sprite&&) noexcept;
		~Sprite();
		Sprite& operator=(Sprite&&) noexcept;

		const SpriteBatch& sprite_batch() const { return _quad->sprite_batch(); }
		SpriteBatch& sprite_batch() { return _quad->sprite_batch(); }
		const SpriteBatch::Quad& quad() const { return *_quad; }
		SpriteBatch::Quad& quad() { return *_quad; }
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
