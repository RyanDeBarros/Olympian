#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <variant>
#include <vector>
#include <unordered_set>

namespace oly
{
	namespace geom
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

		struct Transform2D
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

		struct Transformer2D
		{
			Transform2D local;

		private:
			Transformer2D* parent = nullptr;
			std::unordered_set<Transformer2D*> children;
			
		protected:
			mutable glm::mat3 _global = glm::mat3(1.0f);
			
		private:
			mutable bool _dirty = true;
			mutable bool _dirty_flush = true;

		public:
			Transformer2D(Transform2D local = {}) : local(local) {}
			Transformer2D(const Transformer2D&) = delete; // TODO implement
			Transformer2D(Transformer2D&&) noexcept;
			virtual ~Transformer2D();
			Transformer2D& operator=(Transformer2D&&) noexcept = delete; // TODO implement

			virtual glm::mat3 global() const { return _global; }
			void post_set() const;
			void pre_get() const;
			bool flush() const;

			const Transformer2D* top_level_parent() const;
			Transformer2D* top_level_parent();
			void attach_parent(Transformer2D* parent);
			void insert_chain(Transformer2D* parent_chain);
			void unparent();
			void clear_children();
			void pop_from_chain();
		};

		constexpr glm::mat3 pivot_matrix_2d(glm::vec2 pivot, glm::vec2 size)
		{
			return translation_matrix_2d(size * (pivot - glm::vec2(0.5f)));
		}

		// TODO create subclass of Transformer2D that uses pivot matrices.
		struct PivotTransformer2D : public Transformer2D
		{
			glm::vec2 pivot = { 0.5f, 0.5f };
			glm::vec2 size = { 0.0f, 0.0f };

			PivotTransformer2D(Transform2D local = {}, glm::vec2 pivot = { 0.5f, 0.5f }, glm::vec2 size = {}) : Transformer2D(local), pivot(pivot), size(size) {}
			PivotTransformer2D(const PivotTransformer2D&) = delete; // TODO implement
			PivotTransformer2D(PivotTransformer2D&& other) noexcept : Transformer2D(std::move(other)), pivot(other.pivot), size(other.size) {}
			PivotTransformer2D& operator=(PivotTransformer2D&&) noexcept = delete; // TODO implement

			virtual glm::mat3 global() const override
			{
				glm::mat3 P = pivot_matrix_2d(pivot, size);
				return P * _global * glm::inverse(P);
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
			glm::quat rotation = glm::quat(0.0f, { 0.0f, 0.0f, 0.0f });
			glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
			glm::mat3x2 shearing = { { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f } };

			glm::mat4 matrix() const
			{
				return translation_matrix_3d(position) * (glm::mat4)rotation * scale_matrix_3d(scale) * shearing_matrix_3d(shearing);
			}
		};

		struct Transformer3D
		{
			Transform3D local;

		private:
			Transformer3D* parent = nullptr;
			std::unordered_set<Transformer3D*> children;
			
			mutable glm::mat4 _global = glm::mat4(1.0f);
			mutable bool _dirty = true;
			mutable bool _dirty_flush = true;

		public:
			Transformer3D(const Transform3D& local = {}) : local(local) {}
			Transformer3D(const Transformer3D&) = delete; // TODO implement
			Transformer3D(Transformer3D&&) noexcept = delete; // TODO implement
			~Transformer3D();

			glm::mat4 global() const { return _global; }
			void post_set() const;
			void pre_get() const;
			bool flush() const;

			const Transformer3D* top_level_parent() const;
			Transformer3D* top_level_parent();
			void attach_parent(Transformer3D* parent);
			void insert_chain(Transformer3D* parent_chain);
			void unparent();
			void clear_children();
			void pop_from_chain();
		};
	}
}
