#pragma once

#include "graphics/particles/AttributeView.h"

namespace oly::particles
{
	struct GenericAttributeView1D : public IAttributeView1D
	{
		std::function<void(ParticleEmitter&, float&)> fn; // fn(emitter, attribute)

		using IAttributeView1D::IAttributeView1D;

		void on_tick() override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(GenericAttributeView1D);
	};

	struct SineAttributeView1D : public IAttributeView1D
	{
		// attribute = a * sin(b * t - k) + c
		float a = 1.0f;
		float b = 1.0f;
		float k = 0.0f;
		float c = 0.0f;

		using IAttributeView1D::IAttributeView1D;

		void on_tick() override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(SineAttributeView1D);
	};

	struct GenericAttributeView2D : public IAttributeView2D
	{
		std::function<void(ParticleEmitter&, glm::vec2&)> fn; // fn(emitter, attribute)

		using IAttributeView2D::IAttributeView2D;

		void on_tick() override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(GenericAttributeView2D);
	};

	struct GenericAttributeView3D : public IAttributeView3D
	{
		std::function<void(ParticleEmitter&, glm::vec3&)> fn; // fn(emitter, attribute)

		using IAttributeView3D::IAttributeView3D;

		void on_tick() override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(GenericAttributeView3D);
	};

	struct GenericAttributeView4D : public IAttributeView4D
	{
		std::function<void(ParticleEmitter&, glm::vec4&)> fn; // fn(emitter, attribute)

		using IAttributeView4D::IAttributeView4D;

		void on_tick() override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(GenericAttributeView4D);
	};
}
