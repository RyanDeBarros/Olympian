#pragma once

#include <unordered_set>
#include <memory>

#include "external/GLM.h"
#include "core/base/UnitVector.h"

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
		virtual void operator()(glm::mat3& global) const {}
		virtual std::unique_ptr<TransformModifier2D> clone() const { return std::make_unique<TransformModifier2D>(); }
	};

#define OLY_TRANSFORM_MODIFIER_2D_CLONE_OVERRIDE(Class)\
	virtual std::unique_ptr<TransformModifier2D> clone() const override\
	{\
		return std::make_unique<Class>(*this);\
	}

	class Transformer2D
	{
		Transform2D local;
		mutable glm::mat3 _global = glm::mat3(1.0f);
		mutable bool _dirty_internal = true;
		mutable bool _dirty_external = true;
		Transformer2D* parent = nullptr;
		size_t index_in_parent = size_t(-1);
		std::vector<Transformer2D*> children;
		std::unique_ptr<TransformModifier2D> modifier;

	public:
		Transformer2D(Transform2D local = {}, std::unique_ptr<TransformModifier2D>&& modifier = std::make_unique<TransformModifier2D>()) : local(local), modifier(std::move(modifier)) {}
		Transformer2D(const Transformer2D&); // NOTE this does not carry over children
		Transformer2D(Transformer2D&&) noexcept;
		~Transformer2D();
		Transformer2D& operator=(const Transformer2D&); // NOTE this keeps old children
		Transformer2D& operator=(Transformer2D&&) noexcept;

		glm::mat3 global() const { pre_get(); return _global; }
		void set_global(const glm::mat3& g) { _global = g; post_set(); _dirty_internal = false; }

	private:
		void post_set() const;
		void post_set_internal() const;
		void post_set_external() const;
		void pre_get() const;

	public:
		bool flush() const;
		bool dirty() const { return _dirty_external; }
		const Transform2D& get_local() const { return local; }
		Transform2D& set_local() { post_set(); return local; }

		std::unique_ptr<TransformModifier2D>& set_modifier() { post_set(); return modifier; }
		template<std::derived_from<TransformModifier2D> T>
		const T& get_modifier() const { return *static_cast<T*>(modifier.get()); }
		template<std::derived_from<TransformModifier2D> T>
		T& ref_modifier() { post_set(); return *static_cast<T*>(modifier.get()); }

		const Transformer2D* get_parent() const { return parent; }
		Transformer2D* get_parent() { return parent; }
		const Transformer2D* top_level_parent() const;
		Transformer2D* top_level_parent();
		void attach_parent(Transformer2D* parent);
		void attach_child(Transformer2D* child);
		void unparent();
		void clear_children();
		void pop_from_chain();
	};

	constexpr glm::mat3 pivot_matrix(glm::vec2 pivot, glm::vec2 size)
	{
		return translation_matrix(size * (glm::vec2(0.5f) - pivot));
	}

	struct PivotTransformModifier2D : public TransformModifier2D
	{
		glm::vec2 pivot = { 0.5f, 0.5f };
		glm::vec2 size = { 0.0f, 0.0f };

		PivotTransformModifier2D(glm::vec2 pivot = glm::vec2(0.5f), glm::vec2 size = glm::vec2(0.0f)) : pivot(pivot), size(size) {}

		virtual void operator()(glm::mat3& global) const override;

		OLY_TRANSFORM_MODIFIER_2D_CLONE_OVERRIDE(PivotTransformModifier2D);
	};

	constexpr glm::mat3 shearing_matrix(glm::vec2 shearing)
	{
		return { { 1.0f, shearing.y, 0.0f }, { shearing.x, 1.0f, 0.0f }, vectors::H3 };
	}

	struct ShearTransformModifier2D : public TransformModifier2D
	{
		glm::vec2 shearing = { 0.0f, 0.0f };

		ShearTransformModifier2D(glm::vec2 shearing = glm::vec2(0.0f)) : shearing(shearing) {}

		virtual void operator()(glm::mat3& global) const override;

		OLY_TRANSFORM_MODIFIER_2D_CLONE_OVERRIDE(ShearTransformModifier2D);
	};

	struct OffsetTransformModifier2D : public TransformModifier2D
	{
		glm::vec2 offset = { 0.0f, 0.0f };

		OffsetTransformModifier2D(glm::vec2 offset = glm::vec2(0.0f)) : offset(offset) {}

		virtual void operator()(glm::mat3& global) const override;

		OLY_TRANSFORM_MODIFIER_2D_CLONE_OVERRIDE(OffsetTransformModifier2D);
	};

	constexpr glm::mat4 translation_matrix(glm::vec3 position)
	{
		return { vectors::I4, vectors::J4, vectors::K4, glm::vec4(position, 1.0f) };
	}

	constexpr glm::mat4 scale_matrix(glm::vec3 scale)
	{
		return { scale.x * vectors::I4, scale.y * vectors::J4, scale.z * vectors::K4, vectors::H4 };
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
		virtual void operator()(glm::mat4& global) const {}
		virtual std::unique_ptr<TransformModifier3D> clone() const { return std::make_unique<TransformModifier3D>(); }
	};

