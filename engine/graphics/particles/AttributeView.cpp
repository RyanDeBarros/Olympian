#include "AttributeView.h"

namespace oly::particles
{
	void internal::AttributeViewManager::on_tick()
	{
		for (AttributeView1D* view : views_1d)
			view->on_tick();
		for (AttributeView2D* view : views_2d)
			view->on_tick();
		for (AttributeView3D* view : views_3d)
			view->on_tick();
		for (AttributeView4D* view : views_4d)
			view->on_tick();
	}

	AttributeView1D::AttributeView1D(const ParticleEmitter& emitter, float& attribute, Polymorphic<IAttributeOperation1D>&& op)
		: emitter(emitter), attribute(attribute), op(std::move(op))
	{
		internal::AttributeViewManager::instance().views_1d.insert(this);
	}

	AttributeView1D::AttributeView1D(const AttributeView1D& other)
		: emitter(other.emitter), attribute(other.attribute), op(other.op)
	{
		internal::AttributeViewManager::instance().views_1d.insert(this);
	}

	AttributeView1D::AttributeView1D(AttributeView1D&& other) noexcept
		: emitter(other.emitter), attribute(other.attribute), op(std::move(other.op))
	{
		internal::AttributeViewManager::instance().views_1d.insert(this);
	}

	AttributeView1D::~AttributeView1D()
	{
		internal::AttributeViewManager::instance().views_1d.erase(this);
	}

	AttributeView2D::AttributeView2D(const ParticleEmitter& emitter, glm::vec2& attribute, Polymorphic<IAttributeOperation2D>&& op)
		: emitter(emitter), attribute(attribute), op(std::move(op))
	{
		internal::AttributeViewManager::instance().views_2d.insert(this);
	}

	AttributeView2D::AttributeView2D(const AttributeView2D& other)
		: emitter(other.emitter), attribute(other.attribute), op(other.op)
	{
		internal::AttributeViewManager::instance().views_2d.insert(this);
	}

	AttributeView2D::AttributeView2D(AttributeView2D&& other) noexcept
		: emitter(other.emitter), attribute(other.attribute), op(std::move(other.op))
	{
		internal::AttributeViewManager::instance().views_2d.insert(this);
	}

	AttributeView2D::~AttributeView2D()
	{
		internal::AttributeViewManager::instance().views_2d.erase(this);
	}

	AttributeView3D::AttributeView3D(const ParticleEmitter& emitter, glm::vec3& attribute, Polymorphic<IAttributeOperation3D>&& op)
		: emitter(emitter), attribute(attribute), op(std::move(op))
	{
		internal::AttributeViewManager::instance().views_3d.insert(this);
	}

	AttributeView3D::AttributeView3D(const AttributeView3D& other)
		: emitter(other.emitter), attribute(other.attribute), op(other.op)
	{
		internal::AttributeViewManager::instance().views_3d.insert(this);
	}

	AttributeView3D::AttributeView3D(AttributeView3D&& other) noexcept
		: emitter(other.emitter), attribute(other.attribute), op(std::move(other.op))
	{
		internal::AttributeViewManager::instance().views_3d.insert(this);
	}

	AttributeView3D::~AttributeView3D()
	{
		internal::AttributeViewManager::instance().views_3d.erase(this);
	}

	AttributeView4D::AttributeView4D(const ParticleEmitter& emitter, glm::vec4& attribute, Polymorphic<IAttributeOperation4D>&& op)
		: emitter(emitter), attribute(attribute), op(std::move(op))
	{
		internal::AttributeViewManager::instance().views_4d.insert(this);
	}

	AttributeView4D::AttributeView4D(const AttributeView4D& other)
		: emitter(other.emitter), attribute(other.attribute), op(other.op)
	{
		internal::AttributeViewManager::instance().views_4d.insert(this);
	}

	AttributeView4D::AttributeView4D(AttributeView4D&& other) noexcept
		: emitter(other.emitter), attribute(other.attribute), op(std::move(other.op))
	{
		internal::AttributeViewManager::instance().views_4d.insert(this);
	}

	AttributeView4D::~AttributeView4D()
	{
		internal::AttributeViewManager::instance().views_4d.erase(this);
	}
}
