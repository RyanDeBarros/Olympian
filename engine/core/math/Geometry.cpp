#include "Geometry.h"

namespace oly::math
{
	float cross(glm::vec2 u, glm::vec2 v)
	{
		return u.x * v.y - u.y * v.x;
	}

	float magnitude(glm::vec2 v)
	{
		return glm::sqrt(glm::dot(v, v));
	}

	float mag_sqrd(glm::vec2 v)
	{
		return glm::dot(v, v);
	}

	float inv_magnitude(glm::vec2 v)
	{
		return glm::inversesqrt(glm::dot(v, v));
	}

	glm::vec2 project(glm::vec2 point, glm::vec2 axis)
	{
		return axis * glm::dot(point, axis) / glm::dot(axis, axis);
	}

	float projection_distance(glm::vec2 point, glm::vec2 axis)
	{
		return glm::dot(point, axis) / magnitude(axis);
	}

	bool in_convex_sector(glm::vec2 u1, glm::vec2 u2, glm::vec2 test)
	{
		if (cross(u1, u2) >= 0.0f)
			return cross(u1, test) >= 0.0f && cross(test, u2) >= 0.0f;
		else
			return cross(u1, test) <= 0.0f && cross(test, u2) <= 0.0f;
	}

	float signed_area(const std::vector<glm::vec2>& points)
	{
		float signed_area = cross(points.back(), points[0]);
		for (size_t i = 1; i < points.size(); ++i)
			signed_area += cross(points[i - 1], points[i]);
		return 0.5f * signed_area;
	}

	Edge::Edge(glm::uint a, glm::uint b)
		: a(std::min(a, b)), b(std::max(a, b))
	{
	}

	std::unordered_map<Edge, std::vector<glm::uint>, EdgeHash> build_adjecency(const Triangulation& triangulation)
	{
		std::unordered_map<Edge, std::vector<glm::uint>, EdgeHash> adjacency;
		for (glm::uint i = 0; i < triangulation.size(); ++i)
		{
			const auto& face = triangulation[i];
			adjacency[Edge(face[0], face[1])].push_back(i);
			adjacency[Edge(face[1], face[2])].push_back(i);
			adjacency[Edge(face[2], face[0])].push_back(i);
		}
		return adjacency;
	}
}
