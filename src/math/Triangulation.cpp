#include "Geometry.h"

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "util/General.h"

struct Node;
struct EarClippingData
{
	std::vector<std::shared_ptr<Node>> nodes;
	std::weak_ptr<Node> head_polygon;
	std::weak_ptr<Node> head_convex;
	std::weak_ptr<Node> tail_convex;
	std::weak_ptr<Node> head_reflex;
	std::weak_ptr<Node> tail_reflex;
	std::weak_ptr<Node> head_ear;
	size_t size = 0;
	bool ccw;
	const std::vector<glm::vec2>* vertices;
};

struct Node
{
	glm::uint v;

	explicit Node(glm::uint v) : v(v) {}

	bool is_reflex = false;
	bool is_ear = false;

	// cyclical
	std::weak_ptr<Node> next_vertex;
	std::weak_ptr<Node> prev_vertex;
	// linear
	std::weak_ptr<Node> next_convex;
	std::weak_ptr<Node> prev_convex;
	// linear
	std::weak_ptr<Node> next_reflex;
	std::weak_ptr<Node> prev_reflex;
	// cyclical
	std::weak_ptr<Node> next_ear;
	std::weak_ptr<Node> prev_ear;

	oly::math::Triangle2D triangle(const EarClippingData& data) const
	{
		return oly::math::Triangle2D{ (*data.vertices)[v], (*data.vertices)[prev_vertex.lock()->v], (*data.vertices)[next_vertex.lock()->v]};
	}

	glm::uvec3 face() const
	{
		return glm::uvec3{ prev_vertex.lock()->v, v, next_vertex.lock()->v};
	}

	bool should_be_reflexive(const EarClippingData& data) const
	{
		return data.ccw == (triangle(data).cross() <= 0.0f);
	}

	bool should_be_ear(const EarClippingData& data) const
	{
		if (is_reflex)
			return false;
		auto tr = triangle(data);
		auto pv = prev_vertex.lock();
		for (auto tester = next_vertex.lock()->next_vertex.lock(); tester != pv; tester = tester->next_vertex.lock())
		{
			if (tester->should_be_reflexive(data) && tr.barycentric((*data.vertices)[tester->v]).inside())
				return false;
		}
		return true;
	}
};

static std::shared_ptr<Node> append_vertex(EarClippingData& data, glm::uint v)
{
	std::shared_ptr<Node> insert = std::make_shared<Node>(v);
	data.nodes.push_back(insert);
	if (auto h = data.head_polygon.lock())
	{
		const std::shared_ptr<Node>& tail = h->prev_vertex.lock();
		insert->prev_vertex = tail;
		tail->next_vertex = insert;
		insert->next_vertex = data.head_polygon;
		h->prev_vertex = insert;
	}
	else
	{
		data.head_polygon = insert;
		h = data.head_polygon.lock();
		h->next_vertex = data.head_polygon;
		h->prev_vertex = data.head_polygon;
	}
	return insert;
}

static void append_reflex(EarClippingData& data, const std::shared_ptr<Node>& insert)
{
	if (auto h = data.head_reflex.lock())
	{
		data.tail_reflex.lock()->next_reflex = insert;
		insert->prev_reflex = data.tail_reflex;
		data.tail_reflex = insert;
	}
	else
	{
		data.head_reflex = insert;
		data.tail_reflex = insert;
	}
}

static void append_convex(EarClippingData& data, const std::shared_ptr<Node>& insert)
{
	if (auto h = data.head_convex.lock())
	{
		data.tail_convex.lock()->next_convex = insert;
		insert->prev_convex = data.tail_convex;
		data.tail_convex = insert;
	}
	else
	{
		data.head_convex = insert;
		data.tail_convex = insert;
	}
}

