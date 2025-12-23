#include "AttributeView.h"

namespace oly::particles
{
	void internal::AttributeViewManager::on_tick()
	{
		for (IAttributeView1D* view : views_1d)
			view->on_tick();
		for (IAttributeView2D* view : views_2d)
			view->on_tick();
		for (IAttributeView3D* view : views_3d)
			view->on_tick();
		for (IAttributeView4D* view : views_4d)
			view->on_tick();
	}

	IAttributeView1D::IAttributeView1D(ParticleEmitter& emitter, float& attribute)
		: emitter(emitter), attribute(attribute)
	{
		internal::AttributeViewManager::instance().views_1d.insert(this);
	}

	IAttributeView1D::IAttributeView1D(const IAttributeView1D& other)
		: emitter(other.emitter), attribute(other.attribute)
	{
		internal::AttributeViewManager::instance().views_1d.insert(this);
	}

	IAttributeView1D::IAttributeView1D(IAttributeView1D&& other) noexcept
		: emitter(other.emitter), attribute(other.attribute)
	{
		internal::AttributeViewManager::instance().views_1d.insert(this);
	}

	IAttributeView1D::~IAttributeView1D()
	{
		internal::AttributeViewManager::instance().views_1d.erase(this);
	}

	IAttributeView2D::IAttributeView2D(ParticleEmitter& emitter, glm::vec2& attribute)
		: emitter(emitter), attribute(attribute)
	{
		internal::AttributeViewManager::instance().views_2d.insert(this);
	}

	IAttributeView2D::IAttributeView2D(const IAttributeView2D& other)
		: emitter(other.emitter), attribute(other.attribute)
	{
		internal::AttributeViewManager::instance().views_2d.insert(this);
	}

	IAttributeView2D::IAttributeView2D(IAttributeView2D&& other) noexcept
		: emitter(other.emitter), attribute(other.attribute)
	{
		internal::AttributeViewManager::instance().views_2d.insert(this);
	}

	IAttributeView2D::~IAttributeView2D()
	{
		internal::AttributeViewManager::instance().views_2d.erase(this);
	}

	IAttributeView3D::IAttributeView3D(ParticleEmitter& emitter, glm::vec3& attribute)
		: emitter(emitter), attribute(attribute)
	{
		internal::AttributeViewManager::instance().views_3d.insert(this);
	}

	IAttributeView3D::IAttributeView3D(const IAttributeView3D& other)
		: emitter(other.emitter), attribute(other.attribute)
	{
		internal::AttributeViewManager::instance().views_3d.insert(this);
	}

	IAttributeView3D::IAttributeView3D(IAttributeView3D&& other) noexcept
		: emitter(other.emitter), attribute(other.attribute)
	{
		internal::AttributeViewManager::instance().views_3d.insert(this);
	}

	IAttributeView3D::~IAttributeView3D()
	{
		internal::AttributeViewManager::instance().views_3d.erase(this);
	}

	IAttributeView4D::IAttributeView4D(ParticleEmitter& emitter, glm::vec4& attribute)
		: emitter(emitter), attribute(attribute)
	{
		internal::AttributeViewManager::instance().views_4d.insert(this);
	}

	IAttributeView4D::IAttributeView4D(const IAttributeView4D& other)
		: emitter(other.emitter), attribute(other.attribute)
	{
		internal::AttributeViewManager::instance().views_4d.insert(this);
	}

	IAttributeView4D::IAttributeView4D(IAttributeView4D&& other) noexcept
		: emitter(other.emitter), attribute(other.attribute)
	{
		internal::AttributeViewManager::instance().views_4d.insert(this);
	}

	IAttributeView4D::~IAttributeView4D()
	{
		internal::AttributeViewManager::instance().views_4d.erase(this);
	}
}
