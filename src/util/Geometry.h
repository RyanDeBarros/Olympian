#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <variant>

namespace oly
{
	namespace math
	{
		namespace vectors
		{
			constexpr glm::vec2 I2 = { 1.0f, 0.0f };
			constexpr glm::vec3 I3 = { 1.0f, 0.0f, 0.0f };
			constexpr glm::vec4 I4 = { 1.0f, 0.0f, 0.0f, 0.0f };
			constexpr glm::vec2 J2 = { 0.0f, 1.0f };
			constexpr glm::vec3 J3 = { 0.0f, 1.0f, 0.0f };
			constexpr glm::vec4 J4 = { 0.0f, 1.0f, 0.0f, 0.0f };
			constexpr glm::vec3 K3 = { 0.0f, 0.0f, 1.0f };
			constexpr glm::vec4 K4 = { 0.0f, 0.0f, 1.0f, 0.0f };
			constexpr glm::vec3 H3 = { 0.0f, 0.0f, 1.0f };
			constexpr glm::vec4 H4 = { 0.0f, 0.0f, 0.0f, 1.0f };
		}

		constexpr glm::mat3 translation_matrix_2d(glm::vec2 position)
		{
			return { vectors::I3, vectors::J3, glm::vec3(position, 1.0f) };
		}

		inline glm::mat3 rotation_matrix_2d(float rotation)
		{
			float cos = glm::cos(rotation);
			float sin = glm::sin(rotation);
			return { { cos, sin, 0.0f }, { -sin, cos, 0.0f }, vectors::H3 };
		}

		constexpr glm::mat3 scale_matrix_2d(glm::vec2 scale)
		{
			return { scale.x * vectors::I3, scale.y * vectors::J3, vectors::H3 };
		}

		constexpr glm::mat3 shearing_matrix_2d(glm::vec2 shearing)
		{
			return { { 1.0f, shearing.y, 0.0f }, { shearing.x, 1.0f, 0.0f }, vectors::H3 };
		}

		struct FlatTransform2D
		{
			glm::vec2 position = { 0.0f, 0.0f };
			glm::vec2 scale = { 1.0f, 1.0f };

			constexpr glm::mat3 matrix() const
			{
				return translation_matrix_2d(position) * scale_matrix_2d(scale);
			}
		};

		struct Transform2D
		{
			glm::vec2 position = { 0.0f, 0.0f };
			float rotation = 0.0f;
			glm::vec2 scale = { 1.0f, 1.0f };

			constexpr glm::mat3 matrix() const
			{
				return translation_matrix_2d(position) * rotation_matrix_2d(rotation) * scale_matrix_2d(scale);
			}
		};

		struct AffineTransform2D
		{
			glm::vec2 position = { 0.0f, 0.0f };
			float rotation = 0.0f;
			glm::vec2 scale = { 1.0f, 1.0f };
			glm::vec2 shearing = { 0.0f, 0.0f };

			constexpr glm::mat3 matrix() const
			{
				return translation_matrix_2d(position) * rotation_matrix_2d(rotation) * scale_matrix_2d(scale) * shearing_matrix_2d(shearing);
			}
		};

		typedef std::variant<FlatTransform2D, Transform2D, AffineTransform2D> Mat3Variant;
		struct Mat3 : public Mat3Variant
		{	
			enum class Type
			{
				FLAT,
				STANDARD,
				AFFINE
			};

			struct unsupported_operation : public std::bad_variant_access {};

			constexpr Mat3(Type type = Type::STANDARD)
			{
				if (type == Type::STANDARD)
					*this = Transform2D();
				else if (type == Type::FLAT)
					*this = FlatTransform2D();
				else
					*this = AffineTransform2D();
			}
			constexpr Mat3(const FlatTransform2D& t) : Mat3Variant(t) {}
			constexpr Mat3(const Transform2D& t) : Mat3Variant(t) {}
			constexpr Mat3(const AffineTransform2D& t) : Mat3Variant(t) {}

			constexpr glm::mat3 matrix() const
			{
				return std::visit([](auto&& transform) -> glm::mat3 { return transform.matrix(); }, *this);
			}

			constexpr glm::vec2 position() const
			{
				return std::visit([](auto&& transform) -> glm::vec2 { return transform.position; }, *this);
			}

			constexpr glm::vec2& position()
			{
				return std::visit([](auto&& transform) -> glm::vec2& { return transform.position; }, *this);
			}

			constexpr float rotation() const
			{
				return std::visit([](auto&& transform) -> float {
					if constexpr (std::is_same_v<std::decay_t<decltype(transform)>, Transform2D> || std::is_same_v<std::decay_t<decltype(transform)>, AffineTransform2D>)
						return transform.rotation;
					else
						throw unsupported_operation();
					}, *this);
			}

			constexpr float& rotation()
			{
				return std::visit([](auto&& transform) -> float& {
					if constexpr (std::is_same_v<std::decay_t<decltype(transform)>, Transform2D> || std::is_same_v<std::decay_t<decltype(transform)>, AffineTransform2D>)
						return transform.rotation;
					else
						throw unsupported_operation();
					}, *this);
			}