static void append_ear(EarClippingData& data, const std::shared_ptr<Node>& insert)
{
	if (auto h = data.head_ear.lock())
	{
		const std::weak_ptr<Node>& tail = h->prev_ear;
		insert->prev_ear = tail;
		tail.lock()->next_ear = insert;
		insert->next_ear = data.head_ear;
		h->prev_ear = insert;
	}
	else
	{
		data.head_ear = insert;
		h = data.head_ear.lock();
		h->next_ear = data.head_ear;
		h->prev_ear = data.head_ear;
	}
}
static void update_adjacent(EarClippingData& data, const std::shared_ptr<Node>& adj)
{
	// note that if an adjacent vertex is convex, it will remain convex after removing ear.
	if (adj->is_reflex)
	{
		if (adj->should_be_reflexive(data))
			return; // was reflexive and continues to be reflexive --> no change
		else
		{
			// change to convex
			adj->is_reflex = false;
			append_convex(data, adj);

			// remove from reflexive
			if (data.head_reflex.lock() == adj)
			{
				if (data.tail_reflex.lock() == adj)
					data.tail_reflex.reset();
				else
					adj->next_reflex.lock()->prev_reflex.reset();
				data.head_reflex = adj->next_reflex;
			}
			else
			{
				if (data.tail_reflex.lock() == adj)
					data.tail_reflex = adj->prev_reflex.lock();
				else
					adj->next_reflex.lock()->prev_reflex = adj->prev_reflex;
				adj->prev_reflex.lock()->next_reflex = adj->next_reflex;
			}
			adj->next_reflex.reset();
			adj->prev_reflex.reset();
		}
	}

	if (adj->should_be_ear(data))
	{
		if (!adj->is_ear)
		{
			adj->is_ear = true;
			append_ear(data, adj);
		}
	}
	else
	{
		if (adj->is_ear)
		{
			adj->is_ear = false;

			// remove from ear
			if (adj == data.head_ear.lock())
			{
				if (adj->next_ear.lock() == data.head_ear.lock())
					data.head_ear.reset();
				else
				{
					adj->next_ear.lock()->prev_ear = adj->prev_ear;
					adj->prev_ear.lock()->next_ear = adj->next_ear;
					data.head_ear = adj->next_ear;
				}
			}
			else
			{
				adj->next_ear.lock()->prev_ear = adj->prev_ear;
				adj->prev_ear.lock()->next_ear = adj->next_ear;
			}
			adj->next_ear.reset();
			adj->prev_ear.reset();
		}
	}
};

static void remove_ear(EarClippingData& data, std::shared_ptr<Node> remove)
{
	assert(remove->is_ear);
	// remove from polygon
	auto hp = data.head_polygon.lock();
	if (remove == hp)
	{
		if (hp->next_vertex.lock() == hp)
			data.head_polygon.reset();
		else
		{
			data.head_polygon = remove->next_vertex;
			remove->next_vertex.lock()->prev_vertex = remove->prev_vertex;
			remove->prev_vertex.lock()->next_vertex = remove->next_vertex;
		}
	}
	else
	{
		remove->next_vertex.lock()->prev_vertex = remove->prev_vertex;
		remove->prev_vertex.lock()->next_vertex = remove->next_vertex;
	}
	// remove from reflex
	if (remove->is_reflex)
	{
		auto hr = data.head_reflex.lock();
		if (remove == hr)
		{
			if (hr->next_reflex.lock() == hr)
				data.head_reflex.reset();
			else
			{
				data.head_reflex = remove->next_reflex;
				remove->next_reflex.lock()->prev_vertex.reset();
			}
		}
		else
		{
			std::shared_ptr<Node> following = remove->next_reflex.lock();
			if (following != nullptr)
				following->prev_reflex = remove->prev_reflex;
			remove->prev_reflex.lock()->next_reflex = following;
		}
		remove->next_reflex.reset();
		remove->prev_reflex.reset();
	}
	// remove from convex
	else
	{
		auto hc = data.head_convex.lock();
		if (remove == hc)
		{
			if (hc->next_convex.lock() == hc)
				data.head_convex.reset();
			else
			{
				data.head_convex = remove->next_convex;
				remove->next_convex.lock()->prev_convex.reset();
			}
		}
		else
		{
			std::shared_ptr<Node> following = remove->next_convex.lock();
			if (following != nullptr)
				following->prev_convex = remove->prev_convex;
			remove->prev_convex.lock()->next_convex = following;
		}
		remove->next_convex.reset();
		remove->prev_convex.reset();
	}
	// remove from ears
	auto he = data.head_ear.lock();
	if (remove == he)
	{
		if (he->next_ear.lock() == he)
			data.head_ear.reset();
		else
		{
			remove->next_ear.lock()->prev_ear = remove->prev_ear;
			remove->prev_ear.lock()->next_ear = remove->next_ear;
			data.head_ear = remove->next_ear;
		}
	}
	else
	{
		remove->next_ear.lock()->prev_ear = remove->prev_ear;
		remove->prev_ear.lock()->next_ear = remove->next_ear;
	}
	remove->next_ear.reset();
	remove->prev_ear.reset();

	// update categorization of adjacent vertices
	update_adjacent(data, remove->next_vertex.lock());
	update_adjacent(data, remove->prev_vertex.lock());
	remove->next_vertex.reset();
	remove->prev_vertex.reset();
	--data.size;

	assert(data.size == 3 || data.head_ear.lock());
}

