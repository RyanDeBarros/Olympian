#include "Random.h"

#include "Geometry.h"
#include "math/Solvers.h"

namespace oly
{
	namespace random
	{
		namespace bound
		{
			float Uniform::operator()() const
			{
				return a + (b - a) * rng();
			}

			float PowerSpike::operator()() const
			{
				float r = rng();
				float res;
				if (power == 0)
					res = r * (b - a) + a;
				else
				{
					float comp = -a / (b - a);
					if (r < comp)
						res = a + pow(pow(-a, power) * r * (b - a), 1.0f / (power + 1.0f));
					else
						res = b - pow(pow(b, power) * (1.0f - r) * (b - a), 1.0f / (power + 1.0f));
				}
				return inverted ? (res > 0.0f ? b - res : a - res) : res;
			}

			float DualPowerSpike::operator()() const
			{
				float r = rng();
				float res;
				if (alpha == 0 && beta == 0)
					res = r * (b - a) + a;
				else
				{
					float m = 1.0f / (b / (beta + 1.0f) - a / (alpha + 1.0f));
					float comp = -a * m / (alpha + 1.0f);
					if (r < comp)
						res = a + pow((alpha + 1.0f) * pow(-a, alpha) * r / m, 1.0f / (alpha + 1.0f));
					else
						res = b - pow((beta + 1.0f) * pow(b, beta) * (1.0f - r) / m, 1.0f / (beta + 1.0f));
				}
				return inverted ? (res > 0.0f ? b - res : a - res) : res;
			}

			float Sine::operator()() const
			{
				float c = glm::abs(a);
				float R = rng() * (c * (max - min) - (a / (k * glm::pi<float>())) * (glm::cos(k * glm::pi<float>() * (max - b)) - glm::cos(k * glm::pi<float>() * (min - b))));
				math::solver::LinearCosine solver;
				solver.A = -k * glm::pi<float>() * c / a;
				solver.B = k * glm::pi<float>();
				solver.C = -k * glm::pi<float>() * b;
				solver.D = (k * glm::pi<float>() / a) * (c * min + R) - glm::cos(k * glm::pi<float>() * (min - b));
				return std::clamp(solver.solve(), min, max);
			}

			float PowerSpikeArray::operator()() const
			{
				float total_weight = 0.0f;
				for (const auto& spike : spikes)
					total_weight += spike.w;

				float target = rng() * total_weight;
				float sum = 0.0f;
				for (size_t i = 0; i < spikes.size(); ++i)
				{
					sum += spikes[i].w;
					if (sum > target)
						return spikes[i].pos + spikes[i].spike();
				}
				return 0.0f;
			}
		}

		namespace domain1d
		{
			float Interval::operator()() const
			{
				float r = bound::eval(fn);
				return mean + std::clamp(r * scale, -max_offset, max_offset);
			}

			float Domain::operator()() const
			{
				if (shapes.empty())
					return mean;
				else if (shapes.size() > 1)
				{
					float total_weight = 0.0f;
					for (const auto& shp : shapes)
						total_weight += shp.w;

					float target = rng() * total_weight;
					float sum = 0.0f;
					for (size_t i = 0; i < shapes.size(); ++i)
					{
						sum += shapes[i].w;
						if (sum > target)
							return mean + scale * eval(shapes[i].shape);
					}
					return mean;
				}
				else
					return mean + scale * eval(shapes[0].shape);
			}
		}

		namespace domain2d
		{
			glm::vec2 Rect::operator()() const
			{
				float rx = bound::eval(fnx);
				float ry = bound::eval(fny);
				return transform.matrix() * glm::vec3{ rx, ry, 1.0f };
			}

			glm::vec2 Ellipse::operator()() const
			{
				float rr = bound::eval(fnr);
				float ra = glm::pi<float>() * bound::eval(fna);
				return transform.matrix() * glm::vec3(math::coordinates::to_cartesian({ rr, ra }), 1.0f);
			}

