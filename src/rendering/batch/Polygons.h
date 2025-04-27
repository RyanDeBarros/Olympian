#pragma once

#include <map>

#include "../SpecializedBuffers.h"
#include "math/Transforms.h"
#include "math/Geometry.h"
#include "util/FixedVector.h"
#include "util/FreeSpaceTracker.h"
#include "util/IDGenerator.h"

namespace oly
{
	namespace rendering
	{
		class Polygonal;

		class PolygonBatch
		{
			friend class Polygonal;
		
		public:
			typedef GLuint Index;
		
		private:
			GLuint shader;
			VertexArray vao;
			PersistentEBO<1> ebo;

			GLuint projection_location;
			GLuint degree_location;

			// TODO PersistentBufferBlock
			LazyPersistentGPUBuffer<glm::vec2> position_vbo;
			LazyPersistentGPUBuffer<glm::vec4> color_vbo;
			LazyPersistentGPUBuffer<glm::mat3> transform_ssbo;

		public:
			typedef StrictIDGenerator<Index>::ID PolygonID;

			struct Capacity
			{
				Index vertices = 0;
				Index indices = 0;
				Index primitives = 0;
				const Index degree = 0;
				const Index polygon_index_count = 0;

				// TODO remove degree ??
				// max(F) = V - 2 + 2H
				// max(H) = [V / 3] - 1
				// --> max(F) = V + 2 * [V / 3] - 4
				// --> index count = 3 * max(F)
				Capacity(Index primitives, Index degree = 6)
					: primitives(primitives), degree(degree), polygon_index_count(3 * degree + 6 * (degree / 3) - 12)
				{
					OLY_ASSERT(degree >= 3);
					OLY_ASSERT(degree * primitives <= UINT_MAX);

					vertices = primitives * degree;
					indices = primitives * polygon_index_count;
				}
			};

		private:
			const Index degree;
			const Index polygon_index_count;

		public:
			PolygonBatch(Capacity capacity, const glm::vec4& projection_bounds);

			void render() const;
			
			glm::vec4 projection_bounds;

		private:
			void set_primitive_points(Index pos, const glm::vec2* points, Index count);
			void set_primitive_colors(Index pos, const glm::vec4* colors, Index count);
			void set_primitive_transform(Index pos, const glm::mat3& transform);
			void set_polygon_primitive(Index vertex_pos, const math::Polygon2D& polygon, const glm::mat3& transform);

		public:
			void set_polygon_transform(Index id, const glm::mat3& transform);

		private:
			PolygonID generate_id(const math::Polygon2DComposite& composite, Index min_range = 0, Index max_range = 0);
			PolygonID generate_id(math::Polygon2DComposite& composite, Index min_range = 0, Index max_range = 0);
			void terminate_id(Index id);
			void resize_range(PolygonID& id, const math::Polygon2DComposite& composite, Index min_range = 0, Index max_range = 0);
			void resize_range(PolygonID& id, math::Polygon2DComposite& composite, Index min_range = 0, Index max_range = 0);

		public:
			bool is_valid_id(Index id) const;
		
		private:
			Range<Index> get_vertex_range(Index id) const;
		
		public:
			void set_polygon(Index id, const math::TriangulatedPolygon2D& polygon, const glm::mat3& transform);
			void set_polygon(Index id, math::TriangulatedPolygon2D&& polygon, const glm::mat3& transform);
			void set_polygon(Index id, const math::Polygon2DComposite& composite, const glm::mat3& transform);
			void set_polygon(Index id, math::Polygon2DComposite& composite, const glm::mat3& transform);
			void set_ngon(Index id, const math::NGonBase& ngon, const glm::mat3& transform);
			void set_bordered_ngon(Index id, const math::NGonBase& ngon, const glm::mat3& transform);

			math::Polygon2DComposite create_ngon(const math::NGonBase& ngon) const;
			math::Polygon2DComposite create_bordered_ngon(const math::NGonBase& ngon) const;

		private:
			StrictFreeSpaceTracker<Index> vertex_free_space;
			std::unordered_map<Index, Range<Index>> polygon_indexer;
			std::map<Index, Index> id_order;
			StrictIDGenerator<Index> id_generator;
		};

		class Polygonal
		{
			friend PolygonBatch;
			PolygonBatch* _batch = nullptr;
			PolygonBatch::PolygonID id;

		public:
			Transformer2D transformer;

			Polygonal(PolygonBatch* batch);
			Polygonal(const Polygonal&) = delete;
			Polygonal(Polygonal&&) noexcept;
			virtual ~Polygonal();
			Polygonal& operator=(Polygonal&&) noexcept;

			const PolygonBatch& batch() const { return *_batch; }
			PolygonBatch& batch() { return *_batch; }
			PolygonBatch::Index get_id() const { return id.get(); }
			bool initialized() const { return id.get() != PolygonBatch::Index(-1); }
			const Transform2D& local() const { return transformer.local; }
			Transform2D& local() { return transformer.local; }
			void post_set() const; // call after modifying local
			void pre_get() const; // call before reading global

			void init(PolygonBatch::Index min_range = 0, PolygonBatch::Index max_range = 0);
			void resize(PolygonBatch::Index min_range = 0, PolygonBatch::Index max_range = 0);
			void send_polygon() const;
			void draw() const;

		protected:
			virtual math::Polygon2DComposite calc_composite() const = 0;
		};

		struct Polygon : public Polygonal
		{
			math::Polygon2D polygon;

			using Polygonal::Polygonal;
			Polygon(Polygon&&) noexcept = default;
			Polygon& operator=(Polygon&&) noexcept = default;

			virtual math::Polygon2DComposite calc_composite() const override;
		};

		struct Composite : public Polygonal
		{
			math::Polygon2DComposite composite;

			using Polygonal::Polygonal;
			Composite(Composite&&) noexcept = default;
			Composite& operator=(Composite&&) noexcept = default;

			virtual math::Polygon2DComposite calc_composite() const override;
		};

		struct NGon : public Polygonal
		{
			bool bordered = false;
			math::NGonBase base;

			using Polygonal::Polygonal;
			NGon(NGon&&) noexcept = default;
			NGon& operator=(NGon&&) noexcept = default;

			virtual math::Polygon2DComposite calc_composite() const override;
		};
	}
}
