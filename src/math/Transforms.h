#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <unordered_set>
#include <memory>

namespace oly
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

	constexpr glm::mat3 translation_matrix(glm::vec2 position)
	{
		return { vectors::I3, vectors::J3, glm::vec3(position, 1.0f) };
	}

	inline glm::mat2 rotation_matrix_2x2(float rotation)
	{
		float cos = glm::cos(rotation);
		float sin = glm::sin(rotation);
		return { { cos, sin }, { -sin, cos } };
	}

	inline glm::mat3 rotation_matrix(float rotation)
	{
		float cos = glm::cos(rotation);
		float sin = glm::sin(rotation);
		return { { cos, sin, 0.0f }, { -sin, cos, 0.0f }, vectors::H3 };
	}

	constexpr glm::mat3 scale_matrix(glm::vec2 scale)
	{
		return { scale.x * vectors::I3, scale.y * vectors::J3, vectors::H3 };
	}

	constexpr glm::vec2 extract_translation(const glm::mat3& transform)
	{
		return { transform[2][0], transform[2][1] };
	}

	inline float extract_rotation(const glm::mat3& transform)
	{
		float scale_x = glm::length(transform[0]);
		return glm::atan(transform[0][1] / scale_x, transform[0][0] / scale_x);
	}

	constexpr glm::vec2 extract_scale(const glm::mat3& transform)
	{
		return { glm::length(transform[0]), glm::length(transform[1]) };
	}

	struct Transform2D
	{
		glm::vec2 position = { 0.0f, 0.0f };
		float rotation = 0.0f;
		glm::vec2 scale = { 1.0f, 1.0f };

		constexpr glm::mat3 matrix() const
		{
			return translation_matrix(position) * rotation_matrix(rotation) * scale_matrix(scale);
		}
	};

	struct TransformModifier2D
	{
		virtual ~TransformModifier2D() = default;
		virtual glm::mat3 operator()(const glm::mat3& global) const { return global; }
	};

	struct Transformer2D
	{
		Transform2D local;
	
	private:
		mutable glm::mat3 _global = glm::mat3(1.0f);
		mutable bool _dirty = true;
		mutable bool _dirty_flush = true;
		Transformer2D* parent = nullptr;
		std::unordered_set<Transformer2D*> children;

	public:
		std::unique_ptr<TransformModifier2D> modifier;

		Transformer2D(Transform2D local = {}, std::unique_ptr<TransformModifier2D>&& modifier = std::make_unique<TransformModifier2D>()) : local(local), modifier(std::move(modifier)) {}
		Transformer2D(const Transformer2D&) = delete;
		Transformer2D(Transformer2D&&) noexcept;
		~Transformer2D();
		Transformer2D& operator=(Transformer2D&&) noexcept;

		glm::mat3 global() const { return (*modifier)(_global); }
		void post_set() const;
		void pre_get() const;
		bool flush() const;

		template<std::derived_from<TransformModifier2D> T>
		const T& get_modifier() const { return *static_cast<T*>(modifier.get()); }
		template<std::derived_from<TransformModifier2D> T>
		T& get_modifier() { return *static_cast<T*>(modifier.get()); }
		const Transformer2D* get_parent() const { return parent; }
		Transformer2D* get_parent() { return parent; }
		const Transformer2D* top_level_parent() const;
		Transformer2D* top_level_parent();
		void attach_parent(Transformer2D* parent);
		void insert_chain(Transformer2D* parent_chain);
		void unparent();
		void clear_children();
		void pop_from_chain();
	};

	constexpr glm::mat3 pivot_matrix(glm::vec2 pivot, glm::vec2 size)
	{
		return translation_matrix(size * (pivot - glm::vec2(0.5f)));
	}

	struct PivotTransformModifier2D : public TransformModifier2D
	{
		glm::vec2 pivot = { 0.5f, 0.5f };
		glm::vec2 size = { 0.0f, 0.0f };

		virtual glm::mat3 operator()(const glm::mat3& global) const override
		{
			glm::mat3 P = pivot_matrix(pivot, size);
			return P * global * glm::inverse(P);
		}
	};

	constexpr glm::mat3 shearing_matrix(glm::vec2 shearing)
	{
		return { { 1.0f, shearing.y, 0.0f }, { shearing.x, 1.0f, 0.0f }, vectors::H3 };
	}

	struct ShearTransformModifier2D : public TransformModifier2D
	{
		glm::vec2 shearing = { 0.0f, 0.0f };

		virtual glm::mat3 operator()(const glm::mat3& global) const override
		{
			return global * shearing_matrix(shearing);
		}
	};

	struct PivotShearTransformModifier2D : public TransformModifier2D
	{
		glm::vec2 pivot = { 0.5f, 0.5f };
		glm::vec2 size = { 0.0f, 0.0f };
		glm::vec2 shearing = { 0.0f, 0.0f };

		virtual glm::mat3 operator()(const glm::mat3& global) const override
		{
			glm::mat3 P = pivot_matrix(pivot, size);
			return P * global * shearing_matrix(shearing) * glm::inverse(P);
		}
	};

	constexpr glm::mat4 translation_matrix(glm::vec3 position)
	{
		return { vectors::I4, vectors::J4, vectors::K4, glm::vec4(position, 1.0f) };
	}

	constexpr glm::mat4 scale_matrix(glm::vec3 scale)
	{
		return { scale.x * vectors::I4, scale.y * vectors::J4, scale.z * vectors::K4, vectors::H4 };
	}

	constexpr glm::vec3 extract_translation(const glm::mat4& transform)
	{
		return { transform[3][0], transform[3][1], transform[3][2] };
	}

	inline glm::quat extract_rotation(const glm::mat4& transform)
	{
		return glm::quat_cast(glm::mat3{ transform[0] / glm::length(transform[0]), transform[1] / glm::length(transform[1]), transform[2] / glm::length(transform[2]) });
	}

	constexpr glm::vec3 extract_scale(const glm::mat4& transform)
	{
		return { glm::length(transform[0]), glm::length(transform[1]), glm::length(transform[2])};
	}

	struct Transform3D
	{
		glm::vec3 position = { 0.0f, 0.0f, 0.0f };
		glm::quat rotation = glm::quat(0.0f, { 0.0f, 0.0f, 0.0f });
		glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

		glm::mat4 matrix() const
		{
			return translation_matrix(position) * (glm::mat4)rotation * scale_matrix(scale);
		}
	};

	struct TransformModifier3D
	{
		virtual ~TransformModifier3D() = default;
		virtual glm::mat4 operator()(const glm::mat4& global) const { return global; }
	};

	struct Transformer3D
	{
		Transform3D local;
		
	private:
		mutable glm::mat4 _global = glm::mat4(1.0f);
		mutable bool _dirty = true;
		mutable bool _dirty_flush = true;
		Transformer3D* parent = nullptr;
		std::unordered_set<Transformer3D*> children;

	public:
		std::unique_ptr<TransformModifier3D> modifier;

		Transformer3D(const Transform3D& local = {}, std::unique_ptr<TransformModifier3D>&& modifier = std::make_unique<TransformModifier3D>()) : local(local), modifier(std::move(modifier)) {}
		Transformer3D(const Transformer3D&) = delete;
		Transformer3D(Transformer3D&&) noexcept;
		virtual ~Transformer3D();
		Transformer3D& operator=(Transformer3D&&) noexcept;

		virtual glm::mat4 global() const { return (*modifier)(_global); }
		void post_set() const;
		void pre_get() const;
		bool flush() const;

		template<std::derived_from<TransformModifier2D> T>
		const T& get_modifier() const { return *static_cast<T*>(modifier.get()); }
		template<std::derived_from<TransformModifier2D> T>
		T& get_modifier() { return *static_cast<T*>(modifier.get()); }
		const Transformer3D* get_parent() const { return parent; }
		Transformer3D* get_parent() { return parent; }
		const Transformer3D* top_level_parent() const;
		Transformer3D* top_level_parent();
		void attach_parent(Transformer3D* parent);
		void insert_chain(Transformer3D* parent_chain);
		void unparent();
		void clear_children();
		void pop_from_chain();
	};

	constexpr glm::mat4 pivot_matrix(glm::vec3 pivot, glm::vec3 size)
	{
		return translation_matrix(size * (pivot - glm::vec3(0.5f)));
	}

	struct PivotTransformModifier3D : public TransformModifier3D
	{
		glm::vec3 pivot = { 0.5f, 0.5f, 0.5f };
		glm::vec3 size = { 0.0f, 0.0f, 0.0f };

		virtual glm::mat4 operator()(const glm::mat4& global) const override
		{
			glm::mat4 P = pivot_matrix(pivot, size);
			return P * global * glm::inverse(P);
		}
	};

	constexpr glm::mat4 shearing_matrix(glm::mat3x2 shearing)
	{
		return { { 1.0f, shearing[1][0], shearing[2][0], 0.0f }, { shearing[0][0], 1.0f, shearing[2][1], 0.0f }, { shearing[0][1], shearing[1][1], 1.0f, 0.0f }, vectors::H4 };
	}

	struct ShearTransformModifier3D : public TransformModifier3D
	{
		glm::mat3x2 shearing = { { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f } };
		
		virtual glm::mat4 operator()(const glm::mat4& global) const override
		{
			return global * shearing_matrix(shearing);
		}
	};

	struct PivotShearTransformModifier3D : public TransformModifier3D
	{
		glm::vec3 pivot = { 0.5f, 0.5f, 0.5f };
		glm::vec3 size = { 0.0f, 0.0f, 0.0f };
		glm::mat3x2 shearing = { { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f } };

		virtual glm::mat4 operator()(const glm::mat4& global) const override
		{
			glm::mat4 P = pivot_matrix(pivot, size);
			return P * global * shearing_matrix(shearing) * glm::inverse(P);
		}
	};
}