			glm::vec2 BaryTriangle::operator()() const
			{
				float ra = 0.5f * (bound::eval(fna) + 1.0f);
				float rb = 0.5f * (bound::eval(fnb) + 1.0f);
				float rc = 0.5f * (bound::eval(fnc) + 1.0f);
				math::Barycentric b{ ra, rb, rc };
				return b.point(pta, ptb, ptc);
			}

			glm::vec2 EarTriangle::operator()() const
			{
				float rd = 0.5f * (bound::eval(fnd) + 1.0f);
				float ra = 0.5f * (bound::eval(fna) + 1.0f);
				glm::vec2 dir = ra * (next_pt - prev_pt) + prev_pt - root_pt;
				return root_pt + rd * dir;
			}

			glm::vec2 UniformTriangle::operator()() const
			{
				float ra = std::clamp(bound::eval(fna), 0.0f, 1.0f);
				float rb = std::clamp(bound::eval(fnb), 0.0f, 1.0f);
				if (ra + rb > 1.0f)
				{
					ra = 1.0f - ra;
					rb = 1.0f - rb;
				}
				return ra * pta + rb * ptb + (1.0f - ra - rb) * ptc;
			}

			glm::vec2 Domain::operator()() const
			{
				if (shapes.empty())
					return transform.position;
				else if (shapes.size() > 1)
				{
					float total_weight = 0.0f;
					for (const auto& shp : shapes)
						total_weight += shp.w;

					float target = rng() * total_weight;
					float sum = 0.0f;
					for (size_t i = 0; i < shapes.size(); ++i)
					{
						sum += shapes[i].w;
						if (sum > target)
							return transform.matrix() * glm::vec3(eval(shapes[i].shape), 1.0f);
					}
					return transform.position;
				}
				else
					return transform.matrix() * glm::vec3(eval(shapes[0].shape), 1.0f);
			}

			Domain create_triangulated_domain(const std::vector<glm::vec2>& polygon)
			{
				auto tp = math::ear_clipping(polygon);
				Domain d;
				d.shapes.resize(tp.size());
				for (size_t i = 0; i < tp.size(); ++i)
				{
					UniformTriangle t;
					t.pta = polygon[tp[i].x];
					t.ptb = polygon[tp[i].y];
					t.ptc = polygon[tp[i].z];
					d.shapes[i].shape = t;
					d.shapes[i].w = glm::abs(math::cross(t.ptb - t.pta, t.ptc - t.pta));
				}
				return d;
			}
		}

		namespace domain3d
		{
			glm::vec3 Prism::operator()() const
			{
				float rx = bound::eval(fnx);
				float ry = bound::eval(fny);
				float rz = bound::eval(fnz);
				return transform.matrix() * glm::vec4{ rx, ry, rz, 1.0f };
			}

			glm::vec3 Ellipsoid::operator()() const
			{
				float rr = bound::eval(fnr);
				float rtheta = glm::pi<float>() * bound::eval(fntheta);
				float rphi = 0.5f * glm::pi<float>() * bound::eval(fnphi);
				return transform.matrix() * glm::vec4(math::coordinates::to_cartesian({ rr, rtheta, rphi }), 1.0f);
			}

			glm::vec3 Domain::operator()() const
			{
				if (shapes.empty())
					return transform.position;
				else if (shapes.size() > 1)
				{
					float total_weight = 0.0f;
					for (const auto& shp : shapes)
						total_weight += shp.w;

					float target = rng() * total_weight;
					float sum = 0.0f;
					for (size_t i = 0; i < shapes.size(); ++i)
					{
						sum += shapes[i].w;
						if (sum > target)
							return transform.matrix() * glm::vec4(eval(shapes[i].shape), 1.0f);
					}
					return transform.position;
				}
				else
					return transform.matrix() * glm::vec4(eval(shapes[0].shape), 1.0f);
			}
		}

		namespace unbound
		{
			float LogisticBell::operator()() const
			{
				float r = rng();
				return glm::log(r / (1 - r)) / (4 * height);
			}
		}
	}
}
