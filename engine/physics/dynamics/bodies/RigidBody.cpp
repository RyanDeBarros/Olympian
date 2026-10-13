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
        : transformer_(ref)
    {
        internal::RigidBodyTickService::instance(); // only need to call once in non-copy/move ctor.
    }

	RigidBody::RigidBody(const RigidBody& o)
		: colliders_(o.colliders_), transformer_(o.transformer_)
	{
		for (auto it = colliders_.begin(); it != colliders_.end(); ++it)
		{
			it->rigid_body = this;
			it->set_transformer().attach_parent(transformer_.base());
		}
	}
	
	RigidBody::RigidBody(RigidBody&& o) noexcept
		: colliders_(std::move(o.colliders_)), transformer_(std::move(o.transformer_))
	{
		for (auto it = colliders_.begin(); it != colliders_.end(); ++it)
		{
			it->rigid_body = this;
			it->set_transformer().attach_parent(transformer_.base());
			o.unbind(*it);
		}
	}
	
	RigidBody& RigidBody::operator=(const RigidBody& o)
	{
		if (this != &o)
		{
			clear_colliders();
			colliders_ = o.colliders_;
			transformer_ = o.transformer_;
			for (auto it = colliders_.begin(); it != colliders_.end(); ++it)
			{
				it->rigid_body = this;
				it->set_transformer().attach_parent(transformer_.base());
				bind(*it);
			}
		}
		return *this;
	}
	
	RigidBody& RigidBody::operator=(RigidBody&& o) noexcept
	{
		if (this != &o)
		{
			clear_colliders();
			colliders_ = std::move(o.colliders_);
			transformer_ = std::move(o.transformer_);
			for (auto it = colliders_.begin(); it != colliders_.end(); ++it)
			{
				it->rigid_body = this;
				it->set_transformer().attach_parent(transformer_.base());
				bind(*it);
				o.unbind(*it);
			}
		}
		return *this;
	}

    Transformer2DExposure<> RigidBody::get_transformer() const
    {
        return *transformer_;
    }

    Transformer2DExposure<ex::Full> RigidBody::set_transformer()
    {
        return *transformer_;
    }

    void RigidBody::assign_transformer(const Transformer2DRef& ref)
    {
        transformer_ = ref;
    }

    const Transform2D& RigidBody::get_local() const
    {
        return transformer_->get_local();
    }

    Transform2D& RigidBody::set_local()
    {
        return transformer_->set_local();
    }
	
	col2d::Collider& RigidBody::add_collider(col2d::Collider&& collider)
	{
		if (collider.rigid_body)
			collider.rigid_body->remove_collider(collider);
		col2d::Collider& c = colliders_.emplace_back(std::move(collider));
		c.rigid_body = this;
		c.set_transformer().attach_parent(transformer_.base());
		c.handles.attach();
		bind(c);
		return c;
	}

	void RigidBody::erase_collider(size_t i)
	{
		unbind(colliders_[i]);
		colliders_.erase(colliders_.begin() + i);
	}

	void RigidBody::remove_collider(const col2d::Collider& collider)
	{
		for (auto it = colliders_.begin(); it != colliders_.end(); ++it)
		{
			if (&*it == &collider)
			{
				unbind(*it);
				colliders_.erase(it);
				return;
			}
		}
	}

	void RigidBody::clear_colliders()
	{
		for (auto it = colliders_.begin(); it != colliders_.end(); ++it)
			unbind(*it);
		colliders_.clear();
	}

	const col2d::Collider& RigidBody::collider(size_t i) const
	{
		return colliders_[i];
	}

	col2d::Collider& RigidBody::collider(size_t i)
	{
		return colliders_[i];
	}

	size_t RigidBody::collider_index(const col2d::Collider& col) const
	{
		auto it = std::find_if(colliders_.begin(), colliders_.end(), [&col](const col2d::Collider& c) { return &c == &col; });
		if (it != colliders_.end())
			return it - colliders_.begin();
		else
			throw Error(ErrorCode::DoesNotExist);
	}

	debug::DebugOverlay RigidBody::create_debug_overlay(debug::DebugOverlayLayer& layer, size_t i, glm::vec4 color, debug::DebugOverlay::PaintOptions paint_options) const
	{
		return colliders_[i].create_debug_overlay(layer, color, paint_options);
	}

	void RigidBody::modify_debug_overlay(size_t i, debug::DebugOverlay& overlay) const
	{
		colliders_[i].modify_debug_overlay(overlay);
	}

	void RigidBody::bind_all() const
	{
		for (auto it = colliders_.begin(); it != colliders_.end(); ++it)
			bind(*it);
	}

	void RigidBody::unbind_all() const
	{
		for (auto it = colliders_.begin(); it != colliders_.end(); ++it)
			unbind(*it);
	}

	const RigidBody* RigidBody::rigid_body(const col2d::Collider& collider)
	{
		return collider.rigid_body;
	}
}
