#include "Polygons.h"

#include <algorithm>

#include "core/context/rendering/Rendering.h"
#include "core/math/Triangulation.h"
#include "core/cmath/Triangulation.h"
#include "graphics/resources/Shaders.h"
#include "core/util/Loader.h"

namespace oly::rendering
{
	internal::PolygonBatch::PolygonBatch()
		: ebo(vao), vbo_block(vao), vertex_free_space({ 0, nmax<GLuint>() })
	{
		shader_locations.projection = glGetUniformLocation(graphics::internal_shaders::polygon_batch, "uProjection");
		shader_locations.invariant_projection = glGetUniformLocation(graphics::internal_shaders::polygon_batch, "uInvariantProjection");

		vbo_block.attributes[POSITION] = graphics::VertexAttribute<graphics::VertexAttributeType::FLOAT>{ .index = 0, .size = 2 };
		vbo_block.attributes[COLOR] = graphics::VertexAttribute<graphics::VertexAttributeType::FLOAT>{ .index = 1, .size = 4 };
		vbo_block.attributes[INDEX] = graphics::VertexAttribute<graphics::VertexAttributeType::INT>{ .index = 2, .size = 1 };
		vbo_block.setup();
	}

	void internal::PolygonBatch::render() const
	{
		if (ebo.empty() || !camera)
			return;

		vbo_block.pre_draw_all();
		transform_ssbo.pre_draw();
		glBindVertexArray(vao);
		glUseProgram(graphics::internal_shaders::polygon_batch);
		glUniformMatrix3fv(shader_locations.projection, 1, GL_FALSE, glm::value_ptr(camera->projection_matrix()));
		glUniformMatrix3fv(shader_locations.invariant_projection, 1, GL_FALSE, glm::value_ptr(camera->invariant_projection_matrix()));

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
		transform_ssbo.set(id).transform = transform;
	}

	void internal::PolygonBatch::set_polygon_camera_invariant(GLuint id, bool camera_invariant)
	{
		assert_valid_id(id);
		transform_ssbo.set(id).camera_invariant = camera_invariant;
	}

	const glm::mat3& internal::PolygonBatch::get_polygon_transform(GLuint id) const
	{
		assert_valid_id(id);
		return transform_ssbo.get(id).transform;
	}