oly::math::Triangulation oly::math::ear_clipping(const std::vector<glm::vec2>& polygon, bool increasing, int starting_offset, int ear_cycle)
{
	assert(polygon.size() >= 3);
	Triangulation triangulation;

	if (polygon.size() == 3)
	{
		glm::uvec3 face{ unsigned_mod(0 + starting_offset, (int)polygon.size()), unsigned_mod(1 + starting_offset, (int)polygon.size()), unsigned_mod(2 + starting_offset, (int)polygon.size()) };
		triangulation.push_back(increasing ? face : reverse(face));
		return triangulation;
	}

	EarClippingData data{};
	data.size = polygon.size();
	data.vertices = &polygon;

	// load polygon vertices
	for (int i = 0; i < polygon.size(); ++i)
		append_vertex(data, unsigned_mod(i + starting_offset, (int)polygon.size()));

	// determine orientation
	data.ccw = (signed_area(polygon) >= 0.0f);

	std::shared_ptr<Node> indexer = data.head_polygon.lock();
	// categorize initial vertices
	do
	{
		if (indexer->should_be_reflexive(data))
		{
			indexer->is_reflex = true;
			append_reflex(data, indexer);
		}
		else
		{
			indexer->is_reflex = false;
			append_convex(data, indexer);
			if (indexer->should_be_ear(data))
			{
				indexer->is_ear = true;
				append_ear(data, indexer);
			}
		}

		indexer = indexer->next_vertex.lock();
	} while (indexer != data.head_polygon.lock());

	assert(data.head_ear.lock());

	// remove ears and form faces
	if (ear_cycle == 0)
	{
		while (data.size > 3)
		{
			auto face = data.head_ear.lock()->face();
			triangulation.push_back(increasing ? face : reverse(face));
			remove_ear(data, data.head_ear.lock());
		}
	}
	else if (ear_cycle > 0)
	{
		std::shared_ptr<Node> indexer = data.head_ear.lock();
		while (data.size > 3)
		{
			std::shared_ptr<Node> next_indexer = indexer;
			for (int i = 0; i < ear_cycle; ++i)
				next_indexer = next_indexer->next_ear.lock();
			auto face = indexer->face();
			triangulation.push_back(increasing ? face : reverse(face));
			remove_ear(data, indexer);
			indexer = next_indexer;
		}
	}
	else // if (ear_cycle < 0)
	{
		std::shared_ptr<Node> indexer = data.head_ear.lock();
		while (data.size > 3)
		{
			std::shared_ptr<Node> prev_indexer = indexer;
			for (int i = 0; i > ear_cycle; --i)
				prev_indexer = prev_indexer->prev_ear.lock();
			auto face = indexer->face();
			triangulation.push_back(increasing ? face : reverse(face));
			remove_ear(data, indexer);
			indexer = prev_indexer;
		}
	}
	// final face
	auto face = data.head_ear.lock()->face();
	triangulation.push_back(increasing ? face : reverse(face));
	return triangulation;
}

