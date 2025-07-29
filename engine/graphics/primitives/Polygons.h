#pragma once

#include "core/base/Transforms.h"
#include "core/base/Constants.h"
#include "core/cmath/ColoredGeometry.h"
#include "core/containers/FreeSpaceTracker.h"
#include "core/containers/IDGenerator.h"
#include "core/types/SmartReference.h"

#include "graphics/backend/specialized/ElementBuffers.h"
#include "graphics/backend/specialized/VertexBuffers.h"

#include <map>

namespace oly::rendering
{
	class StaticPolygon;
	class Polygonal;

	class PolygonBatch
	{
		friend class StaticPolygon;
		friend class Polygonal;
		typedef GLuint Index;

		GLuint projection_location;

		graphics::VertexArray vao;
		graphics::PersistentEBO<1> ebo;

		enum
		{
			POSITION,
			COLOR,
			INDEX
		};
		graphics::PersistentVertexBufferBlock<glm::vec2, glm::vec4, GLuint> vbo_block;

		graphics::LazyPersistentGPUBuffer<glm::mat3> transform_ssbo;

	public:
		typedef StrictIDGenerator<Index>::ID PolygonID;

		struct Capacity
		{
			Index vertices = 0;
			Index indices = 0;

			Capacity(Index primitives, Index degree = 6)
			{
				OLY_ASSERT(degree >= 3);
				OLY_ASSERT(degree * primitives <= nmax<unsigned int>());

				// max(F) = V - 2 + 2H
				// max(H) = [V / 3] - 1
				// --> max(F) = V + 2 * [V / 3] - 4
				// --> index count = 3 * max(F)
				Index polygon_index_count(3 * degree + 6 * (degree / 3) - 12);

				vertices = primitives * degree;
				indices = primitives * polygon_index_count;
			}
		};

		PolygonBatch(Capacity capacity);

		void render() const;
			
		glm::mat3 projection = 1.0f;

	private:
		void set_primitive_points(Range<Index> vbo_range, const glm::vec2* points, Index count);
		void set_primitive_colors(Range<Index> vbo_range, const glm::vec4* colors, Index count);
		void set_polygon(Index id, const cmath::Polygon2D& polygon);
		void set_polygon(Index id, const std::vector<cmath::Polygon2D>& polygons);
		void set_polygon(Index id, const cmath::Polygon2DComposite& composite);
		void set_polygon_transform(Index id, const glm::mat3& transform);

		PolygonID generate_id(Index vertices);
		void terminate_id(const PolygonID& id);
		void resize_range(PolygonID& id, Index vertices);
		Range<Index> get_vertex_range(Index id) const;
		bool is_valid_id(Index id) const;

		StrictFreeSpaceTracker<Index> vertex_free_space;
		std::unordered_map<Index, Range<Index>> polygon_indexer;
		StrictIDGenerator<Index> id_generator;
	};

	// ASSET
	class StaticPolygon
	{
		friend PolygonBatch;
		PolygonBatch::PolygonID id;
		mutable math::Triangulation triangulation;

	public:
		cmath::Polygon2D polygon;

		StaticPolygon();
		StaticPolygon(const StaticPolygon&);
		StaticPolygon(StaticPolygon&&) noexcept;
		~StaticPolygon();
		StaticPolygon& operator=(const StaticPolygon&);
		StaticPolygon& operator=(StaticPolygon&&) noexcept;

		void init();
		void send_polygon() const;
		void send_colors_only() const;
		void draw() const;
	};

	class Polygonal
	{
		friend PolygonBatch;
		PolygonBatch::PolygonID id;

	public:
		Transformer2D transformer;

		Polygonal();
		Polygonal(const Polygonal&) = delete;
		Polygonal(Polygonal&&) noexcept;
		virtual ~Polygonal();
		Polygonal& operator=(Polygonal&&) noexcept;

		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		void init();
		void send_polygon();
		virtual GLuint num_vertices() const = 0;
		void draw() const;

	protected:
		void set_polygon(const cmath::Polygon2D& polygon) const;
		void set_polygon(const std::vector<cmath::Polygon2D>& polygons) const;
		void set_polygon(const cmath::Polygon2DComposite& composite) const;
		GLuint& draw_index() const;
		virtual void impl_set_polygon() const = 0;
		virtual void subinit() const = 0;
		virtual void draw_triangulation(GLuint initial_vertex) const = 0;
	};

	struct Polygon : public Polygonal
	{
		cmath::Polygon2D polygon;

		using Polygonal::Polygonal;
		Polygon(Polygon&&) noexcept = default;
		Polygon& operator=(Polygon&&) noexcept = default;

		virtual GLuint num_vertices() const override;

	private:
		mutable math::Triangulation cache;

	protected:
		virtual void impl_set_polygon() const override;
		virtual void subinit() const override;
		virtual void draw_triangulation(GLuint initial_vertex) const override;
	};

	struct PolyComposite : public Polygonal
	{
		cmath::Polygon2DComposite composite;

		using Polygonal::Polygonal;
		PolyComposite(PolyComposite&&) noexcept = default;
		PolyComposite& operator=(PolyComposite&&) noexcept = default;

		virtual GLuint num_vertices() const override;

	protected:
		virtual void impl_set_polygon() const override;
		virtual void subinit() const override;
		virtual void draw_triangulation(GLuint initial_vertex) const override;
	};

	struct NGon : public Polygonal
	{
		bool bordered = false;
		cmath::NGonBase base;

		using Polygonal::Polygonal;
		NGon(NGon&&) noexcept = default;
		NGon& operator=(NGon&&) noexcept = default;

		virtual GLuint num_vertices() const override;

	private:
		mutable cmath::Polygon2DComposite cache;

	protected:
		virtual void impl_set_polygon() const override;
		virtual void subinit() const override;
		virtual void draw_triangulation(GLuint initial_vertex) const override;
	};
}

namespace oly
{
	namespace rendering { typedef SmartReference<Polygonal> PolygonalRef; }

	OLY_SMART_POOL_BASE(rendering::Polygon, rendering::Polygonal);
	namespace rendering { typedef SmartReference<Polygon> PolygonRef; }

	OLY_SMART_POOL_BASE(rendering::PolyComposite, rendering::Polygonal);
	namespace rendering { typedef SmartReference<PolyComposite> PolyCompositeRef; }

	OLY_SMART_POOL_BASE(rendering::NGon, rendering::Polygonal);
	namespace rendering { typedef SmartReference<NGon> NGonRef; }
}
