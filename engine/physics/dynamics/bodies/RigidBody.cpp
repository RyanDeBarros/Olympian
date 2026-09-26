#include "RigidBody.h"

namespace oly::physics
{
	namespace internal
	{
		struct RigidBodyOnTick
		{
			void operator()() const
			{
				auto& rigid_bodies = RigidBody::tracked();

                for (RigidBody* rigid_body : rigid_bodies)
                {
                    if (rigid_body->collision_enabled)
    					rigid_body->physics_pre_tick();
                }

                for (RigidBody* rigid_body : rigid_bodies)
                {
                    if (rigid_body->collision_enabled)
    					rigid_body->physics_post_tick();
                }
			}
		};

		struct RigidBodyOnTerminate
		{
			void operator()() const
			{
				RigidBody::tracker().clear();
			}
		};

		using RigidBodyTickService = SingletonTickService<TickPhase::Physics, RigidBodyOnTick, TerminatePhase::Logic, RigidBodyOnTerminate>;
	}

	RigidBody::RigidBody()
	{
		internal::RigidBodyTickService::instance(); // only need to call once in non-copy/move ctor.
	}

    RigidBody::RigidBody(const Transformer2DRef& ref)
        : transformer(ref)
    {
        internal::RigidBodyTickService::instance(); // only need to call once in non-copy/move ctor.
    }

	RigidBody::RigidBody(const RigidBody& other)
		: colliders(other.colliders), transformer(other.transformer)
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
		{
			it->rigid_body = this;
			it->set_transformer().attach_parent(transformer.base());
		}
	}
	
	RigidBody::RigidBody(RigidBody&& other) noexcept
		: colliders(std::move(other.colliders)), transformer(std::move(other.transformer))
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
		{
			it->rigid_body = this;
			it->set_transformer().attach_parent(transformer.base());
			other.unbind(*it);
		}
	}
	
	RigidBody& RigidBody::operator=(const RigidBody& other)
	{
		if (this != &other)
		{
			clear_colliders();
			colliders = other.colliders;
			transformer = other.transformer;
			for (auto it = colliders.begin(); it != colliders.end(); ++it)
			{
				it->rigid_body = this;
				it->set_transformer().attach_parent(transformer.base());
				bind(*it);
			}
		}
		return *this;
	}
	
	RigidBody& RigidBody::operator=(RigidBody&& other) noexcept
	{
		if (this != &other)
		{
			clear_colliders();
			colliders = std::move(other.colliders);
			transformer = std::move(other.transformer);
			for (auto it = colliders.begin(); it != colliders.end(); ++it)
			{
				it->rigid_body = this;
				it->set_transformer().attach_parent(transformer.base());
				bind(*it);
				other.unbind(*it);
			}
		}
		return *this;
	}

    Transformer2DConstExposure RigidBody::get_transformer() const
    {
        return *transformer;
    }

    Transformer2DExposure<TExposureParams{ .local = exposure::local::Full, .chain = exposure::chain::Full, .modifier = exposure::modifier::Full }> RigidBody::set_transformer()
    {
        return *transformer;
    }

    void RigidBody::assign_transformer(const Transformer2DRef& ref)
    {
        transformer = ref;
    }

    const Transform2D& RigidBody::get_local() const
    {
        return transformer->get_local();
    }

    Transform2D& RigidBody::set_local()
    {
        return transformer->set_local();
    }
	
	col2d::Collider& RigidBody::add_collider(col2d::Collider&& collider)
	{
		if (collider.rigid_body)
			collider.rigid_body->remove_collider(collider);
		col2d::Collider& c = colliders.emplace_back(std::move(collider));
		c.rigid_body = this;
		c.set_transformer().attach_parent(transformer.base());
		c.handles.attach();
		bind(c);
		return c;
	}

	void RigidBody::erase_collider(size_t i)
	{
		unbind(colliders[i]);
		colliders.erase(colliders.begin() + i);
	}

	void RigidBody::remove_collider(const col2d::Collider& collider)
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
		{
			if (&*it == &collider)
			{
				unbind(*it);
				colliders.erase(it);
				return;
			}
		}
	}

	void RigidBody::clear_colliders()
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
			unbind(*it);
		colliders.clear();
	}

	const col2d::Collider& RigidBody::collider(size_t i) const
	{
		return colliders[i];
	}

	col2d::Collider& RigidBody::collider(size_t i)
	{
		return colliders[i];
	}

	size_t RigidBody::collider_index(const col2d::Collider& col) const
	{
		auto it = std::find_if(colliders.begin(), colliders.end(), [&col](const col2d::Collider& c) { return &c == &col; });
		if (it != colliders.end())
			return it - colliders.begin();
		else
			throw Error(ErrorCode::DoesNotExist);
	}

	debug::DebugOverlay RigidBody::create_debug_overlay(debug::DebugOverlayLayer& layer, size_t i, glm::vec4 color, debug::DebugOverlay::PaintOptions paint_options) const
	{
		return colliders[i].create_debug_overlay(layer, color, paint_options);
	}

	void RigidBody::modify_debug_overlay(size_t i, debug::DebugOverlay& overlay) const
	{
		colliders[i].modify_debug_overlay(overlay);
	}

	void RigidBody::bind_all() const
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
			bind(*it);
	}

	void RigidBody::unbind_all() const
	{
		for (auto it = colliders.begin(); it != colliders.end(); ++it)
			unbind(*it);
	}

	const RigidBody* RigidBody::rigid_body(const col2d::Collider& collider)
	{
		return collider.rigid_body;
	}
}