glm::uint oly::math::get_first_ear(const std::vector<glm::vec2>& polygon, int starting_offset)
{
	assert(polygon.size() >= 3);
	if (polygon.size() == 3)
		return 0;

	EarClippingData data{};
	data.size = polygon.size();
	data.vertices = &polygon;

	// load polygon vertices
	for (int i = 0; i < polygon.size(); ++i)
		append_vertex(data, unsigned_mod(i + starting_offset, (int)polygon.size()));

	// determine orientation
	data.ccw = (signed_area(polygon) >= 0.0f);

	std::shared_ptr<Node> indexer = data.head_polygon.lock();
	do
	{
		if (!indexer->should_be_reflexive(data) && indexer->should_be_ear(data))
			return indexer->v;
		indexer = indexer->next_vertex.lock();
	} while (indexer != data.head_polygon.lock());

	// TODO everywhere throughout project, don't use assert, as this only works for debug. Create custom assert (one for debug and one for release) and logger.
	assert(false);
	return -1;
}

std::vector<oly::math::Triangulation> oly::math::convex_decompose_triangulation(const std::vector<glm::vec2>& polygon)
{
	assert(polygon.size() >= 3);
	return convex_decompose_triangulation(polygon, ear_clipping(polygon));
}

std::vector<oly::math::Triangulation> oly::math::convex_decompose_triangulation(const std::vector<glm::vec2>& polygon, const Triangulation& triangulation)
{
	assert(polygon.size() >= 3);
	// flood fill algorithm
	std::vector<Triangulation> sub_triangulations;
	const auto& adjacency = build_adjecency(triangulation);
	std::vector<bool> visited(triangulation.size(), false);
	for (size_t i = 0; i < triangulation.size(); ++i)
	{
		if (visited[i])
			continue;

		Triangulation convex_subtr;
		std::unordered_map<glm::uint, glm::uvec2> boundary;
		std::vector<glm::uint> stack = { (glm::uint)i };

		while (!stack.empty())
		{
			glm::uint curr = stack.back();
			stack.pop_back();

			if (visited[curr])
				continue;
			visited[curr] = true;

			const auto& face = triangulation[curr];
			convex_subtr.push_back(triangulation[curr]);

			// initial triangle boundary
			if (convex_subtr.size() == 1)
			{
				boundary[face[0]] = { face[2], face[1] };
				boundary[face[1]] = { face[0], face[2] };
				boundary[face[2]] = { face[1], face[0] };
			}

			Edge edges[3] = {
				Edge(face[0], face[1]),
				Edge(face[1], face[2]),
				Edge(face[2], face[0])
			};
			for (const auto& edge : edges)
			{
				const auto& adj = adjacency.find(edge)->second;
				for (glm::uint neighbour : adj)
				{
					if (!visited[neighbour])
					{
						// get new vertex that would be added to convex subpolygon
						glm::uvec3 ntr = triangulation[neighbour];
						int new_vertex;
						if (ntr[0] != face[0] && ntr[0] != face[1] && ntr[0] != face[2])
							new_vertex = 0;
						else if (ntr[1] != face[0] && ntr[1] != face[1] && ntr[1] != face[2])
							new_vertex = 1;
						else
							new_vertex = 2;

						glm::uint icpt = ntr[new_vertex];
						glm::uint ippt = ntr[unsigned_mod(new_vertex - 1, 3)];
						glm::uint inpt = ntr[unsigned_mod(new_vertex + 1, 3)];
						glm::vec2 cpt = polygon[icpt];
						glm::vec2 ppt = polygon[ippt];
						glm::vec2 npt = polygon[inpt];
						auto& padj = boundary.find(ippt)->second;
						auto& nadj = boundary.find(inpt)->second;
						glm::vec2 pd{}, nd{};
						if (padj[0] != inpt)
							pd = ppt - polygon[padj[0]];
						else
							pd = ppt - polygon[padj[1]];
						if (nadj[0] != ippt)
							nd = npt - polygon[nadj[0]];
						else
							nd = npt - polygon[nadj[1]];

						// does new vertex maintain convexity?
						if (in_convex_sector(pd, npt - ppt, cpt - ppt) && in_convex_sector(nd, ppt - npt, cpt - npt))
						{
							stack.push_back(neighbour);
							if (padj[0] == inpt)
								padj[0] = icpt;
							else
								padj[1] = icpt;
							if (nadj[0] == ippt)
								nadj[0] = icpt;
							else
								nadj[1] = icpt;
							boundary[icpt] = { ippt, inpt };
						}
					}
				}
			}
		}

		sub_triangulations.push_back(std::move(convex_subtr));
	}
	
	return sub_triangulations;
}

