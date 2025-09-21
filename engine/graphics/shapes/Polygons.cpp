#include "Polygons.h"

#include <algorithm>

#include "core/context/rendering/Rendering.h"
#include "core/math/Triangulation.h"
#include "graphics/resources/Shaders.h"

namespace oly::rendering
{
	PolygonBatch::PolygonBatch()
		: ebo(vao), vbo_block(vao),
		vertex_free_space({ 0, nmax<Index>() })
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

	void PolygonBatch::set_polygon_points(const PolygonID& id, const cmath::Polygon2D& polygon)
	{
		set_primitive_points(get_vertex_range(id), polygon.points.data(), (Index)polygon.points.size());
	}

	void PolygonBatch::set_polygon_points(const PolygonID& id, const std::vector<cmath::Polygon2D>& polygons)
	{
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
			offset += (Index)poly.points.size();
		}
	}

	void PolygonBatch::set_polygon_points(const PolygonID& id, const cmath::Polygon2DComposite& composite)
	{
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
			offset += (Index)poly.polygon.points.size();
		}
	}

	void PolygonBatch::set_polygon_colors(const PolygonID& id, const cmath::Polygon2D& polygon)
	{
		set_primitive_colors(get_vertex_range(id), polygon.colors.data(), (Index)polygon.colors.size());
	}

	void PolygonBatch::set_polygon_colors(const PolygonID& id, const std::vector<cmath::Polygon2D>& polygons)
	{
		auto vertex_range = get_vertex_range(id);
		Index offset = 0;
		for (const cmath::Polygon2D& poly : polygons)
		{
			Range<Index> poly_range;
			poly_range.initial = vertex_range.initial + offset;
			if (poly_range.initial >= vertex_range.end())
				return;
			poly_range.length = std::min((Index)poly.points.size(), vertex_range.end() - poly_range.initial);
			set_primitive_colors(poly_range, poly.colors.data(), (Index)poly.colors.size());
			offset += (Index)poly.points.size();
		}
	}
	
	void PolygonBatch::set_polygon_colors(const PolygonID& id, const cmath::Polygon2DComposite& composite)
	{
		auto vertex_range = get_vertex_range(id);
		Index offset = 0;
		for (const cmath::TriangulatedPolygon2D& poly : composite)
		{
			Range<Index> poly_range;
			poly_range.initial = vertex_range.initial + offset;
			if (poly_range.initial >= vertex_range.end())
				return;
			poly_range.length = std::min((Index)poly.polygon.points.size(), vertex_range.end() - poly_range.initial);
			set_primitive_colors(poly_range, poly.polygon.colors.data(), (Index)poly.polygon.colors.size());
			offset += (Index)poly.polygon.points.size();
		}
	}

	void PolygonBatch::set_polygon(const PolygonID& id, const cmath::Polygon2D& polygon)
	{
		auto vertex_range = get_vertex_range(id);
		set_primitive_points(vertex_range, polygon.points.data(), (Index)polygon.points.size());
		set_primitive_colors(vertex_range, polygon.colors.data(), (Index)polygon.colors.size());
	}

	void PolygonBatch::set_polygon(const PolygonID& id, const std::vector<cmath::Polygon2D>& polygons)
	{
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

	void PolygonBatch::set_polygon(const PolygonID& id, const cmath::Polygon2DComposite& composite)
	{
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

	void PolygonBatch::set_polygon_transform(const PolygonID& id, const glm::mat3& transform)
	{
		if (is_valid_id(id))
			transform_ssbo.set(id.get()) = transform;
		else
			throw Error(ErrorCode::INVALID_ID);
	}

	const glm::mat3& PolygonBatch::get_polygon_transform(const PolygonID& id)
	{
		if (is_valid_id(id)) [[likely]]
			return transform_ssbo.get(id.get());
		else
			throw Error(ErrorCode::INVALID_ID);
	}

	PolygonBatch::PolygonID PolygonBatch::generate_id(Index vertices)
	{
		PolygonID id = id_generator.generate();
		Range<Index> vertex_range;
		OLY_ASSERT(vertex_free_space.next_free(vertices, vertex_range));
		vertex_free_space.reserve(vertex_range);
		polygon_indexer[id.get()] = vertex_range;
		for (Index v = 0; v < vertex_range.length; ++v)
			vbo_block.set<INDEX>(vertex_range.initial + v) = id.get();
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

	bool PolygonBatch::resize_range(PolygonID& id, Index vertices)
	{
		if (id.is_valid())
		{
			if (polygon_indexer.count(id.get()) && vertices == get_vertex_range(id).length)
				return false;
			terminate_id(id);
		}
		id = generate_id(vertices);
		return true;
	}

	Range<PolygonBatch::Index> PolygonBatch::get_vertex_range(const PolygonID& id) const
	{
		if (!is_valid_id(id))
			throw Error(ErrorCode::INVALID_ID);
		return polygon_indexer.find(id.get())->second;
	}

	bool PolygonBatch::is_valid_id(const PolygonID& id) const
	{
		return id.is_valid() && polygon_indexer.count(id.get());
	}

	internal::PolygonReference::PolygonReference(PolygonBatch* batch)
	{
		set_batch(batch);
	}

	internal::PolygonReference::PolygonReference(const PolygonReference& other)
	{
		set_batch(other.batch);
	}

	internal::PolygonReference::PolygonReference(PolygonReference&& other) noexcept
		: batch(other.batch), id(std::move(other.id))
	{
	}

	internal::PolygonReference::~PolygonReference()
	{
		if (batch)
			batch->terminate_id(id);
	}

	internal::PolygonReference& internal::PolygonReference::operator=(const PolygonReference& other)
	{
		if (this != &other)
			set_batch(other.batch);
		return *this;
	}

	internal::PolygonReference& internal::PolygonReference::operator=(PolygonReference&& other) noexcept
	{
		if (this != &other)
		{
			if (batch == other.batch)
			{
				if (batch)
				{
					batch->terminate_id(id);
					id = std::move(other.id);
				}
			}
			else
				set_batch(other.batch);
		}
		return *this;
	}

	void internal::PolygonReference::set_batch(PolygonBatch* batch)
	{
		if (this->batch != batch)
		{
			id.yield();
			this->batch = batch;
			// TODO v4 copy raw data over
			resize_range(3);
			set_polygon(cmath::Polygon2D{ .points = { {}, {}, {} }, .colors = { glm::vec4(1.0f) }});
			set_polygon_transform(glm::mat3(1.0f));
		}
	}

	bool internal::PolygonReference::resize_range(PolygonBatch::Index vertices) const
	{
		if (batch) [[likely]]
		{
			glm::mat3 transform = batch->is_valid_id(id) ? batch->get_polygon_transform(id) : 1.0f;
			bool resized = batch->resize_range(id, vertices);
			if (resized)
				batch->set_polygon_transform(id, transform);
			return resized;
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	Range<PolygonBatch::Index> internal::PolygonReference::get_vertex_range() const
	{
		if (batch) [[likely]]
			return batch->get_vertex_range(id);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::PolygonReference::set_polygon_points(const cmath::Polygon2D& polygon) const
	{
		if (batch) [[likely]]
			batch->set_polygon_points(id, polygon);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}
	
	void internal::PolygonReference::set_polygon_points(const std::vector<cmath::Polygon2D>& polygons) const
	{
		if (batch) [[likely]]
			batch->set_polygon_points(id, polygons);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}
	
	void internal::PolygonReference::set_polygon_points(const cmath::Polygon2DComposite& composite) const
	{
		if (batch) [[likely]]
			batch->set_polygon_points(id, composite);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}
	
	void internal::PolygonReference::set_polygon_colors(const cmath::Polygon2D& polygon) const
	{
		if (batch) [[likely]]
			batch->set_polygon_colors(id, polygon);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}
	
	void internal::PolygonReference::set_polygon_colors(const std::vector<cmath::Polygon2D>& polygons) const
	{
		if (batch) [[likely]]
			batch->set_polygon_colors(id, polygons);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}
	
	void internal::PolygonReference::set_polygon_colors(const cmath::Polygon2DComposite& composite) const
	{
		if (batch) [[likely]]
			batch->set_polygon_colors(id, composite);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::PolygonReference::set_polygon(const cmath::Polygon2D& polygon) const
	{
		if (batch) [[likely]]
			batch->set_polygon(id, polygon);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}
	
	void internal::PolygonReference::set_polygon(const std::vector<cmath::Polygon2D>& polygons) const
	{
		if (batch) [[likely]]
			batch->set_polygon(id, polygons);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}
	
	void internal::PolygonReference::set_polygon(const cmath::Polygon2DComposite& composite) const
	{
		if (batch) [[likely]]
			batch->set_polygon(id, composite);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::PolygonReference::set_polygon_transform(const glm::mat3& transform) const
	{
		if (batch) [[likely]]
			batch->set_polygon_transform(id, transform);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	GLuint& internal::PolygonReference::draw_index() const
	{
		if (batch) [[likely]]
			return batch->ebo.draw_primitive()[0];
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	StaticPolygon::StaticPolygon(PolygonBatch* batch)
		: ref(batch)
	{
	}

	StaticPolygon::StaticPolygon(const StaticPolygon& other)
		: ref(other.ref), triangulation(other.triangulation), polygon(other.polygon)
	{
	}

	StaticPolygon& StaticPolygon::operator=(const StaticPolygon& other)
	{
		if (this != &other)
		{
			polygon = other.polygon;
			triangulation = other.triangulation;
			ref = other.ref;
			dirty = {};
		}
		return *this;
	}

	void StaticPolygon::draw() const
	{
		submit_dirty();
		GLuint initial_vertex = ref.get_vertex_range().initial;
		for (size_t i = 0; i < triangulation.size(); ++i)
		{
			ref.draw_index() = triangulation[i][0] + initial_vertex;
			ref.draw_index() = triangulation[i][1] + initial_vertex;
			ref.draw_index() = triangulation[i][2] + initial_vertex;
		}
	}

	void StaticPolygon::submit_dirty() const
	{
		if (dirty.points)
		{
			dirty.points = false;

			try
			{
				triangulation = math::triangulate(polygon.points);
			}
			catch (Error e)
			{
				if (e.code == ErrorCode::TRIANGULATION)
					OLY_LOG_WARNING(true, "RENDERING") << LOG.source_info.full_source() << "Could not send polygon - bad triangulation." << LOG.nl;
				else
					throw e;
			}

			ref.resize_range((PolygonBatch::Index)polygon.points.size());

			if (dirty.colors)
			{
				dirty.colors = false;
				ref.set_polygon(polygon);
			}
			else
				ref.set_polygon_points(polygon);
		}
		else if (dirty.colors)
		{
			dirty.colors = false;
			ref.set_polygon_colors(polygon);
		}
	}

	Polygonal::Polygonal(PolygonBatch* batch)
		: ref(batch)
	{
	}

	Polygonal::Polygonal(const Polygonal& other)
		: ref(other.ref), transformer(other.transformer)
	{
	}

	Polygonal& Polygonal::operator=(const Polygonal& other)
	{
		if (this != &other)
		{
			ref = other.ref;
			transformer = other.transformer;
			dirty = {};
		}
		return *this;
	}

	void Polygonal::draw() const
	{
		submit_dirty();
		if (transformer.flush())
			ref.set_polygon_transform(transformer.global());
		draw_triangulation(ref.get_vertex_range().initial);
	}

	void Polygonal::submit_dirty() const
	{
		if (dirty.points)
		{
			dirty.points = false;

			try
			{
				triangulate();
			}
			catch (Error e)
			{
				if (e.code == ErrorCode::TRIANGULATION)
					OLY_LOG_WARNING(true, "RENDERING") << LOG.source_info.full_source() << "Could not send polygon - bad triangulation." << LOG.nl;
				else
					throw e;
			}

			ref.resize_range(num_vertices());

			if (dirty.colors)
			{
				dirty.colors = false;
				impl_set_polygon();
			}
			else
				impl_set_polygon_points();
		}
		else if (dirty.colors)
		{
			dirty.colors = false;
			impl_set_polygon_colors();
		}
	}

	GLuint& Polygonal::draw_index() const
	{
		return ref.draw_index();
	}
	
	Polygon::Polygon(PolygonBatch* batch)
		: Polygonal(batch)
	{
	}
	
	Polygon::Polygon(const Polygon& other)
		: Polygonal(other), polygon(other.polygon), cache(other.cache)
	{
	}
	
	Polygon::Polygon(Polygon&& other) noexcept
		: Polygonal(std::move(other)), polygon(std::move(other.polygon)), cache(std::move(other.cache))
	{
	}
	
	Polygon& Polygon::operator=(const Polygon& other)
	{
		if (this != &other)
		{
			Polygonal::operator=(other);
			polygon = other.polygon;
			cache = other.cache;
		}
		return *this;
	}
	
	Polygon& Polygon::operator=(Polygon&& other) noexcept
	{
		if (this != &other)
		{
			Polygonal::operator=(std::move(other));
			polygon = std::move(other.polygon);
			cache = std::move(other.cache);
		}
		return *this;
	}

	GLuint Polygon::num_vertices() const
	{
		return (GLuint)polygon.points.size();
	}

	void Polygon::impl_set_polygon() const
	{
		Polygonal::set_polygon(polygon);
	}

	void Polygon::impl_set_polygon_points() const
	{
		Polygonal::set_polygon_points(polygon);
	}

	void Polygon::impl_set_polygon_colors() const
	{
		Polygonal::set_polygon_colors(polygon);
	}

	void Polygon::triangulate() const
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

	PolyComposite::PolyComposite(PolygonBatch* batch)
		: Polygonal(batch)
	{
	}

	PolyComposite::PolyComposite(const PolyComposite& other)
		: Polygonal(other), composite(other.composite)
	{
	}

	PolyComposite::PolyComposite(PolyComposite&& other) noexcept
		: Polygonal(std::move(other)), composite(std::move(other.composite))
	{
	}

	PolyComposite& PolyComposite::operator=(const PolyComposite& other)
	{
		if (this != &other)
		{
			Polygonal::operator=(other);
			composite = other.composite;
		}
		return *this;
	}

	PolyComposite& PolyComposite::operator=(PolyComposite&& other) noexcept
	{
		if (this != &other)
		{
			Polygonal::operator=(std::move(other));
			composite = std::move(other.composite);
		}
		return *this;
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
		Polygonal::set_polygon(composite);
	}

	void PolyComposite::impl_set_polygon_points() const
	{
		Polygonal::set_polygon_points(composite);
	}

	void PolyComposite::impl_set_polygon_colors() const
	{
		Polygonal::set_polygon_colors(composite);
	}

	void PolyComposite::triangulate() const
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

	NGon::NGon(PolygonBatch* batch)
		: Polygonal(batch)
	{
	}

	NGon::NGon(const NGon& other)
		: Polygonal(other), bordered(other.bordered), base(other.base), cache(other.cache)
	{
	}

	NGon::NGon(NGon&& other) noexcept
		: Polygonal(std::move(other)), bordered(other.bordered), base(std::move(other.base)), cache(std::move(other.cache))
	{
	}

	NGon& NGon::operator=(const NGon& other)
	{
		if (this != &other)
		{
			Polygonal::operator=(other);
			bordered = other.bordered;
			base = other.base;
			cache = other.cache;
		}
		return *this;
	}

	NGon& NGon::operator=(NGon&& other) noexcept
	{
		if (this != &other)
		{
			Polygonal::operator=(std::move(other));
			bordered = std::move(other.bordered);
			base = std::move(other.base);
			cache = std::move(other.cache);
		}
		return *this;
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
		Polygonal::set_polygon(cache);
	}

	void NGon::impl_set_polygon_points() const
	{
		Polygonal::set_polygon_points(cache);
	}

	void NGon::impl_set_polygon_colors() const
	{
		Polygonal::set_polygon_colors(cache);
	}

	void NGon::triangulate() const
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