			constexpr glm::vec2 scale() const
			{
				return std::visit([](auto&& transform) -> glm::vec2 { return transform.scale; }, *this);
			}

			constexpr glm::vec2& scale()
			{
				return std::visit([](auto&& transform) -> glm::vec2& { return transform.scale; }, *this);
			}

			constexpr glm::vec2 shearing() const
			{
				return std::visit([](auto&& transform) -> glm::vec2 {
					if constexpr (std::is_same_v<std::decay_t<decltype(transform)>, AffineTransform2D>)
						return transform.shearing;
					else
						throw unsupported_operation();
					}, *this);
			}

			constexpr glm::vec2& shearing()
			{
				return std::visit([](auto&& transform) -> glm::vec2& {
					if constexpr (std::is_same_v<std::decay_t<decltype(transform)>, AffineTransform2D>)
						return transform.shearing;
					else
						throw unsupported_operation();
					}, *this);
			}
		};

		constexpr glm::mat4 translation_matrix_3d(glm::vec3 position)
		{
			return { vectors::I4, vectors::J4, vectors::K4, glm::vec4(position, 1.0f) };
		}

		struct Rotator
		{
			float pitch = 0.0f;
			float yaw = 0.0f;
			float roll = 0.0f;

			constexpr operator glm::quat() const
			{
				return glm::quat({ roll, pitch, yaw });
			}
		};

		inline Rotator rotator(glm::quat quaternion)
		{
			glm::vec3 angles = glm::eulerAngles(quaternion);
			return { angles.y, angles.z, angles.x };
		}

		constexpr glm::mat4 scale_matrix_3d(glm::vec3 scale)
		{
			return { scale.x * vectors::I4, scale.y * vectors::J4, scale.z * vectors::K4, vectors::H4 };
		}

		constexpr glm::mat4 shearing_matrix_3d(glm::mat3x2 shearing)
		{
			return { { 1.0f, shearing[1][0], shearing[2][0], 0.0f }, { shearing[0][0], 1.0f, shearing[2][1], 0.0f }, { shearing[0][1], shearing[1][1], 1.0f, 0.0f }, vectors::H4 };
		}

		struct Transform3D
		{
			glm::vec3 position = { 0.0f, 0.0f, 0.0f };
			glm::quat rotation = Rotator{};
			glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

			glm::mat4 matrix() const
			{
				return translation_matrix_3d(position) * (glm::mat4)rotation * scale_matrix_3d(scale);
			}
		};

		struct AffineTransform3D
		{
			glm::vec3 position = { 0.0f, 0.0f, 0.0f };
			glm::quat rotation = glm::quat(0.0f, { 0.0f, 0.0f, 0.0f });
			glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
			glm::mat3x2 shearing = { { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f } };

			glm::mat4 matrix() const
			{
				return translation_matrix_3d(position) * (glm::mat4)rotation * scale_matrix_3d(scale) * shearing_matrix_3d(shearing);
			}
		};

		typedef std::variant<Transform3D, AffineTransform3D> Mat4Variant;
		struct Mat4 : public Mat4Variant
		{
			enum class Type
			{
				STANDARD,
				AFFINE
			};

			struct unsupported_operation : public std::bad_variant_access {};

			constexpr Mat4(Type type = Type::STANDARD)
			{
				if (type == Type::STANDARD)
					*this = Transform3D();
				else
					*this = AffineTransform3D();
			}
			constexpr Mat4(const Transform3D& t) : Mat4Variant(t) {}
			constexpr Mat4(const AffineTransform3D& t) : Mat4Variant(t) {}

			constexpr glm::mat4 matrix() const
			{
				return std::visit([](auto&& transform) -> glm::mat4 { return transform.matrix(); }, *this);
			}

			constexpr glm::vec3 position() const
			{
				return std::visit([](auto&& transform) -> glm::vec3 { return transform.position; }, *this);
			}

			constexpr glm::vec3& position()
			{
				return std::visit([](auto&& transform) -> glm::vec3& { return transform.position; }, *this);
			}

			constexpr glm::quat rotation() const
			{
				return std::visit([](auto&& transform) -> glm::quat { return transform.rotation; }, *this);
			}

			constexpr glm::quat& rotation()
			{
				return std::visit([](auto&& transform) -> glm::quat& { return transform.rotation; }, *this);
			}

			constexpr glm::vec3 scale() const
			{
				return std::visit([](auto&& transform) -> glm::vec3 { return transform.scale; }, *this);
			}

			constexpr glm::vec3& scale()
			{
				return std::visit([](auto&& transform) -> glm::vec3& { return transform.scale; }, *this);
			}

			constexpr glm::mat3x2 shearing() const
			{
				return std::visit([](auto&& transform) -> glm::mat3x2 {
					if constexpr (std::is_same_v<std::decay_t<decltype(transform)>, AffineTransform3D>)
						return transform.shearing;
					else
						throw unsupported_operation();
					}, *this);
			}

			constexpr glm::mat3x2& shearing()
			{
				return std::visit([](auto&& transform) -> glm::mat3x2& {
					if constexpr (std::is_same_v<std::decay_t<decltype(transform)>, AffineTransform3D>)
						return transform.shearing;
					else
						throw unsupported_operation();
					}, *this);
			}
		};
	}
}