#define OLY_TRANSFORM_MODIFIER_3D_CLONE_OVERRIDE(Class)\
	virtual std::unique_ptr<TransformModifier3D> clone() const override\
	{\
		return std::make_unique<Class>(*this);\
	}

	class Transformer3D
	{
		Transform3D local;
		mutable glm::mat4 _global = glm::mat4(1.0f);
		mutable bool _dirty_internal = true;
		mutable bool _dirty_external = true;
		Transformer3D* parent = nullptr;
		size_t index_in_parent = size_t(-1);
		std::vector<Transformer3D*> children;
		std::unique_ptr<TransformModifier3D> modifier;

	public:
		Transformer3D(const Transform3D& local = {}, std::unique_ptr<TransformModifier3D>&& modifier = std::make_unique<TransformModifier3D>()) : local(local), modifier(std::move(modifier)) {}
		Transformer3D(const Transformer3D&);
		Transformer3D(Transformer3D&&) noexcept;
		~Transformer3D();
		Transformer3D& operator=(const Transformer3D&);
		Transformer3D& operator=(Transformer3D&&) noexcept;

		glm::mat4 global() const { pre_get(); return _global; }
		void set_global(const glm::mat4& g) { _global = g; post_set(); _dirty_internal = false; }

	private:
		void post_set() const;
		void post_set_internal() const;
		void post_set_external() const;
		void pre_get() const;

	public:
		bool flush() const;
		bool dirty() const { return _dirty_external; }
		const Transform3D& get_local() const { return local; }
		Transform3D& set_local() { post_set(); return local; }

		std::unique_ptr<TransformModifier3D>& set_modifier() { post_set(); return modifier; }
		template<std::derived_from<TransformModifier3D> T>
		const T& get_modifier() const { return *static_cast<T*>(modifier.get()); }
		template<std::derived_from<TransformModifier3D> T>
		T& ref_modifier() { post_set(); return *static_cast<T*>(modifier.get()); }

		const Transformer3D* get_parent() const { return parent; }
		Transformer3D* get_parent() { return parent; }
		const Transformer3D* top_level_parent() const;
		Transformer3D* top_level_parent();
		void attach_parent(Transformer3D* parent);
		void attach_child(Transformer3D* child);
		void unparent();
		void clear_children();
		void pop_from_chain();
	};

	constexpr glm::mat4 pivot_matrix(glm::vec3 pivot, glm::vec3 size)
	{
		return translation_matrix(size * (glm::vec3(0.5f) - pivot));
	}

	struct PivotTransformModifier3D : public TransformModifier3D
	{
		glm::vec3 pivot = { 0.5f, 0.5f, 0.5f };
		glm::vec3 size = { 0.0f, 0.0f, 0.0f };

		PivotTransformModifier3D(glm::vec3 pivot = glm::vec3(0.5f), glm::vec3 size = glm::vec3(0.0f)) : pivot(pivot), size(size) {}

		virtual void operator()(glm::mat4& global) const override;

		OLY_TRANSFORM_MODIFIER_3D_CLONE_OVERRIDE(PivotTransformModifier3D);
	};

	constexpr glm::mat4 shearing_matrix(glm::mat3x2 shearing)
	{
		return { { 1.0f, shearing[1][0], shearing[2][0], 0.0f }, { shearing[0][0], 1.0f, shearing[2][1], 0.0f }, { shearing[0][1], shearing[1][1], 1.0f, 0.0f }, vectors::H4 };
	}

	struct ShearTransformModifier3D : public TransformModifier3D
	{
		glm::mat3x2 shearing = { { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f } };

		ShearTransformModifier3D(glm::mat3x2 shearing = {}) : shearing(shearing) {}

		virtual void operator()(glm::mat4& global) const override;

		OLY_TRANSFORM_MODIFIER_3D_CLONE_OVERRIDE(ShearTransformModifier3D);
	};

	struct OffsetTransformModifier3D : public TransformModifier3D
	{
		glm::vec3 offset = { 0.0f, 0.0f, 0.0f };

		OffsetTransformModifier3D(glm::vec3 offset = glm::vec3(0.0f)) : offset(offset) {}

		virtual void operator()(glm::mat4& global) const override;

		OLY_TRANSFORM_MODIFIER_3D_CLONE_OVERRIDE(OffsetTransformModifier3D);
	};

	extern glm::vec2 transform_point(const glm::mat3& tr, glm::vec2 point);
	extern glm::vec2 transform_point(const glm::mat3x2& tr, glm::vec2 point);
	extern glm::vec2 transform_direction(const glm::mat3& tr, glm::vec2 direction);
	extern glm::vec2 transform_direction(const glm::mat3x2& tr, glm::vec2 direction);
	extern glm::vec2 transform_normal(const glm::mat3& tr, glm::vec2 normal);
	extern glm::vec2 transform_normal(const glm::mat3x2& tr, glm::vec2 normal);
	
	inline glm::mat3 augment(const glm::mat2& l, glm::vec2 t) { return { glm::vec3(l[0], 0.0f), glm::vec3(l[1], 0.0f), glm::vec3(t, 1.0f) }; }
	inline glm::mat3 augment(const glm::mat3x2& tr) { return { glm::vec3(tr[0], 0.0f), glm::vec3(tr[1], 0.0f), glm::vec3(tr[2], 1.0f) }; }
}
