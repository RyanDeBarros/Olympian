#include "Polygons.h"

#include <algorithm>

#include "core/context/rendering/Rendering.h"
#include "core/math/Triangulation.h"
#include "graphics/resources/Shaders.h"

namespace oly::rendering
{
	internal::PolygonBatch::PolygonBatch()
		: ebo(vao), vbo_block(vao), vertex_free_space({ 0, nmax<GLuint>() })
	{
		projection_location = glGetUniformLocation(graphics::internal_shaders::polygon_batch, "uProjection");

		vbo_block.attributes[POSITION] = graphics::VertexAttribute<float>{ 0, 2 };
		vbo_block.attributes[COLOR] = graphics::VertexAttribute<float>{ 1, 4 };
		vbo_block.attributes[INDEX] = graphics::VertexAttribute<int>{ 2, 1 };
		vbo_block.setup();
	}

	void internal::PolygonBatch::render() const
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

	void internal::PolygonBatch::set_primitive_points(Range<GLuint> range, const glm::vec2* points, GLuint count)
	{
		count = std::min(range.length, count);
		for (GLuint v = 0; v < count; ++v)
			vbo_block.set<POSITION>(range.initial + v) = points[v];
	}

	void internal::PolygonBatch::set_primitive_colors(Range<GLuint> range, const glm::vec4* colors, GLuint count)
	{
		count = std::min(range.length, count);
		if (count == 1)
		{
			for (GLuint v = 0; v < range.length; ++v)
				vbo_block.set<COLOR>(range.initial + v) = colors[0];
		}
		else
		{
			for (GLuint v = 0; v < count; ++v)
				vbo_block.set<COLOR>(range.initial + v) = colors[v];
		}
	}

	void internal::PolygonBatch::set_polygon_transform(GLuint id, const glm::mat3& transform)
	{
		assert_valid_id(id);
		transform_ssbo.set(id) = transform;
	}

	const glm::mat3& internal::PolygonBatch::get_polygon_transform(GLuint id)
	{
		assert_valid_id(id);
		return transform_ssbo.get(id);
	}

	GLuint internal::PolygonBatch::generate_id(GLuint vertices)
	{
		GLuint id = id_generator.gen();
		if (id == NULL_ID) [[unlikely]]
			throw Error(ErrorCode::STORAGE_OVERFLOW);

		Range<GLuint> vertex_range;
		OLY_ASSERT(vertex_free_space.next_free(vertices, vertex_range));
		vertex_free_space.reserve(vertex_range);
		polygon_indexer[id] = vertex_range;
		for (GLuint v = 0; v < vertex_range.length; ++v)
			vbo_block.set<INDEX>(vertex_range.initial + v) = id;
		return id;
	}

	void internal::PolygonBatch::terminate_id(GLuint id)
	{
		if (is_valid_id(id))
		{
			auto it = polygon_indexer.find(id);
			if (it != polygon_indexer.end())
			{
				vertex_free_space.release(it->second);
				polygon_indexer.erase(it);
			}
		}
	}

	bool internal::PolygonBatch::resize_range(GLuint& id, GLuint vertices)
	{
		if (is_valid_id(id))
		{
			if (vertices == get_vertex_range(id).length)
				return false;
			terminate_id(id);
		}
		id = generate_id(vertices);
		return true;
	}

	Range<GLuint> internal::PolygonBatch::get_vertex_range(GLuint id) const
	{
		assert_valid_id(id);
		return polygon_indexer.find(id)->second;
	}

	bool internal::PolygonBatch::is_valid_id(GLuint id) const
	{
		return id != NULL_ID && polygon_indexer.count(id);
	}

	void internal::PolygonBatch::assert_valid_id(GLuint id) const
	{
		if (!is_valid_id(id))
			throw Error(ErrorCode::INVALID_ID);
	}

	internal::PolygonReference::PolygonReference(Unbatched)
	{
	}

	internal::PolygonReference::PolygonReference(rendering::PolygonBatch& batch)
	{
		set_batch(batch);
	}

