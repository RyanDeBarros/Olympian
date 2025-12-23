#pragma once

#include "graphics/particles/AttributeView.h"

namespace oly::particles
{
	struct SineAttributeView : public IAttributeView
	{
		// attribute = a * sin(b * t - k) + c
		float a = 1.0f;
		float b = 1.0f;
		float k = 0.0f;
		float c = 0.0f;

		using IAttributeView::IAttributeView;

		void on_tick() override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(SineAttributeView);
	};
}
