#pragma once

#include "graphics/particles/ParticleEmitter.h"

#include <unordered_set>

namespace oly::particles
{
	struct IAttributeView1D;
	struct IAttributeView2D;
	struct IAttributeView3D;
	struct IAttributeView4D;

	namespace internal
	{
		class AttributeViewManager
		{
			friend struct IAttributeView1D;
			friend struct IAttributeView2D;
			friend struct IAttributeView3D;
			friend struct IAttributeView4D;
			std::unordered_set<IAttributeView1D*> views_1d;
			std::unordered_set<IAttributeView2D*> views_2d;
			std::unordered_set<IAttributeView3D*> views_3d;
			std::unordered_set<IAttributeView4D*> views_4d;

			AttributeViewManager() = default;
			AttributeViewManager(const AttributeViewManager&) = delete;
			AttributeViewManager(AttributeViewManager&&) = delete;

		public:
			static AttributeViewManager& instance()
			{
				static AttributeViewManager manager;
				return manager;
			}

			void clear()
			{
				views_1d.clear();
				views_2d.clear();
				views_3d.clear();
				views_4d.clear();
			}

			void on_tick();
		};
	}

	struct IAttributeView1D
	{
		ParticleEmitter& emitter;
		float& attribute;

		IAttributeView1D(ParticleEmitter& emitter, float& attribute);
		IAttributeView1D(const IAttributeView1D&);
		IAttributeView1D(IAttributeView1D&&) noexcept;
		~IAttributeView1D();

		virtual void on_tick() = 0;

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(IAttributeView1D);
	};

	struct IAttributeView2D
	{
		ParticleEmitter& emitter;
		glm::vec2& attribute;

		IAttributeView2D(ParticleEmitter& emitter, glm::vec2& attribute);
		IAttributeView2D(const IAttributeView2D&);
		IAttributeView2D(IAttributeView2D&&) noexcept;
		~IAttributeView2D();

		virtual void on_tick() = 0;

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(IAttributeView2D);
	};

	struct IAttributeView3D
	{
		ParticleEmitter& emitter;
		glm::vec3& attribute;

		IAttributeView3D(ParticleEmitter& emitter, glm::vec3& attribute);
		IAttributeView3D(const IAttributeView3D&);
		IAttributeView3D(IAttributeView3D&&) noexcept;
		~IAttributeView3D();

		virtual void on_tick() = 0;

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(IAttributeView3D);
	};

	struct IAttributeView4D
	{
		ParticleEmitter& emitter;
		glm::vec4& attribute;

		IAttributeView4D(ParticleEmitter& emitter, glm::vec4& attribute);
		IAttributeView4D(const IAttributeView4D&);
		IAttributeView4D(IAttributeView4D&&) noexcept;
		~IAttributeView4D();

		virtual void on_tick() = 0;

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(IAttributeView4D);
	};
}
