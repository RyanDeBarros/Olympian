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

		namespace bound
		{
			struct Constant
			{
				float c = 0.0f;
				float operator()() const { return c; }
			};

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

			struct Sine
			{
				// a * sin(k * pi * (x - b)) + (1 - N) / (max - min), where N is a normalizing factor
				float min = -1.0f, max = 1.0f;
				float a = 1.0f, k = 1.0f, b = 0.0f;

				float operator()() const;
			};

			struct PowerSpikeArray
			{
				struct WeightedSpike
				{
					PowerSpike spike;
					float pos;
					float w = 1.0f;
				};
				std::vector<WeightedSpike> spikes;
				float operator()() const;
			};

			using Function = std::variant<
				Constant,
				Uniform,
				PowerSpike,
				DualPowerSpike,
				Sine,
				PowerSpikeArray
			>;
			constexpr float eval(const Function& func)
			{
				return std::visit([](const auto& fn) { return fn(); }, func);
			}
			struct Function2D
			{
				Function x, y;
				glm::vec2 eval() const { return { bound::eval(x), bound::eval(y) }; }
			};
			struct Function3D
			{
				Function x, y, z;
				glm::vec3 eval() const { return { bound::eval(x), bound::eval(y), bound::eval(z) }; }
			};
			struct Function4D
			{
				Function x, y, z, w;
				glm::vec4 eval() const { return { bound::eval(x), bound::eval(y), bound::eval(z), bound::eval(w) }; }
			};
		}

		namespace domain1d
		{
			struct Interval
			{
				bound::Function fn;
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
				bound::Function fnx, fny;
				Transform2D transform;

				glm::vec2 operator()() const;
			};

			struct Ellipse
			{
				bound::Function fnr, fna; // radial, angular
				Transform2D transform;

				glm::vec2 operator()() const;
			};

			struct BaryTriangle
			{
				bound::Function fna, fnb, fnc; // barycentric coordinates relative to each point. Note that [-1, 1] from distribution maps to [0, 1] in barycentric coordiantes
				glm::vec2 pta = {}, ptb = {}, ptc = {};

				glm::vec2 operator()() const;
			};

			struct EarTriangle
			{
				bound::Function fnd, fna; // distance from ear, interpolation from prev_pt to next_pt. Note that [-1, 1] from distribution maps to [0, 1] in ear triangle coordinates
				glm::vec2 root_pt = {}, prev_pt = {}, next_pt = {};

				glm::vec2 operator()() const;
			};

			struct UniformTriangle
			{
				bound::Function fna = bound::Uniform{ 0.0f, 1.0f };
				bound::Function fnb = bound::Uniform{ 0.0f, 1.0f };
				glm::vec2 pta = {}, ptb = {}, ptc = {};

				glm::vec2 operator()() const;
			};

			using Shape = std::variant<
				Rect,
				Ellipse,
				BaryTriangle,
				EarTriangle,
				UniformTriangle
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

			extern std::vector<Domain::WeightedShape> create_triangulated_domain_shapes(const std::vector<glm::vec2>& polygon);
		}
		
		namespace domain3d
		{
			struct Prism
			{
				bound::Function fnx, fny, fnz;
				Transform3D transform;

				glm::vec3 operator()() const;
			};

			struct Ellipsoid
			{
				bound::Function fnr, fntheta, fnphi; // radial, azimuthal, polar
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

		namespace unbound
		{
			struct LogisticBell
			{
				float height = 1.0f;
				float operator()() const;
			};
		}
	}
}
