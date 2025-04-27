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

			// TODO PersistentBufferBlock
			LazyPersistentGPUBuffer<glm::vec2> position_vbo;
			LazyPersistentGPUBuffer<glm::vec4> color_vbo;
			LazyPersistentGPUBuffer<GLuint> transform_index_vbo;
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

			PolygonBatch(Capacity capacity, const glm::vec4& projection_bounds);

			void render() const;
			
			glm::vec4 projection_bounds;

		private:
			void set_primitive_points(Range<Index> vbo_range, const glm::vec2* points, Index count);
			void set_primitive_colors(Range<Index> vbo_range, const glm::vec4* colors, Index count);
			void set_polygon(Index id, const math::Polygon2DComposite& composite);
			void set_polygon_transform(Index id, const glm::mat3& transform);

			PolygonID generate_id(Index vertices);
			void terminate_id(Index id);
			void resize_range(PolygonID& id, Index vertices);
			Range<Index> get_vertex_range(Index id) const;
			bool is_valid_id(Index id) const;

			StrictFreeSpaceTracker<Index> vertex_free_space;
			std::unordered_map<Index, Range<Index>> polygon_indexer;
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
			const Transform2D& local() const { return transformer.local; }
			Transform2D& local() { return transformer.local; }
			void post_set() const; // call after modifying local
			void pre_get() const; // call before reading global

			void init();
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
