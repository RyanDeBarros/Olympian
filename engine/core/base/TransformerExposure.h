#pragma once

#include "core/base/Transforms.h"

namespace oly
{
	struct Transformer2DConstExposure
	{
	private:
		const Transformer2D& transformer;

	public:
		Transformer2DConstExposure(const Transformer2D& transformer) : transformer(transformer) {}

		glm::mat3 global() const { return transformer.global(); }
		const Transform2D& get_local() const { return transformer.get_local(); }

		template<std::derived_from<TransformModifier2D> T>
		const T& get_modifier() const { return transformer.get_modifier<T>(); }
	};

	namespace exposure
	{
		constexpr bool NONZERO(auto param) { return (bool)param != 0; }

		namespace local
		{
			enum Params
			{
				NONE = 0,
				SET_LOCAL = 1,
				FULL = SET_LOCAL
			};

			constexpr Params operator&(Params a, Params b) { return (Params)((int)a & (int)b); }
			constexpr Params operator|(Params a, Params b) { return (Params)((int)a | (int)b); }
		}

		namespace chain
		{
			enum Params
			{
				NONE = 0,
				ATTACH_PARENT = 1,
				ATTACH_CHILD = 2,
				CLEAR_CHILDREN = 4,
				ATTACH_ONLY = ATTACH_PARENT | ATTACH_CHILD,
				FULL = ATTACH_PARENT | ATTACH_CHILD | CLEAR_CHILDREN
			};

			constexpr Params operator&(Params a, Params b) { return (Params)((int)a & (int)b); }
			constexpr Params operator|(Params a, Params b) { return (Params)((int)a | (int)b); }
		}

		namespace modifier
		{
			enum Params
			{
				NONE = 0,
				REF_MODIFIER = 1,
				SET_MODIFIER = 2,
				FULL = REF_MODIFIER | SET_MODIFIER
			};

			constexpr Params operator&(Params a, Params b) { return (Params)((int)a & (int)b); }
			constexpr Params operator|(Params a, Params b) { return (Params)((int)a | (int)b); }
		}
	}

	struct TExposureParams
	{
		exposure::local::Params local = exposure::local::Params::NONE;
		exposure::chain::Params chain = exposure::chain::Params::NONE;
		exposure::modifier::Params modifier = exposure::modifier::Params::NONE;
	};

	template<TExposureParams Params>
	struct Transformer2DExposure
	{
	private:
		template<TExposureParams>
		friend struct Transformer2DExposure;

		Transformer2D& transformer;

	public:
		Transformer2DExposure(Transformer2D& transformer) : transformer(transformer) {}

		glm::mat3 global() const { return transformer.global(); }
		const Transform2D& get_local() const { return transformer.get_local(); }

		Transform2D& set_local() requires (exposure::NONZERO(Params.local & exposure::local::SET_LOCAL)) { return transformer.set_local(); }

		void attach_parent(Transformer2D* parent) requires (exposure::NONZERO(Params.chain & exposure::chain::ATTACH_PARENT)) { transformer.attach_parent(parent); }
		template<TExposureParams OtherParams>
		void attach_parent(Transformer2DExposure<OtherParams>& parent)
			requires (exposure::NONZERO(Params.chain & exposure::chain::ATTACH_PARENT) && exposure::NONZERO(OtherParams.chain & exposure::chain::ATTACH_CHILD))
		{
			transformer.attach_parent(&parent.transformer);
		}

		void attach_child(Transformer2D* child) requires (exposure::NONZERO(Params.chain & exposure::chain::ATTACH_CHILD)) { transformer.attach_child(child); }
		template<TExposureParams OtherParams>
		void attach_child(Transformer2DExposure<OtherParams>& child)
			requires (exposure::NONZERO(Params.chain & exposure::chain::ATTACH_CHILD) && exposure::NONZERO(OtherParams.chain & exposure::chain::ATTACH_PARENT))
		{
			transformer.get_handle().attach_child(&child.transformer);
		}

		void clear_children() requires (exposure::NONZERO(Params.chain & exposure::chain::CLEAR_CHILDREN)) { transformer.get_handle().clear_children(); }

		template<PolymorphicBaseOf<TransformModifier2D> T>
		const T& get_modifier() const { return transformer.get_modifier<T>(); }

		Polymorphic<TransformModifier2D>& set_modifier() requires (exposure::NONZERO(Params.modifier & exposure::modifier::SET_MODIFIER)) { return transformer.set_modifier(); }
		template<PolymorphicBaseOf<TransformModifier2D> T>
		T& ref_modifier() requires (exposure::NONZERO(Params.modifier & exposure::modifier::REF_MODIFIER)) { return transformer.ref_modifier<T>(); }
	};
}
