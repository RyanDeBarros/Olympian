#pragma once

#include <random>
#include <variant>

#include "Transforms.h"

namespace oly
{
	inline float rng() { return (float)rand() / RAND_MAX; }
	
	namespace random
	{
		inline float seed() { srand((unsigned int)time(nullptr)); }

		namespace bound1d
		{
			struct Uniform
			{
				float a = -1.0f, b = 1.0f;
				float operator()() const;
			};

			struct PowerSpike
			{
				// t = 0, w = 1, alpha = beta = power
				float a = -1.0f, b = 1.0f, power = 1.0f;
				bool inverted = false;
				float operator()() const;
			};

			struct DualPowerSpike
			{
				// t = 0, w = 1
				float a = -1.0f, b = 1.0f;
				float alpha = 1.0f, beta = 1.0f;
				bool inverted = false;
				float operator()() const;
			};

			enum
			{
				UNIFORM,
				POWER_SPIKE,
				DUAL_POWER_SPIKE,
			};
			using Function = std::variant<
				Uniform,
				PowerSpike,
				DualPowerSpike
			>;
			constexpr float eval(const Function& func)
			{
				return std::visit([](const auto& fn) { return fn(); }, func);
			}
		}

		namespace domain1d
		{
			struct Interval
			{
				bound1d::Function fn;
				float mean = 0.0f;
				float scale = 1.0f;
				float max_offset = 1.0f;
			
				float operator()() const;
			};

			enum
			{
				INTERVAL,
			};
			using Shape = std::variant<
				Interval
			>;
			constexpr float eval(const Shape& shape)
			{
				return std::visit([](const auto& shp) { return shp(); }, shape);
			}
			struct Domain
			{
				float mean = 0.0f, scale = 1.0f;
				struct WeightedShape
				{
					Shape shape;
					float w = 1.0f;
				};
				std::vector<WeightedShape> shapes;

				float operator()() const;
			};
		}

		namespace domain2d
		{
			struct Rect
			{
				bound1d::Function fnx, fny;
				Transform2D transform;

				glm::vec2 operator()() const;
			};

			struct Ellipse
			{
				bound1d::Function fnr, fna; // radial, angular
				Transform2D transform;

				glm::vec2 operator()() const;
			};

			struct BaryTriangle
			{
				bound1d::Function fna, fnb, fnc; // barycentric coordinates relative to each point. Note that [-1, 1] from distribution maps to [0, 1] in barycentric coordiantes
				glm::vec2 pta = {}, ptb = {}, ptc = {};

				glm::vec2 operator()() const;
			};

			struct EarTriangle
			{
				bound1d::Function fnd, fna; // distance from ear, interpolation from prev_pt to next_pt. Note that [-1, 1] from distribution maps to [0, 1] in ear triangle coordinates
				glm::vec2 root_pt = {}, prev_pt = {}, next_pt = {};

				glm::vec2 operator()() const;
			};

			enum
			{
				RECT,
				ELLIPSE,
				BARY_TRIANGLE,
				EAR_TRIANGLE,
			};
			using Shape = std::variant<
				Rect,
				Ellipse,
				BaryTriangle,
				EarTriangle
			>;
			constexpr glm::vec2 eval(const Shape& shape)
			{
				return std::visit([](const auto& shp) { return shp(); }, shape);
			}
			struct Domain
			{
				Transform2D transform;
				struct WeightedShape
				{
					Shape shape;
					float w = 1.0f;
				};
				std::vector<WeightedShape> shapes;

				glm::vec2 operator()() const;
			};

			extern Domain create_triangulated_domain(const std::vector<glm::vec2>& polygon);
		}
		
		namespace domain3d
		{
			struct Prism
			{
				bound1d::Function fnx, fny, fnz;
				Transform3D transform;

				glm::vec3 operator()() const;
			};

			struct Ellipsoid
			{
				bound1d::Function fnr, fntheta, fnphi; // radial, azimuthal, polar
				Transform3D transform;
			
				glm::vec3 operator()() const;
			};

			enum
			{
				PRISM,
				ELLIPSOID,
			};
			using Shape = std::variant<
				Prism,
				Ellipsoid
			>;
			constexpr glm::vec3 eval(const Shape& shape)
			{
				return std::visit([](const auto& shp) { return shp(); }, shape);
			}
			struct Domain
			{
				Transform3D transform;
				struct WeightedShape
				{
					Shape shape;
					float w = 1.0f;
				};
				std::vector<WeightedShape> shapes;

				glm::vec3 operator()() const;
			};
		}

		namespace unbound1d
		{
			struct LogisticBell
			{
				float height = 1.0f;
				float operator()() const;
			};
		}
	}
}
