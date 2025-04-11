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

		// NOTE: all random distributions have mean = 0, and generate an offset to the mean.
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
				bound1d::Function fnr, fna;
				Transform2D transform;

				glm::vec2 operator()() const;
			};

			enum
			{
				RECT,
				ELLIPSE,
			};
			using Shape = std::variant<
				Rect,
				Ellipse
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
		}
		
		namespace domain3d
		{

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
