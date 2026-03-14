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

		UnitVector2D forward() const { return transformer.forward(); }

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
				None = 0,
				SetLocal = 1,
				Full = SetLocal
			};

			constexpr Params operator&(Params a, Params b) { return (Params)((int)a & (int)b); }
			constexpr Params operator|(Params a, Params b) { return (Params)((int)a | (int)b); }
		}

		namespace chain
		{
			enum Params
			{
				None = 0,
				AttachParent = 1,
				AttachChild = 2,
				ClearChildren = 4,
				AttachOnly = AttachParent | AttachChild,
				Full = AttachParent | AttachChild | ClearChildren
			};

			constexpr Params operator&(Params a, Params b) { return (Params)((int)a & (int)b); }
			constexpr Params operator|(Params a, Params b) { return (Params)((int)a | (int)b); }
		}

		namespace modifier
		{
			enum Params
			{
				None = 0,
				RefModifier = 1,
				SetModifier = 2,
				SetFundamentalExtension = 4,
				Full = RefModifier | SetModifier
			};

			constexpr Params operator&(Params a, Params b) { return (Params)((int)a & (int)b); }
			constexpr Params operator|(Params a, Params b) { return (Params)((int)a | (int)b); }
		}
	}

	struct TExposureParams
	{
		exposure::local::Params local = exposure::local::Params::None;
		exposure::chain::Params chain = exposure::chain::Params::None;
		exposure::modifier::Params modifier = exposure::modifier::Params::None;
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

		UnitVector2D forward() const { return transformer.forward(); }

		Transform2D& set_local()
			requires (exposure::NONZERO(Params.local & exposure::local::SetLocal))
		{
			return transformer.set_local();
		}

		void attach_parent(Transformer2D* parent)
			requires (exposure::NONZERO(Params.chain & exposure::chain::AttachParent))
		{
			transformer.attach_parent(parent);
		}
		
		template<TExposureParams OtherParams>
		void attach_parent(Transformer2DExposure<OtherParams>& parent)
			requires (exposure::NONZERO(Params.chain & exposure::chain::AttachParent) && exposure::NONZERO(OtherParams.chain & exposure::chain::AttachChild))
		{
			transformer.attach_parent(&parent.transformer);
		}

		void attach_child(Transformer2D& child) requires
			(exposure::NONZERO(Params.chain & exposure::chain::AttachChild))
		{
			transformer.attach_child(child);
		}

		template<TExposureParams OtherParams>
		void attach_child(Transformer2DExposure<OtherParams>& child)
			requires (exposure::NONZERO(Params.chain & exposure::chain::AttachChild) && exposure::NONZERO(OtherParams.chain & exposure::chain::AttachParent))
		{
			transformer.get_handle().attach_child(&child.transformer);
		}

		void clear_children()
			requires (exposure::NONZERO(Params.chain & exposure::chain::ClearChildren))
		{
			transformer.get_handle().clear_children();
		}

		template<PolymorphicBaseOf<TransformModifier2D> T>
		const T& get_modifier() const
		{
			return transformer.get_modifier<T>();
		}

		Polymorphic<TransformModifier2D>& set_modifier()
			requires (exposure::NONZERO(Params.modifier & exposure::modifier::SetModifier))
		{
			return transformer.set_modifier();
		}

		template<PolymorphicBaseOf<TransformModifier2D> T>
		T& ref_modifier()
			requires (exposure::NONZERO(Params.modifier & exposure::modifier::RefModifier))
		{
			return transformer.ref_modifier<T>();
		}
		
		Polymorphic<TransformModifier2D>& set_fundamental_modifier_extension()
			requires (exposure::NONZERO(Params.modifier & exposure::modifier::SetFundamentalExtension))
		{
			return transformer.ref_modifier<FundamentalTransformModifier2D>().extension;
		}
	};
}