std::vector<std::pair<std::vector<glm::vec2>, oly::math::Triangulation>> oly::math::convex_decompose_polygon(const std::vector<glm::vec2>& polygon)
{
	assert(polygon.size() >= 3);
	std::vector<Triangulation> decomposition = convex_decompose_triangulation(polygon);
	return decompose_polygon(polygon, decomposition);
}

std::vector<std::pair<std::vector<glm::vec2>, oly::math::Triangulation>> oly::math::convex_decompose_polygon(const std::vector<glm::vec2>& polygon, const Triangulation& triangulation)
{
	assert(polygon.size() >= 3);
	std::vector<Triangulation> decomposition = convex_decompose_triangulation(polygon, triangulation);
	return decompose_polygon(polygon, decomposition);
}

std::vector<std::pair<std::vector<glm::vec2>, oly::math::Triangulation>> oly::math::decompose_polygon(const std::vector<glm::vec2>& polygon, const std::vector<Triangulation>& triangulations)
{
	assert(polygon.size() >= 3);
	std::vector<std::pair<std::vector<glm::vec2>, oly::math::Triangulation>> subpolygons;
	subpolygons.reserve(triangulations.size());

	for (const Triangulation& triangulation : triangulations)
	{
		std::pair<std::vector<glm::vec2>, Triangulation> subpolygon;
		std::unordered_map<glm::uint, glm::uint> point_indices;
		for (glm::uvec3 face : triangulation)
		{
			glm::uvec3 new_face{};
			for (glm::length_t i = 0; i < 3; ++i)
			{
				if (!point_indices.count(face[i]))
				{
					point_indices[face[i]] = (glm::uint)point_indices.size();
					subpolygon.first.push_back(polygon[face[i]]);
				}
				new_face[i] = point_indices[face[i]];
			}
			subpolygon.second.push_back(new_face);
		}
		subpolygons.push_back(std::move(subpolygon));
	}

	return subpolygons;
}

oly::math::Polygon2DComposite oly::math::composite_convex_decomposition(const std::vector<glm::vec2>& points)
{
	auto decomposition = oly::math::convex_decompose_polygon(points);
	Polygon2DComposite composite;
	composite.reserve(decomposition.size());
	for (auto& subconvex : decomposition)
	{
		oly::math::TriangulatedPolygon2D tp;
		tp.polygon.points = std::move(subconvex.first);
		tp.polygon.colors = { glm::vec4(1.0f) };
		tp.triangulation = std::move(subconvex.second);
		composite.push_back(std::move(tp));
	}
	return composite;
}