	bool internal::PolygonBatch::is_polygon_camera_invariant(GLuint id) const
	{
		assert_valid_id(id);
		return transform_ssbo.get(id).camera_invariant;
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
				bool camera_invariant = batch->is_polygon_camera_invariant(other.id);
				id = batch->generate_id(num_vertices);
				batch->set_polygon_transform(id, transform);
				batch->set_polygon_camera_invariant(id, camera_invariant);
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
				const bool camera_invariant = batch->is_polygon_camera_invariant(id);
				batch->terminate_id(id);
				reset(*new_batch);
				batch = lock();
				id = batch->generate_id(num_vertices);
				batch->set_polygon_transform(id, transform);
				batch->set_polygon_camera_invariant(id, camera_invariant);
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
			const bool camera_invariant = batch->is_valid_id(id) ? batch->is_polygon_camera_invariant(id) : false;
			const bool resized = batch->resize_range(id, vertices);
			if (resized)
			{
				batch->set_polygon_transform(id, transform);
				batch->set_polygon_camera_invariant(id, camera_invariant);
			}
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

	void internal::PolygonReference::set_camera_invariant(bool camera_invariant) const
	{
		if (auto batch = lock()) [[likely]]
			batch->set_polygon_camera_invariant(id, camera_invariant);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	const glm::mat3& internal::PolygonReference::get_polygon_transform() const
	{
		if (auto batch = lock()) [[likely]]
			return batch->get_polygon_transform(id);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	bool internal::PolygonReference::is_camera_invariant() const
	{
		if (auto batch = lock()) [[likely]]
			return batch->is_polygon_camera_invariant(id);
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

	void internal::PolygonSubmitter::set_camera_invariant(bool invariant) const
	{
		if (invariant)
			camera_invariant = CameraInvariantFlag(camera_invariant | CameraInvariantFlag::VALUE);
		else
			camera_invariant = CameraInvariantFlag(camera_invariant & ~CameraInvariantFlag::VALUE);

		camera_invariant = CameraInvariantFlag(camera_invariant | CameraInvariantFlag::DIRTY);
	}

	bool internal::PolygonSubmitter::is_camera_invariant() const
	{
		return camera_invariant & CameraInvariantFlag::VALUE;
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
					_OLY_ENGINE_LOG_WARNING("RENDERING") << "Error occurred during polygon triangulation." << LOG.nl;
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

		if (camera_invariant & CameraInvariantFlag::DIRTY)
		{
			camera_invariant = CameraInvariantFlag(camera_invariant & ~CameraInvariantFlag::DIRTY);
			ref.set_camera_invariant(camera_invariant & CameraInvariantFlag::VALUE);
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

	Polygon Polygon::load(TOMLNode node)
	{
		if (!node)
			return {};

		Polygon polygon;

		polygon.transformer = Transformer2D::load(node["transformer"]);

		std::vector<glm::vec2> points;
		auto toml_points = node["points"].as_array();
		if (toml_points)
		{
			size_t pt_idx = 0;
			for (auto& toml_point : *toml_points)
			{
				glm::vec2 pt;
				if (io::parse_vec((TOMLNode)toml_point, pt))
					points.push_back(pt);
				else
					_OLY_ENGINE_LOG_WARNING("ASSETS") << "Cannot convert polygon point #" << pt_idx << " to vec2." << LOG.nl;
				++pt_idx;
			}
		}
		polygon.set_points() = std::move(points);

		std::vector<glm::vec4> colors;
		auto toml_colors = node["colors"].as_array();
		if (toml_colors)
		{
			size_t color_idx = 0;
			for (auto& toml_color : *toml_colors)
			{
				glm::vec4 col;
				if (io::parse_vec((TOMLNode)toml_color, col))
					colors.push_back(col);
				else
					_OLY_ENGINE_LOG_WARNING("ASSETS") << "Cannot convert polygon point color #" << color_idx << " to vec4." << LOG.nl;
				++color_idx;
			}
		}
		polygon.set_colors() = std::move(colors);

		return polygon;
	}

	Polygon Polygon::load(TOMLNode node, const DebugTrace& trace)
	{
		auto scope = trace.scope("ASSETS", "oly::rendering::Polygon::load()");
		return load(node);
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

	PolyComposite PolyComposite::load(TOMLNode node)
	{
		if (!node)
			return {};

		PolyComposite polygon;

		polygon.transformer = Transformer2D::load(node["transformer"]);

		auto toml_method = node["method"].value<std::string>();
		if (toml_method)
		{
			const std::string& method = toml_method.value();
			if (method == "ngon")
			{
				std::vector<glm::vec2> points;
				auto toml_points = node["points"].as_array();
				if (toml_points)
				{
					size_t pt_idx = 0;
					for (auto& toml_point : *toml_points)
					{
						glm::vec2 pt;
						if (io::parse_vec((TOMLNode)toml_point, pt))
							points.push_back(pt);
						else
							_OLY_ENGINE_LOG_WARNING("ASSETS") << "Cannot convert poly composite point #" << pt_idx << " to vec2." << LOG.nl;
						++pt_idx;
					}
				}

				std::vector<glm::vec4> colors;
				auto toml_fill_colors = node["colors"].as_array();
				if (toml_fill_colors)
				{
					size_t color_idx = 0;
					for (auto& toml_color : *toml_fill_colors)
					{
						glm::vec4 col;
						if (io::parse_vec((TOMLNode)toml_color, col))
							colors.push_back(col);
						else
							_OLY_ENGINE_LOG_WARNING("ASSETS") << "Cannot convert poly composite point color #" << color_idx << " to vec4." << LOG.nl;
						++color_idx;
					}
				}

				polygon.set_composite() = cmath::Polygon2DComposite{ cmath::create_ngon(std::move(colors), std::move(points)) };
			}
			else if (method == "bordered_ngon")
			{
				cmath::NGonBase ngon_base;

				auto toml_points = node["points"].as_array();
				if (toml_points)
				{
					size_t pt_idx = 0;
					for (auto& toml_point : *toml_points)
					{
						glm::vec2 pt;
						if (io::parse_vec((TOMLNode)toml_point, pt))
							ngon_base.points.push_back(pt);
						else
							_OLY_ENGINE_LOG_WARNING("ASSETS") << "Cannot convert poly composite point #" << pt_idx << " to vec2." << LOG.nl;
						++pt_idx;
					}
				}

				auto toml_fill_colors = node["fill_colors"].as_array();
				if (toml_fill_colors)
				{
					size_t color_idx = 0;
					for (auto& toml_color : *toml_fill_colors)
					{
						glm::vec4 col;
						if (io::parse_vec((TOMLNode)toml_color, col))
							ngon_base.fill_colors.push_back(col);
						else
							_OLY_ENGINE_LOG_WARNING("ASSETS") << "Cannot convert poly composite fill color #" << color_idx << " to vec4." << LOG.nl;
						++color_idx;
					}
				}

				auto toml_border_colors = node["border_colors"].as_array();
				if (toml_border_colors)
				{
					size_t color_idx = 0;
					for (auto& toml_color : *toml_border_colors)
					{
						glm::vec4 col;
						if (io::parse_vec((TOMLNode)toml_color, col))
							ngon_base.border_colors.push_back(col);
						else
							_OLY_ENGINE_LOG_WARNING("ASSETS") << "Cannot convert poly composite border color #" << color_idx << " to vec4." << LOG.nl;
						++color_idx;
					}
				}

				io::parse_float(node["border_width"], ngon_base.border_width);

				if (auto border_pivot = node["border_pivot"])
				{
					if (auto str_border_pivot = border_pivot.value<std::string>())
					{
						const std::string& str = str_border_pivot.value();
						if (str == "outer")
							ngon_base.border_pivot = cmath::BorderPivot::OUTER;
						else if (str == "middle")
							ngon_base.border_pivot = cmath::BorderPivot::MIDDLE;
						else if (str == "inner")
							ngon_base.border_pivot = cmath::BorderPivot::INNER;
						else
							_OLY_ENGINE_LOG_WARNING("ASSETS") << "Unrecognized border pivot named value \"" << str << "\"." << LOG.nl;
					}
					else
						io::parse_float(border_pivot, ngon_base.border_pivot.v);
				}

				polygon.set_composite() = cmath::create_bordered_ngon(std::move(ngon_base.fill_colors), std::move(ngon_base.border_colors),
					ngon_base.border_width, ngon_base.border_pivot, std::move(ngon_base.points));
			}
			else if (method == "convex_decomposition")
			{
				std::vector<glm::vec2> points;

				auto toml_points = node["points"].as_array();
				if (toml_points)
				{
					size_t pt_idx = 0;
					for (auto& toml_point : *toml_points)
					{
						glm::vec2 pt;
						if (io::parse_vec((TOMLNode)toml_point, pt))
							points.push_back(pt);
						else
							_OLY_ENGINE_LOG_WARNING("ASSETS") << "Cannot convert poly composite point #" << pt_idx << " to vec2." << LOG.nl;
						++pt_idx;
					}
				}

				polygon.set_composite() = cmath::Decompose{}(std::move(points));
			}
		}

		return polygon;
	}

	PolyComposite PolyComposite::load(TOMLNode node, const DebugTrace& trace)
	{
		auto scope = trace.scope("ASSETS", "oly::rendering::PolyComposite::load()");
		return load(node);
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

	NGon NGon::load(TOMLNode node)
	{
		if (!node)
			return {};

		NGon polygon;

		polygon.transformer = Transformer2D::load(node["transformer"]);

		cmath::NGonBase ngon_base;

		auto toml_points = node["points"].as_array();
		if (toml_points)
		{
			size_t pt_idx = 0;
			for (auto& toml_point : *toml_points)
			{
				glm::vec2 pt;
				if (io::parse_vec((TOMLNode)toml_point, pt))
					ngon_base.points.push_back(pt);
				else
					_OLY_ENGINE_LOG_WARNING("ASSETS") << "Cannot convert ngon point #" << pt_idx << " to vec2." << LOG.nl;
				++pt_idx;
			}
		}

		auto toml_fill_colors = node["fill_colors"].as_array();
		if (toml_fill_colors)
		{
			size_t color_idx = 0;
			for (auto& toml_color : *toml_fill_colors)
			{
				glm::vec4 col;
				if (io::parse_vec((TOMLNode)toml_color, col))
					ngon_base.fill_colors.push_back(col);
				else
					_OLY_ENGINE_LOG_WARNING("ASSETS") << "Cannot convert ngon fill color #" << color_idx << " to vec4." << LOG.nl;
				++color_idx;
			}
		}

		auto toml_border_colors = node["border_colors"].as_array();
		if (toml_border_colors)
		{
			size_t color_idx = 0;
			for (auto& toml_color : *toml_border_colors)
			{
				glm::vec4 col;
				if (io::parse_vec((TOMLNode)toml_color, col))
					ngon_base.border_colors.push_back(col);
				else
					_OLY_ENGINE_LOG_WARNING("ASSETS") << "Cannot convert ngon border color #" << color_idx << " to vec4." << LOG.nl;
				++color_idx;
			}
		}

		bool bordered;
		if (io::parse_bool(node["bordered"], bordered))
			polygon.set_bordered(bordered);
		io::parse_float(node["border_width"], ngon_base.border_width);

		auto border_pivot = node["border_pivot"];
		if (auto str_border_pivot = border_pivot.value<std::string>())
		{
			const std::string& str = str_border_pivot.value();
			if (str == "outer")
				ngon_base.border_pivot = cmath::BorderPivot::OUTER;
			else if (str == "middle")
				ngon_base.border_pivot = cmath::BorderPivot::MIDDLE;
			else if (str == "inner")
				ngon_base.border_pivot = cmath::BorderPivot::INNER;
			else
				_OLY_ENGINE_LOG_WARNING("ASSETS") << "Unrecognized border pivot named value \"" << str << "\"." << LOG.nl;
		}
		else
			io::parse_float(border_pivot, ngon_base.border_pivot.v);

		polygon.set_base() = std::move(ngon_base);

		return polygon;
	}

	NGon NGon::load(TOMLNode node, const DebugTrace& trace)
	{
		auto scope = trace.scope("ASSETS", "oly::rendering::NGon::load()");
		return load(node);
	}
}
