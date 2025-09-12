#include "Polygons.h"

#include <algorithm>

#include "core/context/rendering/Rendering.h"
#include "core/context/rendering/Polygons.h"
#include "core/math/Triangulation.h"
#include "graphics/resources/Shaders.h"

namespace oly::rendering
{
	PolygonBatch::PolygonBatch(Capacity capacity)
		: ebo(vao, capacity.indices), vbo_block(vao, { capacity.vertices, capacity.vertices, capacity.vertices }), transform_ssbo(capacity.indices),
		vertex_free_space({ 0, capacity.indices })
	{
		projection_location = glGetUniformLocation(graphics::internal_shaders::polygon_batch, "uProjection");

		vbo_block.attributes[POSITION] = graphics::VertexAttribute<float>{ 0, 2 };
		vbo_block.attributes[COLOR] = graphics::VertexAttribute<float>{ 1, 4 };
		vbo_block.attributes[INDEX] = graphics::VertexAttribute<int>{ 2, 1 };
		vbo_block.setup();
	}

	void PolygonBatch::render() const
	{
		if (ebo.empty())
			return;

		vbo_block.pre_draw_all();
		transform_ssbo.pre_draw();
		glBindVertexArray(vao);
		glUseProgram(graphics::internal_shaders::polygon_batch);
		glUniformMatrix3fv(projection_location, 1, GL_FALSE, glm::value_ptr(projection));

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, transform_ssbo.buf.get_buffer());
		ebo.render_elements(GL_TRIANGLES);
		vbo_block.post_draw_all();
		transform_ssbo.post_draw();
	}

	void PolygonBatch::set_primitive_points(Range<Index> range, const glm::vec2* points, Index count)
	{
		count = std::min(range.length, count);
		for (Index v = 0; v < count; ++v)
			vbo_block.set<POSITION>(range.initial + v) = points[v];
	}

	void PolygonBatch::set_primitive_colors(Range<Index> range, const glm::vec4* colors, Index count)
	{
		count = std::min(range.length, count);
		if (count == 1)
		{
			for (Index v = 0; v < range.length; ++v)
				vbo_block.set<COLOR>(range.initial + v) = colors[0];
		}
		else
		{
			for (Index v = 0; v < count; ++v)
				vbo_block.set<COLOR>(range.initial + v) = colors[v];
		}
	}

	void PolygonBatch::set_polygon(Index id, const cmath::Polygon2D& polygon)
	{
		OLY_ASSERT(is_valid_id(id));
		auto vertex_range = get_vertex_range(id);
		set_primitive_points(vertex_range, polygon.points.data(), (Index)polygon.points.size());
		set_primitive_colors(vertex_range, polygon.colors.data(), (Index)polygon.colors.size());
	}

	void PolygonBatch::set_polygon(Index id, const std::vector<cmath::Polygon2D>& polygons)
	{
		OLY_ASSERT(is_valid_id(id));
		auto vertex_range = get_vertex_range(id);
		Index offset = 0;
		for (const cmath::Polygon2D& poly : polygons)
		{
			Range<Index> poly_range;
			poly_range.initial = vertex_range.initial + offset;
			if (poly_range.initial >= vertex_range.end())
				return;
			poly_range.length = std::min((Index)poly.points.size(), vertex_range.end() - poly_range.initial);
			set_primitive_points(poly_range, poly.points.data(), (Index)poly.points.size());
			set_primitive_colors(poly_range, poly.colors.data(), (Index)poly.colors.size());
			offset += (Index)poly.points.size();
		}
	}

	void PolygonBatch::set_polygon(Index id, const cmath::Polygon2DComposite& composite)
	{
		OLY_ASSERT(is_valid_id(id));
		auto vertex_range = get_vertex_range(id);
		Index offset = 0;
		for (const cmath::TriangulatedPolygon2D& poly : composite)
		{
			Range<Index> poly_range;
			poly_range.initial = vertex_range.initial + offset;
			if (poly_range.initial >= vertex_range.end())
				return;
			poly_range.length = std::min((Index)poly.polygon.points.size(), vertex_range.end() - poly_range.initial);
			set_primitive_points(poly_range, poly.polygon.points.data(), (Index)poly.polygon.points.size());
			set_primitive_colors(poly_range, poly.polygon.colors.data(), (Index)poly.polygon.colors.size());
			offset += (Index)poly.polygon.points.size();
		}
	}

	void PolygonBatch::set_polygon_transform(Index id, const glm::mat3& transform)
	{
		if (id >= transform_ssbo.buf.get_size())
		{
			transform_ssbo.grow();
			vertex_free_space.extend_rightward(transform_ssbo.buf.get_size());
		}
		transform_ssbo.buf[id] = transform;
		transform_ssbo.flag(id);
	}

	PolygonBatch::PolygonID PolygonBatch::generate_id(Index vertices)
	{
		PolygonID id = id_generator.generate();
		auto it = polygon_indexer.find(id.get());
		if (it == polygon_indexer.end())
		{
			Range<Index> vertex_range;
			OLY_ASSERT(vertex_free_space.next_free(vertices, vertex_range));
			vertex_free_space.reserve(vertex_range);
			polygon_indexer[id.get()] = vertex_range;
			for (Index v = 0; v < vertex_range.length; ++v)
				vbo_block.set<INDEX>(vertex_range.initial + v) = id.get();
		}
		return id;
	}

	void PolygonBatch::terminate_id(const PolygonID& id)
	{
		if (id.is_valid())
		{
			auto it = polygon_indexer.find(id.get());
			if (it != polygon_indexer.end())
			{
				vertex_free_space.release(it->second);
				polygon_indexer.erase(it);
			}
		}
	}

	void PolygonBatch::resize_range(PolygonID& id, Index vertices)
	{
		if (id.is_valid())
		{
			if (is_valid_id(id.get()) && vertices == get_vertex_range(id.get()).length)
				return;
			terminate_id(id);
		}
		id = generate_id(vertices);
	}

	Range<PolygonBatch::Index> PolygonBatch::get_vertex_range(Index id) const
	{
		if (!is_valid_id(id))
			throw Error(ErrorCode::OTHER, "Invalid Index");
		return polygon_indexer.find(id)->second;
	}

	bool PolygonBatch::is_valid_id(Index id) const
	{
		return polygon_indexer.count(id);
	}

	StaticPolygon::StaticPolygon(PolygonBatch* batch)
		: batch(batch ? *batch : context::polygon_batch()), in_context(!batch)
	{
	}

	StaticPolygon::StaticPolygon(const StaticPolygon& other)
		: batch(other.batch), in_context(other.in_context), triangulation(other.triangulation), polygon(other.polygon)
	{
		init();
	}

	StaticPolygon::StaticPolygon(StaticPolygon&& other) noexcept
		: batch(other.batch), in_context(other.in_context), id(std::move(other.id)), triangulation(std::move(other.triangulation)), polygon(std::move(other.polygon))
	{
	}

	StaticPolygon::~StaticPolygon()
	{
		batch.terminate_id(id);
	}

	StaticPolygon& StaticPolygon::operator=(const StaticPolygon& other)
	{
		if (this != &other)
		{
			polygon = other.polygon;
			triangulation = other.triangulation;
			batch.set_polygon(id.get(), polygon);
		}
		return *this;
	}

	StaticPolygon& StaticPolygon::operator=(StaticPolygon&& other) noexcept
	{
		if (this != &other)
		{
			if (&batch == &other.batch)
			{
				batch.terminate_id(id);
				id = std::move(other.id);
			}
			else
			{
				triangulation = std::move(other.triangulation);
				polygon = std::move(other.polygon);
			}
		}
		return *this;
	}

	void StaticPolygon::init()
	{
		triangulation = math::triangulate(polygon.points);
		batch.resize_range(id, (PolygonBatch::Index)polygon.points.size());
		batch.set_polygon(id.get(), polygon);
		batch.set_polygon_transform(id.get(), glm::mat3(1.0f));
	}

	void StaticPolygon::send_polygon() const
	{
		try
		{
			triangulation = math::triangulate(polygon.points);
			batch.set_polygon(id.get(), polygon);
		}
		catch (Error e)
		{
			if (e.code == ErrorCode::TRIANGULATION)
				OLY_LOG_WARNING(true, "RENDERING") << LOG.source_info.full_source() << "Could not send polygon - bad triangulation." << LOG.nl;
			else
				throw e;
		}
	}

	void StaticPolygon::send_colors_only() const
	{
		if (batch.is_valid_id(id.get()))
			batch.set_primitive_colors(batch.get_vertex_range(id.get()), polygon.colors.data(), (PolygonBatch::Index)polygon.colors.size());
	}

	void StaticPolygon::draw(BatchBarrier barrier) const
	{
		if (in_context) [[likely]]
			if (barrier) [[likely]]
				context::internal::flush_batches_except(context::InternalBatch::POLYGON);
		GLuint initial_vertex = batch.get_vertex_range(id.get()).initial;
		for (size_t i = 0; i < triangulation.size(); ++i)
		{
			batch.ebo.draw_primitive()[0] = triangulation[i][0] + initial_vertex;
			batch.ebo.draw_primitive()[0] = triangulation[i][1] + initial_vertex;
			batch.ebo.draw_primitive()[0] = triangulation[i][2] + initial_vertex;
		}
		if (in_context) [[likely]]
			context::internal::set_batch_rendering_tracker(context::InternalBatch::POLYGON, true);
	}

	Polygonal::Polygonal(PolygonBatch* batch)
		: batch(batch ? *batch : context::polygon_batch()), in_context(!batch)
	{
	}

	Polygonal::Polygonal(Polygonal&& other) noexcept
		: batch(other.batch), in_context(other.in_context), id(std::move(other.id)), transformer(std::move(other.transformer))
	{
	}

	Polygonal::~Polygonal()
	{
		batch.terminate_id(id);
	}

	Polygonal& Polygonal::operator=(Polygonal&& other) noexcept
	{
		if (this != &other)
		{
			if (&batch == &other.batch)
			{
				batch.terminate_id(id);
				id = std::move(other.id);
			}
			transformer = std::move(other.transformer);
		}
		return *this;
	}

	void Polygonal::init()
	{
		subinit();
		batch.resize_range(id, num_vertices());
		impl_set_polygon();
	}

	void Polygonal::send_polygon()
	{
		try
		{
			subinit();
			impl_set_polygon();
		}
		catch (Error e)
		{
			if (e.code == ErrorCode::TRIANGULATION)
				OLY_LOG_WARNING(true, "RENDERING") << LOG.source_info.full_source() << "Could not send polygon - bad triangulation." << LOG.nl;
			else
				throw e;
		}
	}
		
	void Polygonal::draw(BatchBarrier barrier) const
	{
		if (in_context) [[likely]]
			if (barrier) [[likely]]
				context::internal::flush_batches_except(context::InternalBatch::POLYGON);
		if (transformer.flush())
			batch.set_polygon_transform(id.get(), transformer.global());
		draw_triangulation(batch.get_vertex_range(id.get()).initial);
		if (in_context) [[likely]]
			context::internal::set_batch_rendering_tracker(context::InternalBatch::POLYGON, true);
	}

	void Polygonal::set_polygon(const cmath::Polygon2D& polygon) const
	{
		batch.set_polygon(id.get(), polygon);
	}

	void Polygonal::set_polygon(const std::vector<cmath::Polygon2D>& polygons) const
	{
		batch.set_polygon(id.get(), polygons);
	}

	void Polygonal::set_polygon(const cmath::Polygon2DComposite& composite) const
	{
		batch.set_polygon(id.get(), composite);
	}

	GLuint& Polygonal::draw_index() const
	{
		return batch.ebo.draw_primitive()[0];
	}

	GLuint Polygon::num_vertices() const
	{
		return (GLuint)polygon.points.size();
	}

	void Polygon::impl_set_polygon() const
	{
		set_polygon(polygon);
	}

	void Polygon::subinit() const
	{
		cache = math::triangulate(polygon.points);
	}

	void Polygon::draw_triangulation(GLuint initial_vertex) const
	{
		for (size_t i = 0; i < cache.size(); ++i)
		{
			draw_index() = cache[i][0] + initial_vertex;
			draw_index() = cache[i][1] + initial_vertex;
			draw_index() = cache[i][2] + initial_vertex;
		}
	}

	GLuint PolyComposite::num_vertices() const
	{
		GLuint vertices = 0;
		for (const auto& primitive : composite)
			vertices += (GLuint)primitive.polygon.points.size();
		return vertices;
	}

	void PolyComposite::impl_set_polygon() const
	{
		set_polygon(composite);
	}

	void PolyComposite::subinit() const
	{
	}

	void PolyComposite::draw_triangulation(GLuint initial_vertex) const
	{
		GLuint offset = 0;
		for (const auto& tp : composite)
		{
			const auto& faces = tp.triangulation;
			for (size_t i = 0; i < faces.size(); ++i)
			{
				draw_index() = faces[i][0] + initial_vertex + offset;
				draw_index() = faces[i][1] + initial_vertex + offset;
				draw_index() = faces[i][2] + initial_vertex + offset;
			}
			offset += (GLuint)tp.polygon.points.size();
		}
	}

	GLuint NGon::num_vertices() const
	{
		GLuint vertices = 0;
		for (const auto& primitive : cache)
			vertices += (GLuint)primitive.polygon.points.size();
		return vertices;
	}

	void NGon::impl_set_polygon() const
	{
		set_polygon(cache);
	}

	void NGon::subinit() const
	{
		cache = bordered ? base.bordered_composite() : base.composite();
	}

	void NGon::draw_triangulation(GLuint initial_vertex) const
	{
		GLuint offset = 0;
		for (const auto& tp : cache)
		{
			const auto& faces = tp.triangulation;
			for (size_t i = 0; i < faces.size(); ++i)
			{
				draw_index() = faces[i][0] + initial_vertex + offset;
				draw_index() = faces[i][1] + initial_vertex + offset;
				draw_index() = faces[i][2] + initial_vertex + offset;
			}
			offset += (GLuint)tp.polygon.points.size();
		}
	}
}