	internal::PolygonReference::PolygonReference(const PolygonReference& other)
		: Super(other)
	{
		if (auto batch = lock())
		{
			if (batch->is_valid_id(other.id))
			{
				const GLuint num_vertices = batch->get_vertex_range(other.id).length;
				const glm::mat3 transform = batch->get_polygon_transform(other.id);
				id = batch->generate_id(num_vertices);
				batch->set_polygon_transform(id, transform);
			}
		}
	}

	internal::PolygonReference::PolygonReference(PolygonReference&& other) noexcept
		: Super(std::move(other)), id(other.id)
	{
		other.id = PolygonBatch::NULL_ID;
	}

	internal::PolygonReference::~PolygonReference()
	{
		if (auto batch = lock())
			batch->terminate_id(id);
	}

	internal::PolygonReference& internal::PolygonReference::operator=(const PolygonReference& other)
	{
		if (this != &other)
			*this = dupl(other);
		return *this;
	}

	internal::PolygonReference& internal::PolygonReference::operator=(PolygonReference&& other) noexcept
	{
		if (this != &other)
		{
			if (auto batch = lock())
				batch->terminate_id(id);
			Super::operator=(std::move(other));
			id = other.id;
			other.id = PolygonBatch::NULL_ID;
		}
		return *this;
	}

	void internal::PolygonReference::set_batch(Unbatched)
	{
		if (auto batch = lock())
			batch->terminate_id(id);
		id = PolygonBatch::NULL_ID;
		reset();
	}

	void internal::PolygonReference::set_batch(rendering::PolygonBatch& new_batch)
	{
		if (auto batch = lock())
		{
			if (batch.get() == new_batch.address())
				return;

			if (batch->is_valid_id(id))
			{
				const GLuint num_vertices = batch->get_vertex_range(id).length;
				const glm::mat3 transform = batch->get_polygon_transform(id);
				batch->terminate_id(id);
				reset(*new_batch);
				batch = lock();
				id = batch->generate_id(num_vertices);
				batch->set_polygon_transform(id, transform);
			}
			else
				reset(*new_batch);
		}
		else
			reset(*new_batch);
	}

