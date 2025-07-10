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
	};

	enum TExposureParams
	{
		SET_LOCAL = 0b1,
		ATTACH_PARENT = 0b10,
		ATTACH_CHILD = 0b100,
		CLEAR_CHILDREN = 0b1000
	};

	constexpr TExposureParams operator&(TExposureParams a, TExposureParams b) { return (TExposureParams)((int)a & (int)b); }
	constexpr TExposureParams operator|(TExposureParams a, TExposureParams b) { return (TExposureParams)((int)a | (int)b); }
	constexpr TExposureParams operator~(TExposureParams a) { return (TExposureParams)(~(int)a); }
	constexpr bool and_(TExposureParams a, TExposureParams b) { return (a & b) != 0; }

	namespace exposure
	{
		constexpr TExposureParams STANDARD_HIDDEN = TExposureParams::SET_LOCAL | TExposureParams::ATTACH_PARENT | TExposureParams::ATTACH_CHILD;
		constexpr TExposureParams FULL = TExposureParams::SET_LOCAL | TExposureParams::ATTACH_PARENT | TExposureParams::ATTACH_CHILD | TExposureParams::CLEAR_CHILDREN;
	}

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

		Transform2D& set_local() requires (and_(Params, TExposureParams::SET_LOCAL)) { return transformer.set_local(); }

		void attach_parent(Transformer2D* parent) requires (and_(Params, TExposureParams::ATTACH_PARENT)) { transformer.attach_parent(parent); }
		template<TExposureParams OtherParams>
		void attach_parent(Transformer2DExposure& parent) requires (and_(Params, TExposureParams::ATTACH_PARENT) && and_(OtherParams, TExposureParams::ATTACH_CHILD))
		{
			transformer.attach_parent(&parent.transformer);
		}

		void attach_child(Transformer2D* child) requires (and_(Params, TExposureParams::ATTACH_CHILD)) { transformer.attach_child(child); }
		template<TExposureParams OtherParams>
		void attach_child(Transformer2DExposure& child) requires (and_(Params, TExposureParams::ATTACH_CHILD) && and_(OtherParams, TExposureParams::ATTACH_PARENT))
		{
			transformer.attach_child(&child.transformer);
		}

		void clear_children() requires (and_(Params, TExposureParams::CLEAR_CHILDREN)) { transformer.clear_children(); }
	};
}
