#pragma once

#include "graphics/particles/ParticleEmitter.h"

#include <unordered_set>

namespace oly::particles
{
	struct AttributeView1D;
	struct AttributeView2D;
	struct AttributeView3D;
	struct AttributeView4D;

	namespace internal
	{
		class AttributeViewManager
		{
			friend struct AttributeView1D;
			friend struct AttributeView2D;
			friend struct AttributeView3D;
			friend struct AttributeView4D;
			std::unordered_set<AttributeView1D*> views_1d;
			std::unordered_set<AttributeView2D*> views_2d;
			std::unordered_set<AttributeView3D*> views_3d;
			std::unordered_set<AttributeView4D*> views_4d;

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

	struct IAttributeOperation1D
	{
		virtual float op(const ParticleEmitter& emitter, float attribute) const { return attribute; }

		OLY_POLYMORPHIC_CLONE_DEFINITION(IAttributeOperation1D);
	};

	struct AttributeView1D
	{
		const ParticleEmitter& emitter;
		float& attribute;
		Polymorphic<IAttributeOperation1D> op;

		AttributeView1D(const ParticleEmitter& emitter, float& attribute, Polymorphic<IAttributeOperation1D>&& op);
		AttributeView1D(const AttributeView1D&);
		AttributeView1D(AttributeView1D&&) noexcept;
		~AttributeView1D();

		void on_tick() const { attribute = op->op(emitter, attribute); }
	};

	struct IAttributeOperation2D
	{
		virtual glm::vec2 op(const ParticleEmitter& emitter, glm::vec2 attribute) const { return attribute; }

		OLY_POLYMORPHIC_CLONE_DEFINITION(IAttributeOperation2D);
	};

	struct AttributeView2D
	{
		const ParticleEmitter& emitter;
		glm::vec2& attribute;
		Polymorphic<IAttributeOperation2D> op;

		AttributeView2D(const ParticleEmitter& emitter, glm::vec2& attribute, Polymorphic<IAttributeOperation2D>&& op);
		AttributeView2D(const AttributeView2D&);
		AttributeView2D(AttributeView2D&&) noexcept;
		~AttributeView2D();

		void on_tick() const { attribute = op->op(emitter, attribute); }
	};

	struct IAttributeOperation3D
	{
		virtual glm::vec3 op(const ParticleEmitter& emitter, glm::vec3 attribute) const { return attribute; }

		OLY_POLYMORPHIC_CLONE_DEFINITION(IAttributeOperation3D);
	};

	struct AttributeView3D
	{
		const ParticleEmitter& emitter;
		glm::vec3& attribute;
		Polymorphic<IAttributeOperation3D> op;

		AttributeView3D(const ParticleEmitter& emitter, glm::vec3& attribute, Polymorphic<IAttributeOperation3D>&& op);
		AttributeView3D(const AttributeView3D&);
		AttributeView3D(AttributeView3D&&) noexcept;
		~AttributeView3D();

		void on_tick() const { attribute = op->op(emitter, attribute); }
	};

	struct IAttributeOperation4D
	{
		virtual glm::vec4 op(const ParticleEmitter& emitter, glm::vec4 attribute) const { return attribute; }

		OLY_POLYMORPHIC_CLONE_DEFINITION(IAttributeOperation4D);
	};

	struct AttributeView4D
	{
		const ParticleEmitter& emitter;
		glm::vec4& attribute;
		Polymorphic<IAttributeOperation4D> op;

		AttributeView4D(const ParticleEmitter& emitter, glm::vec4& attribute, Polymorphic<IAttributeOperation4D>&& op);
		AttributeView4D(const AttributeView4D&);
		AttributeView4D(AttributeView4D&&) noexcept;
		~AttributeView4D();

		void on_tick() const { attribute = op->op(emitter, attribute); }
	};
	
	struct AttributeViewList
	{
		std::vector<AttributeView1D> views_1d;
		std::vector<AttributeView2D> views_2d;
		std::vector<AttributeView3D> views_3d;
		std::vector<AttributeView4D> views_4d;
	};
}
