#pragma once

#include <unordered_set>
#include <memory>

#include "external/GLM.h"
#include "core/base/UnitVector.h"
#include "core/containers/IDGenerator.h"
#include "core/base/Constants.h"

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

	class Transformer2D;

	namespace internal
	{
		class Transformer2DRegistry
		{
			typedef glm::uint Index;

			oly::SoftIDGenerator<Index> id_generator;
			static const Index NULL_INDEX = Index(-1);

			Transformer2DRegistry()
				: id_generator(0, nmax<Index>() - 1)
			{
			}

			Transformer2DRegistry(const Transformer2DRegistry&) = delete;
			Transformer2DRegistry(Transformer2DRegistry&&) = delete;

			std::vector<Transformer2D*> transformers;
			std::vector<Index> parent;
			std::vector<Index> index_in_parent;
			std::vector<std::vector<Index>> children;

		public:
			static Transformer2DRegistry& instance() { static Transformer2DRegistry reg; return reg; }

			class Handle
			{
				friend class Transformer2D;

				Index id;

				void init(Transformer2D* transformer);
				void del();

				Handle(Transformer2D* transformer);
				Handle(Transformer2D* transformer, const Handle&);
				Handle(Transformer2D* transformer, Handle&&) noexcept;

			public:
				~Handle();

			private:
				Handle& operator=(const Handle&);
				Handle& operator=(Handle&&) noexcept;

			public:
				void unparent() const;
				void clear_children() const;
				Transformer2D* get_parent() const;

			private:
				void set_parent(Index new_parent) const;
				void children_post_set_internal() const;
				void children_post_set_external() const;

			public:
				Transformer2D* get_top_level_parent() const;

				void attach_parent(Transformer2D* parent) const;
				void attach_child(Transformer2D& child) const;
				Index children_count() const;
				Transformer2D* get_child(Index index) const;

				void break_from_chain() const;
			};
		};
	}

	class Transformer2D
	{
		friend class internal::Transformer2DRegistry::Handle;

		Transform2D local;
		internal::Transformer2DRegistry::Handle handle;
		mutable glm::mat3 _global = glm::mat3(1.0f);
		mutable bool _dirty_internal = true;
		mutable bool _dirty_external = true;
		std::unique_ptr<TransformModifier2D> modifier;

	public:
		Transformer2D(Transform2D local = {}, std::unique_ptr<TransformModifier2D>&& modifier = std::make_unique<TransformModifier2D>());
		Transformer2D(const Transformer2D&);
		Transformer2D(Transformer2D&&) noexcept;
		~Transformer2D();
		Transformer2D& operator=(const Transformer2D&);
		Transformer2D& operator=(Transformer2D&&) noexcept;

		const internal::Transformer2DRegistry::Handle& get_handle() const { return handle; }

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

		void attach_parent(Transformer2D* parent) const { handle.attach_parent(parent); }
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

	extern glm::vec2 transform_point(const glm::mat3& tr, glm::vec2 point);
	extern glm::vec2 transform_point(const glm::mat3x2& tr, glm::vec2 point);
	extern glm::vec2 transform_direction(const glm::mat3& tr, glm::vec2 direction);
	extern glm::vec2 transform_direction(const glm::mat3x2& tr, glm::vec2 direction);
	extern glm::vec2 transform_normal(const glm::mat3& tr, glm::vec2 normal);
	extern glm::vec2 transform_normal(const glm::mat3x2& tr, glm::vec2 normal);
	
	inline glm::mat3 augment(const glm::mat2& l, glm::vec2 t) { return { glm::vec3(l[0], 0.0f), glm::vec3(l[1], 0.0f), glm::vec3(t, 1.0f) }; }
	inline glm::mat3 augment(const glm::mat3x2& tr) { return { glm::vec3(tr[0], 0.0f), glm::vec3(tr[1], 0.0f), glm::vec3(tr[2], 1.0f) }; }

	// TODO v5 Rather than combining shear + pivot, use transform modifier composition with template size. Then implement a QuadDeform modifier that can morpjh the parallelogram to any quad shape by creating a more complex global 3x3 matrix. Following this, create utility to subdivide sprites into set of distinct sprites for use in destruction.
}