	bool internal::PolygonReference::resize_range(GLuint vertices) const
	{
		if (auto batch = lock()) [[likely]]
		{
			const glm::mat3 transform = batch->is_valid_id(id) ? batch->get_polygon_transform(id) : 1.0f;
			const bool resized = batch->resize_range(id, vertices);
			if (resized)
				batch->set_polygon_transform(id, transform);
			return resized;
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	Range<GLuint> internal::PolygonReference::get_vertex_range() const
	{
		if (auto batch = lock()) [[likely]]
			return batch->get_vertex_range(id);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::PolygonReference::set_primitive_points(Range<GLuint> vertex_range, const glm::vec2* points, GLuint count) const
	{
		if (auto batch = lock()) [[likely]]
			batch->set_primitive_points(vertex_range, points, count);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::PolygonReference::set_primitive_colors(Range<GLuint> vertex_range, const glm::vec4* colors, GLuint count) const
	{
		if (auto batch = lock()) [[likely]]
			batch->set_primitive_colors(vertex_range, colors, count);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::PolygonReference::set_primitive_points(const glm::vec2* points, GLuint count) const
	{
		if (auto batch = lock()) [[likely]]
			batch->set_primitive_points(batch->get_vertex_range(id), points, count);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::PolygonReference::set_primitive_colors(const glm::vec4* colors, GLuint count) const
	{
		if (auto batch = lock()) [[likely]]
			batch->set_primitive_colors(batch->get_vertex_range(id), colors, count);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void internal::PolygonReference::set_polygon_transform(const glm::mat3& transform) const
	{
		if (auto batch = lock()) [[likely]]
			batch->set_polygon_transform(id, transform);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	GLuint& internal::PolygonReference::draw_index() const
	{
		if (auto batch = lock()) [[likely]]
		{
			batch->assert_valid_id(id);
			return batch->ebo.draw_primitive()[0];
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	internal::PolygonSubmitter::PolygonSubmitter(Unbatched)
		: ref(UNBATCHED)
	{
	}

	internal::PolygonSubmitter::PolygonSubmitter(rendering::PolygonBatch& batch)
		: ref(batch)
	{
	}

	internal::PolygonSubmitter::PolygonSubmitter(const PolygonSubmitter& other)
		: ref(other.ref)
	{
	}

	internal::PolygonSubmitter& internal::PolygonSubmitter::operator=(const PolygonSubmitter& other)
	{
		if (this != &other)
		{
			ref = other.ref;
			flag_all();
		}
		return *this;
	}

	void internal::PolygonSubmitter::submit_dirty() const
	{
		if (points)
		{
			points = false;

			try
			{
				triangulate();
			}
			catch (Error e)
			{
				if (e.code == ErrorCode::TRIANGULATION)
					OLY_LOG_WARNING(true, "RENDERING") << LOG.source_info.full_source() << "Error occurred during polygon triangulation." << LOG.nl;
				else
					throw e;
			}

			ref.resize_range(num_vertices());

			if (colors)
			{
				colors = false;
				impl_set_polygon();
			}
			else
				impl_set_polygon_points();
		}
		else if (colors)
		{
			colors = false;
			impl_set_polygon_colors();
		}
	}

	void StaticPolygon::draw() const
	{
		submit_dirty();
		GLuint initial_vertex = get_ref().get_vertex_range().initial;
		for (size_t i = 0; i < triangulation.size(); ++i)
		{
			get_ref().draw_index() = triangulation[i][0] + initial_vertex;
			get_ref().draw_index() = triangulation[i][1] + initial_vertex;
			get_ref().draw_index() = triangulation[i][2] + initial_vertex;
		}
	}

	void StaticPolygon::triangulate() const
	{
		triangulation = math::triangulate(polygon.points);
	}
	
	GLuint StaticPolygon::num_vertices() const
	{
		return (GLuint)polygon.points.size();
	}
	
	void StaticPolygon::impl_set_polygon() const
	{
		auto vertex_range = get_ref().get_vertex_range();
		get_ref().set_primitive_points(vertex_range, polygon.points.data(), (GLuint)polygon.points.size());
		get_ref().set_primitive_colors(vertex_range, polygon.colors.data(), (GLuint)polygon.colors.size());
	}
	
	void StaticPolygon::impl_set_polygon_points() const
	{
		get_ref().set_primitive_points(polygon.points.data(), (GLuint)polygon.points.size());
	}
	
	void StaticPolygon::impl_set_polygon_colors() const
	{
		get_ref().set_primitive_colors(polygon.colors.data(), (GLuint)polygon.colors.size());
	}

	void Polygonal::draw() const
	{
		submit_dirty();
		if (transformer.flush())
			get_ref().set_polygon_transform(transformer.global());
		draw_triangulation(get_ref().get_vertex_range().initial);
	}

	GLuint Polygon::num_vertices() const
	{
		return (GLuint)polygon.points.size();
	}

	void Polygon::impl_set_polygon() const
	{
		auto vertex_range = get_ref().get_vertex_range();
		get_ref().set_primitive_points(vertex_range, polygon.points.data(), (GLuint)polygon.points.size());
		get_ref().set_primitive_colors(vertex_range, polygon.colors.data(), (GLuint)polygon.colors.size());
	}

	void Polygon::impl_set_polygon_points() const
	{
		get_ref().set_primitive_points(polygon.points.data(), (GLuint)polygon.points.size());
	}

	void Polygon::impl_set_polygon_colors() const
	{
		get_ref().set_primitive_colors(polygon.colors.data(), (GLuint)polygon.colors.size());
	}

	void Polygon::triangulate() const
	{
		cache = math::triangulate(polygon.points);
	}

	void Polygon::draw_triangulation(GLuint initial_vertex) const
	{
		for (size_t i = 0; i < cache.size(); ++i)
		{
			get_ref().draw_index() = cache[i][0] + initial_vertex;
			get_ref().draw_index() = cache[i][1] + initial_vertex;
			get_ref().draw_index() = cache[i][2] + initial_vertex;
		}
	}

	GLuint PolyComposite::num_vertices() const
	{
		GLuint vertices = 0;
		for (const auto& primitive : composite)
			vertices += (GLuint)primitive.polygon.points.size();
		return vertices;
	}

	static void set_polygon_composite(const internal::PolygonReference& ref, const cmath::Polygon2DComposite& composite)
	{
		auto vertex_range = ref.get_vertex_range();
		GLuint offset = 0;
		for (const cmath::TriangulatedPolygon2D& poly : composite)
		{
			Range<GLuint> poly_range;
			poly_range.initial = vertex_range.initial + offset;
			if (poly_range.initial >= vertex_range.end())
				return;
			poly_range.length = std::min((GLuint)poly.polygon.points.size(), vertex_range.end() - poly_range.initial);
			ref.set_primitive_points(poly_range, poly.polygon.points.data(), (GLuint)poly.polygon.points.size());
			ref.set_primitive_colors(poly_range, poly.polygon.colors.data(), (GLuint)poly.polygon.colors.size());
			offset += (GLuint)poly.polygon.points.size();
		}
	}

	void PolyComposite::impl_set_polygon() const
	{
		set_polygon_composite(get_ref(), composite);
	}

	static void set_polygon_points(const internal::PolygonReference& ref, const cmath::Polygon2DComposite& composite)
	{
		auto vertex_range = ref.get_vertex_range();
		GLuint offset = 0;
		for (const cmath::TriangulatedPolygon2D& poly : composite)
		{
			Range<GLuint> poly_range;
			poly_range.initial = vertex_range.initial + offset;
			if (poly_range.initial >= vertex_range.end())
				return;
			poly_range.length = std::min((GLuint)poly.polygon.points.size(), vertex_range.end() - poly_range.initial);
			ref.set_primitive_points(poly_range, poly.polygon.points.data(), (GLuint)poly.polygon.points.size());
			offset += (GLuint)poly.polygon.points.size();
		}
	}

	void PolyComposite::impl_set_polygon_points() const
	{
		set_polygon_points(get_ref(), composite);
	}

	static void set_polygon_colors(const internal::PolygonReference& ref, const cmath::Polygon2DComposite& composite)
	{
		auto vertex_range = ref.get_vertex_range();
		GLuint offset = 0;
		for (const cmath::TriangulatedPolygon2D& poly : composite)
		{
			Range<GLuint> poly_range;
			poly_range.initial = vertex_range.initial + offset;
			if (poly_range.initial >= vertex_range.end())
				return;
			poly_range.length = std::min((GLuint)poly.polygon.points.size(), vertex_range.end() - poly_range.initial);
			ref.set_primitive_colors(poly_range, poly.polygon.colors.data(), (GLuint)poly.polygon.colors.size());
			offset += (GLuint)poly.polygon.points.size();
		}
	}

	void PolyComposite::impl_set_polygon_colors() const
	{
		set_polygon_colors(get_ref(), composite);
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
				get_ref().draw_index() = faces[i][0] + initial_vertex + offset;
				get_ref().draw_index() = faces[i][1] + initial_vertex + offset;
				get_ref().draw_index() = faces[i][2] + initial_vertex + offset;
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
		set_polygon_composite(get_ref(), cache);
	}

	void NGon::impl_set_polygon_points() const
	{
		set_polygon_points(get_ref(), cache);
	}

	void NGon::impl_set_polygon_colors() const
	{
		set_polygon_colors(get_ref(), cache);
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
				get_ref().draw_index() = faces[i][0] + initial_vertex + offset;
				get_ref().draw_index() = faces[i][1] + initial_vertex + offset;
				get_ref().draw_index() = faces[i][2] + initial_vertex + offset;
			}
			offset += (GLuint)tp.polygon.points.size();
		}
	}
}
